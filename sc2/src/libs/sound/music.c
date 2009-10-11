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

#include <string.h>
#include "libs/file.h"
#include "options.h"
#include "sound.h"
#include "libs/reslib.h"
#include "libs/log.h"
#include "libs/memlib.h"


static MUSIC_REF curMusicRef;
static MUSIC_REF curSpeechRef;

void
PLRPlaySong (MUSIC_REF MusicRef, BOOLEAN Continuous, BYTE Priority)
{
	TFB_SoundSample **pmus = MusicRef;

	if (pmus)
	{
		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		PlayStream ((*pmus), MUSIC_SOURCE, Continuous, 
			speechVolumeScale == 0.0f, true);
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		
		curMusicRef = MusicRef;
	}

	(void) Priority;  /* Satisfy compiler because of unused variable */
}

void
PLRStop (MUSIC_REF MusicRef)
{
	if (MusicRef == curMusicRef || MusicRef == (MUSIC_REF)~0)
	{
		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		StopStream (MUSIC_SOURCE);
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);

		curMusicRef = 0;
	}
}

BOOLEAN
PLRPlaying (MUSIC_REF MusicRef)
{
	if (curMusicRef && (MusicRef == curMusicRef || MusicRef == (MUSIC_REF)~0))
	{
		BOOLEAN playing;

		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		playing = PlayingStream (MUSIC_SOURCE);
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);

		return playing;
	}

	return FALSE;
}

void
PLRPause (MUSIC_REF MusicRef)
{	
	if (MusicRef == curMusicRef || MusicRef == (MUSIC_REF)~0)
	{
		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		PauseStream (MUSIC_SOURCE);
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
	}
}

void
PLRResume (MUSIC_REF MusicRef)
{
	if (MusicRef == curMusicRef || MusicRef == (MUSIC_REF)~0)
	{
		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		ResumeStream (MUSIC_SOURCE);
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
	}
}

void
snd_PlaySpeech (MUSIC_REF SpeechRef)
{
	TFB_SoundSample **pmus = SpeechRef;

	if (pmus)
	{
		LockMutex (soundSource[SPEECH_SOURCE].stream_mutex);
		PlayStream (*pmus, SPEECH_SOURCE, false, false, true);
		UnlockMutex (soundSource[SPEECH_SOURCE].stream_mutex);
		
		curSpeechRef = SpeechRef;
	}
}

void
snd_StopSpeech (void)
{
	if (!curSpeechRef)
		return;
	
	LockMutex (soundSource[SPEECH_SOURCE].stream_mutex);
	StopStream (SPEECH_SOURCE);
	UnlockMutex (soundSource[SPEECH_SOURCE].stream_mutex);

	curSpeechRef = 0;
}

BOOLEAN
DestroyMusic (MUSIC_REF MusicRef)
{
	FreeMusicData (MusicRef);
	return (TRUE);
}

void
SetMusicVolume (COUNT Volume)
{
	float f = (Volume / (float)MAX_VOLUME) * musicVolumeScale;
	musicVolume = Volume;
	audio_Sourcef (soundSource[MUSIC_SOURCE].handle, audio_GAIN, f);
}

char*
CheckMusicResName (char* fileName)
{
	if (!fileExists2 (contentDir, fileName))
		log_add (log_Warning, "Requested track '%s' not found.", fileName);
	return fileName;
}

void *
_GetMusicData (uio_Stream *fp, DWORD length)
{
	MUSIC_REF h;

	if (_cur_resfile_name && (h = AllocMusicData (sizeof (void *))))
	{
		TFB_SoundSample **pmus = h;

		if (!pmus)
		{
			return NULL;
		}		
		else
		{
            char filename[256];

			*pmus = (TFB_SoundSample *) HCalloc (sizeof (TFB_SoundSample));
			strncpy (filename, _cur_resfile_name, sizeof(filename) - 1);
			filename[sizeof(filename) - 1] = '\0';
			CheckMusicResName (filename);

			log_add (log_Info, "_GetMusicData(): loading %s", filename);
			if (((*pmus)->decoder = SoundDecoder_Load (contentDir, filename,
							4096, 0, 0)) == 0)
			{
				log_add (log_Warning, "_GetMusicData(): couldn't load %s", filename);

				HFree (h);
				return NULL;
			}
			else
			{
				log_add (log_Info, "    decoder: %s, rate %d format %x",
					SoundDecoder_GetName ((*pmus)->decoder),
					(*pmus)->decoder->frequency, (*pmus)->decoder->format);

				(*pmus)->num_buffers = 64;
				(*pmus)->buffer = (audio_Object *) HMalloc (sizeof (audio_Object) * (*pmus)->num_buffers);
				audio_GenBuffers ((*pmus)->num_buffers, (*pmus)->buffer);
			}
		}
	}

	(void) fp;  /* satisfy compiler (unused parameter) */
	(void) length;  /* satisfy compiler (unused parameter) */
	return (h);
}

BOOLEAN
_ReleaseMusicData (void *data)
{
	TFB_SoundSample **pmus = data;

	if (pmus == NULL)
		return (FALSE);

	if ((*pmus)->decoder)
	{
		TFB_SoundDecoder *decoder = (*pmus)->decoder;
		LockMutex (soundSource[MUSIC_SOURCE].stream_mutex);
		if (soundSource[MUSIC_SOURCE].sample == (*pmus))
		{
			StopStream (MUSIC_SOURCE);
		}
		UnlockMutex (soundSource[MUSIC_SOURCE].stream_mutex);

		(*pmus)->decoder = NULL;
		SoundDecoder_Free (decoder);
		audio_DeleteBuffers ((*pmus)->num_buffers, (*pmus)->buffer);
		HFree ((*pmus)->buffer);
		if ((*pmus)->buffer_tag)
			HFree ((*pmus)->buffer_tag);
	}
	HFree (*pmus);
	HFree (pmus);

	return (TRUE);
}

