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

#include "starcon.h"

void
DrawCrewFuelString (COORD y, SIZE state)
{
	STAMP Stamp;
	extern FRAME status;

	Stamp.origin.y = y + GAUGE_YOFFS + STARCON_TEXT_HEIGHT;
	if (state == 0)
	{
		Stamp.origin.x = CREW_XOFFS + (STAT_WIDTH >> 1) + 6;
		Stamp.frame = SetAbsFrameIndex (status, 0);
		DrawStamp (&Stamp);
	}

	Stamp.origin.x = ENERGY_XOFFS + (STAT_WIDTH >> 1) - 5;
	Stamp.frame = SetAbsFrameIndex (status, 1);
	if (state >= 0)
		DrawStamp (&Stamp);
	else
	{
#define LOW_FUEL_COLOR BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0xA), 0x0E)
		SetContextForeGroundColor (LOW_FUEL_COLOR);
		DrawFilledStamp (&Stamp);
	}
}

static void
DrawShipNameString (UNICODE *pStr, COUNT CharCount, COORD y)
{
	TEXT Text;
	FONT OldFont;

	OldFont = SetContextFont (StarConFont);

	Text.pStr = pStr;
	Text.CharCount = CharCount;
	Text.align = ALIGN_CENTER;

	Text.baseline.y = STARCON_TEXT_HEIGHT + 3 + y;
	Text.baseline.x = STATUS_WIDTH >> 1;

	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19));
	DrawText (&Text);
	--Text.baseline.y;
	SetContextForeGroundColor (BLACK_COLOR);
	DrawText (&Text);

	SetContextFont (OldFont);
}

void
ClearShipStatus (COORD y)
{
	RECT r;

	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0xA, 0xA, 0xA), 0x08));
	r.corner.x = 2;
	r.corner.y = 3 + y;
	r.extent.width = STATUS_WIDTH - 4;
	r.extent.height = SHIP_INFO_HEIGHT - 3;
	DrawFilledRectangle (&r);
}

void
OutlineShipStatus (COORD y)
{
	RECT r;

	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x8, 0x8, 0x8), 0x1F));
	r.corner.x = 0;
	r.corner.y = 1 + y;
	r.extent.width = STATUS_WIDTH;
	r.extent.height = 1;
	DrawFilledRectangle (&r);
	++r.corner.y;
	--r.extent.width;
	DrawFilledRectangle (&r);
	r.extent.width = 1;
	r.extent.height = SHIP_INFO_HEIGHT - 2;
	DrawFilledRectangle (&r);
	++r.corner.x;
	DrawFilledRectangle (&r);

	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19));
	r.corner.x = STATUS_WIDTH - 1;
	DrawFilledRectangle (&r);
	r.corner.x = STATUS_WIDTH - 2;
	++r.corner.y;
	--r.extent.height;
	DrawFilledRectangle (&r);

	SetContextForeGroundColor (BLACK_COLOR);
	r.corner.x = 0;
	r.corner.y = y;
	r.extent.width = STATUS_WIDTH;
	r.extent.height = 1;
	DrawFilledRectangle (&r);
}

