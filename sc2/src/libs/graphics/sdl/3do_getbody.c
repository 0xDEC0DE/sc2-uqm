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

#ifdef WIN32
#include <io.h>
#endif
#include <fcntl.h>

#include "options.h"
#include "port.h"
#include "sdl_common.h"
#include "sdluio.h"
#include "libs/file.h"
#include "libs/reslib.h"
#include "libs/log.h"
#include "libs/memlib.h"
#include "../font.h"
#include "primitives.h"


typedef struct anidata
{
	int transparent_color;
	int colormap_index;
	int hotspot_x;
	int hotspot_y;
} AniData;

extern uio_Repository *repository;
static uio_AutoMount *autoMount[] = { NULL };

static void
process_image (FRAME FramePtr, SDL_Surface *img[], AniData *ani, int cel_ct)
{
	TFB_Image *tfbimg;
	int hx, hy;

	FramePtr->Type = ROM_DRAWABLE;
	FramePtr->Index = cel_ct;

	// handle transparency cases
	if (img[cel_ct]->format->palette)
	{	// indexed color image
		if (ani[cel_ct].transparent_color >= 0)
		    SDL_SetColorKey (img[cel_ct], SDL_SRCCOLORKEY,
					ani[cel_ct].transparent_color);
	}
	else if (img[cel_ct]->format->BitsPerPixel > 8)
	{	// special transparency cases for truecolor images
		if (ani[cel_ct].transparent_color == 0)
			// make RGB=0,0,0 transparent
		    SDL_SetColorKey (img[cel_ct], SDL_SRCCOLORKEY,
					SDL_MapRGBA (img[cel_ct]->format, 0, 0, 0, 0));
	}
	if (ani[cel_ct].transparent_color == -1)
	{	// enforce -1 to mean 'no transparency'
		SDL_SetColorKey (img[cel_ct], 0, 0);
		// set transparent_color == -2 to use PNG tRNS transparency
	}
	
	hx = ani[cel_ct].hotspot_x;
	hy = ani[cel_ct].hotspot_y;

	FramePtr->image = TFB_DrawImage_New (img[cel_ct]);

	tfbimg = FramePtr->image;
	tfbimg->colormap_index = ani[cel_ct].colormap_index;
	img[cel_ct] = (SDL_Surface *)tfbimg->NormalImg;
	
	FramePtr->HotSpot = MAKE_HOT_SPOT (hx, hy);
	SetFrameBounds (FramePtr, img[cel_ct]->w, img[cel_ct]->h);

#ifdef CLIPDEBUG
	{
		/* for debugging clipping:
		   draws white (or most matching color from palette) pixels to
	       every corner of the image
		 */
		Uint32 color = SDL_MapRGB (img[cel_ct]->format, 255, 255, 255);
		SDL_Rect r = {0, 0, 1, 1};
		if (img[cel_ct]->w > 2 && img[cel_ct]->h > 2)
		{
			SDL_FillRect (img[cel_ct], &r, color);
			r.x = img[cel_ct]->w - 1;
			SDL_FillRect (img[cel_ct], &r, color);
			r.y = img[cel_ct]->h - 1;
			SDL_FillRect (img[cel_ct], &r, color);
			r.x = 0;
			SDL_FillRect (img[cel_ct], &r, color);
		}
	}
#endif
}

static void
processFontChar (TFB_Char* CharPtr, SDL_Surface *surf)
{
	int x,y;
	Uint8 r,g,b,a;
	Uint32 p;
	SDL_PixelFormat* srcfmt = surf->format;
	GetPixelFn getpix;
	BYTE* newdata;
	Uint32 dpitch;

	// Currently, each font char has its own separate data
	//  but that can change to common mem area
	newdata = HMalloc (surf->w * surf->h * sizeof(BYTE));
	dpitch = surf->w;

	SDL_LockSurface (surf);

	getpix = getpixel_for (surf);

	// produce an alpha-only image in internal BYTE[] format
	//  from the SDL surface
	for (y = 0; y < surf->h; ++y)
	{
		BYTE* dst = newdata + dpitch * y;

		for (x = 0; x < surf->w; ++x, ++dst)
		{
			p = getpix (surf, x, y);
			SDL_GetRGBA (p, srcfmt, &r, &g, &b, &a);

			if (!srcfmt->Amask)
			{	// produce alpha from intensity (Y component)
				// using a fast approximation
				// contributions to Y are: R=2, G=4, B=1
				a = (((int)r << 1) + ((int)g << 2) + b) / 7;
			}
			
			*dst = a;
		}
	}

	SDL_UnlockSurface (surf);

	CharPtr->data = newdata;
	CharPtr->pitch = dpitch;
	CharPtr->extent.width = surf->w;
	CharPtr->extent.height = surf->h;
	CharPtr->disp.width = surf->w + 1;
	CharPtr->disp.height = surf->h + 1;
			// XXX: why the +1?
			// I brought it into this function from the only calling
			// function, but I don't know why it was there in the first
			// place.
			// XXX: the +1 appears to be for character and line spacing
			//  text_blt just adds the frame width to move to the next char
	
	{
		// This tunes the font positioning to be about what it should
		// TODO: prolly needs a little tweaking still

		int tune_amount = 0;

		if (CharPtr->extent.height == 8)
			tune_amount = -1;
		else if (CharPtr->extent.height == 9)
			tune_amount = -2;
		else if (CharPtr->extent.height > 9)
			tune_amount = -3;

		CharPtr->HotSpot = MAKE_HOT_SPOT (0,
				CharPtr->extent.height + tune_amount);
	}
}

