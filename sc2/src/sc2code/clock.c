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

#define IsLeapYear(yi) (!((yi) & 3) && (((yi) % 100) || ((yi) % 400)))

int clock_task_func(void* data)
{
	BOOLEAN LastPilot;
	DWORD LastTime;
	BYTE days_in_month[12] =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	};
	COUNT cycle_index, delay_count;
	COLOR cycle_tab[] =
	{
		BUILD_COLOR (MAKE_RGB15 (0xA, 0x14, 0x18), 0x5B),
		BUILD_COLOR (MAKE_RGB15 (0x6, 0x10, 0x16), 0x5C),
		BUILD_COLOR (MAKE_RGB15 (0x3, 0xE, 0x14), 0x5D),
		BUILD_COLOR (MAKE_RGB15 (0x2, 0xC, 0x11), 0x5E),
		BUILD_COLOR (MAKE_RGB15 (0x1, 0xB, 0xF), 0x5F),
		BUILD_COLOR (MAKE_RGB15 (0x1, 0x9, 0xD), 0x60),
		BUILD_COLOR (MAKE_RGB15 (0x0, 0x7, 0xB), 0x61),
	};
#define NUM_CYCLES (sizeof (cycle_tab) / sizeof (cycle_tab[0]))
#define NUM_DELAYS 9
	Task task = (Task) data;

	LastPilot = FALSE;
	LastTime = 0;
	cycle_index = delay_count = 0;
	while (GLOBAL (GameClock).day_in_ticks == 0 && !Task_ReadState (task, TASK_EXIT))
		TaskSwitch ();

	while (!Task_ReadState (task, TASK_EXIT))
	{
		BOOLEAN OnAutoPilot;
		DWORD TimeIn;

				/* use semaphore so that time passage
				 * can be halted. (e.g. during battle
				 * or communications)
				 */
		SetSemaphore (GLOBAL (GameClock).clock_sem);
		TimeIn = GetTimeCounter ();

		if (GLOBAL (GameClock).tick_count <= 0
				&& (GLOBAL (GameClock).tick_count = GLOBAL (GameClock).day_in_ticks) > 0)
		{			
			if (GLOBAL (GameClock).month_index == 2)
			{
				if (IsLeapYear (GLOBAL (GameClock).year_index))
					days_in_month[1] = 29; /* leap year */
				else
					days_in_month[1] = 28;
			}

			if (++GLOBAL (GameClock).day_index > days_in_month[GLOBAL (GameClock).month_index - 1])
			{
				GLOBAL (GameClock).day_index = 1;
				if (++GLOBAL (GameClock).month_index > 12)
				{
					GLOBAL (GameClock).month_index = 1;
					++GLOBAL (GameClock).year_index;
				}
			}

			SetSemaphore (GraphicsSem);
			DrawStatusMessage (NULL_PTR);
			{
				HEVENT hEvent;

				while ((hEvent = GetHeadEvent ()))
				{
					EVENTPTR EventPtr;

					LockEvent (hEvent, &EventPtr);

					if (GLOBAL (GameClock).day_index != EventPtr->day_index
							|| GLOBAL (GameClock).month_index != EventPtr->month_index
							|| GLOBAL (GameClock).year_index != EventPtr->year_index)
					{
						UnlockEvent (hEvent);
						break;
					}
					RemoveEvent (hEvent);
					EventHandler (EventPtr->func_index);

					UnlockEvent (hEvent);
					FreeEvent (hEvent);
				}
			}
			ClearSemaphore (GraphicsSem);
		}

		OnAutoPilot = (BOOLEAN)(
				(GLOBAL (autopilot.x) != ~0
				&& GLOBAL (autopilot.y) != ~0)
				|| GLOBAL_SIS (FuelOnBoard) == 0
				);
		if (OnAutoPilot || OnAutoPilot != LastPilot)
		{
			COUNT num_ticks;

			SetSemaphore (GraphicsSem);
			num_ticks = GetTimeCounter () - LastTime;
			if (!OnAutoPilot)
			{
				DrawSISMessage (NULL_PTR);
				cycle_index = delay_count = 0;
			}
			else if (delay_count > num_ticks)
			{
				delay_count -= num_ticks;
			}
			else
			{
				if (!(GLOBAL (CurrentActivity) & CHECK_ABORT)
						&& GLOBAL_SIS (CrewEnlisted) != (COUNT)~0)
				{
					CONTEXT OldContext;

					OldContext = SetContext (OffScreenContext);
					SetContextForeGroundColor (cycle_tab[cycle_index]);
					DrawSISMessage ((UNICODE *)~0L);
					SetContext (OldContext);
				}

				cycle_index = (cycle_index + 1) % NUM_CYCLES;
				delay_count = NUM_DELAYS;
			}
			ClearSemaphore (GraphicsSem);

			LastPilot = OnAutoPilot;
			LastTime += num_ticks;
		}
		
		ClearSemaphore (GLOBAL (GameClock).clock_sem);
		SleepThreadUntil (TimeIn + 1);
	}
	FinishTask (task);
	return(0);
}

BOOLEAN
InitGameClock (void)
{
	if (!InitQueue (&GLOBAL (GameClock.event_q), NUM_EVENTS, sizeof (EVENT)))
		return (FALSE);

	GLOBAL (GameClock.month_index) = 2;
	GLOBAL (GameClock.day_index) = 17;
	GLOBAL (GameClock.year_index) = START_YEAR; /* Feb 17, START_YEAR */
	GLOBAL (GameClock).tick_count = GLOBAL (GameClock).day_in_ticks = 0;
	SuspendGameClock ();
	if ((GLOBAL (GameClock.clock_task) =
			AssignTask (clock_task_func, 2048,
			"game clock")) == 0)
		return (FALSE);

	return (TRUE);
}