void
InitShipStatus (STARSHIPPTR StarShipPtr, PRECT pClipRect)
{
	RECT r;
	COORD y, y_stat;
	STAMP Stamp;
	CONTEXT OldContext;
	SHIP_INFOPTR SIPtr;

	SIPtr = &StarShipPtr->RaceDescPtr->ship_info;
	y_stat = (SIPtr->ship_flags & GOOD_GUY) ?
			GOOD_GUY_YOFFS : BAD_GUY_YOFFS;

	OldContext = SetContext (StatusContext);
	y = y_stat;
	if (pClipRect)
	{
		GetContextClipRect (&r);
		r.corner.x += pClipRect->corner.x;
		r.corner.y += (pClipRect->corner.y & ~1);
		r.extent = pClipRect->extent;
		r.extent.height += pClipRect->corner.y & 1;
		SetContextClipRect (&r);
		SetFrameHot (Screen, MAKE_HOT_SPOT (
				pClipRect->corner.x,
				(pClipRect->corner.y & ~1)
				));
	}

	BatchGraphics ();
	
	OutlineShipStatus (y);
	ClearShipStatus (y);

	Stamp.origin.x = (STATUS_WIDTH >> 1);
	Stamp.origin.y = 31 + y;
	Stamp.frame = IncFrameIndex (SIPtr->icons);
	DrawStamp (&Stamp);

	{
		SIZE crew_height, energy_height;

		crew_height = (((SIPtr->max_crew + 1) >> 1) << 1) + 1;
		energy_height = (((SIPtr->max_energy + 1) >> 1) << 1) + 1;

		SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x8, 0x8, 0x8), 0x1F));
		r.corner.x = CREW_XOFFS - 1;
		r.corner.y = GAUGE_YOFFS + 1 + y;
		r.extent.width = STAT_WIDTH + 2;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = ENERGY_XOFFS - 1;
		DrawFilledRectangle (&r);
		r.corner.x = ENERGY_XOFFS + STAT_WIDTH;
		r.corner.y -= energy_height;
		r.extent.width = 1;
		r.extent.height = energy_height;
		DrawFilledRectangle (&r);
		r.corner.x = CREW_XOFFS + STAT_WIDTH;
		r.corner.y = (GAUGE_YOFFS + 1 + y) - crew_height;
		r.extent.width = 1;
		r.extent.height = crew_height;
		DrawFilledRectangle (&r);
		SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19));
		r.corner.x = CREW_XOFFS - 1;
		r.corner.y = GAUGE_YOFFS - crew_height + y;
		r.extent.width = STAT_WIDTH + 2;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = ENERGY_XOFFS - 1;
		r.corner.y = GAUGE_YOFFS - energy_height + y;
		DrawFilledRectangle (&r);
		r.extent.width = 1;
		r.extent.height = energy_height + 1;
		DrawFilledRectangle (&r);
		r.corner.x = CREW_XOFFS - 1;
		r.corner.y = GAUGE_YOFFS - crew_height + y;
		r.extent.height = crew_height + 1;
		DrawFilledRectangle (&r);

		SetContextForeGroundColor (BLACK_COLOR);

		r.extent.width = STAT_WIDTH;
		r.corner.x = CREW_XOFFS;
		r.extent.height = crew_height;
		r.corner.y = y - r.extent.height + GAUGE_YOFFS + 1;
		DrawFilledRectangle (&r);
		r.corner.x = ENERGY_XOFFS;
		r.extent.height = energy_height;
		r.corner.y = y - r.extent.height + GAUGE_YOFFS + 1;
		DrawFilledRectangle (&r);
	}

	if (StarShipPtr->captains_name_index
			|| LOBYTE (GLOBAL (CurrentActivity)) == SUPER_MELEE)
	{
		STRING locString;

		DrawCrewFuelString (y, 0);

		locString = SetAbsStringTableIndex (SIPtr->race_strings, 1);
		DrawShipNameString (
				(UNICODE *)GetStringAddress (locString),
				GetStringLength (locString),
				y
				);

		{
			UNICODE buf[3];
			TEXT Text;
			FONT OldFont;

			OldFont = SetContextFont (TinyFont);

			if (!(GLOBAL (CurrentActivity) & IN_BATTLE))
			{
				wsprintf (buf, "%d", SIPtr->ship_cost);
				Text.pStr = buf;
				Text.CharCount = (COUNT)~0;
			}
			else
			{
				locString = SetAbsStringTableIndex (SIPtr->race_strings,
						StarShipPtr->captains_name_index);
				Text.pStr = (UNICODE *)GetStringAddress (locString);
				Text.CharCount = GetStringLength (locString);
			}
			Text.align = ALIGN_CENTER;

			Text.baseline.x = STATUS_WIDTH >> 1;
			Text.baseline.y = y + GAUGE_YOFFS + 9;

			SetContextForeGroundColor (BLACK_COLOR);
			DrawText (&Text);

			SetContextFont (OldFont);
		}
	}
	else if (StarShipPtr->captains_name_index == 0)
	{
		if (SIPtr->ship_flags & GOOD_GUY)
		{
			DrawCrewFuelString (y, 0);
			DrawShipNameString (GLOBAL_SIS (ShipName), (COUNT)~0, y);
		}
	}

	{
		SIZE crew_delta, energy_delta;

		crew_delta = SIPtr->crew_level;
		energy_delta = SIPtr->energy_level;
		SIPtr->crew_level = SIPtr->energy_level = 0;
		DeltaStatistics (StarShipPtr, crew_delta, energy_delta);
	}

	UnbatchGraphics ();

	if (pClipRect)
	{
		SetFrameHot (Screen, MAKE_HOT_SPOT (0, 0));
		r.corner.x = SPACE_WIDTH + SAFE_X;
		r.corner.y = SAFE_Y;
		r.extent.width = STATUS_WIDTH;
		r.extent.height = STATUS_HEIGHT;
		SetContextClipRect (&r);
	}

	SetContext (OldContext);
}

