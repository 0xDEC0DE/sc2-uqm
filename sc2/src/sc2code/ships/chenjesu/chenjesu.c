//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "reslib.h"
#include "ships/chenjesu/resinst.h"
#include "starcon.h"

#define MAX_CREW 36
#define MAX_ENERGY 30
#define ENERGY_REGENERATION 1
#define WEAPON_ENERGY_COST 5
#define SPECIAL_ENERGY_COST MAX_ENERGY
#define ENERGY_WAIT 4
#define MAX_THRUST /* DISPLAY_TO_WORLD (7) */ 27
#define THRUST_INCREMENT /* DISPLAY_TO_WORLD (2) */ 3
#define TURN_WAIT 6
#define THRUST_WAIT 4
#define WEAPON_WAIT 0
#define SPECIAL_WAIT 0

#define MAX_DOGGIES 4

#define SHIP_MASS 10
#define MISSILE_SPEED DISPLAY_TO_WORLD (16)
#define MISSILE_LIFE 90 /* actually, it's as long as you
										 * hold the button down.
										 */

static RACE_DESC chenjesu_desc =
{
	{
		FIRES_FORE | SEEKING_SPECIAL | SEEKING_WEAPON,
		28, /* Super Melee cost */
		0 / SPHERE_RADIUS_INCREMENT, /* Initial sphere of influence radius */
		MAX_CREW, MAX_CREW,
		MAX_ENERGY, MAX_ENERGY,
		{
			0, 0,
		},
		(STRING)CHENJESU_RACE_STRINGS,
		(FRAME)CHENJESU_ICON_MASK_PMAP_ANIM,
		(FRAME)CHENJESU_MICON_MASK_PMAP_ANIM,
	},
	{
		MAX_THRUST,
		THRUST_INCREMENT,
		ENERGY_REGENERATION,
		WEAPON_ENERGY_COST,
		SPECIAL_ENERGY_COST,
		ENERGY_WAIT,
		TURN_WAIT,
		THRUST_WAIT,
		WEAPON_WAIT,
		SPECIAL_WAIT,
		SHIP_MASS,
	},
	{
		{
			(FRAME)CHENJESU_BIG_MASK_PMAP_ANIM,
			(FRAME)CHENJESU_MED_MASK_PMAP_ANIM,
			(FRAME)CHENJESU_SML_MASK_PMAP_ANIM,
		},
		{
			(FRAME)SPARK_BIG_MASK_PMAP_ANIM,
			(FRAME)SPARK_MED_MASK_PMAP_ANIM,
			(FRAME)SPARK_SML_MASK_PMAP_ANIM,
		},
		{
			(FRAME)DOGGY_BIG_MASK_PMAP_ANIM,
			(FRAME)DOGGY_MED_MASK_PMAP_ANIM,
			(FRAME)DOGGY_SML_MASK_PMAP_ANIM,
		},
		{
			(FRAME)CHENJESU_CAPTAIN_MASK_PMAP_ANIM,
			(FRAME)0,
			(FRAME)0,
			(FRAME)0,
			(FRAME)0,
		},
		(SOUND)CHENJESU_VICTORY_SONG,
		(SOUND)CHENJESU_SHIP_SOUNDS,
	},
	{
		0,
		LONG_RANGE_WEAPON,
		NULL_PTR,
	},
	NULL_PTR,
	NULL_PTR,
	NULL_PTR,
	0,
};

#define FRAGMENT_LIFE 10
#define FRAGMENT_SPEED MISSILE_SPEED
#define FRAGMENT_RANGE (FRAGMENT_LIFE * FRAGMENT_SPEED)

