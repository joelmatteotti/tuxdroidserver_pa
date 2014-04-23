/* =============== GPL HEADER =====================
 * TuxDroidServer
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

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h> /* lrint() */

#include <TuxCompat.h>
#include <TuxTime.h>
#include <TuxLogger.h>
#include <TuxDriver.h>
#include <TuxAudio.h>
#include <TuxServer.h>
#include <TuxStrings.h>
#include <TuxAttitune.h>
#include <TuxDownloader.h>

int SERVER_PORT = -1;                                  /* Port d'écoute */
char *USER_KEY;                                        /* user key */

tux_client* clients=NULL;                                   /* pointeur pour tableau de type tux_client */

int nClients=0;                                        /* nombre actuel de clients */
int isRunning=0;                                       /* Permet de savoir si le server est démarrer ou non */

static bool server_started=false;

/* Fonction de lecture des données arrivant des clients */
void* ReadClient(void *data)
{
	TuxLogger_writeLog(TUX_LOG_DEBUG,"Début de ReadClient()");

	if(data == NULL)
		return NULL;

    tux_client client = (tux_client)data;
	
	if(client == NULL)
	{
		printf("client is null\n");
		return NULL;
	}

	char *buff = (char *)malloc(sizeof(char)*2048);
    
    int iResult;
    int isAlive=0;
    
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Les variables de ReadClient() sont intiailisées");

    TuxLogger_writeLog(TUX_LOG_DEBUG,"Début de la lecture des données pour le client %d",client->id);
	
	do
   	{
    	iResult = recv(client->sock, buff, 2048, 0);

        if(iResult > 0)
        {
			isAlive=1;
			char *cmd = (char *)malloc(sizeof(char)*(iResult+1));
			snprintf(cmd,iResult+1,"%s",buff);
			TuxLogger_writeLog(TUX_LOG_DEBUG, "Donnée brut recue: %s",cmd);
			
			if(buff != NULL)
			{
				/*free(buff);
				buff=NULL;*/
			}
           		
           		ParseCommand(client,trim(cmd));
           		
			if(cmd != NULL)
			{
           		/*free(cmd);
				cmd=NULL;*/
			}
           		
       	}
       	else
			isAlive=0;

		break;
 	} 
	while(iResult > 0);

	if(isAlive == 1)
	{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Le client est toujours en vie on continue de surveiller les données pouvant arrivées du client");
		ReadClient(data);
	}
    else
	{
        TuxLogger_writeLog(TUX_LOG_DEBUG,"La connexion avec le client a été fermer par le client %d, on ferme la socket",client->id);
    	close(client->sock);
  	}
  
  	if(client->uKey != NULL)
	{
		/*ree(client->uKey);
		client->uKey = NULL;*/
	}

	if(client->username != NULL)
	{
  		/*free(client->username);
		client->username=NULL;*/
	}

	if(client != NULL)
	{
  		/*free(client);
		client=NULL;*/
	}

	if(data != NULL)
	{
  		/*free(data);
		data=NULL;*/
	}
  
    return 0;
}

/* envoie un message à tout les clients */
void SendMsgToAll(char *msg)
{
     if(!server_started)
         return;

     TuxLogger_writeLog(TUX_LOG_DEBUG,"Envoie d'un message par le sever à tout les clients: %s",msg);
 
 
 	 int dSize = strlen(msg) + 2;
     char *data = (char *)malloc(sizeof(char)*dSize);
     
     sprintf(data,"%s\r",msg);


     int i;
     for(i = 0; i < nClients; i++)
     {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Vérifie si la connexion avec le client %d est toujours active..",clients[i]->id);
        if(clients[i]->sock)
        {
            TuxLogger_writeLog(TUX_LOG_DEBUG,"la connexion est toujours active !");
            if(!strcmp(clients[i]->uKey,USER_KEY))
            {
                TuxLogger_writeLog(TUX_LOG_DEBUG,"Le client %d à une clef correcte (%s) on envoie le message :)",i,clients[i]->uKey);
                send(clients[i]->sock,data,dSize,0);
            }
            else
            {
                TuxLogger_writeLog(TUX_LOG_DEBUG,"Le client %d a une clef invalide (%s), on le deconnecte",clients[i]->id,clients[i]->uKey);
                if(clients[i]->sock)
                {
                	close(clients[i]->sock);
			
			if(clients[i]->uKey != NULL)
			{
				/*free(clients[i]->uKey);
				clients[i]->uKey =NULL;*/
			}

			if(clients[i]->username != NULL)
			{
  				/*free(clients[i]->username);
				clients[i]->username=NULL;*/
			}

			if(clients[i] != NULL)
			{
  				/*free(clients[i]);
				clients[i] = NULL;*/
			}
		 }
	   }
        }
        else
        {
        	TuxLogger_writeLog(TUX_LOG_DEBUG,"La connexion n'est plus active on ferme la socket");
            	close(clients[i]->sock);
            	
		if(clients[i]->uKey != NULL)
		{
			/*free(clients[i]->uKey);
			clients[i]->uKey=NULL;*/
		}

		if(clients[i]->username != NULL)
		{
  	    		/*free(clients[i]->username);
  	    		clients[i]->username=NULL;*/
		}

		if(clients[i] != NULL)
		{
			/*free(clients[i]);
			clients[i] = NULL;*/
		}
        }
     }
     
	if(data != NULL)
	{
     		free(data);
		data=NULL;
	}
}

