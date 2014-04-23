/* =============== GPL HEADER =====================
 * TuxDriver.h
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

#ifndef __TUXDRIVER_H__
#define __TUXDRIVER_H__

#include <TuxVersion.h>
#include <TuxServer.h>


#define TUXDRIVER_AUTHOR "Joel Matteotti" /* développeur initial (ne peut pas changer !) */

#define TUXDRIVER_MAINTENER "Joel Matteotti" /* développeur actuel (si abandon de ma part un jour :p) */

#include <stdio.h>
#include <pthread.h>
#include <inttypes.h> /* uintptr_t */


#ifdef _WIN32
	#include <windows.h> /* gestion des dll */
#else
	#include <dlfcn.h> /* gestion des .so */
#endif

typedef unsigned char byte; /* byte type */


/*
----------------------------
 FROM tux_driver.h
----------------------------
*/

/**
 * Id enumeration of high level status.
 */
typedef enum {
    SW_ID_FLIPPERS_POSITION = 0,
    SW_ID_FLIPPERS_REMAINING_MVM,
    SW_ID_SPINNING_DIRECTION,
    SW_ID_SPINNING_REMAINING_MVM,
    SW_ID_LEFT_WING_BUTTON,
    SW_ID_RIGHT_WING_BUTTON,
    SW_ID_HEAD_BUTTON,
    SW_ID_REMOTE_BUTTON,
    SW_ID_MOUTH_POSITION,
    SW_ID_MOUTH_REMAINING_MVM,
    SW_ID_EYES_POSITION,
    SW_ID_EYES_REMAINING_MVM,
    SW_ID_DESCRIPTOR_COMPLETE,
    SW_ID_RF_STATE,
    SW_ID_DONGLE_PLUG,
    SW_ID_CHARGER_STATE,
    SW_ID_BATTERY_LEVEL,
    SW_ID_BATTERY_STATE,
    SW_ID_LIGHT_LEVEL,
    SW_ID_LEFT_LED_STATE,
    SW_ID_RIGHT_LED_STATE,
    SW_ID_CONNECTION_QUALITY,
    SW_ID_AUDIO_FLASH_PLAY,
    SW_ID_AUDIO_GENERAL_PLAY,
    SW_ID_FLASH_PROG_CURR_TRACK,
    SW_ID_FLASH_PROG_LAST_TRACK_SIZE,
    SW_ID_TUXCORE_SYMBOLIC_VERSION,
    SW_ID_TUXAUDIO_SYMBOLIC_VERSION,
    SW_ID_FUXUSB_SYMBOLIC_VERSION,
    SW_ID_FUXRF_SYMBOLIC_VERSION,
    SW_ID_TUXRF_SYMBOLIC_VERSION,
    SW_ID_DRIVER_SYMBOLIC_VERSION,
    SW_ID_SOUND_REFLASH_BEGIN,
    SW_ID_SOUND_REFLASH_END,
    SW_ID_SOUND_REFLASH_CURRENT_TRACK,
    SW_ID_EYES_MOTOR_ON,
    SW_ID_MOUTH_MOTOR_ON,
    SW_ID_FLIPPERS_MOTOR_ON,
    SW_ID_SPIN_LEFT_MOTOR_ON,
    SW_ID_SPIN_RIGHT_MOTOR_ON,
    SW_ID_FLASH_SOUND_COUNT,
} SW_ID_DRIVER;

/**
 * Error codes
 */
#define TUX_ERROR_BEGIN         256
typedef int TuxDrvError;
typedef enum
{
    E_TUXDRV_NOERROR                = 0,
    E_TUXDRV_PARSERISDISABLED       = TUX_ERROR_BEGIN,
    E_TUXDRV_INVALIDCOMMAND,
    E_TUXDRV_STACKOVERFLOW,
    E_TUXDRV_FILEERROR,
    E_TUXDRV_BADWAVFILE,
    E_TUXDRV_INVALIDIDENTIFIER,
    E_TUXDRV_INVALIDNAME,
    E_TUXDRV_INVALIDPARAMETER,
    E_TUXDRV_BUSY,
    E_TUXDRV_WAVSIZEEXCEDED,

    E_TUXDRV_NOTVALID_LED, /* Choix de led invalide */
    E_TUXDRV_NOTVALID_EFFECT, /* effet de "pulse" invalide */
    E_TUXDRV_DRIVERNOTFOUND, /* driver non trouver */

    E_TUXDRV_ALREADYINITIALIZED, /* dÃ©jÃ  initialiser */
    E_TUXDRV_NOTINITIALIZED, /* non initialiser */    
} tux_drv_error_t;

/**
 * Simple callback definition.
 */
typedef void(*drv_simple_callback_t)(void);

/*=====================================================*/


/*
-----------------------------
 FROM tux_leds.h
-----------------------------
*/

/**
 * Type indicating which led should be affected by the command.
 * The left LED is affected to bit0 and the right LED is at bit1. This
 * simplifies comparisons. Assigning an hex value helps keep in mind the bit
 * relation.
 */