static void
crystal_postprocess (PELEMENT ElementPtr)
{
#define FRAGMENT_HITS 1
#define FRAGMENT_DAMAGE 2
#define FRAGMENT_OFFSET 2
#define NUM_FRAGMENTS 8
	STARSHIPPTR StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	MissileBlock.cx = ElementPtr->next.location.x;
	MissileBlock.cy = ElementPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.index = 1;
	MissileBlock.sender =
			(ElementPtr->state_flags & (GOOD_GUY | BAD_GUY))
			| IGNORE_SIMILAR;
	MissileBlock.pixoffs = 0;
	MissileBlock.speed = FRAGMENT_SPEED;
	MissileBlock.hit_points = FRAGMENT_HITS;
	MissileBlock.damage = FRAGMENT_DAMAGE;
	MissileBlock.life = FRAGMENT_LIFE;
	MissileBlock.preprocess_func = NULL_PTR;
	MissileBlock.blast_offs = FRAGMENT_OFFSET;

	for (MissileBlock.face = 0;
			MissileBlock.face < ANGLE_TO_FACING (FULL_CIRCLE);
			MissileBlock.face +=
			(ANGLE_TO_FACING (FULL_CIRCLE) / NUM_FRAGMENTS))
	{
		HELEMENT hFragment;

		hFragment = initialize_missile (&MissileBlock);
		if (hFragment)
		{
			ELEMENTPTR FragPtr;

			LockElement (hFragment, &FragPtr);
			SetElementStarShip (FragPtr, StarShipPtr);
			UnlockElement (hFragment);
			PutElement (hFragment);
		}
	}

	ProcessSound (SetAbsSoundIndex (
					/* CRYSTAL_FRAGMENTS */
			StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 1));
}

static void
crystal_preprocess (PELEMENT ElementPtr)
{
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if (StarShipPtr->cur_status_flags & WEAPON)
		++ElementPtr->life_span; /* keep it going while key pressed */
	else
	{
		ElementPtr->life_span = 1;

		ElementPtr->postprocess_func = crystal_postprocess;
	}
}

static void
animate (PELEMENT ElementPtr)
{
	if (ElementPtr->turn_wait > 0)
		--ElementPtr->turn_wait;
	else
	{
		ElementPtr->next.image.frame =
				IncFrameIndex (ElementPtr->current.image.frame);
		ElementPtr->state_flags |= CHANGING;

		ElementPtr->turn_wait = ElementPtr->next_turn;
	}
}

static void
crystal_collision (PELEMENT ElementPtr0, PPOINT pPt0, PELEMENT ElementPtr1, PPOINT pPt1)
{
	HELEMENT hBlastElement;

	hBlastElement =
			weapon_collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
	if (hBlastElement)
	{
		ELEMENTPTR BlastElementPtr;

#define NUM_SPARKLES 8
		LockElement (hBlastElement, &BlastElementPtr);
		BlastElementPtr->current.location = ElementPtr1->current.location;

		BlastElementPtr->life_span = NUM_SPARKLES;
		BlastElementPtr->turn_wait = BlastElementPtr->next_turn = 0;
		{
			BlastElementPtr->preprocess_func = animate;
		}

		BlastElementPtr->current.image.farray = ElementPtr0->next.image.farray;
		BlastElementPtr->current.image.frame =
				SetAbsFrameIndex (BlastElementPtr->current.image.farray[0],
				2); /* skip stones */

		UnlockElement (hBlastElement);
	}
}

#define DOGGY_OFFSET 18
#define DOGGY_SPEED DISPLAY_TO_WORLD (8)

static void
doggy_preprocess (PELEMENT ElementPtr)
{
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	++StarShipPtr->special_counter;
	if (ElementPtr->thrust_wait > 0) /* could be non-zero after a collision */
		--ElementPtr->thrust_wait;
	else
	{
		COUNT facing, orig_facing;
		SIZE delta_facing;

		facing = orig_facing =
				NORMALIZE_FACING (ANGLE_TO_FACING (
				GetVelocityTravelAngle (&ElementPtr->velocity)
				));
		if ((delta_facing = TrackShip (ElementPtr, &facing)) < 0)
			facing = NORMALIZE_FACING (Random ());
		else
		{
			ELEMENTPTR ShipPtr;

			LockElement (ElementPtr->hTarget, &ShipPtr);
			facing = NORMALIZE_FACING (ANGLE_TO_FACING (
					ARCTAN (ShipPtr->current.location.x -
					ElementPtr->current.location.x,
					ShipPtr->current.location.y -
					ElementPtr->current.location.y)
					));
			delta_facing = NORMALIZE_FACING (facing -
					GetFrameIndex (ShipPtr->current.image.frame));
			UnlockElement (ElementPtr->hTarget);

			if (delta_facing > ANGLE_TO_FACING (HALF_CIRCLE - OCTANT) &&
					delta_facing < ANGLE_TO_FACING (HALF_CIRCLE + OCTANT))
			{
				if (delta_facing >= ANGLE_TO_FACING (HALF_CIRCLE))
					facing -= ANGLE_TO_FACING (QUADRANT);
				else
					facing += ANGLE_TO_FACING (QUADRANT);
			}

			facing = NORMALIZE_FACING (facing);
		}

		if (facing != orig_facing)
			SetVelocityVector (&ElementPtr->velocity,
					DOGGY_SPEED, facing);
	}
}

