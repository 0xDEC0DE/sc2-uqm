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
#include "pure.h"
#include "opengl.h"

int batch_depth = 0;
static const BOOLEAN pausing_transition = TRUE; // change to FALSE for non-pausing version


//Status: Not entirely unimplemented!
BOOLEAN
InitGraphics (int argc, char* argv[], COUNT KbytesRequired)
		// Kbytes should only matter if we wanted to port Starcon2 to the
		// hand-helds...
{
	BOOLEAN ret;

	LoadDisplay (&_pCurDisplay);
	ActivateDisplay ();

	DrawCommandQueue = TFB_DrawCommandQueue_Create ();

	ret = TRUE;
	return (ret);
}

// Status: Unimplemented
void
UninitGraphics ()  // Also probably empty
{
	// HFree (DrawCommandQueue);  This is static now!

	//mem_uninit ();
}


// Batch/UnbatchGraphics: These routines appear to be used to ensure
// that the screen doesn't update with a half-drawn region.  This can
// be implemented in all sorts of ways - I've chosen to fold it into
// our DrawCommandQueue, for the most part.  These just forward to
// the TFB_ versions in dcqueue.c.  They may also make continuity_break
// redundant, but I haven't tested that yet.  --Michael
void
BatchGraphics (void)
{
	TFB_BatchGraphics ();
}

void
UnbatchGraphics (void)
{
	TFB_UnbatchGraphics ();
}

void
FlushGraphics (void)
{
	TFB_DrawCommand DrawCommand;
	TFB_BatchReset ();
	continuity_break = 1;
	DrawCommand.Type = TFB_DRAWCOMMANDTYPE_FLUSHGRAPHICS;
	DrawCommand.image = 0;
	TFB_EnqueueDrawCommand(&DrawCommand);
}

void
SkipGraphics (void)
{
	TFB_DrawCommand DrawCommand;
	DrawCommand.Type = TFB_DRAWCOMMANDTYPE_SKIPGRAPHICS;
	DrawCommand.image = 0;
	TFB_EnqueueDrawCommand(&DrawCommand);
}
// Status: Ignored (only used in fmv.c)
void
SetGraphicUseOtherExtra (int other) //Could this possibly be more cryptic?!? :)
{
	//fprintf(stderr, "SetGraphicUseOtherExtra %d\n", other);
}

// Status: Ignored (only used in solarsys.c)
void
SetGraphicGrabOther (int grab_other)
{
	//fprintf(stderr, "SetGraphicGrabOther %d\n", grab_other);
}

static int
transition_task_func (void *data)
{
	const float DURATION = (31.0f / 60.0f); // in seconds
	Uint32 last_time = 0, current_time, delta_time, add_amount;
	Task task = (Task)data;

	TransitionAmount = 0;
	last_time = SDL_GetTicks ();

	for (;;)
	{
		SleepThread (10);

		current_time = SDL_GetTicks ();
		delta_time = current_time - last_time;
		last_time = current_time;
		
		add_amount = (Uint32) ((delta_time / 1000.0f / DURATION) * 255);
		if (TransitionAmount + add_amount >= 255)
			break;

		TransitionAmount += add_amount;
	}
	
	TransitionAmount = 255;
	FinishTask (task);
	return 0;
}

// Status: Implemented
void
ScreenTransition (int TransType, PRECT pRect)
{
	Task transition_task;

	//fprintf(stderr, "ScreenTransition %d\n", TransType);

	if (TransitionAmount != 255)
		return;

	if (pRect)
	{
		TransitionClipRect.x = pRect->corner.x;
		TransitionClipRect.y = pRect->corner.y;
		TransitionClipRect.w = pRect->extent.width;
		TransitionClipRect.h = pRect->extent.height;
	}
	else
	{
		TransitionClipRect.x = TransitionClipRect.y = 0;
		TransitionClipRect.w = ScreenWidth;
		TransitionClipRect.h = ScreenHeight;
	}

	SDL_BlitSurface (SDL_Screen, &TransitionClipRect, TransitionScreen, &TransitionClipRect);

#ifdef HAVE_OPENGL
	if (GraphicsDriver == TFB_GFXDRIVER_SDL_OPENGL)
	{
		TFB_GL_UploadTransitionScreen ();
	}
#endif
	
	if (pausing_transition)
	{
		TransitionAmount = 0;
		FlushGraphics ();
	}

	transition_task = AssignTask (transition_task_func, 1024, "screen transition");

	if (pausing_transition)
	{
		WaitThread (transition_task->thread, NULL);
	}
}

