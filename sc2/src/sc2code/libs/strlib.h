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

#ifndef _STRLIB_H
#define _STRLIB_H

#include "compiler.h"

typedef DWORD STRING_TABLE;
typedef DWORD STRING;
typedef PBYTE STRINGPTR;

extern BOOLEAN InstallStringTableResType (COUNT string_type);
extern STRING_TABLE LoadStringTableInstance (DWORD res);
extern STRING_TABLE LoadStringTableFile (PVOID pStr);
extern BOOLEAN DestroyStringTable (STRING_TABLE StringTable);
extern STRING CaptureStringTable (STRING_TABLE StringTable);
extern STRING_TABLE ReleaseStringTable (STRING String);
extern STRING_TABLE GetStringTable (STRING String);
extern COUNT GetStringTableCount (STRING String);
extern COUNT GetStringTableIndex (STRING String);
extern STRING SetAbsStringTableIndex (STRING String, COUNT
		StringTableIndex);
extern STRING SetRelStringTableIndex (STRING String, SIZE
		StringTableOffs);
extern COUNT GetStringLength (STRING String);
extern STRINGPTR GetStringAddress (STRING String);
extern STRINGPTR GetStringSoundClip (STRING String);
extern BOOLEAN GetStringContents (STRING String, STRINGPTR StringBuf,
		BOOLEAN AppendSpace);

#endif /* _STRLIB_H */