/* envoie un message à un client spécifique */
void SendMsgToClient(tux_client client, char *message)
{
    if(!server_started)
        return;

    TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)Envoie d'un message du server au client %d message: %s",client->id,message);

	int dSize = sizeof(message) + 2;
    
    char *data = (char *)malloc(sizeof(char)*dSize);
   	    
    sprintf(data,"%s\r",message);
    

    TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)Vérifie si la connexion avec le client %d est toujours active..",client->id);

    if(client->sock)
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)La connexion est toujours active");

        if(!strcmp(client->uKey,USER_KEY))
        {
            TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)Le client %d a une clef correcte (%s) on envoie le message :)",client->id,client->uKey);
            send(client->sock, data, strlen(data), 0);
            TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)Le message est partie :)");
        }
        else
        {
            TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)Le client %d a une clef invalide (%s) on le déconnecte",client->id,client->uKey);

            if(client->sock)
                close(client->sock);
        }
    }
    else
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"(X)La connexion avec le client %d n'est plus active on ferme la socket",client->id);
        close(client->sock);
    }
    
	if(data != NULL)
	{
    	/*free(data);
		data=NULL;*/
	}
}

/* chargement de la configuration */
void loadConfigFile()
{
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Fonction loadConfigFile()");
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Ouverture du fichier de configuration");

     char *pch=NULL;
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Apres pch");
	 char *buff = (char *)malloc(sizeof(char)*1024);
     
     #ifdef _WIN32
     FILE *fp = fopen("config.txt","r");
     #else
     FILE *fp = fopen("/etc/tuxdroidserver/config.txt","r");
     #endif

     int log_level = -1;

     if(!fp)
     {
            TuxLogger_writeLog(TUX_LOG_ERROR,"Le fichier de configuration est introuvable !!");
            TuxLogger_writeLog(TUX_LOG_ERROR,"Ecriture d'un fichier de configuration par défaut..");

            #ifdef _WIN32
            fp = fopen("config.txt","w");
            #else
            fp = fopen("/etc/tuxdroidserver/config.txt","w");
            #endif

            if(fp)
            {
                fputs("SERVER_PORT=9595\n\0",fp);
                fputs("USER_KEY=test\n\0",fp);
                fputs("LOG_LEVEL=0\n\0",fp);
                fclose(fp);
            }
            else
                TuxLogger_writeLog(TUX_LOG_ERROR,"Impossible de créer le fichier de configuration config.txt !!");

            TuxLogger_writeLog(TUX_LOG_DEBUG,"Réglage des paramètre sur les valeurs par défaut");

            SERVER_PORT = 9595;
            USER_KEY = (char *)malloc(strlen("test")*sizeof(char));
            strcpy(USER_KEY,"test");
            TuxLogger_setLevel(TUX_LOG_DEBUG);
     }
     else
     {
	char **param;
      while(!feof(fp))
         {
            fgets(buff,1024,fp);
            
            if(strcmp(buff,""))
            {
		if(strstr(buff,"="))
		{
			param = explode(buff,'=');

			if(!strcmp(param[0],"SERVER_PORT"))
			{
				TuxLogger_writeLog(TUX_LOG_DEBUG,"SERVER PORT DEFINI A %s",param[1]);
				SERVER_PORT = atoi(trim(param[1]));
			}

			if(!strcmp(param[0],"USER_KEY"))
			{
				TuxLogger_writeLog(TUX_LOG_DEBUG,"USER_KEY defini a %s",param[1]);
				USER_KEY = (char *)malloc(sizeof(char)*strlen(trim(param[1])));
				strcpy(USER_KEY,trim(param[1]));
			}

			if(!strcmp(param[0],"LOG_LEVEL"))
			{
				TuxLogger_writeLog(TUX_LOG_DEBUG,"LOG LEVEL DEFINI A %s",param[1]);
				TuxLogger_setLevel(atoi(trim(param[1])));
			}
		}
            }
         }

         TuxLogger_writeLog(TUX_LOG_DEBUG,"Fin de la lecture du fichier config.txt");
         fclose(fp);
         
     }
     
	if(pch != NULL)
	{
     	/*free(pch);
		pch=NULL;*/
	}

	if(buff != NULL)
	{
		/*free(buff);
		buff=NULL;*/
	}
}

