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

#include <ctype.h>
#include "starcon.h"

static UWORD cur_char = ' ';

void
SetJoystickChar (UWORD which_char)
{
	if (!isalnum (cur_char = which_char))
		cur_char = ' ';
}

UWORD
GetJoystickChar (INPUT_STATE InputState)
{
	int dy;
	UWORD old_char;

	if (InputState & DEVICE_BUTTON1)
		return ('\n');
	else if (InputState & DEVICE_BUTTON2)
		return (0x1B);
	else if (InputState & DEVICE_BUTTON3)
		return (0x7F);
		
	old_char = cur_char;
	dy = GetInputYComponent (InputState);
	if (dy)
	{
		if (cur_char == ' ')
		{
			if (dy > 0)
				cur_char = 'a';
			else /* if (dy < 0) */
				cur_char = '9';
		}
		else if (isdigit (cur_char))
		{
			cur_char += dy;
			if (cur_char < '0')
				cur_char = 'z';
			else if (cur_char > '9')
				cur_char = ' ';
		}
		else if (!isalpha (cur_char += dy))
		{
			cur_char -= dy;
			if (cur_char == 'a' || cur_char == 'A')
				cur_char = ' ';
			else
				cur_char = '0';
		}
	}
	
	if ((InputState & (DEVICE_LEFTSHIFT | DEVICE_RIGHTSHIFT)) && isalpha (cur_char))
	{
		if (islower (cur_char))
			cur_char = toupper (cur_char);
		else
			cur_char = tolower (cur_char);
	}
	
	return (cur_char == old_char ? 0 : cur_char);
}
