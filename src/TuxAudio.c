/* =============== GPL HEADER =====================
 * TuxAudio.c
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

#include <TuxCompat.h>
#include <TuxAudio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h> /* lrint() */
#include <TuxTime.h>
#include <TuxLogger.h>
#include <TuxStrings.h>
#include <TuxUtils.h>


int                   TuxAudio_micDeviceId=-1;      /* micro Tux */
int                   TuxAudio_musicDeviceId=-1;      /* Audio Tux */
int                   TuxAudio_ttsDeviceId=-1;      /* TTS Tux */
int                   TuxAudio_isInitialized=0;

static char *TuxAudio_SoundCardName;
static char *TuxAudio_MicroName;

int isRecording=0;                                  /* permet de savoir si on est en cours d'enregistrement ou non */
int isMusicPlaying=0;
int isMusicStopping=0;
int isTTSPlaying=0;
int isTTSStopping=0;

long TuxAudio_MusicSRC_Callback(void *cb_data, float **data);

int TuxAudio_MusicPA_Callback(	const void *inputBuffer,
							void *outputBuffer,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData );
							
long TuxAudio_TTSSRC_Callback(void *cb_data, float **data);

int TuxAudio_TTSPA_Callback(	const void *inputBuffer,
							void *outputBuffer,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData );

TuxAudioError __TuxAudio_StopMusic();
TuxAudioError __TuxAudio_StopTTS();

/* Récupère l'id du micro TuxDroid-Micro */
TuxAudioError TuxAudio_getTuxMicroDeviceId()
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxAudio_getTuxMicroDeviceId()");

	/* It seems that PortAudio doesn't differenciates play and record devices, so let's use already discovered music ID */
	TuxAudio_MicroName = (char *)malloc(sizeof(char)*strlen(TuxAudio_SoundCardName));
	strcpy(TuxAudio_MicroName, TuxAudio_SoundCardName);
	TuxAudio_micDeviceId=TuxAudio_musicDeviceId;

    return E_TUXAUDIO_NOERROR;
}

/* Récupère l'id de la carte sons TuxDroid-Audio */
TuxAudioError TuxAudio_getTuxAudioDeviceId()
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxAudio_getTuxAudioDeviceId()");
	
	int id=-1;
	int numdrivers;

    /* 	numdrivers = Pa_CountDevices(); */
	numdrivers = Pa_GetDeviceCount();
	const   PaDeviceInfo *deviceInfo;

	TuxLogger_writeLog(TUX_LOG_DEBUG, "Audio devices count: %i\n", numdrivers);

	int i;
	for(i = 0; i < numdrivers; i++)
	{
		char name[256];
		deviceInfo = Pa_GetDeviceInfo( i );
		strncpy (name, deviceInfo->name, 256);
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Carte son: %s (ID=%d)",name,i);

		if((strstr(strtolower(name),"tuxdroid") != NULL || strstr(strtolower(name),"tux droid") != NULL)
		&& strstr(strtolower(name),"tts") == NULL)
		{
            TuxAudio_SoundCardName = (char *)malloc(sizeof(char)*strlen(name));
            sprintf(TuxAudio_SoundCardName,"%s", deviceInfo->name);
            id=i;
			/* TODO: Manage second device for TTS (TuxDroid: USB Audio #1) ? */
			break;
        }
	}
	
	if(id >= 0)
	{
	   TuxLogger_writeLog(TUX_LOG_DEBUG,"Carte son Tuxdroid trouvée: %s, ID=%i", TuxAudio_SoundCardName, id);
	   TuxLogger_writeLog(TUX_LOG_DEBUG,"Max Channels: %d, Default Sample Rate: %f", deviceInfo->maxOutputChannels, deviceInfo->defaultSampleRate);
       TuxAudio_musicDeviceId=id;
       TuxAudio_ttsDeviceId=id;
    }
    else
    {
        TuxLogger_writeLog(TUX_LOG_ERROR,"La carte son du TuxDroid n'a pas été trouvée :(");
        return E_TUXAUDIO_SNDCARD_NOTFOUND;
    }
	/* Pa_Terminate(); */

    return E_TUXAUDIO_NOERROR;
}