// stretch_frame
// create a new frame of size neww x newh, and blit a scaled version FramePtr
// into it.
// destroy the old frame if 'destroy' is 1
FRAME stretch_frame (FRAME FramePtr, int neww, int newh, int destroy)
{
	FRAME NewFrame;
	CREATE_FLAGS flags;
	TFB_Image *tfbImg;
	TFB_Canvas src, dst;

	flags = GetFrameParentDrawable (FramePtr)->Flags;
	NewFrame = CaptureDrawable (
				CreateDrawable (flags, (SIZE)neww, (SIZE)newh, 1));
	tfbImg = FramePtr->image;
	LockMutex (tfbImg->mutex);
	src = tfbImg->NormalImg;
	dst = NewFrame->image->NormalImg;
	TFB_DrawCanvas_Rescale_Nearest (src, dst, -1, NULL, NULL, NULL);
	UnlockMutex (tfbImg->mutex);
	if (destroy)
		DestroyDrawable (ReleaseDrawable (FramePtr));
	return (NewFrame);
}

void process_rgb_bmp (FRAME FramePtr, Uint32 *rgba, int maxx, int maxy)
{
	int x, y;
	TFB_Image *tfbImg;
	SDL_Surface *img;
	PutPixelFn putpix;

	tfbImg = FramePtr->image;
	LockMutex (tfbImg->mutex);
	img = (SDL_Surface *)tfbImg->NormalImg;
	SDL_LockSurface (img);

	putpix = putpixel_for (img);

	for (y = 0; y < maxy; ++y)
		for (x = 0; x < maxx; ++x)
			putpix (img, x, y, *rgba++);
	SDL_UnlockSurface (img);
	UnlockMutex (tfbImg->mutex);
}

void fill_frame_rgb (FRAME FramePtr, Uint32 color, int x0, int y0,
		int x, int y)
{
	SDL_Surface *img;
	TFB_Image *tfbImg;
	SDL_Rect rect;

	tfbImg = FramePtr->image;
	LockMutex (tfbImg->mutex);
	img = (SDL_Surface *)tfbImg->NormalImg;
	SDL_LockSurface (img);
	if (x0 == 0 && y0 == 0 && x == 0 && y == 0)
		SDL_FillRect(img, NULL, color);
	else
	{
		rect.x = x0;
		rect.y = y0;
		rect.w = x - x0;
		rect.h = y - y0;
		SDL_FillRect(img, &rect, color);
	}
	SDL_UnlockSurface (img);
	UnlockMutex (tfbImg->mutex);
}

