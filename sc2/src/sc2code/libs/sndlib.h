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

#ifndef _SNDLIB_H
#define _SNDLIB_H

#include "strlib.h"
#include "memlib.h"

typedef STRING_TABLE SOUND_REF;
typedef STRING SOUND;
typedef STRINGPTR SOUNDPTR;

#define InitSoundResources InitStringTableResources
#define CaptureSound CaptureStringTable
#define ReleaseSound ReleaseStringTable
#define GetSoundRef GetStringTable
#define GetSoundCount GetStringTableCount
#define GetSoundIndex GetStringTableIndex
#define SetAbsSoundIndex SetAbsStringTableIndex
#define SetRelSoundIndex SetRelStringTableIndex
#define GetSoundLength GetStringLength
#define GetSoundAddress GetStringAddress
#define GetSoundContents GetStringContents

typedef MEM_HANDLE MUSIC_REF;

extern BOOLEAN InitSound (int argc, char *argv[]);
extern void UninitSound (void);
extern SOUND_REF LoadSoundFile (PVOID pStr);
extern MUSIC_REF LoadMusicFile (PVOID pStr);
extern BOOLEAN InstallAudioResTypes (COUNT sound_type, COUNT
		music_type);
extern SOUND_REF LoadSoundInstance (DWORD res);
extern MUSIC_REF LoadMusicInstance (DWORD res);
extern BOOLEAN DestroySound (SOUND_REF SoundRef);
extern BOOLEAN DestroyMusic (MUSIC_REF MusicRef);

#define MAX_CHANNELS 8
#define MAX_VOLUME 255

extern void PLRPlaySong (MUSIC_REF MusicRef, BOOLEAN Continuous, BYTE
		Priority);
extern void PLRStop (MUSIC_REF MusicRef);
extern BOOLEAN PLRPlaying (MUSIC_REF MusicRef);
extern void PLRPause (MUSIC_REF MusicRef);
extern void PLRResume (MUSIC_REF MusicRef);
extern void PlayChannel (COUNT Channel, PVOID lpSample, COUNT SampLen,
		COUNT LoopBegin, COUNT LoopLen, BYTE Priority);
extern BOOLEAN ChannelPlaying (COUNT Channel);
extern void StopChannel (COUNT Channel, BYTE Priority);
extern void SetMusicVolume (COUNT Volume);
extern void SetChannelVolume (COUNT Channel, COUNT Volume, BYTE
		Priority);
extern void SetChannelRate (COUNT Channel, DWORD Rate, BYTE Priority);

extern void StopSound (void);
extern BOOLEAN SoundPlaying (void);

extern BOOLEAN AllocHardwareSample (PBYTE lpSnd, DWORD SampleRate, COUNT
		SampleLength, COUNT LoopBegin, COUNT LoopLen);
extern BOOLEAN FreeHardwareSample (PBYTE lpSnd, COUNT SampleLength);

extern COUNT GetSampleRate (SOUND Sound);
extern COUNT GetSampleLength (SOUND Sound);
extern PBYTE GetSampleAddress (SOUND Sound);
extern DWORD FadeMusic (BYTE end_vol, SIZE TimeInterval);

extern void SetSoundTracking (BOOLEAN toggle);
extern void GetSoundTrackingBuf (PSBYTE pdigi_buf);

#endif /* _SNDLIB_H */