/* Renvoie le nom de la carte son si elle a été trouvée
 sinon renvoie une chaine vide
*/
char *TuxAudio_getSoundCardName(void)
{
    if(!TuxAudio_isInitialized)
        return "";

    if(TuxAudio_SoundCardName != NULL)
        return TuxAudio_SoundCardName;
    
    return "";
}

/* Renvoie le nom du micro s'il a été trouvé
 sinon renvoie une chaine vide
*/
char *TuxAudio_getMicroName(void)
{
    if(!TuxAudio_isInitialized)
        return "";    

    if(TuxAudio_MicroName != NULL)
        return TuxAudio_MicroName;
        
    return "";
}


/* arrête l'enregistrement du micro */
TuxAudioError TuxAudio_StopRecord()
{
   if(!TuxAudio_isInitialized)
       return E_TUXAUDIO_NOTINITIALIZED;
              
   TuxLogger_writeLog(TUX_LOG_DEBUG,"Arrêt de l'enregistrement du micro");
   isRecording=0;

   return E_TUXAUDIO_NOERROR;
}

#ifdef USE_FMOD
/* écrit l'en-tête du fichier wav */
void WriteWavHeader(FILE *fp, FMOD_SOUND *sound, int length)
{
     isRecording=1;
     
    int             channels, bits;
    float           rate;

    if (!sound)
    {
        return;
    }

    fseek(fp, 0, SEEK_SET);

    FMOD_Sound_GetFormat  (sound, 0, 0, &channels, &bits);
    FMOD_Sound_GetDefaults(sound, &rate, 0, 0, 0);

    {
 	    #if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
             #pragma pack(1)
             #define __PACKED                         /* dummy */
        #else
             #define __PACKED __attribute__((packed)) /* gcc packed */
        #endif

        
        /*
            WAV Structures
        */
        typedef struct
        {
	        signed char id[4];
	        int 		size;
        } RiffChunk;
    
        struct
        {
            RiffChunk       chunk           __PACKED;
            unsigned short	wFormatTag      __PACKED;    /* format type  */
            unsigned short	nChannels       __PACKED;    /* number of channels (i.e. mono, stereo...)  */
            unsigned int	nSamplesPerSec  __PACKED;    /* sample rate  */
            unsigned int	nAvgBytesPerSec __PACKED;    /* for buffer estimation  */
            unsigned short	nBlockAlign     __PACKED;    /* block size of data  */
            unsigned short	wBitsPerSample  __PACKED;    /* number of bits per sample of mono data */
    #ifdef _WIN32
        } FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, (int)rate, (int)rate * channels * bits / 8, 1 * channels * bits / 8, bits } __PACKED;
	#else
        } __PACKED FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, (int)rate, (int)rate * channels * bits / 8, 1 * channels * bits / 8, bits };
	#endif


        struct
        {
            RiffChunk   chunk;
        } DataChunk = { {{'d','a','t','a'}, length } };

        struct
        {
            RiffChunk   chunk;
	        signed char rifftype[4];
        } WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + length }, {'W','A','V','E'} };

      
        /*
            Write out the WAV header.
        */
        fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
        fwrite(&FmtChunk, sizeof(FmtChunk), 1, fp);
        fwrite(&DataChunk, sizeof(DataChunk), 1, fp);
    }
}
#endif