static void
doggy_death (PELEMENT ElementPtr)
{
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	ProcessSound (SetAbsSoundIndex (
					/* DOGGY_DIES */
			StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 3));

	ElementPtr->state_flags &= ~DISAPPEARING;
	ElementPtr->state_flags |= NONSOLID | FINITE_LIFE;
	ElementPtr->life_span = 6;
	{
		ElementPtr->preprocess_func = animate;
	}
	ElementPtr->death_func = NULL_PTR;
	ElementPtr->collision_func = NULL_PTR;
	ZeroVelocityComponents (&ElementPtr->velocity);

	ElementPtr->turn_wait = ElementPtr->next_turn = 0;
}

static void
doggy_collision (PELEMENT ElementPtr0, PPOINT pPt0, PELEMENT ElementPtr1, PPOINT pPt1)
{
#define ENERGY_DRAIN 10
	collision (ElementPtr0, pPt0, ElementPtr1, pPt1);
	if ((ElementPtr1->state_flags & PLAYER_SHIP)
			&& (ElementPtr0->state_flags
			& (GOOD_GUY | BAD_GUY)) !=
			(ElementPtr1->state_flags
			& (GOOD_GUY | BAD_GUY)))
	{
		STARSHIPPTR StarShipPtr;

		GetElementStarShip (ElementPtr0, &StarShipPtr);
		ProcessSound (SetAbsSoundIndex (
						/* DOGGY_STEALS_ENERGY */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 2));
		GetElementStarShip (ElementPtr1, &StarShipPtr);
		if (StarShipPtr->RaceDescPtr->ship_info.energy_level < ENERGY_DRAIN)
			DeltaEnergy (ElementPtr1, -StarShipPtr->RaceDescPtr->ship_info.energy_level);
		else
			DeltaEnergy (ElementPtr1, -ENERGY_DRAIN);
	}
	if (ElementPtr0->thrust_wait <= COLLISION_THRUST_WAIT)
		ElementPtr0->thrust_wait += COLLISION_THRUST_WAIT << 1;
}

#define CHENJESU_OFFSET 16

static void
spawn_doggy (ElementPtr)
PELEMENT ElementPtr;
{
	HELEMENT hDoggyElement;

	if ((hDoggyElement = AllocElement ()) != 0)
	{
		COUNT angle;
		ELEMENTPTR DoggyElementPtr;
		STARSHIPPTR StarShipPtr;

		ElementPtr->state_flags |= DEFY_PHYSICS;

		PutElement (hDoggyElement);
		LockElement (hDoggyElement, &DoggyElementPtr);
		DoggyElementPtr->hit_points = 3;
		DoggyElementPtr->mass_points = 4;
		DoggyElementPtr->thrust_wait = 0;
		DoggyElementPtr->state_flags = APPEARING
				| (ElementPtr->state_flags & (GOOD_GUY | BAD_GUY));
		DoggyElementPtr->life_span = NORMAL_LIFE;
		SetPrimType (&(GLOBAL (DisplayArray))[DoggyElementPtr->PrimIndex],
				STAMP_PRIM);
		{
			DoggyElementPtr->preprocess_func = doggy_preprocess;
			DoggyElementPtr->postprocess_func = NULL_PTR;
			DoggyElementPtr->collision_func = doggy_collision;
			DoggyElementPtr->death_func = doggy_death;
		}

		GetElementStarShip (ElementPtr, &StarShipPtr);
		angle = FACING_TO_ANGLE (StarShipPtr->ShipFacing) + HALF_CIRCLE;
		DoggyElementPtr->current.location.x = ElementPtr->next.location.x
				+ COSINE (angle, DISPLAY_TO_WORLD (CHENJESU_OFFSET + DOGGY_OFFSET));
		DoggyElementPtr->current.location.y = ElementPtr->next.location.y
				+ SINE (angle, DISPLAY_TO_WORLD (CHENJESU_OFFSET + DOGGY_OFFSET));
		DoggyElementPtr->current.image.farray = StarShipPtr->RaceDescPtr->ship_data.special;
		DoggyElementPtr->current.image.frame = StarShipPtr->RaceDescPtr->ship_data.special[0];

		SetVelocityVector (&DoggyElementPtr->velocity,
				DOGGY_SPEED, NORMALIZE_FACING (ANGLE_TO_FACING (angle)));

		SetElementStarShip (DoggyElementPtr, StarShipPtr);

		UnlockElement (hDoggyElement);
	}
}