typedef enum
{
    LED_NONE = 0,
    LED_LEFT = 0x01,
    LED_RIGHT = 0x02,
    LED_BOTH = 0x03,
} leds_t;

/** Types of effects applied when changing the intensity of the LEDs. */
typedef enum
{
    UNAFFECTED,     /**< Don't update the effect parameters. This can either be
                      the last effect set by software, or by firmware in the
                      autonomous mode. This is probably not what you want. */
    LAST,           /**< Last effect requested by software. */
    NONE,           /**< Don't use effects, equivalent to on/off mode. */
    DEFAULT,        /**< Default effect which is a short fading effect. */
    FADE_DURATION,  /**< Fading effect, 'effect.speed' sets the duration (in
                      seconds) the effect will last. */
    FADE_RATE,      /**< Fading effect, 'effect.speed' sets the rate of the
                      effect. Its value represents the number of seconds it
                      takes to apply the effect from off to on. So the actual
                      effect duration will take less time than specified if the
                      intensity starts or ends at intermediate values.
                      Therefore this parameter guarantees a constant rate of
                      the effect, not the duration.
                      */
    GRADIENT_NBR,   /**< Gradient effect, the intensity changes gradually by a
                      number of steps given by 'effect.step'. 'effect.speed'
                      represents the number of seconds it should take to apply
                      the effect. */
    GRADIENT_DELTA, /**< Gradient effect, the intensity changes by a delta
                      value of 'effect.step'. 'effect.speed' represents the
                      number of seconds it should take to apply the effect. */
} effect_type_t;

/*============================================================*/


typedef void(*event_callback_t)(char *event); /*tux_sw_status.h*/


/*
 Handle de la DLL
*/
void* DLLHANDLE;


/*
-----------------------------
  Declarations des fonctions
------------------------------
*/

typedef void (*TuxDrv_Stop_t)(void);
TuxDrv_Stop_t TuxDrv_Stop;


typedef void (*TuxDrv_Start_t)(void);
TuxDrv_Start_t TuxDrv_Start;

typedef TuxDrvError (*TuxDrv_SetStatusCallback_t)(event_callback_t funct);
TuxDrv_SetStatusCallback_t TuxDrv_SetStatusCallback;

typedef void (*TuxDrv_SetEndCycleCallback_t)(drv_simple_callback_t funct);
TuxDrv_SetEndCycleCallback_t TuxDrv_SetEndCycleCallback;

typedef void (*TuxDrv_SetDongleConnectedCallback_t)(drv_simple_callback_t funct);
TuxDrv_SetDongleConnectedCallback_t TuxDrv_SetDongleConnectedCallback;

typedef void (*TuxDrv_SetDongleDisconnectedCallback_t)(drv_simple_callback_t funct);
TuxDrv_SetDongleDisconnectedCallback_t TuxDrv_SetDongleDisconnectedCallback;

typedef void (*TuxDrv_ResetDongle_t)(void);
TuxDrv_ResetDongle_t TuxDrv_ResetDongle;

typedef TuxDrvError (*TuxDrv_PerformCommand_t)(double delay, const char *cmd_str);
TuxDrv_PerformCommand_t TuxDrv_PerformCommand;

typedef TuxDrvError (*TuxDrv_PerformMacroText_t)(const char *cmd_macro);
TuxDrv_PerformMacroText_t TuxDrv_PerformMacroText;

typedef TuxDrvError (*TuxDrv_PerformMacroFile_t)(const char *macro_file);
TuxDrv_PerformMacroFile_t TuxDrv_PerformMacroFile;

typedef void (*TuxDrv_ClearCommandStack_t)(void);
TuxDrv_ClearCommandStack_t TuxDrv_ClearCommandStack;

typedef void (*TuxDrv_ResetPositions_t)(void);
TuxDrv_ResetPositions_t TuxDrv_ResetPositions;

typedef TuxDrvError (*TuxDrv_GetStatusName_t)(int id, char* name);
TuxDrv_GetStatusName_t TuxDrv_GetStatusName;

typedef TuxDrvError (*TuxDrv_GetStatusId_t)(char* name, int *id);
TuxDrv_GetStatusId_t TuxDrv_GetStatusId;

typedef TuxDrvError (*TuxDrv_GetStatusState_t)(int id, char *state);
TuxDrv_GetStatusState_t TuxDrv_GetStatusState;

typedef TuxDrvError (*TuxDrv_GetStatusValue_t)(int id, char *value);
TuxDrv_GetStatusValue_t TuxDrv_GetStatusValue;

typedef void (*TuxDrv_GetAllStatusState_t)(char *state);
TuxDrv_GetAllStatusState_t TuxDrv_GetAllStatusState;

typedef TuxDrvError (*TuxDrv_SoundReflash_t)(const char *tracks);
TuxDrv_SoundReflash_t TuxDrv_SoundReflash;

typedef bool (*TuxDrv_Eyes_Off_t)(void);
TuxDrv_Eyes_Off_t TuxDrv_Eyes_Off;

typedef bool (*TuxDrv_Mouth_Off_t)(void);
TuxDrv_Mouth_Off_t TuxDrv_Mouth_Off;

