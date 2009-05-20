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
#include "lifeform.h"
#include "nameref.h"
#include "resinst.h"
#include "setup.h"
#include "state.h"
#include "sounds.h"
#include "planets/genall.h"
#include "libs/mathlib.h"


void
GenerateVUX (BYTE control)
{
	DWORD rand_val;

	switch (control)
	{
		case GENERATE_ENERGY:
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0]
					&& CurStarDescPtr->Index != VUX_BEAST_DEFINED)
			{
				if (CurStarDescPtr->Index == MAIDENS_DEFINED
						&& !GET_GAME_STATE (SHOFIXTI_MAIDENS))
				{
					pSolarSysState->SysInfo.PlanetInfo.CurPt.x = MAP_WIDTH / 3;
					pSolarSysState->SysInfo.PlanetInfo.CurPt.y = MAP_HEIGHT * 5 / 8;
					pSolarSysState->SysInfo.PlanetInfo.CurDensity = 0;
					pSolarSysState->SysInfo.PlanetInfo.CurType = 0;
					if (!(pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
							& (1L << 0))
							&& pSolarSysState->CurNode == (COUNT)~0)
						pSolarSysState->CurNode = 1;
					else
					{
						pSolarSysState->CurNode = 0;
						if (pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
								& (1L << 0))
						{
							SET_GAME_STATE (SHOFIXTI_MAIDENS, 1);
							SET_GAME_STATE (MAIDENS_ON_SHIP, 1);
						}
					}
					break;
				}
				else if (CurStarDescPtr->Index == VUX_DEFINED)
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
					break;
				}
			}
			pSolarSysState->CurNode = 0;
			break;
		case GENERATE_MOONS:
			if (CurStarDescPtr->Index == VUX_DEFINED &&
					pSolarSysState->pBaseDesc == &pSolarSysState->PlanetDesc[0])
			{
				// Setup moons, then add a starbase as the last moon
				pSolarSysState->PlanetDesc[0].NumPlanets = 1;
				GenerateRandomIP (GENERATE_MOONS);
				pSolarSysState->PlanetDesc[0].NumPlanets = 2;

				pSolarSysState->MoonDesc[1].data_index =
						(ActivateStarShip (VUX_SHIP, SPHERE_TRACKING)) ?
						HIERARCHY_STARBASE : DESTROYED_STARBASE;
				pSolarSysState->MoonDesc[1].radius = MIN_MOON_RADIUS;
				pSolarSysState->MoonDesc[1].location.x =
						COSINE (HALF_CIRCLE - OCTANT,
						pSolarSysState->MoonDesc[1].radius);
				pSolarSysState->MoonDesc[1].location.y =
						SINE (HALF_CIRCLE - OCTANT,
						pSolarSysState->MoonDesc[1].radius);
				break;
			}
			GenerateRandomIP (GENERATE_MOONS);
			break;
		case GENERATE_PLANETS:
		{
			COUNT angle;

			GenerateRandomIP (GENERATE_PLANETS);

			if (CurStarDescPtr->Index == MAIDENS_DEFINED)
			{
				GenerateRandomIP (GENERATE_PLANETS);
				pSolarSysState->PlanetDesc[0].data_index = REDUX_WORLD;
				pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 212L / 100;
				angle = ARCTAN (
						pSolarSysState->PlanetDesc[0].location.x,
						pSolarSysState->PlanetDesc[0].location.y
						);
				pSolarSysState->PlanetDesc[0].location.x =
						COSINE (angle, pSolarSysState->PlanetDesc[0].radius);
				pSolarSysState->PlanetDesc[0].location.y =
						SINE (angle, pSolarSysState->PlanetDesc[0].radius);
			}
			else
			{
				if (CurStarDescPtr->Index == VUX_DEFINED)
				{
					pSolarSysState->PlanetDesc[0].data_index = REDUX_WORLD;
					pSolarSysState->PlanetDesc[0].NumPlanets = 1;
					pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 42L / 100;
					angle = HALF_CIRCLE + OCTANT;
				}
				else /* if (CurStarDescPtr->Index == VUX_BEAST_DEFINED) */
				{
					memmove (&pSolarSysState->PlanetDesc[1],
							&pSolarSysState->PlanetDesc[0],
							sizeof (pSolarSysState->PlanetDesc[0])
							* pSolarSysState->SunDesc[0].NumPlanets);
					++pSolarSysState->SunDesc[0].NumPlanets;

					angle = HALF_CIRCLE - OCTANT;
					pSolarSysState->PlanetDesc[0].data_index = WATER_WORLD;
					pSolarSysState->PlanetDesc[0].radius = EARTH_RADIUS * 110L / 100;
					pSolarSysState->PlanetDesc[0].NumPlanets = 0;
				}
				pSolarSysState->PlanetDesc[0].location.x =
						COSINE (angle, pSolarSysState->PlanetDesc[0].radius);
				pSolarSysState->PlanetDesc[0].location.y =
						SINE (angle, pSolarSysState->PlanetDesc[0].radius);
				pSolarSysState->PlanetDesc[0].rand_seed = MAKE_DWORD (
						pSolarSysState->PlanetDesc[0].location.x,
						pSolarSysState->PlanetDesc[0].location.y
						);
			}
			break;
		}
		case GENERATE_ORBITAL:
		{
			if ((CurStarDescPtr->Index == VUX_DEFINED) && 
					(pSolarSysState->pOrbitalDesc == &pSolarSysState->MoonDesc[1]) &&
					(pSolarSysState->pOrbitalDesc->pPrevDesc == &pSolarSysState->PlanetDesc[0]))
			{
				// If you go to the starbase, move the ship to
				// the planet instead
				if (ActivateStarShip (VUX_SHIP, SPHERE_TRACKING))
				{
					pSolarSysState->pOrbitalDesc =
							&pSolarSysState->PlanetDesc[0];
					GLOBAL (ShipStamp.origin.x) = SIS_SCREEN_WIDTH >> 1;
					GLOBAL (ShipStamp.origin.y) = SIS_SCREEN_HEIGHT >> 1;
				}
				// ...unless the Kohr-Ah have been through here
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

			if ((pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0]
					&& (CurStarDescPtr->Index == VUX_DEFINED
					|| (CurStarDescPtr->Index == MAIDENS_DEFINED
					&& !GET_GAME_STATE (ZEX_IS_DEAD))))
					&& ActivateStarShip (VUX_SHIP, SPHERE_TRACKING))
			{
				NotifyOthers (VUX_SHIP, (BYTE)~0);
				PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
				ReinitQueue (&GLOBAL (ip_group_q));
				assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

				CloneShipFragment (VUX_SHIP,
						&GLOBAL (npc_built_ship_q), INFINITE_FLEET);
				if (CurStarDescPtr->Index == VUX_DEFINED)
				{
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
				}
				else
				{
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
				}

				pSolarSysState->MenuState.Initialized += 2;
				GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
				InitCommunication (VUX_CONVERSATION);
				pSolarSysState->MenuState.Initialized -= 2;

				if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
					break;
				else
				{
					GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
					ReinitQueue (&GLOBAL (npc_built_ship_q));
					GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);

					if (CurStarDescPtr->Index == VUX_DEFINED
							|| !GET_GAME_STATE (ZEX_IS_DEAD))
						break;

					LockMutex (GraphicsLock);
					RepairSISBorder ();
					UnlockMutex (GraphicsLock);
				}
			}

			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				if (CurStarDescPtr->Index == MAIDENS_DEFINED)
				{
					if (!GET_GAME_STATE (SHOFIXTI_MAIDENS))
					{
						LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
						pSolarSysState->PlanetSideFrame[1] =
								CaptureDrawable (
										LoadGraphic (MAIDENS_MASK_PMAP_ANIM)
										);
						pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
								CaptureStringTable (
										LoadStringTable (MAIDENS_STRTAB)
										);
					}
				}
				else if (CurStarDescPtr->Index == VUX_BEAST_DEFINED)
				{
					if (!GET_GAME_STATE (VUX_BEAST))
					{
						LoadStdLanderFont (&pSolarSysState->SysInfo.PlanetInfo);
						pSolarSysState->PlanetSideFrame[1] = 0;
						pSolarSysState->SysInfo.PlanetInfo.DiscoveryString =
								CaptureStringTable (
										LoadStringTable (BEAST_STRTAB)
										);
					}
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
			GenerateRandomIP (GENERATE_ORBITAL);
			if (pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				pSolarSysState->SysInfo.PlanetInfo.Weather = 2;
				pSolarSysState->SysInfo.PlanetInfo.Tectonics = 0;
			}
			break;
		}
		case GENERATE_LIFE:
			if (CurStarDescPtr->Index == MAIDENS_DEFINED
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				COUNT i;
				DWORD old_rand;

				old_rand = TFB_SeedRandom (pSolarSysState->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN]);

				i = 0;
				do
				{
					rand_val = TFB_Random ();
					pSolarSysState->SysInfo.PlanetInfo.CurPt.x =
							(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
					pSolarSysState->SysInfo.PlanetInfo.CurPt.y =
							(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
					if (i < 4)
						pSolarSysState->SysInfo.PlanetInfo.CurType = 9;
					else if (i < 8)
						pSolarSysState->SysInfo.PlanetInfo.CurType = 14;
					else /* if (i < 12) */
						pSolarSysState->SysInfo.PlanetInfo.CurType = 18;
					if (i >= pSolarSysState->CurNode
							&& !(pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
							& (1L << i)))
						break;
				} while (++i < 12);
				pSolarSysState->CurNode = i;

				TFB_SeedRandom (old_rand);
				break;
			}
			else if (CurStarDescPtr->Index == VUX_BEAST_DEFINED
					&& pSolarSysState->pOrbitalDesc == &pSolarSysState->PlanetDesc[0])
			{
				COUNT i;
				DWORD old_rand;

				old_rand = TFB_SeedRandom (pSolarSysState->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN]);

				i = 0;
				do
				{
					rand_val = TFB_Random ();
					pSolarSysState->SysInfo.PlanetInfo.CurPt.x =
							(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
					pSolarSysState->SysInfo.PlanetInfo.CurPt.y =
							(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
					if (i == 0)
						pSolarSysState->SysInfo.PlanetInfo.CurType = NUM_CREATURE_TYPES + 2;
					else if (i <= 5)
							/* {SPEED_MOTIONLESS | DANGER_NORMAL, MAKE_BYTE (5, 3)}, */
						pSolarSysState->SysInfo.PlanetInfo.CurType = 3;
					else /* if (i <= 10) */
							/* {BEHAVIOR_UNPREDICTABLE | SPEED_SLOW | DANGER_NORMAL, MAKE_BYTE (3, 8)}, */
						pSolarSysState->SysInfo.PlanetInfo.CurType = 8;
					if (i >= pSolarSysState->CurNode
							&& !(pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
							& (1L << i)))
						break;
					else if (i == 0
							&& (pSolarSysState->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
							& (1L << i))
							&& !GET_GAME_STATE (VUX_BEAST))
					{
						PLANETSIDE_DESC *pPSD;

						pPSD = (PLANETSIDE_DESC*)pMenuState->ModuleFrame;
						UnbatchGraphics ();
						DoDiscoveryReport (MenuSounds);
						BatchGraphics ();
						pPSD->InTransit = TRUE;

						SET_GAME_STATE (VUX_BEAST, 1);
						SET_GAME_STATE (VUX_BEAST_ON_SHIP, 1);
					}
				} while (++i < 11);
				pSolarSysState->CurNode = i;

				TFB_SeedRandom (old_rand);
				break;
			}
		default:
			GenerateRandomIP (control);
			break;
	}
}