static void
chenjesu_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter)
PELEMENT ShipPtr;
PEVALUATE_DESC ObjectsOfConcern;
COUNT ConcernCounter;
{
	PEVALUATE_DESC lpEvalDesc;
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	StarShipPtr->ship_input_state &= ~SPECIAL;

	lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
	if (lpEvalDesc->ObjectPtr)
	{
		STARSHIPPTR EnemyStarShipPtr;

		GetElementStarShip (lpEvalDesc->ObjectPtr, &EnemyStarShipPtr);
		if ((lpEvalDesc->which_turn <= 16
				&& MANEUVERABILITY (
				&EnemyStarShipPtr->RaceDescPtr->cyborg_control
				) >= MEDIUM_SHIP)
				|| (MANEUVERABILITY (
				&EnemyStarShipPtr->RaceDescPtr->cyborg_control
				) <= SLOW_SHIP
				&& WEAPON_RANGE (
				&EnemyStarShipPtr->RaceDescPtr->cyborg_control
				) >= LONG_RANGE_WEAPON * 3 / 4
				&& (EnemyStarShipPtr->RaceDescPtr->ship_info.ship_flags & SEEKING_WEAPON)))
			lpEvalDesc->MoveState = PURSUE;
	}

	if (StarShipPtr->special_counter == 1
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].ObjectPtr
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].MoveState == ENTICE
			&& ObjectsOfConcern[ENEMY_WEAPON_INDEX].which_turn <= 8)
	{
		lpEvalDesc = &ObjectsOfConcern[ENEMY_WEAPON_INDEX];
	}

	ship_intelligence (ShipPtr, ObjectsOfConcern, ConcernCounter);

	if (lpEvalDesc->ObjectPtr)
	{
		HELEMENT h, hNext;
		ELEMENTPTR CrystalPtr;

		h = (StarShipPtr->old_status_flags & WEAPON) ?
				GetTailElement () : (HELEMENT)0;
		for (; h; h = hNext)
		{
			LockElement (h, &CrystalPtr);
			hNext = GetPredElement (CrystalPtr);
			if (!(CrystalPtr->state_flags & NONSOLID)
					&& CrystalPtr->next.image.farray == StarShipPtr->RaceDescPtr->ship_data.weapon
					&& CrystalPtr->preprocess_func
					&& CrystalPtr->life_span > 0
					&& (CrystalPtr->state_flags & (GOOD_GUY | BAD_GUY)) ==
					(ShipPtr->state_flags & (GOOD_GUY | BAD_GUY)))
			{
				if (ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr)
				{
					COUNT which_turn;

					if ((which_turn = PlotIntercept (CrystalPtr,
							ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr,
							CrystalPtr->life_span,
							FRAGMENT_RANGE / 2)) == 0
							|| (which_turn == 1
							&& PlotIntercept (CrystalPtr,
							ObjectsOfConcern[ENEMY_SHIP_INDEX].ObjectPtr,
							CrystalPtr->life_span, 0) == 0))
						StarShipPtr->ship_input_state &= ~WEAPON;
					else if (StarShipPtr->weapon_counter == 0)
					{
						StarShipPtr->ship_input_state |= WEAPON;
						lpEvalDesc = &ObjectsOfConcern[ENEMY_SHIP_INDEX];
					}

					UnlockElement (h);
					break;
				}
				hNext = 0;
			}
			UnlockElement (h);
		}

		if (h == 0)
		{
			if (StarShipPtr->old_status_flags & WEAPON)
			{
				StarShipPtr->ship_input_state &= ~WEAPON;
				if (lpEvalDesc == &ObjectsOfConcern[ENEMY_WEAPON_INDEX])
					StarShipPtr->weapon_counter = 3;
			}
			else if (StarShipPtr->weapon_counter == 0
					&& ship_weapons (ShipPtr, lpEvalDesc->ObjectPtr, FRAGMENT_RANGE / 2))
				StarShipPtr->ship_input_state |= WEAPON;
		}
	}

	if (StarShipPtr->special_counter < MAX_DOGGIES)
	{
		if (lpEvalDesc->ObjectPtr
				&& StarShipPtr->RaceDescPtr->ship_info.energy_level <= SPECIAL_ENERGY_COST
				&& !(StarShipPtr->ship_input_state & WEAPON))
			StarShipPtr->ship_input_state |= SPECIAL;
	}
}

