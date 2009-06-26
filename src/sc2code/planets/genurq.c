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

static void
generate_doctrinal_space_junk (BYTE control, int index)
{
	COUNT angle, p;

	if (control != GENERATE_ORBITAL)
		GenerateRandomIP (control);

	// Look for a planet with no moons, to put a destroyed starbase there
	for (p = 0; p < pSolarSysState->SunDesc[0].NumPlanets; p++)
	{
		if (pSolarSysState->PlanetDesc[p].NumPlanets <= 1)
			break;
	}

	switch (control)
	{
		case GENERATE_MOONS:
			if (pSolarSysState->pBaseDesc == &pSolarSysState->PlanetDesc[p])
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
			pSolarSysState->PlanetDesc[p].NumPlanets = 1;
			break;
		case GENERATE_ORBITAL:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[p])
			{
				DoPlanetaryAnalysis (
						&pSolarSysState->SysInfo, pSolarSysState->pOrbitalDesc
						);

				LoadPlanet (NULL);
				break;
			}
			/* Starbase */
			if (pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[p]
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[0])
			{
				RECT r;

				LockMutex (GraphicsLock);

				LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
				pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
						SetRelStringTableIndex (
								CaptureStringTable (
										LoadStringTable (URQUAN_BASE_STRTAB)),
										index);

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
			break;
	}
}

void
GenerateUrQuan (BYTE control)
{
	generate_doctrinal_space_junk (control, 0);
}

void
GenerateKohrAh (BYTE control)
{
	generate_doctrinal_space_junk (control, 1);
}