void arith_frame_blit (FRAME srcFrame, const RECT *rsrc, FRAME dstFrame,
		const RECT *rdst, int num, int denom)
{
	TFB_Image *srcImg, *dstImg;
	SDL_Surface *src, *dst;
	SDL_Rect srcRect, dstRect, *srp = NULL, *drp = NULL;
	srcImg = srcFrame->image;
	dstImg = dstFrame->image;
	LockMutex (srcImg->mutex);
	LockMutex (dstImg->mutex);
	src = (SDL_Surface *)srcImg->NormalImg;
	dst = (SDL_Surface *)dstImg->NormalImg;
	if (rdst)
	{
		dstRect.x = rdst->corner.x;
		dstRect.y = rdst->corner.y;
		dstRect.w = rdst->extent.width;
		dstRect.h = rdst->extent.height;
		drp = &dstRect;
	}
	if (rsrc)
	{
		srcRect.x = rsrc->corner.x;
		srcRect.y = rsrc->corner.y;
		srcRect.w = rsrc->extent.width;
		srcRect.h = rsrc->extent.height;
		srp = &srcRect;
	}
	else if (srcFrame->HotSpot.x || srcFrame->HotSpot.y)
	{
		if (rdst)
		{
			dstRect.x -= srcFrame->HotSpot.x;
			dstRect.y -= srcFrame->HotSpot.y;
		}
		else
		{
			dstRect.x = -srcFrame->HotSpot.x;
			dstRect.y = -srcFrame->HotSpot.y;
			dstRect.w = GetFrameWidth (srcFrame);
			dstRect.h = GetFrameHeight (srcFrame);
			drp = &dstRect;
		}

	}
	TFB_BlitSurface (src, srp, dst, drp, num, denom);
	UnlockMutex (srcImg->mutex);
	UnlockMutex (dstImg->mutex);
}

// Generate an array of all pixels in FramePtr
// The 32bpp pixel format is :
//  bits 24-31 : red
//  bits 16-23 : green
//  bits 8-15  : blue
//  bits 0-7   : alpha
// The 8bpp pixel format is 1 index per pixel
void getpixelarray (void *map, int Bpp, FRAME FramePtr,
		int width, int height)
{
	Uint8 r,g,b,a;
	Uint32 p, pos, row;
	TFB_Image *tfbImg;
	SDL_Surface *img;
	GetPixelFn getpix;
	int x, y, w, h;

	tfbImg = FramePtr->image;
	LockMutex (tfbImg->mutex);
	img = (SDL_Surface *)tfbImg->NormalImg;
	getpix = getpixel_for (img);

	w = width < img->w ? width : img->w;
	h = height < img->h ? height : img->h;
	
	SDL_LockSurface (img);

	if (Bpp == 4)
	{
		Uint32 *dp = (Uint32 *)map;

		for (y = 0, row = 0; y < h; y++, row += width)
		{
			for (x = 0, pos = row; x < w; x++, pos++)
			{
				p = getpix (img, x, y);
				SDL_GetRGBA (p, img->format, &r, &g, &b, &a);
				dp[pos] = r << 24 | g << 16 | b << 8 | a;
			}
		}
	}
	else if (Bpp == 1)
	{
		Uint8 *dp = (Uint8 *)map;

		for (y = 0, row = 0; y < h; y++, row += width)
		{
			for (x = 0, pos = row; x < w; x++, pos++)
			{
				p = getpix (img, x, y);
				dp[pos] = p;
			}
		}
	}

	SDL_UnlockSurface (img);
	UnlockMutex (tfbImg->mutex);
}


// Generate a pixel (in the correct format to be applied to FramePtr) from the
// r,g,b,a values supplied
Uint32 frame_mapRGBA (FRAME FramePtr,Uint8 r, Uint8 g,  Uint8 b, Uint8 a)
{
	SDL_Surface *img= (SDL_Surface *)FramePtr->image->NormalImg;
	return (SDL_MapRGBA (img->format, r, g, b, a));
}