/* TODO: Gestion des erreurs */
void *TuxAudio_StartRecord(void *data)
{
     TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxAudio_StartRecord()");
	#ifdef USE_FMOD     
     /* system => TuxAudio_micsystem */
     /* sound => TuxAudio_micsound */
     /* result => TuxAudio_micresult */
    
    char *file = (char *)data;
    
    FILE                  *fp;
    unsigned int           datalength = 0, soundlength;
    
    int TuxMicroId = TuxAudio_getTuxMicroDeviceId();
    
    TuxAudio_micresult = FMOD_System_Create(&TuxAudio_micsystem);
    
    #ifdef _WIN32
    TuxAudio_micresult = FMOD_System_SetOutput(TuxAudio_micsystem, FMOD_OUTPUTTYPE_WINMM);
    #else
    TuxAudio_micresult = FMOD_System_SetOutput(TuxAudio_micsystem, FMOD_OUTPUTTYPE_ALSA);
    #endif
    
    TuxAudio_micresult = FMOD_System_Init(TuxAudio_micsystem, 32, FMOD_INIT_NORMAL, 0);

    memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

    exinfo.cbsize           = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.numchannels      = 2;
    exinfo.format           = FMOD_SOUND_FORMAT_PCM16;
    exinfo.defaultfrequency = 44100;
    exinfo.length           = exinfo.defaultfrequency * sizeof(short) * exinfo.numchannels * 2;

    TuxAudio_micresult = FMOD_System_CreateSound(TuxAudio_micsystem, 0, FMOD_2D | FMOD_SOFTWARE | FMOD_OPENUSER, &exinfo, &TuxAudio_micsound);
    TuxAudio_micresult = FMOD_System_RecordStart(TuxAudio_micsystem, TuxMicroId, TuxAudio_micsound, 1);
   
    fp = fopen(file, "wb");  

    /* WriteWavHeader(fp, sound, datalength); */
    
    WriteWavHeader(fp,TuxAudio_micsound,datalength);
        
    TuxAudio_micresult = FMOD_Sound_GetLength(TuxAudio_micsound, &soundlength, FMOD_TIMEUNIT_PCM);
    
    do
    {
        static unsigned int lastrecordpos = 0;
        unsigned int recordpos = 0;
        FMOD_System_GetRecordPosition(TuxAudio_micsystem, TuxMicroId, &recordpos);
        
        if (recordpos != lastrecordpos)        
        {
            void *ptr1, *ptr2;
            int blocklength;
            unsigned int len1, len2;
            
            blocklength = (int)recordpos - (int)lastrecordpos;
            if (blocklength < 0)
            {
                blocklength += soundlength;
            }

            /*
                Lock the sound to get access to the raw data.
            */
            FMOD_Sound_Lock(TuxAudio_micsound, lastrecordpos * exinfo.numchannels * 2, blocklength * exinfo.numchannels * 2, &ptr1, &ptr2, &len1, &len2);   /* * exinfo.numchannels * 2 = stereo 16bit.  1 sample = 4 bytes. */

            /*
                Write it to disk.
            */
            if (ptr1 && len1)
            {
                datalength += fwrite(ptr1, 1, len1, fp);
            }
            if (ptr2 && len2)
            {
                datalength += fwrite(ptr2, 1, len2, fp);
            }

            /*
                Unlock the sound to allow FMOD to use it again.
            */
            FMOD_Sound_Unlock(TuxAudio_micsound, ptr1, ptr2, len1, len2);
            
            #ifdef _WIN32
            sleep(10); /* 10 ms */
            #else
            TuxSleep(10); /* 10 ms */
            #endif
            
        }

        lastrecordpos = recordpos;
        
        FMOD_System_Update(TuxAudio_micsystem);
    }
    while(isRecording==1);
    
    WriteWavHeader(fp,TuxAudio_micsound,datalength);
        
    fclose(fp);

    TuxAudio_micresult = FMOD_Sound_Release(TuxAudio_micsound);
    TuxAudio_micresult = FMOD_System_Release(TuxAudio_micsystem);

	if(file != NULL)
	{
		/*free(file);
		file=NULL;*/
	}

	if(fp != NULL)
	{
		/*free(fp);
		fp=NULL;*/
	}
	#endif
	return 0;
}

