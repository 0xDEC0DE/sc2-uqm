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

#include "races.h"
#include "master.h"
#include "setup.h"
#include "libs/compiler.h"
#include "libs/mathlib.h"


// Allocate a new STARSHIP and put it in the queue.
HLINK
Build (QUEUE *pQueue, DWORD RaceResIndex)
{
	HLINK hNewShip;
	void *LinkPtr;

	hNewShip = AllocLink (pQueue);
	if (!hNewShip)
		return 0;

	LinkPtr = LockLink (pQueue, hNewShip);
	memset (LinkPtr, 0, GetLinkSize (pQueue));

	if (GetLinkSize (pQueue) == sizeof (STARSHIP))
	{
		STARSHIP *StarShipPtr = (STARSHIP *) LinkPtr;
		StarShipPtr->RaceResIndex = RaceResIndex;
	}
	else if (GetLinkSize (pQueue) == sizeof (SHIP_FRAGMENT))
	{
		SHIP_FRAGMENT *FragPtr = (SHIP_FRAGMENT *) LinkPtr;
		FragPtr->RaceResIndex = RaceResIndex;
	}
	else
		assert (0 && "Build(): unknown queue type!");

	UnlockLink (pQueue, hNewShip);
	PutQueue (pQueue, hNewShip);

	return hNewShip;
}

HLINK
GetStarShipFromIndex (QUEUE *pShipQ, COUNT Index)
{
	HLINK hStarShip, hNextShip;

	for (hStarShip = GetHeadLink (pShipQ);
			Index > 0 && hStarShip; hStarShip = hNextShip, --Index)
	{
		LINK *StarShipPtr;

		StarShipPtr = LockLink (pShipQ, hStarShip);
		hNextShip = _GetSuccLink (StarShipPtr);
		UnlockLink (pShipQ, hStarShip);
	}

	return (hStarShip);
}

/*
 * What this function does depends on the value of the 'state' argument:
 * SPHERE_TRACKING:
 * 	The sphere of influence for the race for 'which_ship' will be shown
 * 	on the starmap in the future.
 * 	The value returned is 'which_ship', unless the type of ship is only
 * 	available in SuperMelee, in which case 0 is returned.
 * SPHERE_KNOWN:
 * 	The size of the fleet of the race of 'which_ship' when the starmap was
 * 	last checked is returned.
 * ESCORT_WORTH:
 * 	The total value of all the ships escorting the SIS is returned.
 * 	'which_ship' is ignored.
 * ESCORTING_FLAGSHIP:
 * 	Test if a ship of type 'which_ship' is among the escorts of the SIS
 * 	0 is returned if false, 1 if true.
 * FEASIBILITY_STUDY:
 * 	Test if the SIS can have an escort of type 'which_ship'.
 * 	0 is returned if 'which_ship' is not available.
 * 	Otherwise, the number of ships that can be added is returned.
 * CHECK_ALLIANCE:
 * 	Test the alliance status of the race of 'which_ship'.
 *      Either GOOD_GUY (allied) or BAD_GUY (not allied) is returned.
 * SET_ALLIED (0):
 * 	Ally with the race of 'which_ship'. This makes their ship available
 *  for building in the shipyard.
 * SET_NOT_ALLIED:
 * 	End an alliance with the race of 'which_ship'. This ends the possibility
 * 	of building their ships in the shipyard.
 * REMOVE_BUILT: 
 *  Make the already built escorts of the race of 'which_ship' disappear.
 *   (as for the Orz when the alliance with them ends)
 * any other positive number:
 * 	Give the player this many ships of type 'which_ship'.
 */