/* initialise et démarre le server */
void *startServer(void *data)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Appelle de la fonction d'initialisation de TuxDroid");

	if(!InitializeTuxDroid()) 	/* Initialisation TuxDroidInterface */
    {
        TuxLogger_writeLog(TUX_LOG_INFO,"Impossible d'initialiser TuxDroid :(");
        printf("\nImpossible de démarre le server, l'intialisation de TuxDroid à échouée !\n");
        printf("TuxDroidServer> ");
    }
    
    
    #ifdef _WIN32
    Sleep(2000);
    #else
    TuxSleep(2000);
    #endif

    TuxLogger_writeLog(TUX_LOG_DEBUG,"Démarrage du server...");

    TuxLogger_writeLog(TUX_LOG_DEBUG,"Initialisation des sockets..");

    /* Initialisation du server TCP */
	#ifdef _WIN32
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2,0), &WSAData);
	#endif

	int sock;
	int csock;
	struct sockaddr_in sin;
	struct sockaddr_in csin;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_family = AF_INET;
	sin.sin_port = htons((u_short)SERVER_PORT);

	bind(sock, (struct sockaddr *)&sin, sizeof(sin));
	listen(sock, 0);

	socklen_t sinsize = sizeof(csin);

    TuxLogger_writeLog(TUX_LOG_DEBUG,"Initialization des sockets terminer");

    isRunning=1;

    server_started=true;
    
	pthread_t SocketThreads;          
	pthread_attr_t SocketThreads_attr;

	/* En attente de connexions */
	while(isRunning)
	{
    	if((csock = accept(sock, (struct sockaddr *)&csin, &sinsize)))
    	{
    		clients = (tux_client*) realloc(clients,sizeof(tux_client)*(nClients+1));
        
            TuxLogger_writeLog(TUX_LOG_DEBUG,"Nouvelle connexion au server acceptée");
            TuxLogger_writeLog(TUX_LOG_DEBUG,"IP Client: %s",inet_ntoa(csin.sin_addr));


            TuxLogger_writeLog(TUX_LOG_DEBUG,"Création d'un nouveau client -> %d !",nClients);
    		clients[nClients] = (tux_client)malloc(sizeof(tux_client_t)); /* allocation mémoire */
    
	        TuxLogger_writeLog(TUX_LOG_DEBUG,"Nouveau client créer");

    		clients[nClients]->id = nClients;
    		clients[nClients]->sock = csock;
			clients[nClients]->uKey = (char *)malloc(sizeof(char));
    		strcpy(clients[nClients]->uKey,"0");

    		clients[nClients]->username = (char *)malloc(sizeof(char));
    		strcpy(clients[nClients]->username,"0");

    		TuxLogger_writeLog(TUX_LOG_DEBUG,"on essaye de démarrer le nouveau threaad");
            
            
		pthread_attr_init(&SocketThreads_attr);
			pthread_attr_setdetachstate(&SocketThreads_attr, PTHREAD_CREATE_DETACHED);
			pthread_create(&SocketThreads,&SocketThreads_attr,ReadClient,clients[nClients]);


            TuxLogger_writeLog(TUX_LOG_DEBUG,"Nouveau thread démarrer pour le client %d",nClients);


    		nClients++;
            
        }
	}

	/* si on arrive ici c'est que le server a été arrêter, on ferme donc toutes les sockets
    qui on étés ouvertes */
    if(csock)
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Fermeture de la socket client temporaire");
        close(csock);
    }

    if(sock)
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Fermeture de la socket du server");
        close(sock);
    }
	
    #ifdef _WIN32
    WSACleanup();
    #endif
    
	return 0;
}

int main(void)
{
    loadConfigFile();

    char ch='\0';
    char buffer[5];
    int char_count;

    printf("*********************************************************\n");
    printf("* TuxDroidServer (Version %d.%d.%d) revision %d\t\t*\n",TUXSERVER_MAJOR,TUXSERVER_MINOR,TUXSERVER_REVIS,TDS_REVISION);
    printf("*\t\t\t\t\t\t\t*\n");
    printf("* Développeur du projet: %s\t\t\t*\n",TUXSERVER_AUTHOR);
    #ifdef TUXSERVER_MAINTENER
    printf("*\t\t\t\t\t\t\t*\n");
    printf("* Mainteneur TuxDroidServer: %s\t\t*\n",TUXSERVER_MAINTENER);
    #else
    printf("*\t\t\t\t\t\t\t*\n");
    #endif
    #ifdef TUXDRIVER_MAINTENER
    printf("* Mainteneur TuxDriver.....: %s\t\t*\n",TUXDRIVER_MAINTENER);
    #endif
    #ifdef TUXAUDIO_MAINTENER
    printf("* Mainteneur TuxAudio......: %s\t\t*\n",TUXAUDIO_MAINTENER);
    #endif
    printf("*\t\t\t\t\t\t\t*\n");
    printf("* https://sourceforge.net/projects/tuxdroidserver/\t*\n");
    printf("*********************************************************\n\n");

    TuxLogger_writeLog(TUX_LOG_INFO,"TuxDroidServer v%d.%d.%d démarrer.",TUXSERVER_MAJOR,TUXSERVER_MINOR,TUXSERVER_REVIS);

    printf("start - Démarre le server\n");
    printf("stop  - Arrête le server\n\n");

    TuxLogger_writeLog(TUX_LOG_INFO, "Server en attente d'ordre...");
    
    do
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"\n\nBOUCLE BOUCLE\n\n");
        printf("TuxDroidServer> ");
        ch = getchar();
        char_count = 0;
        while( (ch != '\n') && (char_count < 5))
        {
            buffer[char_count++] = ch;
            ch = getchar();
        }
        buffer[char_count] = 0x00; /*NULL*/

        TuxLogger_writeLog(TUX_LOG_DEBUG,"Commande recue ------> %s",buffer);

        if(!strcmp(buffer,"start"))
        {

            TuxLogger_writeLog(TUX_LOG_DEBUG,"demande de démarrage du server ! buffer -----> %s",buffer);

            if(!server_started)
            {
		        TuxLogger_writeLog(TUX_LOG_DEBUG,"préparation du thread principale du server...");
		        pthread_attr_t serverThread_attr;
				pthread_t serverThread;
                pthread_attr_init(&serverThread_attr);
                pthread_attr_setdetachstate (&serverThread_attr, PTHREAD_CREATE_DETACHED);
                TuxLogger_writeLog(TUX_LOG_DEBUG,"Péparation du thread principal terminer");
                pthread_create (&serverThread, &serverThread_attr, startServer, NULL);
                TuxLogger_writeLog(TUX_LOG_DEBUG,"Thread principal démarrer");
            }
            else
            {
                TuxLogger_writeLog(TUX_LOG_DEBUG,"Le server est déjà démarrer !");
                printf("Le server est déjà démarrer !\n");
            }

        }

        if(!strcmp(buffer,"stop"))
        {
            TuxLogger_writeLog(TUX_LOG_INFO,"Arrêt du server demander");
            server_started=false;
            break;
        }
    }
    while(1);

    TuxLogger_writeLog(TUX_LOG_INFO,"Début d'arrêt du server...");

    /* arrêt du server */
    /* fermeture des sockets clientes */
    /* fermeture de la socket server */
    /* fermeture de l'application */

    printf("\nArrêt du server en cours...");

    isRunning=0; /* indique qu'on stop le server */

    /* on attent quelques secondes pour être sur que les premières sockets soient fermées */
    #ifdef _WIN32
    Sleep(2000); /* 2 seconde */
    #else
    TuxSleep(2000); /* 2 secondes */
    #endif

    printf("\nFermeture des connexions clientes...");

    TuxLogger_writeLog(TUX_LOG_INFO,"Fermeture des connexion...");

    int i;
    for(i = 0; i < nClients; i++)
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Vérifie si la connexion du client %d est active",clients[i]->id);
        if(clients[i]->sock)
        {
          	TuxLogger_writeLog(TUX_LOG_DEBUG,"Ferme la connexion pour le client %d",clients[i]->id);
           	close(clients[i]->sock);
              	
			if(clients[i]->uKey != NULL)
			{
				/*free(clients[i]->uKey);
				clients[i]->uKey=NULL;*/
	  	 	}
	
			if(clients[i]->username != NULL)
			{
				/*free(clients[i]->username);
				clients[i]->username=NULL;*/
	 		}
		
			if(clients[i] != NULL)
			{
				/*free(clients[i]);
				clients[i] = NULL;*/
			}
        }
    }