void *StopMusicThread(void *data)
{
	while(! isMusicStopping) { 
#ifdef _WIN32
		Sleep((long)data);
#else
		TuxSleep((long)data);
#endif
	}

	__TuxAudio_StopMusic();

	isMusicStopping = 0;

    return 0;
}

TuxAudioError TuxAudio_StopMusic()
{
	TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_StopMusic()");

	if (! isMusicPlaying) {
		return 0;
	}

	/* Wake up StopMusicThread() thread */
	isMusicStopping = 1;

	/* Wait until StopMusicThread() thread finished and set isMusicStopping=0 */
	while (isMusicStopping) {
#ifdef _WIN32
		Sleep(TUXAUDIO_PA_SLEEP_MS);
#else
		TuxSleep(TUXAUDIO_PA_SLEEP_MS);
#endif
	}
	
	return E_TUXAUDIO_NOERROR;
}

/* Arrêt de la lecture du fichier wav, Tux_Audio(StopMusic) */
TuxAudioError __TuxAudio_StopMusic()
{
    TuxLogger_writeLog(TUX_LOG_DEBUG, "__TuxAudio_StopMusic()");
 
	if(!TuxAudio_isInitialized)
		return E_TUXAUDIO_NOTINITIALIZED;

	TuxAudio_musicresult = Pa_StopStream(TuxAudio_musicsystem);
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StopStream() error: %s", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StopStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_SNDCLOSE;
	}

	TuxAudio_musicresult = Pa_CloseStream(TuxAudio_musicsystem);
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_CloseStream() error: %s", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_CloseStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_SNDCLOSE;
	}

	TuxAudio_musicsrcstate = src_delete(TuxAudio_musicsrcstate);
	if (TuxAudio_musicsrcstate != NULL) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "SRC delete error !");
			return E_TUXAUDIO_FMOD_SNDRELEASE;
	}
	TuxAudio_musicresult = sf_close(TuxAudio_musicsound);
	if(TuxAudio_musicresult != 0) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "SNDFILE close error: %s !", sf_strerror(TuxAudio_musicsound));
	       	return E_TUXAUDIO_FMOD_SNDRELEASE;
	}

	isMusicPlaying = 0;
	
	return E_TUXAUDIO_NOERROR;
}

TuxAudioError TuxAudio_StopTTS()
{
	TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_StopTTS()");

	if (! isTTSPlaying) {
		return 0;
	}

	/* Wake up StopTTSThread() thread */
	isTTSStopping = 1;

	/* Wait until StopTTSThread() thread finished and set isTTSStopping=0 */
	while (isTTSStopping) {
#ifdef _WIN32
		Sleep(TUXAUDIO_PA_SLEEP_MS);
#else
		TuxSleep(TUXAUDIO_PA_SLEEP_MS);
#endif
	}
	
	return E_TUXAUDIO_NOERROR;
}

/* Arrêt de la lecture du TTS */
TuxAudioError __TuxAudio_StopTTS()
{
	TuxLogger_writeLog(TUX_LOG_DEBUG,"__TuxAudio_StopTTS()");
 
	if(!TuxAudio_isInitialized)
		return E_TUXAUDIO_NOTINITIALIZED;

	TuxAudio_ttsresult = Pa_StopStream(TuxAudio_ttssystem);
	if(TuxAudio_ttsresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StopStream() error: %s", Pa_GetErrorText( TuxAudio_ttsresult ));
		if (TuxAudio_ttsresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StopStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_SNDCLOSE;
	}

	TuxAudio_ttsresult = Pa_CloseStream(TuxAudio_ttssystem);
	if(TuxAudio_ttsresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_CloseStream() error: %s", Pa_GetErrorText( TuxAudio_ttsresult ));
		if (TuxAudio_ttsresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_CloseStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_SNDCLOSE;
	}

	TuxAudio_ttssrcstate = src_delete(TuxAudio_ttssrcstate);
	if (TuxAudio_ttssrcstate != NULL) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "SRC delete error !");
			return E_TUXAUDIO_FMOD_SNDRELEASE;
	}
	TuxAudio_ttsresult = sf_close(TuxAudio_ttssound);
	if(TuxAudio_ttsresult != 0) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "SNDFILE close error: %s !", sf_strerror(TuxAudio_ttssound));
	       	return E_TUXAUDIO_FMOD_SNDRELEASE;
	}

	remove("tts.wav");

	isTTSPlaying = 0;

	return E_TUXAUDIO_NOERROR;
}