COUNT
ActivateStarShip (COUNT which_ship, SIZE state)
{
	HFLEETINFO hFleet;

	hFleet = GetStarShipFromIndex (&GLOBAL (avail_race_q), which_ship);
	if (!hFleet)
		return 0;

	switch (state)
	{
		case SPHERE_TRACKING:
		case SPHERE_KNOWN:
		{
			FLEET_INFO *FleetPtr;

			FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q), hFleet);
			if (state == SPHERE_KNOWN)
				which_ship = FleetPtr->known_strength;
			else if (FleetPtr->actual_strength == 0)
			{
				if (!(FleetPtr->ship_flags & (GOOD_GUY | BAD_GUY)))
					which_ship = 0;
			}
			else if (FleetPtr->known_strength == 0
					&& FleetPtr->actual_strength != (COUNT)~0)
			{
				FleetPtr->known_strength = 1;
				FleetPtr->known_loc = FleetPtr->loc;
			}
			UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);
			return (which_ship);
		}
		case ESCORT_WORTH:
		{
			COUNT ShipCost[] =
			{
				RACE_SHIP_COST
			};
			COUNT total = 0;
			HSHIPFRAG hStarShip, hNextShip;

			for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
					hStarShip; hStarShip = hNextShip)
			{
				SHIP_FRAGMENT *StarShipPtr;

				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
				hNextShip = _GetSuccLink (StarShipPtr);
				total += ShipCost[GET_RACE_ID (StarShipPtr)];
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
			}
			return total;
		}
		case ESCORTING_FLAGSHIP:
		{
			HSHIPFRAG hStarShip, hNextShip;

			for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
					hStarShip; hStarShip = hNextShip)
			{
				BYTE ship_type;
				SHIP_FRAGMENT *StarShipPtr;

				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
				hNextShip = _GetSuccLink (StarShipPtr);
				ship_type = GET_RACE_ID (StarShipPtr);
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);

				if (ship_type == which_ship)
					return 1;
			}
			return 0;
		}
		case FEASIBILITY_STUDY:
		{
			return (MAX_BUILT_SHIPS - CountLinks (&GLOBAL (built_ship_q)));
		}
		case CHECK_ALLIANCE:
		{
			COUNT flags;
			FLEET_INFO *FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q),
					hFleet);
			flags = FleetPtr->ship_flags & (GOOD_GUY | BAD_GUY);
			UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);
			return flags;
		}
		case SET_ALLIED:
		case SET_NOT_ALLIED:
		{
			FLEET_INFO *FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q),
					hFleet);

			if (!(FleetPtr->ship_flags & (GOOD_GUY | BAD_GUY)))
			{	/* Strange request, silently ignore it */
				UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);
				break;
			}

			FleetPtr->ship_flags &= ~(GOOD_GUY | BAD_GUY);
			if (state == SET_ALLIED)
				FleetPtr->ship_flags |= GOOD_GUY;
			else
				FleetPtr->ship_flags |= BAD_GUY;
			
			UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);
			break;
		}
		case REMOVE_BUILT:
		{
			HSHIPFRAG hStarShip, hNextShip;
			BOOLEAN ShipRemoved = FALSE;

			for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
					hStarShip; hStarShip = hNextShip)
			{
				BOOLEAN RemoveShip;
				SHIP_FRAGMENT *StarShipPtr;

				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
				hNextShip = _GetSuccLink (StarShipPtr);
				RemoveShip = (GET_RACE_ID (StarShipPtr) == which_ship);
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);

				if (RemoveShip)
				{
					ShipRemoved = TRUE;

					RemoveQueue (&GLOBAL (built_ship_q), hStarShip);
					FreeShipFrag (&GLOBAL (built_ship_q), hStarShip);
				}
			}
			
			if (ShipRemoved)
			{
				LockMutex (GraphicsLock);
				DeltaSISGauges (UNDEFINED_DELTA, UNDEFINED_DELTA,
						UNDEFINED_DELTA);
				UnlockMutex (GraphicsLock);
			}
			break;
		}
		default:
		{
			BYTE which_window;
			COUNT i;

			assert (state > 0);
			/* Add ships to the escorts */
			which_window = 0;
			for (i = 0; i < (COUNT)state; i++)
			{
				HSHIPFRAG hStarShip;
				HSHIPFRAG hOldShip;
				SHIP_FRAGMENT *StarShipPtr;

				hStarShip = CloneShipFragment (which_ship,
						&GLOBAL (built_ship_q), 0);
				if (!hStarShip)
					break;

				RemoveQueue (&GLOBAL (built_ship_q), hStarShip);

				/* Find first available escort window */
				while ((hOldShip = GetStarShipFromIndex (
						&GLOBAL (built_ship_q), which_window++)))
				{
					BYTE win_loc;

					StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q),
							hOldShip);
					win_loc = StarShipPtr->var2;
					UnlockShipFrag (&GLOBAL (built_ship_q), hOldShip);
					if (which_window <= win_loc)
						break;
				}

				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
				StarShipPtr->var2 = which_window - 1;
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);

				InsertQueue (&GLOBAL (built_ship_q), hStarShip, hOldShip);
			}

			LockMutex (GraphicsLock);
			DeltaSISGauges (UNDEFINED_DELTA,
					UNDEFINED_DELTA, UNDEFINED_DELTA);
			UnlockMutex (GraphicsLock);
			return i;
		}
	}

	return 1;
}

COUNT
GetIndexFromStarShip (QUEUE *pShipQ, HLINK hStarShip)
{
	COUNT Index;

	Index = 0;
	while (hStarShip != GetHeadLink (pShipQ))
	{
		HLINK hNextShip;
		LINK *StarShipPtr;

		StarShipPtr = LockLink (pShipQ, hStarShip);
		hNextShip = _GetPredLink (StarShipPtr);
		UnlockLink (pShipQ, hStarShip);

		hStarShip = hNextShip;
		++Index;
	}

	return Index;
}

