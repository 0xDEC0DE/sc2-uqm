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

static BYTE char_delta_array[MAX_DELTAS];

FONT
SetContextFont (FONT Font)
{
	FONT LastFont;

	LastFont = (FONT)_CurFontPtr;
	_CurFontPtr = (FONTPTR)Font;
	if (ContextActive ())
		SwitchContextFont (Font);

	return (LastFont);
}

BOOLEAN
DestroyFont (FONT_REF FontRef)
{
	if (FontRef == 0)
		return (FALSE);

	if (_CurFontPtr && _CurFontPtr->FontRef == FontRef)
		SetContextFont ((FONT)0);

	return (FreeFont (FontRef));
}

FONT
CaptureFont (FONT_REF FontRef)
{
	FONTPTR FontPtr;

	FontPtr = LockFont (FontRef);
	if (FontPtr)
		FontPtr->FontRef = FontRef;

	return ((FONT)FontPtr);
}

FONT_REF
ReleaseFont (FONT Font)
{
	FONTPTR FontPtr;

	FontPtr = (FONTPTR)Font;
	if (FontPtr)
	{
		FONT_REF FontRef;

		FontRef = FontPtr->FontRef;
		UnlockFont (FontRef);

		return (FontRef);
	}

	return (0);
}

void
DrawText (PTEXT lpText)
{
	SetPrimType (&_locPrim, TEXT_PRIM);
	_locPrim.Object.Text = *lpText;
	DrawBatch (&_locPrim, 0, BATCH_SINGLE);
}

BOOLEAN
GetContextFontLeading (PSIZE pheight)
{
	if (_CurFontPtr != 0)
	{
		*pheight = (SIZE)_CurFontPtr->Leading;
		return (TRUE);
	}

	*pheight = 0;
	return (FALSE);
}

BOOLEAN
TextRect (PTEXT lpText, register PRECT pRect, PBYTE pdelta)
{
	COUNT num_chars;
	FONTPTR FontPtr;

	if ((FontPtr = _CurFontPtr) != 0 && (num_chars = lpText->CharCount))
	{
		COORD top_y, bot_y;
		SIZE width;
		UNICODE next_ch, *pStr;

		if (pdelta == 0)
		{
			pdelta = char_delta_array;
			if (num_chars > MAX_DELTAS)
				num_chars = MAX_DELTAS;
		}

		top_y = bot_y = 0;
		width = 0;
		pStr = lpText->pStr;
		if ((next_ch = *pStr++) == '\0')
			num_chars = 0;
		next_ch -= FIRST_CHAR;
		while (num_chars--)
		{
			UNICODE ch;
			SIZE last_width;

				last_width = width;

			ch = next_ch;
			if ((next_ch = *pStr++) == '\0')
				num_chars = 0;
			next_ch -= FIRST_CHAR;
			if (ch < MAX_CHARS && GetFrameWidth (&FontPtr->CharDesc[ch]))
			{
				register COORD y;

				y = -GetFrameHotY (&FontPtr->CharDesc[ch]);
				if (y < top_y)
					top_y = y;
				y += GetFrameHeight (&FontPtr->CharDesc[ch]);
				if (y > bot_y)
					bot_y = y;

				width += GetFrameWidth (&FontPtr->CharDesc[ch]);
#if 0
				if (num_chars && next_ch < MAX_CHARS
						&& !(FontPtr->KernTab[ch]
						& (FontPtr->KernTab[next_ch] >> 2)))
					width -= FontPtr->KernAmount;
#endif
			}

			*pdelta++ = (BYTE)(width - last_width);
		}

		lpText->CharCount = pStr - lpText->pStr - 1;

		if (width > 0 && (bot_y -= top_y) > 0)
		{
			/* subtract off default character spacing */
			--pdelta[-1];
			--width;

			if (lpText->align == ALIGN_LEFT)
				pRect->corner.x = 0;
			else if (lpText->align == ALIGN_CENTER)
				pRect->corner.x = -(width >> 1);
			else
				pRect->corner.x = -width;
			pRect->corner.y = top_y;
			pRect->extent.width = width;
			pRect->extent.height = bot_y;

			pRect->corner.x += lpText->baseline.x;
			pRect->corner.y += lpText->baseline.y;

			return (TRUE);
		}
	}

	pRect->corner = lpText->baseline;
	pRect->extent.width = 0;
	pRect->extent.height = 0;

	return (FALSE);
}

void
_text_blt (PRECT pClipRect, PRIMITIVEPTR PrimPtr)
{
	FONTPTR FontPtr;

	if ((FontPtr = _CurFontPtr) != 0)
	{
		COUNT num_chars;
		UNICODE next_ch, *pStr;
		TEXTPTR TextPtr;
		PRIMITIVE locPrim;

		SetPrimType (&locPrim, STAMPFILL_PRIM);
		SetPrimColor (&locPrim, _get_context_fg_color ());

		TextPtr = &PrimPtr->Object.Text;
		locPrim.Object.Stamp.origin.x = _save_stamp.origin.x;
		locPrim.Object.Stamp.origin.y = TextPtr->baseline.y;
		num_chars = TextPtr->CharCount;

		pStr = TextPtr->pStr;
		if ((next_ch = *pStr++) == '\0')
			num_chars = 0;
		next_ch -= FIRST_CHAR;
		while (num_chars--)
		{
			UNICODE ch;

			ch = next_ch;
			if ((next_ch = *pStr++) == '\0')
				num_chars = 0;
			next_ch -= FIRST_CHAR;
			locPrim.Object.Stamp.frame = &FontPtr->CharDesc[ch];
			if (ch < MAX_CHARS && GetFrameWidth (locPrim.Object.Stamp.frame))
			{
				RECT r;

				r.corner.x = locPrim.Object.Stamp.origin.x
						- GetFrameHotX (locPrim.Object.Stamp.frame);
				r.corner.y = locPrim.Object.Stamp.origin.y
						- GetFrameHotY (locPrim.Object.Stamp.frame);
				r.extent.width = GetFrameWidth (locPrim.Object.Stamp.frame);
				r.extent.height = GetFrameHeight (locPrim.Object.Stamp.frame);
				_save_stamp.origin = r.corner;
				if (BoxIntersect (&r, pClipRect, &r))
					DrawGraphicsFunc (&r, &locPrim);

				locPrim.Object.Stamp.origin.x += GetFrameWidth (locPrim.Object.Stamp.frame);
#if 0
				if (num_chars && next_ch < MAX_CHARS
						&& !(FontPtr->KernTab[ch]
						& (FontPtr->KernTab[next_ch] >> 2)))
					locPrim.Object.Stamp.origin.x -= FontPtr->KernAmount;
#endif
			}
		}
	}
}