typedef bool (*TuxDrv_Spinning_Off_t)(void);
TuxDrv_Spinning_Off_t TuxDrv_Spinning_Off;

typedef bool (*TuxDrv_Flippers_Off_t)(void);
TuxDrv_Flippers_Off_t TuxDrv_Flippers_Off;

typedef void (*TuxDrv_Update_Light_t)(void);
TuxDrv_Update_Light_t TuxDrv_Update_Light;

typedef char * (*TuxDrv_GetFlashSound_Infos_t)(char *p);
TuxDrv_GetFlashSound_Infos_t TuxDrv_GetFlashSound_Infos;

/*
typedef void (*TuxDrv_SetLogLevel_t)(int log_level);
TuxDrv_SetLogLevel_t TuxDrv_SetLogLevel;

typedef void (*TuxDrv_SetLogTarget_t)(int target);
TuxDrv_SetLogTarget_t TuxDrv_SetLogTarget;
*/

/*==============================================*/

/*
----------------
  DONGLE
-----------------
*/
typedef void(*OnTuxDongleConnected_t)(void);
typedef void(*OnTuxDongleDisconnected_t)(void);

OnTuxDongleConnected_t OnTuxDongleConnected;
OnTuxDongleDisconnected_t OnTuxDongleDisconnected;
/*=======================*/


/*
-------------------
  Buttons
-------------------
*/
typedef void(*OnTuxLeftButtonPressed_t)(void); 
typedef void(*OnTuxRightButtonPressed_t)(void);
typedef void(*OnTuxHeadButtonPressed_t)(void);
typedef void(*OnTuxRemoteButtonPressed_t)(tux_client client, char *);
typedef void(*OnTuxRemoteButtonReleased_t)(void);


OnTuxLeftButtonPressed_t OnTuxLeftButtonPressed;
OnTuxRightButtonPressed_t OnTuxRightButtonPressed;
OnTuxHeadButtonPressed_t OnTuxHeadButtonPressed;
OnTuxRemoteButtonPressed_t OnTuxRemoteButtonPressed;
OnTuxRemoteButtonReleased_t OnTuxRemoteButtonReleased;
/*==============================================*/


/*
------------------
 Charger
------------------
*/
typedef void(*OnTuxChargerPlugged_t)(void);
typedef void(*OnTuxChargerUnPlugged_t)(void);

OnTuxChargerPlugged_t OnTuxChargerPlugged;
OnTuxChargerUnPlugged_t OnTuxChargerUnPlugged;
/*==============================================*/


/*
-----------------
End Cycle
-----------------
*/
typedef void(*OnTuxEndCycle_t)(void);
OnTuxEndCycle_t OnTuxEndCycle;
/*==============================================*/


void *IMPORT_FUNC(void *HANDLE, const char *func);
void TuxDriver_Stop(void);
TuxDrvError TuxDriver_Initialize(void);
void* TuxDriver_DrvStart(void *data);
const char *TuxDriver_strerror(TuxDrvError error_code);
void StatusEventCallback(char *status_str);
void Tux_DongleConnected(void);
void Tux_DongleDisconnected(void);
void Tux_ResetDongle(void);
TuxDrvError Tux_PlayFlashSound(byte sound, byte volume);
TuxDrvError Tux_SetCallbackEvent(event_callback_t funct);
void Tux_SetEndCycleCallbackEvent(drv_simple_callback_t funct);
void EndCycleEventCallback(void);
void TuxDriver_Stop(void);
void TuxDriver_Start(void);
TuxDrvError Tux_SoundReflash(const char *tracks);
TuxDrvError Tux_SetFlippersSpeed(byte speed);
TuxDrvError Tux_Flippers_Up(void);
TuxDrvError Tux_Flippers_Down(void);
TuxDrvError Tux_FlippersUpDown(int count);
TuxDrvError Tux_FlippersUpDown_OnDuration(double duration);
TuxDrvError Tux_Open_Eyes(void);
TuxDrvError Tux_Close_Eyes(void);
TuxDrvError Tux_OpenClose_Eyes(int count);
TuxDrvError Tux_OpenClose_Eyes_OnDuration(double duration);
TuxDrvError Tux_Open_Mouth(void);
TuxDrvError Tux_Close_Mouth(void);
TuxDrvError Tux_OpenClose_Mouth(int count);
TuxDrvError Tux_OpenClose_Mouth_OnDuration(double duration);
TuxDrvError Tux_SetRotationSpeed(byte speed);
TuxDrvError Tux_RotateLeft(int count);
TuxDrvError Tux_RotateLeft_OnDuration(double duration);
TuxDrvError Tux_RotateRight(int count);
TuxDrvError Tux_RotateRight_OnDuration(double duration);
TuxDrvError Tux_Led_On(leds_t LED);
TuxDrvError Tux_Led_Off(leds_t LED);
TuxDrvError Tux_BlinkLeds(leds_t LED, int count, double delay);
TuxDrvError Tux_PulseLeds(leds_t LED, double min_intensity, double max_intensity, byte count, double period, effect_type_t effect, double speed, byte step);

#endif
