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

#include "gfxintrn.h"

GRAPHICS_STATUS _GraphicsStatusFlags;
CONTEXTPTR _pCurContext;

PRIMITIVE _locPrim;

FONTPTR _CurFontPtr;

CONTEXT
SetContext (CONTEXT Context)
{
	CONTEXT LastContext;

	if (Context != (LastContext = (CONTEXT)_pCurContext))
	{
		if (LastContext)
		{
			UnsetContextFlags (
					MAKE_WORD (0, GRAPHICS_ACTIVE | DRAWABLE_ACTIVE)
					);
			SetContextFlags (
					MAKE_WORD (0, _GraphicsStatusFlags
							& (GRAPHICS_ACTIVE | DRAWABLE_ACTIVE))
					);

			DeactivateContext ();
		}

		_pCurContext = (CONTEXTPTR)Context;
		if (_pCurContext)
		{
			ActivateContext ();

			_GraphicsStatusFlags &= ~(GRAPHICS_ACTIVE | DRAWABLE_ACTIVE);
			_GraphicsStatusFlags |= HIBYTE (_get_context_flags ());

			SetPrimColor (&_locPrim, _get_context_fg_color ());

			_CurFramePtr = (FRAMEPTR)_get_context_fg_frame ();
			BGFrame = _get_context_bg_frame ();
			_CurFontPtr = (FONTPTR)_get_context_font ();
		}
	}

	return (LastContext);
}

CONTEXT_REF
CreateContext (void)
{
	CONTEXT_REF ContextRef;

	ContextRef = AllocContext ();
	if (ContextRef)
	{
		CONTEXT OldContext;

				/* initialize context */
		OldContext = SetContext (CaptureContext (ContextRef));
		SetContextMapType (MAP_NOXFORM);
		SetContextDrawState (DEST_PIXMAP | DRAW_REPLACE);
		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F)
				);
		SetContextBackGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x00), 0x00)
				);
		SetContextClipping (TRUE);
		ReleaseContext (SetContext (OldContext));
	}

	return (ContextRef);
}

BOOLEAN
DestroyContext (CONTEXT_REF ContextRef)
{
	if (ContextRef == 0)
		return (FALSE);

	if (_pCurContext && _pCurContext->ContextRef == ContextRef)
		SetContext ((CONTEXT)0);

	return (FreeContext (ContextRef));
}

CONTEXT
CaptureContext (CONTEXT_REF ContextRef)
{
	CONTEXTPTR ContextPtr;

	ContextPtr = LockContext (ContextRef);
	if (ContextPtr)
		ContextPtr->ContextRef = ContextRef;

	return ((CONTEXT)ContextPtr);
}

CONTEXT_REF
ReleaseContext (CONTEXT Context)
{
	CONTEXTPTR ContextPtr;

	ContextPtr = (CONTEXTPTR)Context;
	if (ContextPtr)
	{
		CONTEXT_REF ContextRef;

		ContextRef = ContextPtr->ContextRef;
		UnlockContext (ContextRef);

		return (ContextRef);
	}

	return (0);
}

DRAW_STATE
GetContextDrawState (void)
{
	if (!ContextActive ())
		return (DEST_PIXMAP | DRAW_REPLACE);

	return (_get_context_draw_state ());
}

DRAW_STATE
SetContextDrawState (DRAW_STATE DrawState)
{
	DRAW_STATE oldState;

	if (!ContextActive ())
		return (DEST_PIXMAP | DRAW_REPLACE);

	if ((oldState = _get_context_draw_state ()) != DrawState)
	{
		SwitchContextDrawState (DrawState);
		SetContextGraphicsFunctions ();
	}

	return (oldState);
}

COLOR
SetContextForeGroundColor (COLOR Color)
{
	COLOR oldColor;

	if (!ContextActive ())
		return (BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F));

	if ((oldColor = _get_context_fg_color ()) != Color)
	{
		SwitchContextForeGroundColor (Color);
	}
	SetPrimColor (&_locPrim, Color);

	return (oldColor);
}

COLOR
SetContextBackGroundColor (COLOR Color)
{
	COLOR oldColor;

	if (!ContextActive ())
		return (BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x00), 0x00));

	if ((oldColor = _get_context_bg_color ()) != Color)
	{
		SwitchContextBackGroundColor (Color);
	}

	return (oldColor);
}

BG_FUNC
SetContextBGFunc (BG_FUNC BGFunc)
{
	BG_FUNC oldBGFunc;

	if (!ContextActive ())
		return (0);

	if ((oldBGFunc = _get_context_bg_func ()) != BGFunc)
	{
		SwitchContextBGFunc (BGFunc);
	}

	return (oldBGFunc);
}

BOOLEAN
SetContextClipping (BOOLEAN ClipStatus)
{
	BOOLEAN oldClipStatus;

	if (!ContextActive ())
		return (TRUE);

	oldClipStatus = (_get_context_flags () & BATCH_CLIP_GRAPHICS) != 0;
	if (ClipStatus)
	{
		SetContextFlags (BATCH_CLIP_GRAPHICS);
	}
	else
	{
		UnsetContextFlags (BATCH_CLIP_GRAPHICS);
	}

	return (oldClipStatus);
}

BOOLEAN
SetContextClipRect (PRECT lpRect)
{
	if (!ContextActive ())
		return (FALSE);

	if (lpRect)
		_pCurContext->ClipRect = *lpRect;
	else
		_pCurContext->ClipRect.extent.width = 0;

	return (TRUE);
}

BOOLEAN
GetContextClipRect (PRECT lpRect)
{
	if (!ContextActive ())
		return (FALSE);

	*lpRect = _pCurContext->ClipRect;
	return (lpRect->extent.width != 0);
}


