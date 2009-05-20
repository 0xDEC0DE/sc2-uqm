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

#include "build.h"
#include "encount.h"
#include "globdata.h"
#include "lander.h"
#include "nameref.h"
#include "resinst.h"
#include "setup.h"
#include "sounds.h"
#include "state.h"
#include "grpinfo.h"
#include "planets/genall.h"
#include "libs/mathlib.h"


static void
check_scout (void)
{
	HIPGROUP hGroup;

	if (GLOBAL (BattleGroupRef)
			&& (hGroup = GetHeadLink (&GLOBAL (ip_group_q))))
	{
		IP_GROUP *GroupPtr;

		GroupPtr = LockIpGroup (&GLOBAL (ip_group_q), hGroup);

		if (GroupPtr->task & REFORM_GROUP)
		{
			GroupPtr->task = FLEE | IGNORE_FLAGSHIP | REFORM_GROUP;
			GroupPtr->dest_loc = 0;
		}

		UnlockIpGroup (&GLOBAL (ip_group_q), hGroup);
	}
}

static void
GenerateScout (BYTE control)
{
	switch (control)
	{
		case INIT_NPCS:
			if (!GET_GAME_STATE (MET_ZOQFOT))
			{
				GLOBAL (BattleGroupRef) = GET_GAME_STATE_32 (ZOQFOT_GRPOFFS0);
				if (GLOBAL (BattleGroupRef) == 0)
				{
					CloneShipFragment (ZOQFOTPIK_SHIP,
							&GLOBAL (npc_built_ship_q), 0);
					GLOBAL (BattleGroupRef) = PutGroupInfo (GROUPS_ADD_NEW, 1);
					ReinitQueue (&GLOBAL (npc_built_ship_q));
					SET_GAME_STATE_32 (ZOQFOT_GRPOFFS0,
							GLOBAL (BattleGroupRef));
				}
			}
			GenerateRandomIP (INIT_NPCS);
			break;
		case REINIT_NPCS:
			GenerateRandomIP (REINIT_NPCS);
			check_scout ();
			break;
		default:
			GenerateRandomIP (control);
			break;
	}
}

static void
generate_energy_signatures (void)
{
	COUNT i, which_node;
	DWORD rand_val, old_rand;

	old_rand = TFB_SeedRandom (
			pSolarSysState->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN]);

	which_node = i = 0;
	do
	{
		rand_val = TFB_Random ();
		pSolarSysState->SysInfo.PlanetInfo.CurPt.x =
				(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
		pSolarSysState->SysInfo.PlanetInfo.CurPt.y =
				(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
		pSolarSysState->SysInfo.PlanetInfo.CurType = 1;
		pSolarSysState->SysInfo.PlanetInfo.CurDensity = 0;
		if (which_node >= pSolarSysState->CurNode
				&& !(pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
				& (1L << i)))
			break;
		++which_node;
	} while (++i < 16);
	pSolarSysState->CurNode = which_node;

	TFB_SeedRandom (old_rand);
}

static void
GenerateZoqFotPikColonies (BYTE control)
{
	switch (control)
	{
		case GENERATE_ENERGY:
			if ((pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0]
					&& pSolarSysState->pOrbitalDesc->NumPlanets == 0)
					|| (pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[0]
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[1])) 
			{
				generate_energy_signatures ();
				break;
			}
		case GENERATE_ORBITAL:
			if ((pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0]
					&& pSolarSysState->pOrbitalDesc->NumPlanets == 0)
					|| (pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[0]
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[1])) 
			{
				LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
				pSolarSysState->PlanetSideFrame[1] =
						CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
				pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
						CaptureStringTable (LoadStringTable (ZFPRUINS_STRTAB));
			}
			// note the lack of a break statement here
		default:
			GenerateRandomIP (control);
			break;
	}
}

