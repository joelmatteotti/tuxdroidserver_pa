/* =============== GPL HEADER =====================
 * TuxServer.h (part of TuxDroidServer)
 *
 * Copyleft (C) 2012 - Joel Matteotti <joel _DOT_ matteotti _AT_ free _DOT_ fr>
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


#ifndef __TUXSERVER_H__
#define __TUXSERVER_H__

#include <TuxVersion.h>

#define TUXSERVER_AUTHOR "Joel Matteotti" /* développeur initial (ne peut pas changer !) */

#define TUXSERVER_MAINTENER "Joel Matteotti" /* développeur actuel (si abandon de ma part un jour :p) */


#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef size_t socklen_t;
#else
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
typedef unsigned short u_short;
#endif

#include <stdbool.h>

/* Clients Structure */
typedef struct
{
   int id;
   char *username; /* user name */
   char *uKey; /*user key */
 
   #ifdef _WIN32
   SOCKET sock;
   #else
   int sock;
   #endif
} tux_client_t;
typedef tux_client_t *tux_client;

/* structure regroupant les données nécessaire
pour synchroniser les mouvements de bouche avec la lecture d'un TTS */
typedef struct
{
    char *phr;
    int count;
} syncMouthMov_t;
typedef syncMouthMov_t *syncMouthMov;


/* Prototypes */
void* ReadClient(void *data);
void SendMsgToAll(char *msg);
void SendMsgToClient(tux_client client, char *message);
void ParseCommand(tux_client client, char *cmd);
void loadConfigFile(void);
void *startServer(void *data);
void stopServer(void);
bool InitializeTuxDroid();
void onRemoteButtonPressed(tux_client client, char *button);
void onLeftButtonPressed();
void onRightButtonPressed();
void onHeadButtonPressed();
void onChargerPlugged();
void onChargerUnPlugged();
void onDongleConnected();
void onDongleDisconnected();
void *syncMouthMovements(void *data);
void remoteControl(char *button);


#endif
