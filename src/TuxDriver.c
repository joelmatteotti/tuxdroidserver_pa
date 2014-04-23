/* =============== GPL HEADER =====================
 * TuxDriver.c
 * Copyleft (C) 2011-2012 Joel Matteotti <joel _DOT_ matteotti _AT_ free _DOT_ fr>
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

#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#include <TuxCompat.h>
#include <TuxDriver.h>
#include <TuxLogger.h>
#include <TuxStrings.h>
#include <TuxTime.h> /* usleep() & TuxSleep() (linux) */
#include <TuxUtils.h>




static bool isInitialized=false;




void *IMPORT_FUNC(void *HANDLE, const char *func)
{
     #ifdef _WIN32
     return GetProcAddress(HANDLE,func);
     #else
     return dlsym(HANDLE,func);
     #endif
}

/*
 Initialise la DLL et les fonctions du driver
*/
TuxDrvError TuxDriver_Initialize()
{

	TuxLogger_writeLog(TUX_LOG_DEBUG,"TuxDriver_Initialize()");

       if(isInitialized)
           return E_TUXDRV_ALREADYINITIALIZED;


        TuxLogger_writeLog(TUX_LOG_DEBUG,"Initialisation de TuxDriver (handle de la lib du driver)");
            

        #ifdef _WIN32
	if(!file_exists("libtuxdriver.dll"))
        {
        	TuxLogger_writeLog(TUX_LOG_ERROR,"Le driver n'a pas ÈtÈ trouver !!");
		return E_TUXDRV_DRIVERNOTFOUND;
        }
	else
		DLLHANDLE = LoadLibrary("libtuxdriver.dll");
     	#else
	if(!file_exists("./libtuxdriver.so"))
	{
            	TuxLogger_writeLog(TUX_LOG_ERROR,"Le driver n'a pas ÈtÈ trouver !!");
		return E_TUXDRV_DRIVERNOTFOUND;
	}
        else
     		DLLHANDLE = dlopen("./libtuxdriver.so", RTLD_NOW);
     	#endif

	TuxLogger_writeLog(TUX_LOG_DEBUG,"Avant de r√©cup√©rer les fonctions de la dll");

     	TuxDrv_Stop = (TuxDrv_Stop_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE, "TuxDrv_Stop");
        TuxDrv_Start = (TuxDrv_Start_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE, "TuxDrv_Start");
    	TuxDrv_SetStatusCallback = (TuxDrv_SetStatusCallback_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetStatusCallback");
		TuxDrv_SetEndCycleCallback = (TuxDrv_SetEndCycleCallback_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetEndCycleCallback");
     	TuxDrv_SetDongleConnectedCallback = (TuxDrv_SetDongleConnectedCallback_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetDongleConnectedCallback");
     	TuxDrv_SetDongleDisconnectedCallback = (TuxDrv_SetDongleDisconnectedCallback_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetDongleDisconnectedCallback");
		TuxDrv_ResetDongle = (TuxDrv_ResetDongle_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_ResetDongle");
     	TuxDrv_PerformCommand = (TuxDrv_PerformCommand_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_PerformCommand");
     	TuxDrv_PerformMacroText = (TuxDrv_PerformMacroText_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_PerformMacroText");
		TuxDrv_PerformMacroFile = (TuxDrv_PerformMacroFile_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_PerformMacroFile");
		TuxDrv_ResetPositions = (TuxDrv_ResetPositions_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE, "TuxDrv_ResetPositions");
     	TuxDrv_ClearCommandStack = (TuxDrv_ClearCommandStack_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_ClearCommandStack");
		TuxDrv_SoundReflash = (TuxDrv_SoundReflash_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SoundReflash");
     	TuxDrv_GetStatusName = (TuxDrv_GetStatusName_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_GetStatusName");
     	TuxDrv_GetStatusId = (TuxDrv_GetStatusId_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_GetStatusId");
     	TuxDrv_GetStatusState = (TuxDrv_GetStatusState_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_GetStatusState");
     	TuxDrv_GetStatusValue = (TuxDrv_GetStatusValue_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_GetStatusValue");
     	TuxDrv_GetAllStatusState = (TuxDrv_GetAllStatusState_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_GetAllStatusState");
     	TuxDrv_Eyes_Off = (TuxDrv_Eyes_Off_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_Eyes_cmd_off");
        TuxDrv_Mouth_Off = (TuxDrv_Mouth_Off_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_Mouth_cmd_off");
        TuxDrv_Spinning_Off = (TuxDrv_Spinning_Off_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_Spinning_cmd_off");
        TuxDrv_Flippers_Off = (TuxDrv_Flippers_Off_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_Flippers_cmd_off");
        TuxDrv_Update_Light = (TuxDrv_Update_Light_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_LightLevel_update");
        TuxDrv_GetFlashSound_Infos = (TuxDrv_GetFlashSound_Infos_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SoundFlash_dump_descriptor");
        
        /*
        TuxDrv_SetLogLevel = (TuxDrv_SetLogLevel_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetLogLevel");
        TuxDrv_SetLogTarget = (TuxDrv_SetLogTarget_t)(uintptr_t)IMPORT_FUNC(DLLHANDLE,"TuxDrv_SetLogTarget");
        */ 
            
        isInitialized=true;

        TuxLogger_writeLog(TUX_LOG_INFO,"TuxDriver v%d.%d.%d initialiser !",TUXDRIVER_MAJOR,TUXDRIVER_MINOR,TUXDRIVER_REVIS);
        
	    return E_TUXDRV_NOERROR;
}


void TuxDriver_DrvStop(void)
{
     TuxLogger_writeLog(TUX_LOG_DEBUG,"ArrÍt du driver");
     TuxDrv_Stop();
}

/*
 DÈmarre le driver
*/
void* TuxDriver_DrvStart(void *data)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"DÈmarrage du driver");
	TuxDrv_Start();
	return 0;
}

/* from tux_error.c */
const char *TuxDriver_strerror(TuxDrvError error_code)
{
    switch (error_code) {
    case E_TUXDRV_NOERROR:
        return "No error";
    case E_TUXDRV_PARSERISDISABLED:
        return "The parser of command is disabled";
    case E_TUXDRV_INVALIDCOMMAND:
        return "Invalid command";
    case E_TUXDRV_STACKOVERFLOW:
        return "The stack of commands is full";
    case E_TUXDRV_FILEERROR:
        return "File error";
    case E_TUXDRV_BADWAVFILE:
        return "Wave file error";
    case E_TUXDRV_INVALIDIDENTIFIER:
        return "The identifier is unknow";
    case E_TUXDRV_INVALIDNAME:
        return "The name is unknow";
    case E_TUXDRV_INVALIDPARAMETER:
        return "One or more parameters are invalid";
    case E_TUXDRV_BUSY:
        return "The system is busy";
    case E_TUXDRV_WAVSIZEEXCEDED:
        return "The size of the selection exceeds 127 blocks";
    case E_TUXDRV_NOTVALID_LED:
         return "The LED is not valid";
    case E_TUXDRV_NOTVALID_EFFECT:
         return "Invalid pulse effect";
    case E_TUXDRV_DRIVERNOTFOUND:
         return "TuxDriver not found";
    default:
        return "unknow/unhandle error";
    }
}

/*
 Fonction appeller lors de changement d'un ÈvÈnement du driver (boutons, Ètat des leds, ...)
*/
void StatusEventCallback(char *status_str)
{
	char *status;
  	char *type;
    char *value;
 	char *pch=NULL;

	pch = strtok (status_str,":");
	status = (char *)malloc(sizeof(char)*(strlen(pch)+1));
   	strcpy(status,pch);
   	pch = strtok (NULL, ":");
   	type = (char *)malloc(sizeof(char)*(strlen(pch)+1));
   	strcpy(type,pch);
   	pch = strtok (NULL, ":");
   	value = (char *)malloc(sizeof(char)*(strlen(pch)+1));
   	strcpy(value,pch);

  	if(!strcmp(status,"head_button") && !strcmp(value,"True"))
  	{
            TuxLogger_writeLog(TUX_LOG_DEBUG,"Appuie sur le bouton de la tÍte detecter");
   
    		if(OnTuxHeadButtonPressed != NULL)
        		OnTuxHeadButtonPressed();
  	}

  	if(!strcmp(status,"left_wing_button") && !strcmp(value,"True"))
  	{
            TuxLogger_writeLog(TUX_LOG_DEBUG,"Appuie sur l'aile gauche detecter");
            
    		if(OnTuxLeftButtonPressed != NULL)
         		OnTuxLeftButtonPressed();
  	}

  	if(!strcmp(status,"right_wing_button") && !strcmp(value,"True"))
  	{
            TuxLogger_writeLog(TUX_LOG_DEBUG,"Appuie sur l'aile droite detecter");
            
      		if(OnTuxRightButtonPressed != NULL)
          		OnTuxRightButtonPressed();
  	}

  	if(!strcmp(status,"remote_button"))
  	{
     		char button[30];
     		TuxDrv_GetStatusValue(SW_ID_REMOTE_BUTTON, button);

            TuxLogger_writeLog(TUX_LOG_DEBUG,"Appuie sur un bouton de la tÈlÈcommande detecter: %s",button);
                                       

     		if(!strcmp(button,"RELEASE"))
     		{
         		if(OnTuxRemoteButtonReleased != NULL)
            			OnTuxRemoteButtonReleased();
     		}
     		else
     		{
         		if(OnTuxRemoteButtonPressed != NULL)
            			OnTuxRemoteButtonPressed(NULL, button);
     		}
  	}

  	if(!strcmp(status,"charger_state"))
  	{
     		char state[30];
     		TuxDrv_GetStatusValue(SW_ID_CHARGER_STATE, state);


            TuxLogger_writeLog(TUX_LOG_DEBUG,"EvÈnement lier au chargeur detecter (Etat: %s)",state);
                                       

     		if(!strcmp(state,"CHARGING"))
     		{
        		if(OnTuxChargerPlugged != NULL)
           			OnTuxChargerPlugged();
     		}
     		else
     		{
        		if(OnTuxChargerUnPlugged != NULL)
           			OnTuxChargerUnPlugged();
     		}
  	}
  	
	if(pch != NULL)
	{
  		/*free(pch);
		pch=NULL;*/
	}

	if(status != NULL)
	{
  		/*free(status);
		status=NULL;*/
	}

	if(value != NULL)
	{
  		/*free(value);
		value=NULL;*/
	}

	if(type != NULL)
	{
  		/*free(type);
		type=NULL;*/
	}
}

/*
 Lorsque le dongle viens d'Ítre connecter
*/
void Tux_DongleConnected()
{
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Dongle Connecter");
     
	if(OnTuxDongleConnected != NULL)
        	OnTuxDongleConnected();
}

/*
 Lorsque le dongle viens d'Ítre dÈconnecter
*/
void Tux_DongleDisconnected()
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Dongle deconnecter");
          
	if(OnTuxDongleDisconnected != NULL)
        	OnTuxDongleDisconnected();
}

/*
 Reset le dongle
*/
void Tux_ResetDongle()
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"RÈ-initialisation du dongle demander");
	TuxDrv_ResetDongle();
}

/*
 Joue un son flash
 sound => id du son √† jouer
 volume => 0 (min) √† 100 (max)
*/
TuxDrvError Tux_PlayFlashSound(byte sound, byte volume)
{
	char cmd[32];
	snprintf(cmd,32,"TUX_CMD:SOUND_FLASH:PLAY:%d:%d",sound,volume);
	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 DÈfini la fonction de callback des ÈvÈnements liÈs au status du droid
*/
TuxDrvError Tux_SetCallbackEvent(event_callback_t funct)
{
	return TuxDrv_SetStatusCallback(funct);
}

/*
 D√©fini la fonction de callback pour l'√©v√©nement EndCycle
*/
void Tux_SetEndCycleCallbackEvent(drv_simple_callback_t funct)
{
	TuxDrv_SetEndCycleCallback(funct);
}

/*
 Appeller √† la fin de chaque cycle de lecture du port usb de Fux
*/
void EndCycleEventCallback()
{
	if(OnTuxEndCycle != NULL)
		OnTuxEndCycle();
}

/*
 Initialise les fonctions √©v√©nementielle
 Cr√©er le thread pour le driver
*/
void TuxDriver_Start(void)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"PrÈparation du thread pour le driver");
    Tux_SetCallbackEvent(StatusEventCallback);
    Tux_SetEndCycleCallbackEvent(EndCycleEventCallback);
    
    TuxDrv_SetDongleConnectedCallback(Tux_DongleConnected);
    TuxDrv_SetDongleDisconnectedCallback(Tux_DongleDisconnected);
    
	pthread_t DrvThread;
	pthread_attr_t DrvThread_attr;

    pthread_attr_init (&DrvThread_attr);
    pthread_attr_setdetachstate (&DrvThread_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&DrvThread, &DrvThread_attr, TuxDriver_DrvStart, NULL);
    pthread_join(DrvThread, NULL);
    
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Thread du driver dÈmarrer");

}

/*
 Permet de modifier les sons de la m√©moire flash de TuxDroid
 Prend en pram√®tre une liste de fichiers wav s√©par√© par un |
 Ex: Tux_SoundReflash("fichier1.wav|fichier2.wav|fichier3.wav");
*/
TuxDrvError Tux_SoundReflash(const char *tracks)
{
	return TuxDrv_SoundReflash(tracks);
}

/*
 D√©fini la vitesse du moteur des ailes
 0 (le plus rapide) √† 255 (le plus lent)
*/
TuxDrvError Tux_SetFlippersSpeed(byte speed)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Modification de la vitesse des ailes: %d",speed);

	char cmd[26];
	snprintf(cmd,26,"TUX_CMD:FLIPPERS:SPEED:%d",speed);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Met les ailes en l'air
*/
TuxDrvError Tux_Flippers_Up(void)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Flippers_Up()");           
     	return TuxDrv_PerformCommand(0.0, "TUX_CMD:FLIPPERS:UP");
}

/*
 Baisse les ailes
*/
TuxDrvError Tux_Flippers_Down(void)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Flippers_Down()");
	return TuxDrv_PerformCommand(0.0, "TUX_CMD:FLIPPERS:DOWN");
}

/*
 Permet de lever/abaisser les ailes un certain nombre de fois
*/
TuxDrvError Tux_FlippersUpDown(int count)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_FlippersUpDown, count=%d",count);

	char cmd[30];
	snprintf(cmd,30,"TUX_CMD:FLIPPERS:ON:%d,NDEF",count);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Permet de lever/abaisser les ailes pendant x seconde
*/
TuxDrvError Tux_FlippersUpDown_OnDuration(double duration)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_FlippersUpDown_OnDuration, duration=%f",duration);
    
	char cmd[40];
	snprintf(cmd,40,"TUX_CMD:FLIPPERS:ON_DURING:%.1f,DOWN",duration);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Ouvre les yeux
*/
TuxDrvError Tux_Open_Eyes(void)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Open_Eyes()");
     	return TuxDrv_PerformCommand(0.0, "TUX_CMD:EYES:OPEN");
}

/**
* Ferme les yeux
*/
TuxDrvError Tux_Close_Eyes(void)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Close_Eyes()");
     	return TuxDrv_PerformCommand(0.0, "TUX_CMD:EYES:CLOSE");
}

/**
* Ouvre et ferme les yeux <count> fois
*/
TuxDrvError Tux_OpenClose_Eyes(int count)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_OpenClose_Eyes(), count=%d",count);
        
	char cmd[24];
	snprintf(cmd,24,"TUX_CMD:EYES:ON:%d,NDEF",count);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/**
* Ouvre et ferme les yeux pendant <duration> seconde(s)
* @param duration durÈe d'ouverture/fermeture des yeux.
*/
TuxDrvError Tux_OpenClose_Eyes_OnDuration(double duration)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_OpenClose_Eyes_OnDuration(), duration=%d",duration);
    
	char cmd[40];
	snprintf(cmd,40,"TUX_CMD:EYES:ON_DURING:%.1f,CLOSE",duration);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Ouvre la bouche
*/
TuxDrvError Tux_Open_Mouth(void)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Open_Mouth()");
     	return TuxDrv_PerformCommand(0.0, "TUX_CMD:MOUTH:OPEN");
}

/*
 Ferme la bouche
*/
TuxDrvError Tux_Close_Mouth(void)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Close_Mouth()");
     	return TuxDrv_PerformCommand(0.0, "TUX_CMD:MOUTH:CLOSE");
}


/*
 Ouvre et ferme la bouche <count> fois
*/
TuxDrvError Tux_OpenClose_Mouth(int count)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_OpenClose_Mouth(),count=%d",count);
    
	char cmd[26];
	snprintf(cmd,26,"TUX_CMD:MOUTH:ON:%d,NDEF",count);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Ouvre et ferme la bouche pendant <duration> seconde(s)
*/
TuxDrvError Tux_OpenClose_Mouth_OnDuration(double duration)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_OpenClose_Mouth_OnDuration(),duration=%f",duration);
    
	char cmd[40];
	snprintf(cmd,40,"TUX_CMD:MOUTH:ON_DURING:%.1f,CLOSE",duration);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 D√©finition la vitesse de rotation
 0 (le plus rapide) √† 255 (le plus lent)
*/
TuxDrvError Tux_SetRotationSpeed(byte speed)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Modification de la vitesse des rotors, speed=%d",speed);
    
	char cmd[26];
	snprintf(cmd,26,"TUX_CMD:SPINNING:SPEED:%d",speed);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Fait <count> quart de tour sur la gauche
*/
TuxDrvError Tux_RotateLeft(int count)
{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_RotateLeft(),count=%d",count);
        
        char cmd[30];
     	char buff[2]; /*0-99*/

     	snprintf(buff,2,"%d",count);

     	strcpy(cmd, "TUX_CMD:SPINNING:RIGHT_ON:");
     	strcat(cmd,buff);

        return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Fait x quart de tour sur la gauche pendant <duration> seconde(s)
*/
TuxDrvError Tux_RotateLeft_OnDuration(double duration)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_RotateLeft_OnDuration(),duration=%f",duration);
        
	char cmd[40];
	snprintf(cmd,40,"TUX_CMD:SPINNING:LEFT_ON_DURING:%.1f",duration);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Fait <count> quart de tour sur la droite
*/
TuxDrvError Tux_RotateRight(int count)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_RotateRight(),count=%d",count);
        
	char cmd[30];
     	char buff[2]; /*0-99*/

     	snprintf(buff,2,"%d",count);

     	strcpy(cmd, "TUX_CMD:SPINNING:LEFT_ON:");
     	strcat(cmd,buff);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Fait x quart de tour sur la droite pendant <duration> seconde(s)
*/
TuxDrvError Tux_RotateRight_OnDuration(double duration)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_RotateRight_OnDuration(),duration=%f",duration);
        
	char cmd[40];
	snprintf(cmd,40,"TUX_CMD:SPINNING:RIGHt_ON_DURING:%.1f",duration);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Allume les leds
*/
TuxDrvError Tux_Led_On(leds_t LED)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Led_On, LED=%d",LED);
    
    switch(LED)
    {
        case 0: return E_TUXDRV_NOTVALID_LED; /* handle LED_NONE */
        case 0x01: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:ON:LED_LEFT,1.0");
        case 0x02: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:ON:LED_RIGHT,1.0");
        case 0x03: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:ON:LED_BOTH,1.0");
    }

	return E_TUXDRV_NOTVALID_LED;
}

/*
 Eteint les leds
*/
TuxDrvError Tux_Led_Off(leds_t LED)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Led_Off, LED=%d",LED);
    
    switch(LED)
    {
        case 0: return E_TUXDRV_NOTVALID_LED; /* handle LED_NONE */
        case 0x01: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:OFF:LED_LEFT,1.0");
        case 0x02: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:OFF:LED_RIGHT,1.0");
        case 0x03: return TuxDrv_PerformCommand(0.0, "TUX_CMD:LED:OFF:LED_BOTH,1.0");
    }

	return E_TUXDRV_NOTVALID_LED;
}

/*
 Fait clignoter les yeux
*/
TuxDrvError Tux_BlinkLeds(leds_t LED, int count, double delay)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_BlinkLeds, LED=%d - count=%d - delay=%d",LED,count,delay);
    
            
	char cmd[60];
    char tcmd[20];
	char led[12];

	strcpy(tcmd,"TUX_CMD:LED:BLINK:");

    switch(LED)
    {
        case 0: return E_TUXDRV_NOTVALID_LED; /* handle LED_NONE */
        case 0x01: snprintf(led,12,"LED_LEFT");
             break;
        case 0x02: snprintf(led,12,"LED_RIGHT");
             break;
        case 0x03: snprintf(led,12,"LED_BOTH");
             break;
    }

	snprintf(cmd,60,"%s%s,%d,%.1f",tcmd,led,count,delay);

	return TuxDrv_PerformCommand(0.0,cmd);
}

/*
 Fait "pulser" les yeux de Tux
*/
TuxDrvError Tux_PulseLeds(leds_t LED, double min_intensity, double max_intensity, byte count, double period, effect_type_t effect, double speed, byte step)
{
            
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_PulseLeds(), LED=%d - min_intentisyt=%d - max_intensity=%d - count=%d - perio=%f - effect=%s - speed=%f - step=%d",
       LED, min_intensity,max_intensity,count,period,effect,speed,step);
       
	char cmd[80];
	char led[12];
	char eff[15];

	char tcmd[20];
	snprintf(tcmd,20,"TUX_CMD:LED:PULSE:");

	switch(LED)
	{
		case 0: return E_TUXDRV_NOTVALID_LED; /* handle LED_NONE */
		case 0x01: snprintf(led,12,"LED_LEFT");
		break;
		case 0x02: snprintf(led,12,"LED_RIGHT");
		break;
		case 0x03: snprintf(led,12,"LED_BOTH");
		break;
	}


	switch(effect)
	{
		case UNAFFECTED: return E_TUXDRV_NOTVALID_EFFECT; /* handle UNAFFECTED */
		case LAST: snprintf(eff,15,"LAST");
		break;
		case NONE: snprintf(eff,15,"NONE");
		break;
		case DEFAULT: snprintf(eff,15,"DEFAULT");
		break;
		case FADE_DURATION: snprintf(eff,15,"FADE_DURATION");
		break;
		case FADE_RATE: snprintf(eff,15,"FADE_RATE");
		break;
		case GRADIENT_NBR: snprintf(eff,15,"GRADIENT_NBR");
		break;
		case GRADIENT_DELTA: snprintf(eff,15,"GRADIENT_DELTA");
	}

	snprintf(cmd,80,"%s%s,%.1f,%.1f,%d,%.1f,%s,%.1f,%d",tcmd,led,min_intensity,max_intensity,count,period,eff,speed,step);

	return TuxDrv_PerformCommand(0.0,cmd);
}