void
GenerateZoqFotPik (BYTE control)
{
	if (CurStarDescPtr->Index == ZOQ_SCOUT_DEFINED)
	{
		GenerateScout (control);
		return;
	}
	else if (CurStarDescPtr->Index == ZOQ_COLONY_DEFINED)
	{
		GenerateZoqFotPikColonies (control);
		return;
	}

	switch (control)
	{
		case INIT_NPCS:
			if (GET_GAME_STATE (ZOQFOT_DISTRESS) != 1)
				GenerateRandomIP (INIT_NPCS);
			break;
		case GENERATE_ENERGY:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				generate_energy_signatures ();
				break;
			}
			pSolarSysState->CurNode = 0;
			break;
		case GENERATE_MOONS:
			if (CurStarDescPtr->Index == ZOQFOT_DEFINED &&
					pSolarSysState->pBaseDesc == &pSolarSysState->PlanetDesc[0])
			{
				// Setup moons, then add a starbase as the last moon
				pSolarSysState->PlanetDesc[0].NumPlanets = 1;
				GenerateRandomIP (GENERATE_MOONS);
				pSolarSysState->PlanetDesc[0].NumPlanets = 2;

				pSolarSysState->MoonDesc[1].data_index =
						(ActivateStarShip (ZOQFOTPIK_SHIP, SPHERE_TRACKING)) ?
						ZOQFOTPIK_STARBASE : DESTROYED_STARBASE;
				pSolarSysState->MoonDesc[1].radius = MIN_MOON_RADIUS;
				pSolarSysState->MoonDesc[1].location.x =
						COSINE (HALF_CIRCLE + QUADRANT,
								pSolarSysState->MoonDesc[1].radius);
				pSolarSysState->MoonDesc[1].location.y =
						SINE (HALF_CIRCLE + QUADRANT,
								pSolarSysState->MoonDesc[1].radius);
				break;
			}
			GenerateRandomIP (GENERATE_MOONS);
			break;
		case GENERATE_PLANETS:
		{
			COUNT angle;

			GenerateRandomIP (GENERATE_PLANETS);
			pSolarSysState->PlanetDesc[0].data_index = REDUX_WORLD;
			pSolarSysState->PlanetDesc[0].NumPlanets = 1;
			pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 138L / 100;
			angle = ARCTAN (
					pSolarSysState->PlanetDesc[0].location.x,
					pSolarSysState->PlanetDesc[0].location.y
					);
			pSolarSysState->PlanetDesc[0].location.x =
					COSINE (angle, pSolarSysState->PlanetDesc[0].radius);
			pSolarSysState->PlanetDesc[0].location.y =
					SINE (angle, pSolarSysState->PlanetDesc[0].radius);
			break;
		}
		case GENERATE_ORBITAL:
			if ((pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[1]) &&
					(pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[0]))
			{
				// If you go to the starbase, move the ship to
				// the planet instead
				if (ActivateStarShip (ZOQFOTPIK_SHIP, SPHERE_TRACKING))
				{
					pSolarSysState->pOrbitalDesc =
							&pSolarSysState->PlanetDesc[0];
					GLOBAL (ShipStamp.origin.x) = SIS_SCREEN_WIDTH >> 1;
					GLOBAL (ShipStamp.origin.y) = SIS_SCREEN_HEIGHT >> 1;
				}
				// ...unless they're all dead
				else
				{
					RECT r;
					LockMutex (GraphicsLock);

					LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
					pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
							SetRelStringTableIndex (
									CaptureStringTable (
											LoadStringTable (RUINS_STRTAB)), 1);
					ScanContext = CreateContext ();
					SetContext (ScanContext);
					SetContextFGFrame (Screen);
					r.corner.x = (SIS_ORG_X + SIS_SCREEN_WIDTH) - MAP_WIDTH;
					r.corner.y = (SIS_ORG_Y + SIS_SCREEN_HEIGHT) - MAP_HEIGHT;
					r.extent.width = MAP_WIDTH;
					r.extent.height = MAP_HEIGHT;
					SetContextClipRect (&r);

					DoDiscoveryReport (MenuSounds);

					SetContext (SpaceContext);
					DestroyContext (ScanContext);
					ScanContext = 0;

					DestroyStringTable (ReleaseStringTable (
							pSolarSysState->SysInfo.PlanetInfo.DiscoveryString
							));
					pSolarSysState->SysInfo.PlanetInfo.DiscoveryString = 0;
					FreeLanderFont (&pSolarSysState->SysInfo.PlanetInfo);

					UnlockMutex (GraphicsLock);
					break;
				}
			}

			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				if (ActivateStarShip (ZOQFOTPIK_SHIP, SPHERE_TRACKING))
				{
					PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
					ReinitQueue (&GLOBAL (ip_group_q));
					assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

					if (GET_GAME_STATE (ZOQFOT_DISTRESS))
					{
						CloneShipFragment (BLACK_URQUAN_SHIP,
								&GLOBAL (npc_built_ship_q), 0);

						pSolarSysState->MenuState.Initialized += 2;
						GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
						SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
						InitCommunication (BLACKURQ_CONVERSATION);
						pSolarSysState->MenuState.Initialized -= 2;

						if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
							break;

						if (GetHeadLink (&GLOBAL (npc_built_ship_q)))
						{
							GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
							ReinitQueue (&GLOBAL (npc_built_ship_q));
							GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
							break;
						}
					}

					CloneShipFragment (ZOQFOTPIK_SHIP,
							&GLOBAL (npc_built_ship_q), INFINITE_FLEET);

					pSolarSysState->MenuState.Initialized += 2;
					GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
					InitCommunication (ZOQFOTPIK_CONVERSATION);
					pSolarSysState->MenuState.Initialized -= 2;
					if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
					{
						GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
						ReinitQueue (&GLOBAL (npc_built_ship_q));
						GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
					}
					break;
				}
				else
				{
					LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
					pSolarSysState->PlanetSideFrame[1] =
							CaptureDrawable (
							LoadGraphic (RUINS_MASK_PMAP_ANIM)
							);
					pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
							CaptureStringTable (
									LoadStringTable (RUINS_STRTAB)
									);
				}
			}
		default:
			GenerateRandomIP (control);
			break;
	}
}

