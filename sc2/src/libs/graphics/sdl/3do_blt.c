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

#ifdef GFXMODULE_SDL

#include "sdl_common.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/log.h"

static void
read_screen (RECT *lpRect, FRAME DstFramePtr)
{
	if (_CurFramePtr->Type != SCREEN_DRAWABLE
			|| DstFramePtr->Type == SCREEN_DRAWABLE
			|| !(GetFrameParentDrawable (DstFramePtr)->Flags
				& MAPPED_TO_DISPLAY))
	{
		log_add (log_Warning, "Unimplemented function activated: "
				"read_screen()");
	}
	else
	{
		TFB_Image *img = DstFramePtr->image;
		TFB_DrawScreen_CopyToImage (img, lpRect, TFB_SCREEN_MAIN);
	}
}

static DRAWABLE
alloc_image (COUNT NumFrames, DRAWABLE_TYPE DrawableType, CREATE_FLAGS
		flags, SIZE width, SIZE height)
{
        /* dodge compiler warnings */
	(void)DrawableType;
	(void)flags;
	(void)width;
	(void)height;
	return AllocDrawable (NumFrames);
}

static DISPLAY_INTERFACE DisplayInterface =
{
	/* .DisplayFlags  = */ WANT_MASK,

	/* .DisplayDepth  = */ 16,
	/* .DisplayWidth  = */ 320,
	/* .DisplayHeight = */ 240,

	/* .alloc_iamge   = */ alloc_image,
	/* .read_display  = */ read_screen,
};

void
LoadDisplay (DISPLAY_INTERFACE **pDisplay)
{
	*pDisplay = &DisplayInterface;
}

#endif