/* calcul la durée du TTS */
long TuxAudio_ComputeTTSTime(const char *str, int pitch, int speed)
{
    return lrint(((float)(countCharacterOccurency(trim((char *)str),' ') + 1) / (float)speed) * 60 * 1000);
}

void *StopTTSThread(void *data)
{
    	while(! isTTSStopping) { 
#ifdef _WIN32
		Sleep(TUXAUDIO_PA_SLEEP_MS);
#else
		TuxSleep(TUXAUDIO_PA_SLEEP_MS);
#endif
	}

	__TuxAudio_StopTTS();

	isTTSStopping = 0;

    return 0;
}

/* Démarre la lecture d'un TTS */
/* TODO: Choix de la voix */
TuxAudioError TuxAudio_PlayTTS(const char *voice, const char *text, int pitch, int speed, bool autoStop)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxAudio_PlayTTS(), text=%s / pitch=%d / speed=%d",text,pitch,speed);
    
    
     /* speed: (80 à 450) (175 par défaut) */
     /* volume: (0 à 200) (100 par défaut) */
              
	if(!TuxAudio_isInitialized)
		return E_TUXAUDIO_NOTINITIALIZED;

	if (isMusicPlaying) {
		/* Actually, if music is playing, do nothing
		TODO: decide what to do... stop music ? pause music ? merge stream ? */
		/* return 0; */
	}
	
	if(isTTSPlaying) {
		TuxAudio_StopTTS();
	}

    char *cmd = (char *)malloc(sizeof(char)*strlen(text)+100);
	#ifdef _WIN32
 	sprintf(cmd,"espeak.exe --path=./ -v%s -s %d -p %d -a 100 -w tts.wav \"%s\"",voice,speed,pitch,text);
    #else
    sprintf(cmd,"espeak -v%s -s %d -p %d -a 100 -w tts.wav \"%s\"",voice,speed,pitch,text);
    #endif
    
    TuxLogger_writeLog(TUX_LOG_DEBUG,"TTS CMD ----> %s",cmd);

    system(cmd);
    
	if(cmd != NULL)
	{
    	/*free(cmd);
		cmd=NULL;*/
	}

	unsigned int time = 0;

    TuxAudio_ttssound = sf_open("tts.wav", SFM_READ, &TuxAudio_ttsinfos);
    if (!TuxAudio_ttssound) {
		TuxLogger_writeLog(TUX_LOG_ERROR,"SNDfile: Erreur lors de l'ouverture du fichier !");
        return E_TUXAUDIO_FMOD_CREATESOUND;
	}

	time = (TuxAudio_ttsinfos.frames / TuxAudio_ttsinfos.samplerate) * 1000;
	TuxLogger_writeLog(TUX_LOG_DEBUG, "TTS duration: %d ms", time);

	TuxAudio_ttssrcratio = (double) TUXAUDIO_SAMPLERATE / TuxAudio_ttsinfos.samplerate;
	TuxLogger_writeLog(TUX_LOG_DEBUG, "SRC Ratio: %f", TuxAudio_ttssrcratio);
	
	/* Init resampling */
	if ((TuxAudio_ttssrcstate = src_callback_new (TuxAudio_TTSSRC_Callback, SRC_LINEAR, 1, &TuxAudio_ttsresult, NULL)) == NULL)
	{
		TuxLogger_writeLog(TUX_LOG_ERROR, "SRC Error: src_new() failed : %s.", src_strerror (TuxAudio_ttsresult)) ;
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}

	/* Init PortAudio */
	TuxAudio_ttsresult = Pa_Initialize();
	if(TuxAudio_ttsresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() error: %s", Pa_GetErrorText( TuxAudio_ttsresult ));
		if (TuxAudio_ttsresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_INIT;
	}

	PaStreamParameters outputParameters;
	memset( &outputParameters, 0, sizeof( outputParameters ) );
	outputParameters.channelCount = TUXAUDIO_CHANNELS;
	outputParameters.device = TuxAudio_ttsDeviceId;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.sampleFormat = TUXAUDIO_PA_SAMPLEFMT;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(TuxAudio_ttsDeviceId)->defaultLowOutputLatency ;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	
	TuxAudio_ttsresult = Pa_OpenStream(
                &TuxAudio_ttssystem,
                NULL,
                &outputParameters,
                TUXAUDIO_SAMPLERATE,
                paFramesPerBufferUnspecified,
                paNoFlag,
                TuxAudio_TTSPA_Callback,
                NULL
	);
	if(TuxAudio_ttsresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_OpenStream() error: %s", Pa_GetErrorText( TuxAudio_ttsresult ));
		if (TuxAudio_ttsresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_OpenStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}
	
	if(time > 0) /* -1 = streaming */
    {
		pthread_t Thread;
		pthread_attr_t Thread_attr;
		pthread_attr_init (&Thread_attr);
		pthread_attr_setdetachstate (&Thread_attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&Thread, &Thread_attr, StopTTSThread, (void *) TUXAUDIO_PA_SLEEP_MS);
	}

	TuxAudio_ttsresult = Pa_StartStream(TuxAudio_ttssystem);
	if(TuxAudio_ttsresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StartStream() error: %s", Pa_GetErrorText( TuxAudio_ttsresult ));
		if (TuxAudio_ttsresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StartStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}
	
	isTTSPlaying = 1;

	return E_TUXAUDIO_NOERROR;
}

/*
 Démarre la lecture d'un fichier son
*/
TuxAudioError TuxAudio_PlayMusic(const char *url)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxAudio_PlayMusic()");
    
    #ifndef _WIN32
    if(url[0] == '/')
    {
		/* c'est un path Linux */
		if(!file_exists(url))
		{
			TuxLogger_writeLog(TUX_LOG_ERROR,"Le fichier audio n'existe pas !");
			
			return E_TUXAUDIO_FILE_NOTFOUND;
		}
	}
	#else
	if(	(url[0] == 'c' || url[0] == 'C'
		|| url[0] == 'd' || url[0] == 'D'
		|| url[0] == 'e' || url[0] == 'E'
		|| url[0] == 'f' || url[0] == 'F'
		|| url[0] == 'g' || url[0] == 'G')
		&& url[1] == '\\')
	{
			/* c'est un path windows */
			
			if(!file_exists(url))
			{
				TuxLogger_writeLog(TUX_LOG_ERROR,"Le fichier audio n'existe pas !");	
				
				return;
			}
	}
	#endif    
    
    
	if(!TuxAudio_isInitialized)
		return E_TUXAUDIO_NOTINITIALIZED;

	if(isMusicPlaying) {
		TuxAudio_StopMusic();
	}

	unsigned int time = 0;

    TuxAudio_musicsound = sf_open(url, SFM_READ, &TuxAudio_musicinfos);
    if (!TuxAudio_musicsound) {
		TuxLogger_writeLog(TUX_LOG_ERROR,"SNDfile: Erreur lors de l'ouverture du fichier !");
        return E_TUXAUDIO_FMOD_CREATESOUND;
	}

	time = (TuxAudio_musicinfos.frames / TuxAudio_musicinfos.samplerate) * 1000;
	TuxLogger_writeLog(TUX_LOG_DEBUG, "Music duration: %d ms", time);

	TuxAudio_musicsrcratio = (double) TUXAUDIO_SAMPLERATE / TuxAudio_musicinfos.samplerate;
	TuxLogger_writeLog(TUX_LOG_DEBUG, "SRC Ratio: %f", TuxAudio_musicsrcratio);
	
	/* Init resampling */
	if ((TuxAudio_musicsrcstate = src_callback_new (TuxAudio_MusicSRC_Callback, SRC_LINEAR, 2, &TuxAudio_musicresult, NULL)) == NULL)
	{
		TuxLogger_writeLog(TUX_LOG_ERROR, "SRC Error: src_new() failed : %s.", src_strerror (TuxAudio_musicresult)) ;
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}

	/* Init PortAudio */
	TuxAudio_musicresult = Pa_Initialize();
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() error: %s", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_INIT;
	}

	PaStreamParameters outputParameters;
	memset( &outputParameters, 0, sizeof( outputParameters ) );
	outputParameters.channelCount = TUXAUDIO_CHANNELS;
	outputParameters.device = TuxAudio_musicDeviceId;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	outputParameters.sampleFormat = TUXAUDIO_PA_SAMPLEFMT;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(TuxAudio_musicDeviceId)->defaultHighOutputLatency;
	
	TuxAudio_musicresult = Pa_OpenStream(
                &TuxAudio_musicsystem,
                NULL,
                &outputParameters,
                TUXAUDIO_SAMPLERATE,
                paFramesPerBufferUnspecified,
				paNoFlag,
                TuxAudio_MusicPA_Callback,
                NULL
	);
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_OpenStream() error: %s", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_OpenStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}
	
	if(time > 0) /* -1 = streaming */
    {
		/* TODO: Arrêt auto */
		pthread_t Thread;
		pthread_attr_t Thread_attr;
		pthread_attr_init (&Thread_attr);
		pthread_attr_setdetachstate (&Thread_attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&Thread, &Thread_attr, StopMusicThread, (void*) TUXAUDIO_PA_SLEEP_MS);
	}

	TuxAudio_musicresult = Pa_StartStream(TuxAudio_musicsystem);
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StartStream() error: %s", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_StartStream() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_CREATESOUND;
	}
	
	isMusicPlaying = 1;

	return E_TUXAUDIO_NOERROR;
}

