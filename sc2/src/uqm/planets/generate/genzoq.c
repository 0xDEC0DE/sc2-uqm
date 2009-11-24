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

#include "genall.h"
#include "../planets.h"
#include "../../build.h"
#include "../../comm.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateZoqFotPik_initNpcs (SOLARSYS_STATE *solarSys);
static bool GenerateZoqFotPik_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateZoqFotPik_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateZoqFotPik_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateZoqFotPikFunctions = {
	/* .initNpcs         = */ GenerateZoqFotPik_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateZoqFotPik_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateZoqFotPik_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateZoqFotPik_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateZoqFotPik_initNpcs (SOLARSYS_STATE *solarSys)
{
	if (GET_GAME_STATE (ZOQFOT_DISTRESS) != 1)
		GenerateDefault_initNpcs (solarSys);

	return true;
}

static bool
GenerateZoqFotPik_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = REDUX_WORLD;
	solarSys->PlanetDesc[0].NumPlanets = 1;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 138L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);

	return true;
}

static bool
GenerateZoqFotPik_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
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

				solarSys->MenuState.Initialized += 2;
				GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
				SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
				InitCommunication (BLACKURQ_CONVERSATION);
				solarSys->MenuState.Initialized -= 2;

				if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
					return true;

				if (GetHeadLink (&GLOBAL (npc_built_ship_q)))
				{
					GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
					ReinitQueue (&GLOBAL (npc_built_ship_q));
					GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
					return true;
				}
			}

			CloneShipFragment (ZOQFOTPIK_SHIP, &GLOBAL (npc_built_ship_q),
					INFINITE_FLEET);

			solarSys->MenuState.Initialized += 2;
			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			InitCommunication (ZOQFOTPIK_CONVERSATION);
			solarSys->MenuState.Initialized -= 2;

			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
			{
				GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
				ReinitQueue (&GLOBAL (npc_built_ship_q));
				GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
			}

			return true;
		}

		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (RUINS_STRTAB));
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static bool
GenerateZoqFotPik_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		COUNT i;
		COUNT nodeI;
		DWORD rand_val;
		DWORD old_rand;

		old_rand = TFB_SeedRandom (
				solarSys->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN]);

		nodeI = 0;
		i = 0;
		do
		{
			rand_val = TFB_Random ();
			solarSys->SysInfo.PlanetInfo.CurPt.x =
					(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurPt.y =
					(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurType = 1;
			solarSys->SysInfo.PlanetInfo.CurDensity = 0;
			if (nodeI >= *whichNode
					&& !(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
					& (1L << i)))
				break;
			++nodeI;
		} while (++i < 16);
		*whichNode = nodeI;

		TFB_SeedRandom (old_rand);
		return true;
	}

	*whichNode = 0;
	return true;
}