/*
    free(SocketThreads);
*/
    /* on attent encore 2 secondes pour être sur que toutes les connexions soit bien fermées avant d'arrpeter le driver */
    #ifdef _WIN32
    Sleep(2000); /* 2 secondes */
    #else
    TuxSleep(2000); /* 2 seconds */
    #endif


    printf("\nArrêt de tout mouvements des moteurs de TuxDroid...");

    TuxLogger_writeLog(TUX_LOG_INFO,"Arrêt de tout mouvement de TuxDroid..");

    TuxDrv_Mouth_Off();
    TuxDrv_Spinning_Off();
    TuxDrv_Flippers_Off();

	#ifdef _WIN32
    TuxDrv_Eyes_Off(); /* désactiver temporairement sous linux pour éviter le crash dû au problème de corruption mémoire provoqué par le driver */
    #endif


    #ifdef _WIN32
    Sleep(2000); /* 2 secondes */
    #else
    TuxSleep(2000); /* 2 seconds */
    #endif


    printf("\nReplace les moteurs à leurs positions initiales...");


    TuxLogger_writeLog(TUX_LOG_INFO,"Ré-intialisation des positions de chaque moteuurs");

    TuxDrv_ResetPositions();

    #ifdef _WIN32
    Sleep(5000); /* 5 secondes */
    #else
    TuxSleep(5000); /* 5 seconds */
    #endif

	printf("\nArrêt du système audio...\n\n");
	TuxAudio_StopMusic();
	TuxAudio_StopTTS();
    TuxAudio_Terminate();
	
    #ifdef _WIN32
    Sleep(2000); /* 2 secondes */
    #else
    TuxSleep(2000); /* 2 seconds */
    #endif

    printf("\nArrêt du driver TuxDroid...\n\n");

    TuxLogger_writeLog(TUX_LOG_INFO,"Arrêt du driver TuxDroid");

    /* arrêt du driver */
    TuxDrv_Stop();

    /* encore un petit temps d'attente pour avoir le temps de lire le message du driver ce qui peut être nécessaire en cas d'erreur :) */
    #ifdef _WIN32
    Sleep(2000); /* 2 secondes */
    #else
    TuxSleep(2000); /* 2 seconds */
    #endif
    /* FIN DU PROGRAMME */

	return 0;
}


/* ============================= TUX DROID PART ================================= */