void
DrawFromExtraScreen (PRECT r) //No scaling?
{
	RECT locRect;
	TFB_DrawCommand DC;

	if (!r)
	{
		locRect.corner.x = locRect.corner.y = 0;
		locRect.extent.width = SCREEN_WIDTH;
		locRect.extent.height = SCREEN_HEIGHT;
		r = &locRect;
	}

	DC.Type = TFB_DRAWCOMMANDTYPE_COPYFROMOTHERBUFFER;
	DC.x = r->corner.x;
	DC.y = r->corner.y;
	DC.w = r->extent.width;
	DC.h = r->extent.height;
	DC.image = 0;

	DC.BlendNumerator = BlendNumerator;
	DC.BlendDenominator = BlendDenominator;

	TFB_EnqueueDrawCommand (&DC);
}

void
LoadIntoExtraScreen (PRECT r)
{
	RECT locRect;
	TFB_DrawCommand DC;

	if (!r)
	{
		locRect.corner.x = locRect.corner.y = 0;
		locRect.extent.width = SCREEN_WIDTH;
		locRect.extent.height = SCREEN_HEIGHT;
		r = &locRect;
	}

// fprintf (stderr, "LoadIntoExtraScreen (%d, %d, {%d, %d})\n", r->corner.x, r->corner.y, r->extent.width, r->extent.height);
//	DC = HMalloc (sizeof (TFB_DrawCommand));

	DC.Type = TFB_DRAWCOMMANDTYPE_COPYBACKBUFFERTOOTHERBUFFER;
	DC.x = r->corner.x;
	DC.y = r->corner.y;
	DC.w = r->extent.width;
	DC.h = r->extent.height;
	DC.image = 0;

	DC.BlendNumerator = BlendNumerator;
	DC.BlendDenominator = BlendDenominator;

	TFB_EnqueueDrawCommand (&DC);
}

// Status: Ignored
BOOLEAN
InitVideo (BOOLEAN useCDROM) //useCDROM doesn't really apply to us...
{
	BOOLEAN ret;

// fprintf (stderr, "Unimplemented function activated: InitVideo()\n");
	ret = TRUE;
	return (ret);
}

// Status: Ignored
void
UninitVideo ()  //empty
{
	// fprintf (stderr, "Unimplemented function activated: UninitVideo()\n");
}

//Status: Unimplemented
void
VidStop() // At least this one makes sense.
{
		// fprintf (stderr, "Unimplemented function activated: VidStop()\n");
}

//Status: Unimplemented
MEM_HANDLE
VidPlaying()  // Easy enough.
{
		MEM_HANDLE ret;

		//fprintf (stderr, "Unimplemented function activated: VidPlaying()\n");
		ret = 0;
		return (ret);
}

//Status: Unimplemented
VIDEO_TYPE
VidPlay (VIDEO_REF VidRef, char *loopname, BOOLEAN uninit)
		// uninit? when done, I guess.
{
	VIDEO_TYPE ret;
	ret = 0;

// fprintf (stderr, "Unimplemented function activated: VidPlay()\n");
	return (ret);
}

VIDEO_REF
_init_video_file(PVOID pStr)
{
	VIDEO_REF ret;

	fprintf (stderr, "Unimplemented function activated: _init_video_file()\n");
	ret = 0;
	return (ret);
}

// Status: Unimplemented
BOOLEAN
_image_intersect (PIMAGE_BOX box1, PIMAGE_BOX box2, PRECT rect)
		// This is a biggie.
{
	BOOLEAN ret;

// fprintf (stderr, "Unimplemented function activated: _image_intersect()\n");
	ret = TRUE;
	return (ret);
}


#endif
