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

#ifndef GFXOTHER_H
#define GFXOTHER_H

#include "gfxintrn.h"
#include "tfb_draw.h"

static inline void
COLORtoPalette (DWORD c32k, TFB_Palette* color)
{
	c32k >>= 8; // shift out color index
	color->r = (UBYTE)((c32k >> (10 - (8 - 5))) & 0xF8);
	color->g = (UBYTE)((c32k >> (5 - (8 - 5))) & 0xF8);
	color->b = (UBYTE)((c32k << (8 - 5)) & 0xF8);
}

#endif /* GFXOTHER */