long TuxAudio_MusicSRC_Callback(void *cb_data, float **data)
{
	/* TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_MusicSRC_Callback()"); */
	static int framesRead;
	static float buffer[2*TUXAUDIO_SNDFILE_FRAMES];

	*data = buffer;

	framesRead = sf_readf_float(TuxAudio_musicsound, buffer, TUXAUDIO_SNDFILE_FRAMES);

	/* When SNDfile has not more frame, framesRead will be 0 and also returned to src_callback_read() */
	return framesRead;
}

int TuxAudio_MusicPA_Callback(	const void *inputBuffer,
							void *outputBuffer,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData )
{
	/* TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_MusicPA_Callback()"); */
	static int framesRead;
	static float outSum = 0;
	float outStereo[frameCount*2];
	uint8_t *outMono = (uint8_t*)outputBuffer;
	(void) inputBuffer; /* Prevent unused variable warning. */

	/* framesRead = sf_readf_float(TuxAudio_musicsound, (float*) &outStereo, frameCount); */
	framesRead = src_callback_read(TuxAudio_musicsrcstate, TuxAudio_musicsrcratio, frameCount, (float*) &outStereo);

	/* Convert each frame from Float (-1.0 to 1.0) to  UInt8 (0 to 255) */
	/* TODO: Use channels count instead of fixed value 2 */
    for(int i = 0; i < framesRead; i++)
    {
		outSum = 0;
		for(int j = 0; j < 2; j++) {
			outSum += (float) outStereo[i*2+j]*127;
		}
		outMono[i] = (outSum / 2) + 128;
    }

	if (framesRead == 0) {
		/* Wake up StopMusicThread() thread */
		isMusicStopping = 1;

		return paComplete;
	} else {
		return paContinue;
	}
}