/* Initialisation du driver TuxDroid et de TuxAudio */
bool InitializeTuxDroid()
{
     TuxLogger_writeLog(TUX_LOG_INFO,"Initialisation de TuxDroid");

 /*    TuxDrv_SetLogLevel(0); */ /* DEBUG */ /* ne pas utiliser pour le moment le handle de la fonction est buguer :/ */


     if(TuxDriver_Initialize() != E_TUXDRV_NOERROR) /* initialisation du driver */
         return false;

	TuxLogger_writeLog(TUX_LOG_DEBUG,"Apres appelle de init du driver");


	 if(TuxAudio_Initialize() != E_TUXAUDIO_NOERROR)
	 	return false;

     TuxLogger_writeLog(TUX_LOG_DEBUG,"Définition des fonctions d'événéments..");

     /* Gestion des boutons du TuxDroid */
     OnTuxRemoteButtonPressed = onRemoteButtonPressed;
     OnTuxLeftButtonPressed = onLeftButtonPressed;
     OnTuxRightButtonPressed = onRightButtonPressed;
     OnTuxHeadButtonPressed = onHeadButtonPressed;
     OnTuxChargerPlugged = onChargerPlugged;
     OnTuxChargerUnPlugged = onChargerUnPlugged;
     OnTuxDongleConnected = onDongleConnected;
     OnTuxDongleDisconnected = onDongleDisconnected;

     TuxLogger_writeLog(TUX_LOG_DEBUG,"Les fonctions d'événements on été définies");
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Tentative de démarrage du driver");

     TuxDriver_Start(); /* démarrage du driver */

    #ifdef _WIN32
    Sleep(1000); /* wait driver */
    #else
    TuxSleep(1000);
    #endif

    return true;
}

/* Gestion des events */
void onRemoteButtonPressed(tux_client client, char *button)
{
    char *cmd = (char *)malloc(sizeof(char)*(strlen(button) + 11));
  
    if(client != NULL && strcmp(client->username,"0"))
        sprintf(cmd,"#TUXREMOTE:%s,%s",button,client->username);
    else
        sprintf(cmd,"#TUXREMOTE:%s",button);
  
    SendMsgToAll(cmd);
    
	if(cmd != NULL)
	{
    	/*free(cmd);
		cmd=NULL;*/
	}
}

void onLeftButtonPressed() { SendMsgToAll("#TUXBUTTON:LEFT"); }
void onRightButtonPressed() { SendMsgToAll("#TUXBUTTON:RIGHT"); }
void onHeadButtonPressed() { SendMsgToAll("#TUXBUTTON:HEAD"); }
void onChargerPlugged() { SendMsgToAll("#TUXCHARGER:PLUGGED"); }
void onChargerUnPlugged() { SendMsgToAll("#TUXCHARGER:UNPLUGGED"); }
void onDongleConnected() { SendMsgToAll("#TUXDONGLE:CONNECTED"); }
void onDongleDisconnected() { SendMsgToAll("#TUXDONGLE:DISCONNECTED"); }

/*
  DEFINITION DES COMMANDES TUXSERVER:

  Tux_Open()
  Tux_Close()
  Tux_OpenClose()
  Tux_Flippers()
  Tux_Rotate()

  Tux_TTS()
  Tux_Flash()

  Tux_State()

  Tux_Micro()
  Tux_Audio()

  Tux_PlayAtt()

  ....

*/