void
DeltaStatistics (STARSHIPPTR StarShipPtr, SIZE crew_delta, SIZE
		energy_delta)
{
	COORD x, y;
	RECT r;
	SHIP_INFOPTR ShipInfoPtr;

	if (crew_delta == 0 && energy_delta == 0)
		return;

	ShipInfoPtr = &StarShipPtr->RaceDescPtr->ship_info;

	x = 0;
	y = GAUGE_YOFFS
			+ ((ShipInfoPtr->ship_flags & GOOD_GUY) ?
			GOOD_GUY_YOFFS : BAD_GUY_YOFFS);

	r.extent.width = UNIT_WIDTH;
	r.extent.height = UNIT_HEIGHT;
	if (crew_delta != 0)
	{
		if (crew_delta > 0)
		{
			r.corner.y = (y + 1) -
					(((ShipInfoPtr->crew_level + 1) >> 1) * (UNIT_HEIGHT + 1));
			if (StarShipPtr->captains_name_index == 0
					&& (ShipInfoPtr->ship_flags & GOOD_GUY)
					&& LOBYTE (GLOBAL (CurrentActivity)) != SUPER_MELEE)
			{
				if ((crew_delta =
						GLOBAL_SIS (CrewEnlisted)) >= MAX_CREW_SIZE)
					crew_delta = MAX_CREW_SIZE;
				else /* if (crew_delta < MAX_CREW_SIZE) */
				{
#define PLAYER_UNIT_COLOR BUILD_COLOR (MAKE_RGB15 (0xA, 0xA, 0x1F), 0x09)
					SetContextForeGroundColor (PLAYER_UNIT_COLOR);
					r.corner.x = x + (CREW_XOFFS + 1);
					if (!(ShipInfoPtr->crew_level & 1))
					{
						r.corner.x += UNIT_WIDTH + 1;
						r.corner.y -= UNIT_HEIGHT + 1;
					}
					DrawFilledRectangle (&r);
					++ShipInfoPtr->crew_level;
				}
			}

#define CREW_UNIT_COLOR BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x00), 0x02)
#define ROBOT_UNIT_COLOR BUILD_COLOR (MAKE_RGB15 (0xA, 0xA, 0xA), 0x08)
			SetContextForeGroundColor (
					(ShipInfoPtr->ship_flags & CREW_IMMUNE) ?
					ROBOT_UNIT_COLOR : CREW_UNIT_COLOR
					);
			while (crew_delta--)
			{
				r.corner.x = x + (CREW_XOFFS + 1);
				if (!(ShipInfoPtr->crew_level & 1))
				{
					r.corner.x += UNIT_WIDTH + 1;
					r.corner.y -= UNIT_HEIGHT + 1;
				}
				DrawFilledRectangle (&r);
				++ShipInfoPtr->crew_level;
			}
		}
		else
		{
			if (StarShipPtr->captains_name_index == 0
					&& (ShipInfoPtr->ship_flags & GOOD_GUY)
					&& LOBYTE (GLOBAL (CurrentActivity)) != SUPER_MELEE)
			{
				COUNT death_count;

				death_count = (COUNT)-crew_delta;
				if (GLOBAL_SIS (CrewEnlisted) >= death_count)
				{
					GLOBAL_SIS (CrewEnlisted) -= death_count;
					death_count = 0;
				}
				else
				{
					death_count -= GLOBAL_SIS (CrewEnlisted);
					GLOBAL_SIS (CrewEnlisted) = 0;
				}

				DrawBattleCrewAmount ((BOOLEAN)(death_count == 0));
			}

			SetContextForeGroundColor (BLACK_COLOR);
			r.corner.y = (y + 1) -
					(((ShipInfoPtr->crew_level + 2) >> 1) * (UNIT_HEIGHT + 1));
			do
			{
				r.corner.x = x + (CREW_XOFFS + 1 + UNIT_WIDTH + 1);
				if (!(ShipInfoPtr->crew_level & 1))
				{
					r.corner.x -= UNIT_WIDTH + 1;
					r.corner.y += UNIT_HEIGHT + 1;
				}
				DrawFilledRectangle (&r);
				--ShipInfoPtr->crew_level;
			} while (++crew_delta);
		}
	}

	if (energy_delta != 0)
	{
		if (energy_delta > 0)
		{
#define FUEL_UNIT_COLOR BUILD_COLOR (MAKE_RGB15 (0x14, 0x00, 0x00), 0x04)
			SetContextForeGroundColor (FUEL_UNIT_COLOR);
			r.corner.y = (y + 1) -
					(((ShipInfoPtr->energy_level + 1) >> 1) * (UNIT_HEIGHT + 1));
			do
			{
				r.corner.x = x + (ENERGY_XOFFS + 1);
				if (!(ShipInfoPtr->energy_level & 1))
				{
					r.corner.x += UNIT_WIDTH + 1;
					r.corner.y -= UNIT_HEIGHT + 1;
				}
				DrawFilledRectangle (&r);
				++ShipInfoPtr->energy_level;
			} while (--energy_delta);
		}
		else
		{
			SetContextForeGroundColor (BLACK_COLOR);
			r.corner.y = (y + 1) -
					(((ShipInfoPtr->energy_level + 2) >> 1) * (UNIT_HEIGHT + 1));
			do
			{
				r.corner.x = x + (ENERGY_XOFFS + 1 + UNIT_WIDTH + 1);
				if (!(ShipInfoPtr->energy_level & 1))
				{
					r.corner.x -= UNIT_WIDTH + 1;
					r.corner.y += UNIT_HEIGHT + 1;
				}
				DrawFilledRectangle (&r);
				--ShipInfoPtr->energy_level;
			} while (++energy_delta);
		}
	}
}