void *
_GetCelData (uio_Stream *fp, DWORD length)
{
	int cel_total, cel_index, n;
	DWORD opos;
	char CurrentLine[1024], filename[PATH_MAX];
	SDL_Surface **img;
	AniData *ani;
	DRAWABLE Drawable;
	uio_MountHandle *aniMount = 0;
	uio_DirHandle *aniDir = 0;
	uio_Stream *aniFile = 0;
	
	opos = uio_ftell (fp);

	{
		char *s1, *s2;
		char aniDirName[PATH_MAX];			
		const char *aniFileName;
		uint8 buf[4] = { 0, 0, 0, 0 };
		uint32 header;

		if (_cur_resfile_name == 0
				|| (((s2 = 0), (s1 = strrchr (_cur_resfile_name, '/')) == 0)
						&& (s2 = strrchr (_cur_resfile_name, '\\')) == 0))
		{
			n = 0;
		}
		else
		{
			if (s2 > s1)
				s1 = s2;
			n = s1 - _cur_resfile_name + 1;
		}

		uio_fread(buf, 4, 1, fp);
		header = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
 		if (_cur_resfile_name && header == 0x04034b50)
		{
			// zipped ani file
			if (n)
			{
				strncpy (aniDirName, _cur_resfile_name, n - 1);
				aniDirName[n - 1] = 0;
				aniFileName = _cur_resfile_name + n;
			}
			else
			{
				strcpy(aniDirName, ".");
				aniFileName = _cur_resfile_name;
			}
			aniDir = uio_openDir (repository, aniDirName, 0);
			aniMount = uio_mountDir (repository, aniDirName, uio_FSTYPE_ZIP,
							aniDir, aniFileName, "/", autoMount,
							uio_MOUNT_RDONLY | uio_MOUNT_TOP,
							NULL);
			aniFile = uio_fopen (aniDir, aniFileName, "r");
			opos = 0;
			n = 0;
		}
		else
		{
			// unpacked ani file
			strncpy (filename, _cur_resfile_name, n);
			aniFile = fp;
			aniDir = contentDir;
		}
	}

	cel_total = 0;
	uio_fseek (aniFile, opos, SEEK_SET);
	while (uio_fgets (CurrentLine, sizeof (CurrentLine), aniFile))
	{
		++cel_total;
	}

	img = HMalloc (sizeof (SDL_Surface *) * cel_total);
	ani = HMalloc (sizeof (AniData) * cel_total);
	if (!img || !ani)
	{
		log_add (log_Warning, "Couldn't allocate space for '%s'", _cur_resfile_name);
		if (aniMount)
		{
			uio_fclose(aniFile);
			uio_closeDir(aniDir);
			uio_unmountDir(aniMount);
		}
		HFree (img);
		HFree (ani);
		return NULL;
	}

	cel_index = 0;
	uio_fseek (aniFile, opos, SEEK_SET);
	while (uio_fgets (CurrentLine, sizeof (CurrentLine), aniFile) && cel_index < cel_total)
	{
		sscanf (CurrentLine, "%s %d %d %d %d", &filename[n], 
			&ani[cel_index].transparent_color, &ani[cel_index].colormap_index, 
			&ani[cel_index].hotspot_x, &ani[cel_index].hotspot_y);
	
		img[cel_index] = sdluio_loadImage (aniDir, filename);
		if (img[cel_index] == NULL)
		{
			const char *err;

			err = SDL_GetError();
			log_add (log_Warning, "_GetCelData: Unable to load image!");
			if (err != NULL)
				log_add (log_Warning, "SDL reports: %s", err);
			SDL_FreeSurface (img[cel_index]);
		}
		else if (img[cel_index]->w < 0 || img[cel_index]->h < 0 ||
				img[cel_index]->format->BitsPerPixel < 8)
		{
			log_add (log_Warning, "_GetCelData: Bad file!");
			SDL_FreeSurface (img[cel_index]);
		}
		else
		{
			++cel_index;
		}

		if ((int)uio_ftell (aniFile) - (int)opos >= (int)length)
			break;
	}

	Drawable = NULL;
	if (cel_index && (Drawable = AllocDrawable (cel_index)))
	{
		if (!Drawable)
		{
			while (cel_index--)
				SDL_FreeSurface (img[cel_index]);

			HFree (Drawable);
			Drawable = NULL;
		}
		else
		{
			FRAME FramePtr;

			Drawable->Flags = WANT_PIXMAP;
			Drawable->MaxIndex = cel_index - 1;

			FramePtr = &Drawable->Frame[cel_index];
			while (--FramePtr, cel_index--)
				process_image (FramePtr, img, ani, cel_index);
		}
	}

	if (Drawable == NULL)
		log_add (log_Warning, "Couldn't get cel data for '%s'",
				_cur_resfile_name);

	if (aniMount)
	{
		uio_fclose(aniFile);
		uio_closeDir(aniDir);
		uio_unmountDir(aniMount);
	}

	HFree (img);
	HFree (ani);
	return Drawable;
}

BOOLEAN
_ReleaseCelData (void *handle)
{
	DRAWABLE DrawablePtr;
	int cel_ct;
	FRAME FramePtr = NULL;

	if ((DrawablePtr = handle) == 0)
		return (FALSE);

	cel_ct = DrawablePtr->MaxIndex + 1;

	if (DrawablePtr->Frame)
	{
		FramePtr = DrawablePtr->Frame;
		if (FramePtr->Type == SCREEN_DRAWABLE)
		{
			FramePtr = NULL;
		}
	}

	HFree (handle);
	if (FramePtr)
	{
		int i;
		for (i = 0; i < cel_ct; i++)
		{
			TFB_Image *img = FramePtr[i].image;
			if (img)
			{
				FramePtr[i].image = NULL;
				TFB_DrawScreen_DeleteImage (img);
			}
		}
		HFree (FramePtr);
	}

	return (TRUE);
}