BYTE
NameCaptain (QUEUE *pQueue, DWORD RaceResIndex)
{
	BYTE name_index;
	HLINK hStarShip;

	do
	{
		HLINK hNextShip;

		name_index = PickCaptainName ();
		for (hStarShip = GetHeadLink (pQueue); hStarShip; hStarShip = hNextShip)
		{
			LINK *LinkPtr;
			BYTE test_name_index;

			LinkPtr = LockLink (pQueue, hStarShip);
			hNextShip = _GetSuccLink (LinkPtr);
			if (GetLinkSize (pQueue) == sizeof (STARSHIP))
			{
				STARSHIP *TestShipPtr = (STARSHIP *) LinkPtr;
				if (TestShipPtr->RaceResIndex != RaceResIndex)
					continue;
				test_name_index = TestShipPtr->captains_name_index;
			}
			else if (GetLinkSize (pQueue) == sizeof (SHIP_FRAGMENT))
			{
				SHIP_FRAGMENT *TestShipPtr = (SHIP_FRAGMENT *) LinkPtr;
				if (TestShipPtr->RaceResIndex != RaceResIndex)
					continue;
				test_name_index = TestShipPtr->captains_name_index;
			}
			else
				assert (0 && "NameCaptain(): unknown queue type!");
			
			if (name_index == test_name_index)
			{
				UnlockLink (pQueue, hStarShip);
				break;
			}
			UnlockLink (pQueue, hStarShip);
		}
	} while (hStarShip);

	return name_index;
}

// crew_level can be set to INFINITE_FLEET for a ship which is to
// represent an infinite number of ships.
HSHIPFRAG
CloneShipFragment (COUNT shipIndex, QUEUE *pDstQueue, COUNT crew_level)
{
	HFLEETINFO hFleet;
	HSHIPFRAG hBuiltShip;
	FLEET_INFO *TemplatePtr;
	BYTE captains_name_index;

	assert (GetLinkSize (pDstQueue) == sizeof (SHIP_FRAGMENT));

	hFleet = GetStarShipFromIndex (&GLOBAL (avail_race_q), shipIndex);
	if (!hFleet)
		return 0;

	TemplatePtr = LockFleetInfo (&GLOBAL (avail_race_q), hFleet);
	if (shipIndex == SAMATRA_SHIP)
		captains_name_index = 0;
	else
		captains_name_index = NameCaptain (pDstQueue,
				TemplatePtr->RaceResIndex);
	hBuiltShip = Build (pDstQueue, TemplatePtr->RaceResIndex);
	if (hBuiltShip)
	{
		SHIP_FRAGMENT *ShipFragPtr;

		ShipFragPtr = LockShipFrag (pDstQueue, hBuiltShip);
		ShipFragPtr->which_side = TemplatePtr->ship_flags &
				(GOOD_GUY | BAD_GUY);
		ShipFragPtr->captains_name_index = captains_name_index;
		// XXX: SHIP_INFO struct copy
		ShipFragPtr->race_strings = TemplatePtr->race_strings;
		ShipFragPtr->icons = TemplatePtr->icons;
		ShipFragPtr->melee_icon = TemplatePtr->melee_icon;
		if (crew_level)
			ShipFragPtr->crew_level = crew_level;
		else
			ShipFragPtr->crew_level = TemplatePtr->crew_level;
		ShipFragPtr->max_crew = TemplatePtr->max_crew;
		ShipFragPtr->energy_level = 0;
		ShipFragPtr->max_energy = TemplatePtr->max_energy;
		ShipFragPtr->ship_flags = 0;
		ShipFragPtr->var1 = 0;
		ShipFragPtr->var2 = 0;
		ShipFragPtr->loc.x = 0;
		ShipFragPtr->loc.y = 0;
		SET_RACE_ID (ShipFragPtr, (BYTE)shipIndex);
		UnlockShipFrag (pDstQueue, hBuiltShip);
	}
	UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);

	return hBuiltShip;
}

/* Set the crew and captain's name on the first fully-crewed escort
 * ship of race 'which_ship' */
int
SetEscortCrewComplement (COUNT which_ship, COUNT crew_level, BYTE captain)
{
	HFLEETINFO hFleet;
	FLEET_INFO *TemplatePtr;
	HSHIPFRAG hStarShip, hNextShip;
	SHIP_FRAGMENT *StarShipPtr = 0;
	int Index;

	hFleet = GetStarShipFromIndex (&GLOBAL (avail_race_q), which_ship);
	if (!hFleet)
		return -1;
	TemplatePtr = LockFleetInfo (&GLOBAL (avail_race_q), hFleet);

	/* Find first ship of which_ship race */
	for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q)), Index = 0;
			hStarShip; hStarShip = hNextShip, ++Index)
	{
		StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
		hNextShip = _GetSuccLink (StarShipPtr);
		if (which_ship == GET_RACE_ID (StarShipPtr) &&
				StarShipPtr->crew_level == TemplatePtr->crew_level)
			break; /* found one */
		UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
	}
	if (hStarShip)
	{
		StarShipPtr->crew_level = crew_level;
		StarShipPtr->captains_name_index = captain;
		UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
	}
	else
		Index = -1;

	UnlockFleetInfo (&GLOBAL (avail_race_q), hFleet);
	return Index;
}
