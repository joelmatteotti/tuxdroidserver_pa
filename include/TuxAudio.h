/* =============== GPL HEADER =====================
 * TuxAudio.h
 * Copyleft (C) 2011-2012 - Joel Matteotti <joel _DOT_ matteotti _AT_ free _DOT_ fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 * ====================================================
*/


#ifndef __TUXAUDIO_H__
#define __TUXAUDIO_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <TuxVersion.h>

#ifdef _WIN32
	#include <windows.h> /* _stdcall() (nécessaire à FMOD) */
#endif

#ifdef USE_FMOD
	#include <fmod/fmod.h>
	#include <fmod/fmod_errors.h>
#endif

#include <portaudio.h>
#include <sndfile.h>
#include <samplerate.h>
#include <stdint.h>

#define TUXSAUDIO_AUTHOR "Joel Matteotti" /* développeur initial (ne peut pas changer !) */

#define TUXAUDIO_MAINTENER "Joel Matteotti" /* développeur actuel (si abandon de ma part un jour :p) */

#define TUXAUDIO_SNDFILE_FRAMES 4096
#define TUXAUDIO_SAMPLERATE 16000
#define TUXAUDIO_CHANNELS 1
#define TUXAUDIO_PA_SAMPLEFMT paUInt8
#define TUXAUDIO_PA_SLEEP_MS 200

typedef int TuxAudioError;
typedef enum
{
	E_TUXAUDIO_NOERROR=0, /* OK */
	E_TUXAUDIO_NOTINITIALIZED,
	E_TUXAUDIO_ALREADYINITIALIZED,
	E_TUXAUDIO_SNDCARD_NOTFOUND,
	E_TUXAUDIO_MICRO_NOTFOUND,
	E_TUXAUDIO_FILE_NOTFOUND,
	E_TUXAUDIO_FMOD_CREATESYS,
	E_TUXAUDIO_FMOD_SETDRIVER,
	E_TUXAUDIO_FMOD_INIT,
	E_TUXAUDIO_FMOD_CREATECHAN,
	E_TUXAUDIO_FMOD_SETVOLUMEERROR,
	E_TUXAUDIO_FMOD_CREATESOUND,
	E_TUXAUDIO_FMOD_PLAYERROR,
	E_TUXAUDIO_FMOD_SNDRELEASE,
	E_TUXAUDIO_FMOD_SNDCLOSE,
	E_TUXAUDIO_FMOD_SYSRELEASE,
	E_TUXAUDIO_FMOD_RECORDERROR,
} tux_audio_error_t;

#ifdef USE_FMOD
FMOD_CREATESOUNDEXINFO exinfo; /* pour le micro */
#endif

PaStream	*TuxAudio_musicsystem;
PaStream	*TuxAudio_ttssystem;
PaStream	*TuxAudio_micsystem;

SNDFILE		*TuxAudio_musicsound;
SNDFILE		*TuxAudio_ttssound;
SNDFILE		*TuxAudio_micsound;

PaError		TuxAudio_musicresult;
PaError		TuxAudio_ttsresult;
PaError		TuxAudio_micresult;

SF_INFO		TuxAudio_musicinfos;
SF_INFO		TuxAudio_ttsinfos;
SF_INFO		TuxAudio_micinfos;

double		TuxAudio_musicsrcratio;
double		TuxAudio_ttssrcratio;
double		TuxAudio_micsrcratio;

SRC_STATE	*TuxAudio_musicsrcstate;
SRC_STATE	*TuxAudio_ttssrcstate;
SRC_STATE	*TuxAudio_micsrcstate;

/* prototypes */
TuxAudioError TuxAudio_getTuxMicroDeviceId(void);
TuxAudioError TuxAudio_getTuxAudioDeviceId(void);
#ifdef USE_FMOD
void WriteWavHeader(FILE *fp, FMOD_SOUND *sound, int length);
#endif
TuxAudioError TuxAudio_StopRecord(void);
void *TuxAudio_StartRecord(void *data);
TuxAudioError TuxAudio_StopMusic(void);
TuxAudioError TuxAudio_StopTTS(void);
void *StopTTSThread(void *data);
void TuxAudio_AutoStopTTS(long time);
long TuxAudio_ComputeTTSTime(const char *str, int pitch, int speed);
TuxAudioError TuxAudio_PlayTTS(const char *voice, const char *text, int pitch, int speed, bool autoStop);
TuxAudioError TuxAudio_PlayMusic(const char *url);
void *StopMusicThread(void *data);
TuxAudioError TuxAudio_Initialize(void);
TuxAudioError TuxAudio_Terminate(void);

char *TuxAudio_getSoundCardName(void);
char *TuxAudio_getMicroName(void);
#endif