static COUNT
initialize_crystal (PELEMENT ShipPtr, HELEMENT CrystalArray[])
{
#define MISSILE_HITS 10
#define MISSILE_DAMAGE 6
#define MISSILE_OFFSET 0
	STARSHIPPTR StarShipPtr;
	MISSILE_BLOCK MissileBlock;

	GetElementStarShip (ShipPtr, &StarShipPtr);
	MissileBlock.cx = ShipPtr->next.location.x;
	MissileBlock.cy = ShipPtr->next.location.y;
	MissileBlock.farray = StarShipPtr->RaceDescPtr->ship_data.weapon;
	MissileBlock.face = StarShipPtr->ShipFacing;
	MissileBlock.index = 0;
	MissileBlock.sender = (ShipPtr->state_flags & (GOOD_GUY | BAD_GUY))
			| IGNORE_SIMILAR;
	MissileBlock.pixoffs = CHENJESU_OFFSET;
	MissileBlock.speed = MISSILE_SPEED;
	MissileBlock.hit_points = MISSILE_HITS;
	MissileBlock.damage = MISSILE_DAMAGE;
	MissileBlock.life = MISSILE_LIFE;
	MissileBlock.preprocess_func = crystal_preprocess;
	MissileBlock.blast_offs = MISSILE_OFFSET;
	CrystalArray[0] = initialize_missile (&MissileBlock);

	if (CrystalArray[0])
	{
		ELEMENTPTR CrystalPtr;

		LockElement (CrystalArray[0], &CrystalPtr);
		CrystalPtr->collision_func = crystal_collision;
		UnlockElement (CrystalArray[0]);
	}

	return (1);
}

static void
chenjesu_postprocess (PELEMENT ElementPtr)
{
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if ((StarShipPtr->cur_status_flags & SPECIAL)
			&& StarShipPtr->special_counter < MAX_DOGGIES
			&& DeltaEnergy (ElementPtr, -SPECIAL_ENERGY_COST))
	{
		spawn_doggy (ElementPtr);

		ProcessSound (SetAbsSoundIndex (
						/* RELEASE_DOGGY */
				StarShipPtr->RaceDescPtr->ship_data.ship_sounds, 4));
	}

	StarShipPtr->special_counter = 1;
			/* say there is one doggy, because ship_postprocess will
			 * decrement special_counter */
}

static void
chenjesu_preprocess (PELEMENT ElementPtr)
{
	STARSHIPPTR StarShipPtr;

	GetElementStarShip (ElementPtr, &StarShipPtr);
	if (StarShipPtr->special_counter > 1) /* only when STANDARD
											 * computer opponent
											 */
		StarShipPtr->special_counter += MAX_DOGGIES;
	if (StarShipPtr->cur_status_flags
			& StarShipPtr->old_status_flags
			& WEAPON)
		++StarShipPtr->weapon_counter;
}

RACE_DESCPTR
init_chenjesu ()
{
	RACE_DESCPTR RaceDescPtr;

	chenjesu_desc.preprocess_func = chenjesu_preprocess;
	chenjesu_desc.postprocess_func = chenjesu_postprocess;
	chenjesu_desc.init_weapon_func = initialize_crystal;
	chenjesu_desc.cyborg_control.intelligence_func =
			(void (*) (PVOID ShipPtr, PVOID ObjectsOfConcern, COUNT
					ConcernCounter)) chenjesu_intelligence;

	RaceDescPtr = &chenjesu_desc;

	return (RaceDescPtr);
}