typedef struct BuildCharDesc {
	SDL_Surface *surface;
	UniChar index;
} BuildCharDesc;

int
compareBCDIndex(const BuildCharDesc *bcd1, const BuildCharDesc *bcd2) {
	return (int) bcd1->index - (int) bcd2->index;
}

void *
_GetFontData (uio_Stream *fp, DWORD length)
{
	COUNT numDirEntries;
	DIRENTRY fontDir = NULL;
	BuildCharDesc *bcds = NULL;
	size_t numBCDs = 0;
	int dirEntryI;
	uio_DirHandle *fontDirHandle = NULL;
	uio_MountHandle *fontMount = NULL;
	FONT fontPtr = NULL;

	if (_cur_resfile_name == 0)
		goto err;

	if (fp != (uio_Stream*)~0)
	{
		// font is zipped instead of being in a directory

		char *s1, *s2;
		int n;
		const char *fontZipName;
		char fontDirName[PATH_MAX];

		if ((((s2 = 0), (s1 = strrchr (_cur_resfile_name, '/')) == 0)
						&& (s2 = strrchr (_cur_resfile_name, '\\')) == 0))
		{
			strcpy(fontDirName, ".");
			fontZipName = _cur_resfile_name;
		}
		else
		{
			if (s2 > s1)
				s1 = s2;
			n = s1 - _cur_resfile_name + 1;
			strncpy (fontDirName, _cur_resfile_name, n - 1);
			fontDirName[n - 1] = 0;
			fontZipName = _cur_resfile_name + n;
		}

		fontDirHandle = uio_openDir (repository, fontDirName, 0);
		fontMount = uio_mountDir (repository, _cur_resfile_name, uio_FSTYPE_ZIP,
						fontDirHandle, fontZipName, "/", autoMount,
						uio_MOUNT_RDONLY | uio_MOUNT_TOP,
						NULL);
		uio_closeDir (fontDirHandle);
	}

	fontDir = CaptureDirEntryTable (LoadDirEntryTable (contentDir,
			_cur_resfile_name, ".", match_MATCH_SUBSTRING));
	if (fontDir == 0)
		goto err;
	numDirEntries = GetDirEntryTableCount (fontDir);

	fontDirHandle = uio_openDirRelative (contentDir, _cur_resfile_name, 0);
	if (fontDirHandle == NULL)
		goto err;
		
	bcds = HMalloc (numDirEntries * sizeof (BuildCharDesc));
	if (bcds == NULL)
		goto err;

	// Load the surfaces for all dir Entries
	for (dirEntryI = 0; dirEntryI < numDirEntries; dirEntryI++)
	{
		char *char_name;
		unsigned int charIndex;
		SDL_Surface *surface;

		char_name = GetDirEntryAddress (SetAbsDirEntryTableIndex (
				fontDir, dirEntryI));
		if (sscanf (char_name, "%x.", &charIndex) != 1)
			continue;
			
		if (charIndex > 0xffff)
			continue;

		surface = sdluio_loadImage (fontDirHandle, char_name);
		if (surface == NULL)
			continue;

		if (surface->w == 0 || surface->h == 0 ||
				surface->format->BitsPerPixel < 8) {
			SDL_FreeSurface (surface);
			continue;
		}

		bcds[numBCDs].surface = surface;
		bcds[numBCDs].index = charIndex;
		numBCDs++;
	}
	uio_closeDir (fontDirHandle);
	DestroyDirEntryTable (ReleaseDirEntryTable (fontDir));
	if (fontMount != 0)
		uio_unmountDir(fontMount);

#if 0
	if (numBCDs == 0)
		goto err;
#endif

	// sort on the character index
	qsort (bcds, numBCDs, sizeof (BuildCharDesc),
			(int (*)(const void *, const void *)) compareBCDIndex);

	fontPtr = AllocFont (0);
	if (fontPtr == NULL)
		goto err;
	
	fontPtr->Leading = 0;
	fontPtr->LeadingWidth = 0;

	{
		size_t startBCD = 0;
		int pageStart;
		FONT_PAGE **pageEndPtr = &fontPtr->fontPages;
		while (startBCD < numBCDs)
		{
			// Process one character page.
			size_t endBCD;
			pageStart = bcds[startBCD].index & CHARACTER_PAGE_MASK;

			endBCD = startBCD;
			while (endBCD < numBCDs &&
					(bcds[endBCD].index & CHARACTER_PAGE_MASK) == pageStart)
				endBCD++;

			{
				size_t bcdI;
				int numChars = bcds[endBCD - 1].index + 1
						- bcds[startBCD].index;
				FONT_PAGE *page = AllocFontPage (numChars);
				page->pageStart = pageStart;
				page->firstChar = bcds[startBCD].index;
				page->numChars = numChars;
				*pageEndPtr = page;
				pageEndPtr = &page->next;

				for (bcdI = startBCD; bcdI < endBCD; bcdI++)
				{
					// Process one character.
					BuildCharDesc *bcd = &bcds[bcdI];
					TFB_Char *destChar =
							&page->charDesc[bcd->index - page->firstChar];
				
					if (destChar->data != NULL)
					{
						// There's already an image for this character.
						log_add (log_Debug, "Duplicate image for character %d "
								"for font %s.", (int) bcd->index,
								_cur_resfile_name);
						SDL_FreeSurface (bcd->surface);
						continue;
					}
					
					processFontChar (destChar, bcd->surface);
					SDL_FreeSurface (bcd->surface);

					if (destChar->disp.height > fontPtr->Leading)
						fontPtr->Leading = destChar->disp.height;
					if (destChar->disp.width > fontPtr->LeadingWidth)
						fontPtr->LeadingWidth = destChar->disp.width;
				}
			}

			startBCD = endBCD;
		}
		*pageEndPtr = NULL;
	}

	fontPtr->Leading++;

	HFree (bcds);

	(void) fp;  /* Satisfying compiler (unused parameter) */
	(void) length;  /* Satisfying compiler (unused parameter) */
	return fontPtr;

err:
	if (fontPtr != 0)
		HFree (fontPtr);
	
	if (bcds != NULL)
	{
		size_t bcdI;
		for (bcdI = 0; bcdI < numBCDs; bcdI++)
			SDL_FreeSurface (bcds[bcdI].surface);
		HFree (bcds);
	}
	
	if (fontDirHandle != NULL)
		uio_closeDir (fontDirHandle);
	
	if (fontDir != 0)
		DestroyDirEntryTable (ReleaseDirEntryTable (fontDir));

	if (fontMount != 0)
		uio_unmountDir(fontMount);

	return 0;
}

