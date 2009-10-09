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
#include "ipdisp.h"
#include "nameref.h"
#include "resinst.h"
#include "state.h"
#include "planets/genall.h"
#include "libs/mathlib.h"


void
GenerateDruuge (BYTE control)
{
	switch (control)
	{
		case GENERATE_ENERGY:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				COUNT i, which_node;
				DWORD rand_val, old_rand;

				old_rand = TFB_SeedRandom (
						pSolarSysState->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN]
						);

				which_node = i = 0;
				do
				{
					rand_val = TFB_Random ();
					pSolarSysState->SysInfo.PlanetInfo.CurPt.x =
							(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
					pSolarSysState->SysInfo.PlanetInfo.CurPt.y =
							(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
					if (!GET_GAME_STATE (ROSY_SPHERE))
						pSolarSysState->SysInfo.PlanetInfo.CurType = 0;
					else
						pSolarSysState->SysInfo.PlanetInfo.CurType = 1;
					pSolarSysState->SysInfo.PlanetInfo.CurDensity = 0;
					if (pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
							& (1L << i))
					{
						pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
								&= ~(1L << i);

						if (!GET_GAME_STATE (ROSY_SPHERE))
						{
							((PLANETSIDE_DESC*)pMenuState->ModuleFrame)->InTransit = TRUE;

							SET_GAME_STATE (ROSY_SPHERE, 1);
							SET_GAME_STATE (ROSY_SPHERE_ON_SHIP, 1);
						}
					}
					if (which_node >= pSolarSysState->CurNode
							&& !(pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
							& (1L << i)))
						break;
					++which_node;
				} while (++i < 16);
				pSolarSysState->CurNode = which_node;

				TFB_SeedRandom (old_rand);
				break;
			}
			pSolarSysState->CurNode = 0;
			break;
		case GENERATE_PLANETS:
		{
			COUNT angle;

			GenerateRandomIP (GENERATE_PLANETS);

			memmove (&pSolarSysState->PlanetDesc[1],
					&pSolarSysState->PlanetDesc[0],
					sizeof (pSolarSysState->PlanetDesc[0])
					* pSolarSysState->SunDesc[0].NumPlanets);
			++pSolarSysState->SunDesc[0].NumPlanets;

			pSolarSysState->PlanetDesc[0].data_index = DUST_WORLD;
			pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 50L / 100;
			pSolarSysState->PlanetDesc[0].NumPlanets = 0;
			angle = HALF_CIRCLE - OCTANT;
			pSolarSysState->PlanetDesc[0].location.x =
					COSINE (angle, pSolarSysState->PlanetDesc[0].radius);
			pSolarSysState->PlanetDesc[0].location.y =
					SINE (angle, pSolarSysState->PlanetDesc[0].radius);
			pSolarSysState->PlanetDesc[0].rand_seed = MAKE_DWORD (
					pSolarSysState->PlanetDesc[0].location.x,
					pSolarSysState->PlanetDesc[0].location.y
					);
			break;
		}
		case GENERATE_ORBITAL:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				if (ActivateStarShip (DRUUGE_SHIP, SPHERE_TRACKING))
				{
					NotifyOthers (DRUUGE_SHIP, (BYTE)~0);
					PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
					ReinitQueue (&GLOBAL (ip_group_q));
					assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

					CloneShipFragment (DRUUGE_SHIP,
							&GLOBAL (npc_built_ship_q), INFINITE_FLEET);

					pSolarSysState->MenuState.Initialized += 2;
					GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
					InitCommunication (DRUUGE_CONVERSATION);
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
									LoadStringTable (DRUUGE_RUINS_STRTAB)
									);
					if (GET_GAME_STATE (ROSY_SPHERE))
						pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
								SetAbsStringTableIndex (
								pSolarSysState->SysInfo.PlanetInfo.DiscoveryString,
								1
								);
				}
			}
		default:
			GenerateRandomIP (control);
			break;
	}
}

