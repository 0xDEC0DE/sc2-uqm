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

#include "build.h"
#include "encount.h"
#include "globdata.h"
#include "lander.h"
#include "nameref.h"
#include "resinst.h"
#include "setup.h"
#include "state.h"
#include "sounds.h"
#include "planets/genall.h"
#include "libs/mathlib.h"


void
GenerateNeglectedStarbase (BYTE control)
{
	COUNT angle;

	switch (control)
	{
		case GENERATE_MOONS:
			GenerateRandomIP (GENERATE_MOONS);
			if (pSolarSysState->pBaseDesc == &pSolarSysState->PlanetDesc[0])
			{
				pSolarSysState->MoonDesc[0].data_index = DESTROYED_STARBASE;
				pSolarSysState->MoonDesc[0].radius = MIN_MOON_RADIUS;
				angle = ARCTAN (pSolarSysState->MoonDesc[0].location.x,
						pSolarSysState->MoonDesc[0].location.y);
				pSolarSysState->MoonDesc[0].location.x =
						COSINE (angle, pSolarSysState->MoonDesc[0].radius);
				pSolarSysState->MoonDesc[0].location.y =
						SINE (angle, pSolarSysState->MoonDesc[0].radius);
			}
			break;
		case GENERATE_PLANETS:
			GenerateRandomIP (GENERATE_PLANETS);
			pSolarSysState->PlanetDesc[0].data_index |= PLANET_SHIELDED;
			pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 2L;
			angle = ARCTAN (pSolarSysState->PlanetDesc[0].location.x,
					pSolarSysState->PlanetDesc[0].location.y);
			pSolarSysState->PlanetDesc[0].location.x =
					COSINE (angle, pSolarSysState->PlanetDesc[0].radius);
			pSolarSysState->PlanetDesc[0].location.y =
					SINE (angle, pSolarSysState->PlanetDesc[0].radius);
			pSolarSysState->PlanetDesc[0].NumPlanets = 1;
			break;
		case GENERATE_ORBITAL:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				DoPlanetaryAnalysis (
						&pSolarSysState->SysInfo, pSolarSysState->pOrbitalDesc
						);

				LoadPlanet (NULL);
				break;
			}
				/* Starbase */
			if (pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[0]
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[0])
			{
				RECT r;

				LockMutex (GraphicsLock);

				LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
				pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
						CaptureStringTable (
								LoadStringTable (NEGLECTED_BASE_STRTAB));

				// use alternate text if the player
				// hasn't freed the Earth starbase yet
				if (! GET_GAME_STATE (STARBASE_AVAILABLE))
					pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
							SetRelStringTableIndex (
									pSolarSysState->SysInfo.PlanetInfo.DiscoveryString,
									1);

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
		default:
			GenerateRandomIP (control);
			break;
	}
}