/* parse les commandes recus par les clients */
void ParseCommand(tux_client client, char *rawcmd)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Nouvelle commande recue du client %d",client->id);
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Commande brut ---> %s",rawcmd);

	char valeur[256]; /* buffer pour getStateValue */


    if(!strchr(rawcmd,'(') && !strchr(rawcmd,')')) /* commande mal formatée / invalide */
        return;
        
    TuxLogger_writeLog(TUX_LOG_DEBUG,"Avant l'explode");
    
    char *cmd;
    char **temp = explode(rawcmd,'(');

	TuxLogger_writeLog(TUX_LOG_DEBUG,"Après l'explode");

    if(temp[0] != NULL)
        cmd = temp[0];
    else
        return;


    int argc=0;
    char **argv=NULL;

	

	if(!strcmp(temp[1],")"))
	{
		/* aucun paramètre */
		TuxLogger_writeLog(TUX_LOG_DEBUG,"La commande n'a aucun paramètre !");
	}
    else
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"La commande a des paramètres");

        char *pch;
        pch = strtok(temp[1],")");
        
        
        if(pch != NULL)
        {
	        if(strchr(pch,','))
	        {
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"La commande a plusieurs paramètre");
	            argv = explode(pch,',');
	            argc = countCharacterOccurency(pch,',') + 1;
	            int i;
	            for(i = 0; i < argc; i++)
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"argv[%d] -----> %s",i,argv[i]);
	        }
	        else
	        {
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"La commande n'a qu'un seul paramètre");
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"-----> %s",pch);
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"Allocation de argv");
	            argv = (char **)malloc(sizeof(char *)*strlen(trim(pch)));
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"Allocation de argv[0]");
	            argv[0] = (char *)malloc(sizeof(char)*strlen(trim(pch)));
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"Copie de pch dans argv[0]");
	            strcpy(argv[0],trim(pch));
	            TuxLogger_writeLog(TUX_LOG_DEBUG,"Incrémente argc");
	            argc++;
	        }
		}
		
		if(pch != NULL)
		{
			/*free(pch);
			pch = NULL;*/
		}
    }
    
        if(!strcmp(strtolower(cmd),"tux_getmicro"))
        {
            SendMsgToClient(client,TuxAudio_getMicroName());
        }
        
        if(!strcmp(strtolower(cmd),"tux_getsoundcard"))
        {
            SendMsgToClient(client, TuxAudio_getSoundCardName());
        }

    	/* arrête tout mouvement des moteurs (indépendement les uns des autres)*/
    	if(!strcmp(strtolower(cmd),"tux_off"))
    	{
            if(!strcmp(strtolower(argv[0]),"mouth"))
                TuxDrv_Mouth_Off();
            if(!strcmp(strtolower(argv[0]),"spin"))
                TuxDrv_Spinning_Off();
            if(!strcmp(strtolower(argv[0]),"flippers"))
                TuxDrv_Flippers_Off();
            if(!strcmp(strtolower(argv[0]),"eyes"))
                TuxDrv_Eyes_Off();
        }
        TuxLogger_writeLog(TUX_LOG_DEBUG,"cmd #1 ok");
        
        if(!strcmp(strtolower(cmd),"tux_reset"))
        {
			TuxDrv_Mouth_Off();
			TuxDrv_Spinning_Off();
			TuxDrv_Flippers_Off();
			TuxDrv_Eyes_Off();
			
			#ifdef _WIN32
			Sleep(2000);
			#else
			TuxSleep(2000);
			#endif
			
			TuxDrv_ResetPositions();
		}

		if(!strcmp(strtolower(cmd),"tux_timestamp"))
		{
    		char *msg = (char *)malloc(sizeof(char)*15);
    		sprintf(msg,"%lu",Tux_TimeStamp());
    		
    		SendMsgToClient(client,msg);
    		
		if(msg != NULL)
		{
    		/*free(msg);
			msg=NULL;*/
		}
	}
		        
    	if(!strcmp(strtolower(cmd),"tux_user"))
    	{
			if(argv[0] == NULL)
				return;
				
            TuxLogger_writeLog(TUX_LOG_DEBUG,"(re)definition du nom d'utilisateur -> %s pour le client %d",argv[0],client->id);

            client->username = (char *)realloc(client->username,strlen(argv[0])*sizeof(char));
            strcpy(client->username,argv[0]);
        }

        
        /* simulation d'un appuie sur un bouton de la télécommande */
        if(!strcmp(strtolower(cmd),"tux_remote"))
        {
			if(argv[0] != NULL)
            	onRemoteButtonPressed(client, argv[0]); /* on simule l'appuie du bouton */
        }
        

    	if(!strcmp(strtolower(cmd),"tux_key"))
    	{
			if(argv[0] == NULL)
				return;
				
            TuxLogger_writeLog(TUX_LOG_DEBUG,"(re)définition de la clef -> %s pour le client %d",argv[0], client->id);

            client->uKey = (char *)realloc(client->uKey,strlen(argv[0])*sizeof(char));
            strcpy(client->uKey,argv[0]);
        }


        TuxLogger_writeLog(TUX_LOG_DEBUG,"Vérification de la validité de la clef d'identification");
        
        if(client == NULL)
        	TuxLogger_writeLog(TUX_LOG_DEBUG,"ERREUR Le client est NULL");
        	
        if(client->uKey == NULL)
        	TuxLogger_writeLog(TUX_LOG_DEBUG,"uKey est NULL");
        	

	if(USER_KEY == NULL)
		TuxLogger_writeLog(TUX_LOG_DEBUG,"USER_KEY est null :(");

	TuxLogger_writeLog(TUX_LOG_DEBUG,"Pas d'erreur on vas vÃ©rifier la clef");




    	if(strcmp(client->uKey,USER_KEY))
        {
            TuxLogger_writeLog(TUX_LOG_DEBUG,"La clef est invalaide ! On déconnecte le client %d !",client->id);
            if(client->sock)
                close(client->sock);
        }

    	if(!strcmp(strtolower(cmd),"tux_playatt"))
    	{
			if(argv[0] == NULL)
				return;
			
            TuxAttitune_loadAttitune(argv[0]);
        }

    	if(!strcmp(strtolower(cmd),"tux_close"))
    	{
			if(argv[0] == NULL)
				return;
			
        	if(!strcmp(strtolower(argv[0]),"eyes"))
           		Tux_Close_Eyes();
        	if(!strcmp(strtolower(argv[0]),"mouth"))
           		Tux_Close_Mouth();
    	}

    	if(!strcmp(strtolower(cmd),"tux_open"))
    	{
			if(argv[0] == NULL)
				return;
				
       		if(!strcmp(strtolower(argv[0]),"eyes"))
          		Tux_Open_Eyes();
       		if(!strcmp(strtolower(argv[0]),"mouth"))
          		Tux_Open_Mouth();
    	}

    	if(!strcmp(strtolower(cmd),"tux_openclose"))
    	{
            int count;
            if(!(count = atoi(argv[1])))
       		   return;

       		if(!strcmp(strtolower(argv[0]),"eyes"))
          		Tux_OpenClose_Eyes(count);
       		if(!strcmp(strtolower(argv[0]),"mouth"))
          		Tux_OpenClose_Mouth(count);
    	}

    	if(!strcmp(strtolower(cmd),"tux_leds"))
    	{
        	leds_t LED;
        	if(!strcmp(strtolower(argv[0]),"left"))
            		LED = LED_LEFT;
        	if(!strcmp(strtolower(argv[0]),"right"))
            		LED = LED_RIGHT;
        	if(!strcmp(strtolower(argv[0]),"both"))
            		LED = LED_BOTH;

              if(LED == 0)
                 return;

         	if(!strcmp(strtolower(argv[1]),"on"))
             		Tux_Led_On(LED);
         	if(!strcmp(strtolower(argv[1]),"off"))
             		Tux_Led_Off(LED);
         	if(!strcmp(strtolower(argv[1]),"blink"))
         	{
                  if(!atoi(argv[2]) || !atoi(argv[3]))
                     return;

        		  Tux_BlinkLeds(LED, atoi(argv[2]), atof(argv[3]));
            }

         	if(!strcmp(strtolower(argv[1]),"pulse"))
         	{
                    if(!strcmp(argv[6],""))
                       return;

             		effect_type_t EFFECT;

			        if(!strcmp(strtoupper(argv[6]),"NONE"))
                		EFFECT=NONE;
            		if(!strcmp(strtoupper(argv[6]),"LAST"))
                		EFFECT=LAST;
            		if(!strcmp(strtoupper(argv[6]),"DEFAULT"))
                		EFFECT=DEFAULT;
            		if(!strcmp(strtoupper(argv[6]),"FADE_DURATION"))
                		EFFECT=FADE_DURATION;
            		if(!strcmp(strtoupper(argv[6]),"FADE_RATE"))
                		EFFECT=FADE_RATE;
            		if(!strcmp(strtoupper(argv[6]),"GRADIENT_NBR"))
                		EFFECT=GRADIENT_NBR;
            		if(!strcmp(strtoupper(argv[6]),"GRADIENT_DELTA"))
                		EFFECT=GRADIENT_DELTA;

                    if(!atof(argv[2]) || !atof(argv[3]) || !atoi(argv[4]) || !atof(argv[5])
                       || !atof(argv[7]) || !atoi(argv[8]))
                          return;

            		Tux_PulseLeds(LED,atof(argv[2]), atof(argv[3]), (byte)atoi(argv[4]),
                 		atof(argv[5]),EFFECT, atof(argv[7]), (byte)atoi(argv[8]));
         	}
     	}

     	if(!strcmp(strtolower(cmd),"tux_micro"))
     	{
         	if(!strcmp(strtolower(argv[0]),"record"))
         	{
               if(!strcmp(argv[1],""))
                   return;

               char *file;
               file = (char *)malloc(sizeof(char)*strlen(argv[1]));
               strcpy(file,argv[1]);
             
               pthread_t recordThread;
               pthread_attr_t recordThread_attr;
               pthread_attr_init (&recordThread_attr);
    	       pthread_attr_setdetachstate (&recordThread_attr, PTHREAD_CREATE_DETACHED);
    	       pthread_create(&recordThread, &recordThread_attr, TuxAudio_StartRecord, file);
               pthread_join(recordThread, NULL);
               
		if(file != NULL)
		{
            /*free(file);
			file=NULL;*/
		}
               

         	}
            if(!strcmp(strtolower(argv[0]),"stop"))
            		TuxAudio_StopRecord();
     	}

     	if(!strcmp(strtolower(cmd),"tux_flippers"))
     	{
        	if(!strcmp(strtolower(argv[0]),"up"))
            		Tux_Flippers_Up();
         	if(!strcmp(strtolower(argv[0]),"down"))
            		Tux_Flippers_Down();
         	if(!strcmp(strtolower(argv[0]),"updown"))
         	{
                if(!atoi(argv[1]))
                  return;

            	Tux_FlippersUpDown(atoi(argv[1]));
            }

            if(!strcmp(strtolower(argv[0]),"speed"))
            {
                if(!atoi(argv[1]))
                   return;

                Tux_SetFlippersSpeed((byte)atoi(argv[1]));
            }
     	}

     	if(!strcmp(strtolower(cmd),"tux_rotate"))
     	{
            if(!atoi(argv[1]))
               return;


         	if(!strcmp(strtolower(argv[0]),"left"))
            	Tux_RotateLeft(atoi(argv[1]));
         	if(!strcmp(strtolower(argv[0]),"right"))
        		Tux_RotateRight(atoi(argv[1]));
    		if(!strcmp(strtolower(argv[0]),"speed"))
                Tux_SetRotationSpeed((byte)atoi(argv[1]));
     	}


     	if(!strcmp(strtolower(cmd),"tux_flash"))
     	{
            if(!strcmp(argv[0],"infos"))
            {
                char infos[8192];
                TuxDrv_GetFlashSound_Infos(infos);
                SendMsgToClient(client,infos);
            }
            else
            {
                if(!atoi(argv[0]) || !atoi(argv[1]))
                   return;

                int sound = atoi(argv[0]);
                int ampli = atoi(argv[1]);

                if(sound < 1)
                  sound = 1;
                if(sound > 17)
                  sound = 17;

                if(ampli < 0)
                   ampli=0;
                if(ampli > 100)
                   ampli=100;

            	Tux_PlayFlashSound(sound,ampli);
             }
        }

		/* Tux_TTS(blablabla,<voice>,<pitch>,<speed>,<stop>,<sync>) */
     	if(!strcmp(strtolower(cmd),"tux_tts"))
     	{
			if(argv[0] == NULL || argv[1] == NULL)
				return;

			if(!strcmp(argv[0],"stoptts"))
			{
				TuxAudio_StopTTS();
				return;
			}
			
            bool syncMouth=true;
			bool autoStop=true;
			
			if(argv[4] != NULL)
			{
				if(!atoi(argv[4]))
					autoStop=false;
			}
			
			if(argv[5] != NULL)
			{
            	if(!atoi(argv[5]))
                	syncMouth=false;
			}
			
            int pitch;
			
			if(argv[2] != NULL)
			 	pitch = atoi(argv[2]);
            else
            	pitch = 50;
            	
            if(pitch < 0)
               pitch = 0;
            if(pitch > 99)
               pitch = 99;

            int speed;
			
			if(argv[3] != NULL)
			 	speed = atoi(argv[3]);
            else
            	speed = 115;
            	
            if(speed < 80)
               speed=80;
            if(speed > 450)
               speed=450;


            int word = countCharacterOccurency(trim(argv[0]),' ') + 1;


            /* mouvement de bouche synchronisé */
            if(syncMouth)
            {
                /* environ 4 carac = 1 mouvement (dans le cas d'une vitesse moyenne) */

                syncMouthMov mov;
                mov = (syncMouthMov)malloc(sizeof(syncMouthMov_t));
                
                mov->phr = (char *)malloc(sizeof(char)*strlen(argv[0]));
                strcpy(mov->phr,argv[0]);
                
                mov->count = word;

                pthread_t syncMouthThread;
                pthread_attr_t syncMouthThread_attr;
                pthread_attr_init (&syncMouthThread_attr);
                pthread_attr_setdetachstate (&syncMouthThread_attr, PTHREAD_CREATE_DETACHED);
                pthread_create(&syncMouthThread, &syncMouthThread_attr, syncMouthMovements, mov);
                
				if(mov->phr != NULL)
				{
		            /*free(mov->phr);
					mov->phr=NULL;*/
				}
		
				if(mov != NULL)
				{
		            /*free(mov);
					mov=NULL;*/
				}

            }
            
            TuxAudio_PlayTTS(argv[1], argv[0], pitch, speed, autoStop);
        }

     	if(!strcmp(strtolower(cmd),"tux_audio"))
     	{
			TuxLogger_writeLog(TUX_LOG_DEBUG,"Tux_Audio()");
			
			if(argv[0] == NULL)
				return;
			
			if(argv[1] != NULL)
			{
	     		if(!strcmp(strtolower(argv[0]),"playmusic"))
            		TuxAudio_PlayMusic(argv[1]);
			}
            
		 	if(!strcmp(strtolower(argv[0]),"stopmusic"))
             	TuxAudio_StopMusic();
     	}

     	if(!strcmp(strtolower(cmd),"tux_state"))
     	{
			if(argv[0] == NULL)
               return;
            
            TuxDrv_GetStatusValue(atoi(argv[0]),valeur);

            SendMsgToClient(client,valeur);
     	}

     	if(!strcmp(strtolower(cmd),"tux_sleep"))
     	{
            char cmd[32];
            snprintf(cmd,32,"RAW_CMD:0x00:0xB7:0x01:0x00:0x00");
            TuxDrv_PerformCommand(0.0,cmd);
        }

        if(!strcmp(strtolower(cmd),"tux_wakeup"))
        {
            char cmd[32];
            snprintf(cmd,32,"RAW_CMD:0x00:0xB6:0xFF:0x01:0x00");
            TuxDrv_PerformCommand(0.0,cmd);
        }

        if(!strcmp(strtolower(cmd),"tux_resetpos"))
            TuxDrv_ResetPositions();

	if(argv != NULL)
	{
		/*free(argv);
		argv=NULL;*/
	}

	if(temp != NULL)
	{
		/*free(temp);
		temp=NULL;*/
	}

	if(cmd != NULL)
	{
		/*free(cmd);
		cmd=NULL;*/
	}
}