BOOLEAN
UninitGameClock (void)
{
	if (GLOBAL (GameClock.clock_task))
	{
		if (!GameClockRunning ())
			ResumeGameClock ();

		ConcludeTask (GLOBAL (GameClock.clock_task));
		
		GLOBAL (GameClock.clock_task) = 0;
	}

	UninitQueue (&GLOBAL (GameClock.event_q));

	return (TRUE);
}

void
SuspendGameClock (void)
{
	if (GameClockRunning ())
	{
		SetSemaphore (GLOBAL (GameClock.clock_sem));
		GLOBAL (GameClock.TimeCounter) = 0;
	}
}

void
ResumeGameClock (void)
{
	if (!GameClockRunning ())
	{
		GLOBAL (GameClock.TimeCounter) = GetTimeCounter ();
		ClearSemaphore (GLOBAL (GameClock.clock_sem));
	}
}

BOOLEAN
GameClockRunning (void)
{
	return ((BOOLEAN)(GLOBAL (GameClock.TimeCounter) != 0));
}

void
SetGameClockRate (COUNT seconds_per_day)
{
	SIZE new_day_in_ticks, new_tick_count;

//if (GLOBAL (GameClock.clock_sem)) fprintf (stderr, "%u\n", GLOBAL (GameClock.clock_sem));
	SetSemaphore (GLOBAL (GameClock.clock_sem));
	new_day_in_ticks = (SIZE)(seconds_per_day * (ONE_SECOND / 5));
	if (GLOBAL (GameClock.day_in_ticks) == 0)
		new_tick_count = new_day_in_ticks;
	else if (GLOBAL (GameClock.tick_count) <= 0)
		new_tick_count = 0;
	else if ((new_tick_count = (SIZE)((DWORD)GLOBAL (GameClock.tick_count)
			* new_day_in_ticks / GLOBAL (GameClock.day_in_ticks))) == 0)
		new_tick_count = 1;
	GLOBAL (GameClock.day_in_ticks) = new_day_in_ticks;
	GLOBAL (GameClock.tick_count) = new_tick_count;
	ClearSemaphore (GLOBAL (GameClock.clock_sem));
}

BOOLEAN
ValidateEvent (EVENT_TYPE type, PCOUNT pmonth_index, PCOUNT pday_index,
		PCOUNT pyear_index)
{
	COUNT month_index, day_index, year_index;

	month_index = *pmonth_index;
	day_index = *pday_index;
	year_index = *pyear_index;
	if (type == RELATIVE_EVENT)
	{
		BYTE days_in_month[12] =
		{
			31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
		};

		month_index += GLOBAL (GameClock.month_index) - 1;
		year_index += GLOBAL (GameClock.year_index) + (month_index / 12);
		month_index = (month_index % 12) + 1;
		if (IsLeapYear (year_index))
			days_in_month[1] = 29; /* leap year */

		day_index += GLOBAL (GameClock.day_index);
		while (day_index > days_in_month[month_index - 1])
		{
			day_index -= days_in_month[month_index - 1];
			if (++month_index > 12)
			{
				month_index = 1;
				++year_index;
				if (IsLeapYear (year_index))
					days_in_month[1] = 29; /* leap year */
				else
					days_in_month[1] = 28;
			}
		}

		*pmonth_index = month_index;
		*pday_index = day_index;
		*pyear_index = year_index;
	}

	return (!(year_index < GLOBAL (GameClock.year_index)
			|| (year_index == GLOBAL (GameClock.year_index)
			&& (month_index < GLOBAL (GameClock.month_index)
			|| (month_index == GLOBAL (GameClock.month_index)
			&& day_index < GLOBAL (GameClock.day_index))))));
}

HEVENT
AddEvent (EVENT_TYPE type, COUNT month_index, COUNT day_index, COUNT
		year_index, BYTE func_index)
{
	HEVENT hNewEvent;

	if (type == RELATIVE_EVENT
			&& month_index == 0
			&& day_index == 0
			&& year_index == 0)
		EventHandler (func_index);
	else if (ValidateEvent (type, &month_index, &day_index, &year_index)
			&& (hNewEvent = AllocEvent ()))
	{
		EVENTPTR EventPtr;

		LockEvent (hNewEvent, &EventPtr);
		EventPtr->day_index = (BYTE)day_index;
		EventPtr->month_index = (BYTE)month_index;
		EventPtr->year_index = year_index;
		EventPtr->func_index = func_index;
		UnlockEvent (hNewEvent);

		{
			HEVENT hEvent, hSuccEvent;
			for (hEvent = GetHeadEvent (); hEvent != 0; hEvent = hSuccEvent)
			{
				LockEvent (hEvent, &EventPtr);
				if (year_index < EventPtr->year_index
						|| (year_index == EventPtr->year_index
						&& (month_index < EventPtr->month_index
						|| (month_index == EventPtr->month_index
						&& day_index < EventPtr->day_index))))
				{
					UnlockEvent (hEvent);
					break;
				}

				hSuccEvent = GetSuccEvent (EventPtr);
				UnlockEvent (hEvent);
			}
			
			InsertEvent (hNewEvent, hEvent);
		}

		return (hNewEvent);
	}

	return (0);
}

SIZE
ClockTick (void)
{
	return (--GLOBAL (GameClock.tick_count));
}