long TuxAudio_TTSSRC_Callback(void *cb_data, float **data)
{
	/* TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_TTSSRC_Callback()"); */
	static int framesRead;
	static float buffer[TUXAUDIO_SNDFILE_FRAMES];

	framesRead = sf_readf_float(TuxAudio_ttssound, buffer, TUXAUDIO_SNDFILE_FRAMES);
	*data = buffer;

	/* When SNDfile has not more frame, framesRead will be 0 and also returned to src_callback_read() */
	return framesRead;
}

int TuxAudio_TTSPA_Callback(	const void *inputBuffer,
							void *outputBuffer,
							unsigned long frameCount,
							const PaStreamCallbackTimeInfo* timeInfo,
							PaStreamCallbackFlags statusFlags,
							void *userData )
{
	/* TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_TTSPA_Callback()"); */
	static int framesRead;
	float outSum = 0;
	float outStereo[frameCount];
	uint8_t *outMono = (uint8_t*)outputBuffer;
	(void) inputBuffer; /* Prevent unused variable warning. */

	framesRead = src_callback_read(TuxAudio_ttssrcstate, TuxAudio_ttssrcratio, frameCount, (float*) &outStereo);

	/* Convert each frame from Float (-1.0 to 1.0) to  UInt8 (0 to 255) */
	/* TODO: Use channels count instead of fixed value 2 */
    for(int i = 0; i < frameCount; i++)
    {
		outSum = 0;
		for(int j = 0; j < 1; j++) {
			outSum += (float) outStereo[i*1+j]*127;
		}
		outMono[i] = (outSum / 1) + 128;
    }

	if (framesRead == 0) {
		/* Wake up StopMusciThread() thread */
		isTTSStopping = 1;

		return paComplete;
	} else {
		return paContinue;
	}
}

