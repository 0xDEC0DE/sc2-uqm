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

#include "fmv.h"

#include "controls.h"
#include "hyper.h"
#include "options.h"
#include "master.h"
#include "resinst.h"
#include "nameref.h"
#include "settings.h"
#include "setup.h"
#include "libs/vidlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/inplib.h"

void
DoShipSpin (COUNT index, MUSIC_REF hMusic)
{
#ifdef WANT_SHIP_SPINS
	char vnbuf[32];
	BYTE clut_buf[1];
	RECT old_r, r;

	LoadIntoExtraScreen (0);
#if 0
	/* This is cut out right now but should be part of the 3DO side */
	clut_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)clut_buf, ONE_SECOND / 4));
	FlushColorXForms ();
#endif
	
	if (hMusic)
		StopMusic ();

	FreeHyperData ();

	// TODO: It would be nice to have better resource names for these.
	sprintf (vnbuf, "slides.spins.%02u", (unsigned)index);
	ShowPresentation (vnbuf);

	clut_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)clut_buf, ONE_SECOND / 4));
	FlushColorXForms ();

	GetContextClipRect (&old_r);
	r.corner.x = r.corner.y = 0;
	r.extent.width = SCREEN_WIDTH;
	r.extent.height = SCREEN_HEIGHT;
	SetContextClipRect (&r);
	DrawFromExtraScreen (0);
	SetContextClipRect (&old_r);

	if (hMusic)
		PlayMusic (hMusic, TRUE, 1);
		
	clut_buf[0] = FadeAllToColor;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)clut_buf, ONE_SECOND / 4));
	FlushColorXForms ();
#else
	(void) index;  /* Satisfy compiler */
	(void) hMusic;  /* Satisfy compiler */
#endif  /* WANT_SHIP_SPINS */
}

void
SplashScreen (void (* DoProcessing)(DWORD TimeOut))
{
	BYTE xform_buf[1];
	STAMP s;
	DWORD TimeOut;

	xform_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap (
			(COLORMAPPTR) xform_buf, ONE_SECOND / 120));
	LockMutex (GraphicsLock);
	SetContext (ScreenContext);
	s.origin.x = s.origin.y = 0;
	s.frame = CaptureDrawable (LoadGraphic (TITLE_ANIM));
	DrawStamp (&s);
	DestroyDrawable (ReleaseDrawable (s.frame));
	UnlockMutex (GraphicsLock);

	xform_buf[0] = FadeAllToColor;
	TimeOut = XFormColorMap ((COLORMAPPTR)xform_buf, ONE_SECOND / 2);

	if (DoProcessing)
		DoProcessing (TimeOut);
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		return;
	}
	
	/* There was a forcible setting of CHECK_ABORT here.  I cannot
	 * find any purpose for this that DoRestart doesn't handle
	 * better (forcing all other threads but this one to quit out,
	 * I believe), and have thus removed it.  It was interfering
	 * with the proper operation of the quit operation.
	 * --Michael */

	WaitForAnyButton (FALSE, ONE_SECOND * 3, TRUE);
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		return;
	}
	GLOBAL (CurrentActivity) &= ~CHECK_ABORT;

	xform_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)xform_buf, ONE_SECOND / 2));
}

void
Introduction (void)
{
	BYTE xform_buf[1];

	ShowPresentation (INTROPRES_STRTAB);

	xform_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)xform_buf, ONE_SECOND / 2));
}

void
Victory (void)
{
	BYTE xform_buf[1];

	xform_buf[0] = FadeAllToBlack;
	SleepThreadUntil (XFormColorMap ((COLORMAPPTR)xform_buf, ONE_SECOND / 2));

	/* by default we do 3DO cinematics; or PC slides when 3DO files are
	 * not present */
	ShowPresentation (FINALPRES_STRTAB);
		
	xform_buf[0] = FadeAllToBlack;
	XFormColorMap ((COLORMAPPTR)xform_buf, 0);
}

void
Logo (void)
{
	ShowPresentation ("slides.logo");
}