BOOLEAN
_ReleaseFontData (void *handle)
{
	FONT font = (FONT) handle;
	if (font == NULL)
		return FALSE;

	{
		FONT_PAGE *page;
		FONT_PAGE *nextPage;

		for (page = font->fontPages; page != NULL; page = nextPage)
		{
			size_t charI;
			for (charI = 0; charI < page->numChars; charI++)
			{
				TFB_Char *c = &page->charDesc[charI];
				
				if (c->data == NULL)
					continue;
				
				// XXX: fix this if fonts get per-page data
				//  rather than per-char
				TFB_DrawScreen_DeleteData (c->data);
			}
		
			nextPage = page->next;
			FreeFontPage (page);
		}
	}

	HFree (font);

	return TRUE;
}

// _request_drawable was in libs/graphics/drawable.c before modularization

DRAWABLE
_request_drawable (COUNT NumFrames, DRAWABLE_TYPE DrawableType,
		CREATE_FLAGS flags, SIZE width, SIZE height)
{
	DRAWABLE Drawable;

	Drawable = AllocDrawableImage (
			NumFrames, DrawableType, flags, width, height
			);
	if (Drawable)
	{
		int imgw, imgh;
		FRAME FramePtr;

		Drawable->Flags = flags;
		Drawable->MaxIndex = NumFrames - 1;

		imgw = width;
		imgh = height;
			
		FramePtr = &Drawable->Frame[NumFrames - 1];
		while (NumFrames--)
		{
			TFB_Image *Image;

			if (DrawableType == RAM_DRAWABLE && imgw > 0 && imgh > 0
					&& (Image = TFB_DrawImage_New (TFB_DrawCanvas_New_TrueColor (
						imgw, imgh, (flags & WANT_ALPHA) ? TRUE : FALSE))))
			{
				FramePtr->image = Image;
			}

			FramePtr->Type = DrawableType;
			FramePtr->Index = NumFrames;
			SetFrameBounds (FramePtr, width, height);
			--FramePtr;
		}
	}

	return (Drawable);
}

#endif