void *syncMouthMovements(void *data)
{
     syncMouthMov mov = (syncMouthMov)data;

     char **WORDS;
     WORDS = explode(mov->phr,' ');
     int i;


     for(i = 0; i < mov->count; i++)
     {
           TuxLogger_writeLog(TUX_LOG_DEBUG,"WORDS[%d] -> %s",i,WORDS[i]);

           if(strlen(WORDS[i]) < 3)
           {
               TuxLogger_writeLog(TUX_LOG_DEBUG,"moins de 3 carac on attent 0.7 seconde");
               sleep(700);
           }
           else
           {
               TuxLogger_writeLog(TUX_LOG_DEBUG,"plus de 3 caractère on compte le nb de caractère pour connaitre le nb de mouvement");

               int len = strlen(WORDS[i]);
               TuxLogger_writeLog(TUX_LOG_DEBUG,"Nombre de caractère -> %d",len);

               if((len % 2))
               {
                   TuxLogger_writeLog(TUX_LOG_DEBUG,"nombre impaire on ajoute 1");
                   len++;
               }

               int x = len / 4;
               if(x < 0)
                   x=0;

               if(x == 0)
               {
                    TuxLogger_writeLog(TUX_LOG_DEBUG,"Trop petit on annule le mouvement !");
               }
               else
               {
                   if(x % 2)
                      x++;

                   TuxLogger_writeLog(TUX_LOG_DEBUG,"Nombre de mouvement calculer -> %d",x);
                   Tux_OpenClose_Mouth(x);
               }
           }
     }
    
	if(mov->phr != NULL)
	{ 
    	/*free(mov->phr);
		mov->phr=NULL;*/
	}

	if(mov != NULL)
	{
    	/*free(mov);
		mov=NULL;*/
	}

	if(WORDS != NULL)
	{
    	/*free(WORDS);
		WORDS=NULL;*/
	}
	return 0;
}

