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

#include "libs/graphics/gfxintrn.h"
#include "libs/input/inpintrn.h"
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/drawcmd.h"

DISPLAY_INTERFACE *_pCurDisplay; //Not a function. Probably has to be initialized...

void (*mask_func_array[]) (RECT *pClipRect, PRIMITIVE *PrimPtr)
		= { 0 };

int ScreenWidth;
int ScreenHeight;
int ScreenWidthActual;
int ScreenHeightActual;
int ScreenColorDepth;
int GraphicsDriver;
int TFB_DEBUG_HALT = 0;

static int gscale = GSCALE_IDENTITY;
static int gscale_mode = TFB_SCALE_NEAREST;

// Status: Ignored (only used in fmv.c)
void
SetGraphicUseOtherExtra (int other) //Could this possibly be more cryptic?!? :)
{
	//log_add (log_Debug, "SetGraphicUseOtherExtra %d", other);
	(void)other; /* lint */
}

// Status: Ignored (only used in solarsys.c)
void
SetGraphicGrabOther (int grab_other)
{
	//log_add (log_Debug, "SetGraphicGrabOther %d", grab_other);
	(void)grab_other; /* lint */
}

void
DrawFromExtraScreen (RECT *r)
{
	TFB_DrawScreen_Copy(r, TFB_SCREEN_EXTRA, TFB_SCREEN_MAIN);
}

void
LoadIntoExtraScreen (RECT *r)
{
	TFB_DrawScreen_Copy(r, TFB_SCREEN_MAIN, TFB_SCREEN_EXTRA);
}

int
SetGraphicScale (int scale)
{
	int old_scale = gscale;
	gscale = (scale ? scale : GSCALE_IDENTITY);
	return old_scale;
}

int
GetGraphicScale (void)
{
	return gscale;
}

int
SetGraphicScaleMode (int mode)
{
	int old_mode = gscale_mode;
	assert (mode >= TFB_SCALE_NEAREST && mode <= TFB_SCALE_TRILINEAR);
	gscale_mode = mode;
	return old_mode;
}

int
GetGraphicScaleMode (void)
{
	return gscale_mode;
}
