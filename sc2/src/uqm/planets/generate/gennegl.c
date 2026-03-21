/*
 *  Copyright (C) 2008  Nicolas Simonds <uqm@submedia.net>
 *
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
#include "../../ipdisp.h"
#include "../../nameref.h"
#include "../../state.h"
#include "../../sounds.h"
#include "libs/mathlib.h"


static bool GenerateNeglected_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateNeglected_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateNeglected_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateNeglectedStarbaseFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateNeglected_generatePlanets,
	/* .generateMoons    = */ GenerateNeglected_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateNeglected_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GenerateNeglected_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index |= PLANET_SHIELDED;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 2L;
	angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].NumPlanets = 1;
	return true;
}

static bool
GenerateNeglected_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	COUNT angle;

	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, 0, MATCH_PLANET))
	{
		solarSys->MoonDesc[0].data_index = DESTROYED_STARBASE;
		solarSys->MoonDesc[0].radius = MIN_MOON_RADIUS;
		angle = ARCTAN (solarSys->MoonDesc[0].location.x,
				solarSys->MoonDesc[0].location.y);
		solarSys->MoonDesc[0].location.x =
				COSINE (angle, solarSys->MoonDesc[0].radius);
		solarSys->MoonDesc[0].location.y =
				SINE (angle, solarSys->MoonDesc[0].radius);
	}

	return true;
}

static bool
GenerateNeglected_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, 0))
	{
		/* Starbase */
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (
						LoadStringTable (NEGLECTED_BASE_STRTAB));

		// use alternate text if the player
		// hasn't freed the Earth starbase yet
		if (! GET_GAME_STATE (STARBASE_AVAILABLE))
		{
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					SetRelStringTableIndex (
							solarSys->SysInfo.PlanetInfo.DiscoveryString,
							1);
		}

		GenerateDefault_landerReportCycle (solarSys);

		DestroyStringTable (ReleaseStringTable (
				solarSys->SysInfo.PlanetInfo.DiscoveryString
				));
		solarSys->SysInfo.PlanetInfo.DiscoveryString = 0;
		FreeLanderFont (&solarSys->SysInfo.PlanetInfo);

		return true;
	}

	return GenerateDefault_generateOrbital (solarSys, world);
}