/* Initialise les périphériques */
TuxAudioError TuxAudio_Initialize()
{	
    if(TuxAudio_isInitialized)
        return E_TUXAUDIO_ALREADYINITIALIZED;
        
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Initialisation de TuxAudio en cours...");

	TuxAudio_musicresult = Pa_Initialize();
	if( TuxAudio_musicresult != paNoError ) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() error: %s\n", Pa_GetErrorText( TuxAudio_musicresult ));
		if (TuxAudio_musicresult == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Initialize() host error: %s\n", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_INIT;
	}

    if(TuxAudio_getTuxAudioDeviceId() != E_TUXAUDIO_NOERROR)
        return E_TUXAUDIO_SNDCARD_NOTFOUND;
    
    if(TuxAudio_getTuxMicroDeviceId() != E_TUXAUDIO_NOERROR)
        return E_TUXAUDIO_MICRO_NOTFOUND;

	TuxAudio_isInitialized=true;
	
    TuxLogger_writeLog(TUX_LOG_INFO,"TuxAudio v%d.%d.%d initialisé !",TUXAUDIO_MAJOR,TUXAUDIO_MINOR,TUXAUDIO_REVIS);
    
	return E_TUXAUDIO_NOERROR;
}

TuxAudioError TuxAudio_Terminate()
{
	TuxLogger_writeLog(TUX_LOG_DEBUG, "TuxAudio_Terminate()");

	int err = Pa_Terminate();
	if(TuxAudio_musicresult != paNoError) {
		TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Terminate() error: %s", Pa_GetErrorText( err ));
		if (err == paUnanticipatedHostError) {
			TuxLogger_writeLog(TUX_LOG_ERROR, "Pa_Terminate() host error: %s", Pa_GetLastHostErrorInfo()->errorText);
		}	
		return E_TUXAUDIO_FMOD_SYSRELEASE;
	}
	
	return 0;
}