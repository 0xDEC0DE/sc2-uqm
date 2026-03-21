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
#include "../lander.h"
#include "../planets.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../resinst.h"
#include "libs/mathlib.h"


extern const GenerateFunctions generateMelnormeFunctions;

static bool GeneratePrecursorStarbase_initNpcs (SOLARSYS_STATE *solarSys);
static bool GeneratePrecursorStarbase_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GeneratePrecursorStarbase_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generatePrecursorStarbaseFunctions = {
	/* .initNpcs         = */ GeneratePrecursorStarbase_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateDefault_generatePlanets,
	/* .generateMoons    = */ GeneratePrecursorStarbase_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GeneratePrecursorStarbase_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
	/* .pickupMinerals   = */ GenerateDefault_pickupMinerals,
	/* .pickupEnergy     = */ GenerateDefault_pickupEnergy,
	/* .pickupLife       = */ GenerateDefault_pickupLife,
};


static bool
GeneratePrecursorStarbase_initNpcs (SOLARSYS_STATE *solarSys)
{
	return generateMelnormeFunctions.initNpcs (solarSys);
}

static bool
GeneratePrecursorStarbase_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet)
{
	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, 0, MATCH_PLANET))
	{
		PLANET_DESC *moon = &solarSys->MoonDesc[0];
		COUNT angle;

		planet->NumPlanets = 1;

		moon->data_index = PRECURSOR_STARBASE;
		moon->radius = MIN_MOON_RADIUS;
		angle = ARCTAN (moon->location.x, moon->location.y);
		moon->location.x = COSINE (angle, moon->radius);
		moon->location.y = SINE (angle, moon->radius);
	}

	return true;
}

static bool
GeneratePrecursorStarbase_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, 0))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (PRECURSOR_BASE_STRTAB));

		GenerateDefault_landerReport (solarSys);

		FreeLanderFont (&solarSys->SysInfo.PlanetInfo);

		// We do not call GenerateDefault_generateOrbital here
		// because the starbase is not landable.
		return true;
	}

	return GenerateDefault_generateOrbital (solarSys, world);
}
