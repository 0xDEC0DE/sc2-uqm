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

#ifndef SDL_COMMON_H
#define SDL_COMMON_H

#include "port.h"
#include SDL_INCLUDE(SDL.h)
#include SDL_INCLUDE(SDL_byteorder.h)
#include SDL_IMAGE_INCLUDE(SDL_image.h)

#include "libs/graphics/gfxintrn.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/input/inpintrn.h"
#include "libs/graphics/gfx_common.h"
#include "libs/input/sdl/input.h"
#include "libs/threadlib.h"

// The Graphics Backend vtable
typedef struct _tfb_graphics_backend {
	void (*preprocess) (int force_redraw, int transition_amount, int fade_amount);
	void (*postprocess) (void);
	void (*screen) (SCREEN screen, Uint8 alpha, SDL_Rect *rect);
	void (*color) (Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Rect *rect);
} TFB_GRAPHICS_BACKEND;

extern TFB_GRAPHICS_BACKEND *graphics_backend;

// constants for TFB_InitGraphics

extern SDL_Surface *SDL_Video;
extern SDL_Surface *SDL_Screen;
extern SDL_Surface *TransitionScreen;

extern SDL_Surface *SDL_Screens[TFB_GFX_NUMSCREENS];

extern SDL_Surface *format_conv_surf;

extern volatile int TransitionAmount;
extern SDL_Rect TransitionClipRect;

void ScreenOrigin (FRAME Display, COORD sx, COORD sy);
void LoadDisplay (DISPLAY_INTERFACE **pDisplay);

void TFB_SwapBuffers (int force_full_redraw);
SDL_Surface* TFB_DisplayFormatAlpha (SDL_Surface *surface);
void TFB_BlitSurface (SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst,
					  SDL_Rect *dstrect, int blend_numer, int blend_denom);

#endif
