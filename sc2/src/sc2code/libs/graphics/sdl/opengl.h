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

#ifndef OPENGL_H
#define OPENGL_H

#include "libs/graphics/sdl/sdl_common.h"

int TFB_GL_InitGraphics (int driver, int flags, int width, int height, int bpp);
void TFB_GL_UploadTransitionScreen (void);
void TFB_GL_SwapBuffers (void);
SDL_Surface* TFB_GL_DisplayFormatAlpha (SDL_Surface *surface);


#ifdef HAVE_OPENGL
#ifdef WIN32

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")

/* To avoid including windows.h,
   Win32's <GL/gl.h> needs APIENTRY and WINGDIAPI defined properly. */

#ifndef APIENTRY
#define GLUT_APIENTRY_DEFINED
#if (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define APIENTRY    __stdcall
#else
#define APIENTRY
#endif
#endif

/* This is from Win32's <winnt.h> */
#ifndef CALLBACK
#if (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#define CALLBACK __stdcall
#else
#define CALLBACK
#endif
#endif

/* This is from Win32's <wingdi.h> and <winnt.h> */
#ifndef WINGDIAPI
#define GLUT_WINGDIAPI_DEFINED
#define WINGDIAPI __declspec(dllimport)
#endif

/* This is from Win32's <ctype.h> */
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#include "GL/glu.h"

#else /* !defined(WIN32) */

#include "GL/glu.h"

#endif /* WIN32 */
#endif /* HAVE_OPENGL */

#endif
