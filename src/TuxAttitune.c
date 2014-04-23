/* =============== GPL HEADER =====================
 * TuxAttitune.c
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
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <pthread.h>
#include <libxml/xmlreader.h>

#include <TuxLogger.h>
#include <TuxStrings.h>
#include <TuxAttitune.h>
#include <TuxDriver.h>
#include <TuxAudio.h>
#include <TuxTime.h>
#include <TuxUtils.h>


pthread_mutex_t TTSMutex,WAVMutex;
pthread_t mThread;
pthread_attr_t mThread_attr;


attitune myAttitune=NULL;    /* créer une attitune */

char current_element[1024];

FILE *fp=NULL;

/* écrit la commande dans le fichier de macro */
void TuxAttitune_writeMacroCMD(attitune_block block)
{
	/*
	TuxLogger_writeLog(TUX_LOG_DEBUG,"Ajout d'une commande au fichier de macro");
*/
	if(block == NULL)
		TuxLogger_writeLog(TUX_LOG_DEBUG,"Le block est Null");
	
	char gCmd[1024]; /* commande formater */
	
	snprintf(gCmd,1024,"NULL"); /* aucune commande reconnu pour le moment */

	if(!strcmp(block->cmd,"sound_play"))
	{
		/* commande sound */
		snprintf(gCmd,1024,"%f:TUX_CMD:SOUND_FLASH:PLAY:%d,100.0\n",block->start_time, block->track);
	}
	else if(!strcmp(block->cmd,"spinl_on"))
	{
		/* rotation gauche */
		snprintf(gCmd,1024,"%f:TUX_CMD:SPINNING:LEFT_ON:%d\n",block->start_time, block->count);
	}
	else if(!strcmp(block->cmd,"spinr_on"))
	{
		/* rotation droite */
		snprintf(gCmd,1024,"%f:TUX_CMD:SPINNING:RIGHT_ON:%d\n",block->start_time, block->count);
	}
	else if(!strcmp(block->cmd,"mouth_close"))
	{
		/* ouverture de la bouche */
		snprintf(gCmd,1024,"%f:TUX_CMD:MOUTH:CLOSE\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"mouth_open"))
	{
		/* ouverture de la bouche */
		snprintf(gCmd,1024,"%f:TUX_CMD:MOUTH:OPEN\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"mouth_on"))
	{
		/* ouverture de la bouche */
		snprintf(gCmd,1024,"%f:TUX_CMD:MOUTH:ON:%d,NDEF\n",block->start_time, block->count);
	}
	else if(!strcmp(block->cmd,"eyes_open"))
	{
		/* ouverture des yeux */
		snprintf(gCmd,1024,"%f:TUX_CMD:EYES:OPEN\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"eyes_close"))
	{
		/* fermeture des yeux */
		snprintf(gCmd,1024,"%f:TUX_CMD:EYES:CLOSE\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"eyes_on"))
	{
		/* ouverture des yeux */
		snprintf(gCmd,1024,"%f:TUX_CMD:EYES:ON:%d,NDEF\n",block->start_time, block->count);
	}
	else if(!strcmp(block->cmd,"leds_on"))
	{
		/* allumage des leds */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:ON:LED_BOTH,1.0\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"ledl_on"))
	{
		/* allumage led gauche */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:ON:LED_LEFT,1.0\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"ledr_on"))
	{
		/* allumage led droite */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:ON:LED_RIGHT,1.0\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"leds_off"))
	{
		/* eteint les leds */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:OFF:LED_BOTH\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"ledl_off"))
	{
		/* eteint la led gauche */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:OFF:LED_LEFT\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"ledr_off"))
	{
		/* eteint la led droite */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:OFF:LED_RIGHT\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"leds_blink"))
	{
		/* fait clignoter les leds */
		snprintf(gCmd,1024,"%f:TUX_CMD:LED:BLINK:LED_BOTH:%d,%d\n",block->start_time, block->count, block->speed);
	}
	else if(!strcmp(block->cmd,"wings_on"))
	{
		/* monte et descend les ailes */
		snprintf(gCmd,1024,"%f:TUX_CMD:FLIPPERS:ON_DURING:%d,NDEF\n",block->start_time, block->count);
	}
	else if(!strcmp(block->cmd,"wings_up"))
	{
		/* monte les ailes */
		snprintf(gCmd,1024,"%f:TUX_CMD:FLIPPERS:UP\n",block->start_time);
	}
	else if(!strcmp(block->cmd,"wings_down"))
	{
		/* descend les ailes */
		snprintf(gCmd,1024,"%f:TUX_CMD:FLIPPERS:DOWN\n",block->start_time);
	}

	if(strcmp(gCmd,"NULL"))
		fputs(gCmd,fp);
}

const xmlChar *name, *value;
    
/* Parsing des éléménets du fichier XML */
void TuxAttitune_processNode(xmlTextReaderPtr reader) 
{
    name = xmlTextReaderConstName(reader);
    value = xmlTextReaderConstValue(reader);

    if(current_element != NULL && value != NULL)
    {
        /*               
        TuxLogger_writeLog(TUX_LOG_DEBUG,"[%d] - name: %s / value: %s",current_block,current_element,(char *)value);
        */
                     
        if(!strcmp(current_element,"start_time"))
        {
			if(myAttitune->total_blocks >= 0)
            	TuxAttitune_writeMacroCMD(myAttitune->attitune_block[myAttitune->total_blocks]);
            
			myAttitune->total_blocks++;
		    
			myAttitune->attitune_block[myAttitune->total_blocks] = (attitune_block)malloc(sizeof(attitune_block_t));
            myAttitune->attitune_block[myAttitune->total_blocks]->start_time = atof((char *)value);
            
            /* initialisation des variables */
            myAttitune->attitune_block[myAttitune->total_blocks]->wav = NULL;
            myAttitune->attitune_block[myAttitune->total_blocks]->text = NULL;
            myAttitune->attitune_block[myAttitune->total_blocks]->cmd = NULL;
            myAttitune->attitune_block[myAttitune->total_blocks]->pitch=50;
            myAttitune->attitune_block[myAttitune->total_blocks]->speed=115;
        }
        else
        {
            if(!strcmp(current_element,"cmd"))
            {
                myAttitune->attitune_block[myAttitune->total_blocks]->cmd = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->attitune_block[myAttitune->total_blocks]->cmd,(char *)value);
            }
            if(!strcmp(current_element,"duration"))
                myAttitune->attitune_block[myAttitune->total_blocks]->duration = atof((char *)value);
            if(!strcmp(current_element,"count"))
                myAttitune->attitune_block[myAttitune->total_blocks]->count = atoi((char *)value);
            if(!strcmp(current_element,"speed"))
                myAttitune->attitune_block[myAttitune->total_blocks]->speed = atoi((char *)value);
            if(!strcmp(current_element,"text"))
            {
				if(strlen((char *)value) > 1)
				{
                	myAttitune->attitune_block[myAttitune->total_blocks]->text = (char *)malloc(sizeof(char)*strlen((char *)value));
                	strcpy(myAttitune->attitune_block[myAttitune->total_blocks]->text,(char *)value);
            	}
			}
            if(!strcmp(current_element,"wav_name"))
            {
                myAttitune->attitune_block[myAttitune->total_blocks]->wav = (char *)malloc(sizeof(char)*(strlen((char *)value)+strlen(myAttitune->directory)+5));
                strcpy(myAttitune->attitune_block[myAttitune->total_blocks]->wav,myAttitune->directory);
                #ifdef _WIN32
				strcat(myAttitune->attitune_block[myAttitune->total_blocks]->wav,"wavs\\");
                #else
                strcat(myAttitune->attitune_block[myAttitune->total_blocks]->wav,"wavs/");
                #endif
				strcat(myAttitune->attitune_block[myAttitune->total_blocks]->wav,(char *)value);
            }
            if(!strcmp(current_element,"pitch"))
                myAttitune->attitune_block[myAttitune->total_blocks]->pitch = atoi((char *)value);
            if(!strcmp(current_element,"track"))
            	myAttitune->attitune_block[myAttitune->total_blocks]->track = atoi((char *)value);
            
			/* header (infos sur l'attitune) */
            if(!strcmp(current_element,"name"))
            {
                myAttitune->name = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->name,(char *)value);
            }
            if(!strcmp(current_element,"category"))
            {
                myAttitune->category = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->category,(char *)value);
            }
            if(!strcmp(current_element,"author"))
            {
                myAttitune->author = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->author,(char *)value);
            }
            if(!strcmp(current_element,"language"))
            {
                myAttitune->language = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->language,(char *)value);
            }
            if(!strcmp(current_element,"keywords"))
            {
                myAttitune->keywords = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->keywords,(char *)value);
            }
            if(!strcmp(current_element,"version"))
            {
                myAttitune->version = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->version,(char *)value);
            }
            if(!strcmp(current_element,"sub_category"))
            {
                myAttitune->sub_cat = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->sub_cat,(char *)value);
            }
            if(!strcmp(current_element,"description"))
            {
                myAttitune->desc = (char *)malloc(sizeof(char)*strlen((char *)value));
                strcpy(myAttitune->desc,(char *)value);
            }
            if(!strcmp(current_element,"length"))
            {
				myAttitune->length = atof((char *)value);
			}
        }
        
        value = NULL;
    }
    
    if(name != NULL)
    {
        if(!strcmp((char *)name,"cmd") || !strcmp((char *)name,"duration") || !strcmp((char *)name,"start_time") || !strcmp((char *)name,"count")
             || !strcmp((char *)name,"speed") || !strcmp((char *)name,"text") || !strcmp((char *)name,"wav_name") || !strcmp((char *)name,"pitch")
             || !strcmp((char *)name,"name") || !strcmp((char *)name,"category") || !strcmp((char *)name,"author") || !strcmp((char *)name,"language")  /* header */
             || !strcmp((char *)name,"keywords") || !strcmp((char *)name, "version") || !strcmp((char *)name,"sub_category") || !strcmp((char *)name,"description")) /* header */
        {
            strcpy(current_element,(char *)name);
        }
        
        name = NULL;
    }
}

/* Parsing du fichier XML */
bool TuxAttitune_ParseXMLFile(const char *xmlfile) 
{
	bool result=true;

    xmlTextReaderPtr reader;
    int ret;

    reader = xmlReaderForFile(xmlfile, NULL, 0);
	if (reader != NULL) 
    {
        ret = xmlTextReaderRead(reader);
        while (ret == 1) 
        {
            TuxAttitune_processNode(reader);
            ret = xmlTextReaderRead(reader);
        }
        xmlFreeTextReader(reader);
        
        if (ret != 0) 
        {
			result = false;
            TuxLogger_writeLog(TUX_LOG_ERROR,"Impossible de parser le fichier xml %s",xmlfile);
        }
    } 
    else 
    {
		result=false;
        TuxLogger_writeLog(TUX_LOG_ERROR,"Impossible d'ouvrir le fichier %s",xmlfile);
    }
    
    return result;
}


/* décompresse le fichier .att dans un répertoire temporaire du système */
int TuxAttitune_unzipAttitune(const char *attfile)
{
    TuxLogger_writeLog(TUX_LOG_DEBUG,"unzipAttitune() -----> %s",attfile);
    
    char *cmd;
    #ifdef _WIN32
    cmd = (char *)malloc(sizeof(char)*strlen(attfile)+34);
    strcpy(cmd,"unzip -q -o \"");
    strcat(cmd,attfile);
    strcat(cmd,"\" -d c:\\windows\\temp\\");
    #else
    cmd = (char *)malloc(sizeof(char)*strlen(attfile)+21);
    strcpy(cmd,"unzip -q -o ");
    strcat(cmd,attfile);
    strcat(cmd," -d /tmp/");
    #endif

    TuxLogger_writeLog(TUX_LOG_DEBUG,"unzip ------> %s",cmd);

    return system(cmd);
}

void *TuxAttitune_startAttituneWav(void *data)
{
	pthread_mutex_lock(&WAVMutex);
	attitune_block block = (attitune_block)data;

	if(block == NULL)
		return NULL; /* error */

	long time = (long)(block->start_time * 100);

	if(time > 0)
	{
	    #ifdef _WIN32
	    Sleep(time);
	    #else
	    TuxSleep(time);
	    #endif
	}

    TuxAudio_PlayMusic(block->wav);
    
    pthread_mutex_unlock(&WAVMutex);
	pthread_exit (0);
}

void *TuxAttitune_startAttituneTTS(void *data)
{
	pthread_mutex_lock(&TTSMutex);
	attitune_block block = (attitune_block)data;


	if(block == NULL)
		return NULL; /* error */
	
	
	long time = (long)(block->start_time * 1000);
	
	
	if(time > 0)
	{
		#ifdef _WIN32
	    Sleep(time);
	    #else
	    TuxSleep(time);
	    #endif
	}
	
	TuxAudio_PlayTTS("mb-fr1",block->text, block->pitch, block->speed, false);

	pthread_mutex_unlock(&TTSMutex);
	pthread_exit (0);
}

int compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}

/* Débute le chargement de l'attitune */
void TuxAttitune_loadAttitune(char *attfile)
{
    if(strstr(attfile,"http://"))
    {
        TuxLogger_writeLog(TUX_LOG_DEBUG,"Il s'agit d'une url on vas donc télécharger !");
        
        char **ex = explode(attfile,'/');
        int argc = countCharacterOccurency(attfile,'/') - 1;
    
        #ifdef _WIN32
        TuxDownloader_DownloadFile(attfile,"c:\\windows\\temp\\");
        attfile = (char *)realloc(attfile,sizeof(char)*(16+strlen(ex[argc])));
        sprintf(attfile,"c:\\windows\\temp\\%s",ex[argc]);
        #else
        TuxDownloader_DownloadFile(attfile,"/tmp/");
        attfile = (char *)realloc(attfile,sizeof(char)*(6+strlen(ex[argc])));
        snprintf(attfile,25,"/tmp/%s",ex[argc]);
        #endif

    }
    else if(!file_exists(attfile))
	{
			TuxLogger_writeLog(TUX_LOG_ERROR,"Le fichier %s n'existe pas !",attfile);
			return;
	}
	
     if(TuxAttitune_unzipAttitune(attfile) != 1)
     {
         TuxLogger_writeLog(TUX_LOG_ERROR,"Erreur lors de la décompression de l'attitune !");
         return;  /*TODO: Gestion d'erreur */
     }
     
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Créer une nouvelle attitune");
     if(myAttitune != NULL)
     {
	 if(myAttitune != NULL)
	{
        /*free(myAttitune);*/   /* on libère la mémoire */
		/*myAttitune=NULL;*/
	}
         myAttitune=NULL;    /* on réinitialise à NULL */
     }
    
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Allocation mémoire pour myAttitune");
     myAttitune = (attitune)malloc(sizeof(attitune_t));
     myAttitune->total_blocks = -1;
     
     
     char **sp;
     char **sp2;
     int occ;
     char *xmlpath;
     char *macropath;
     #ifdef _WIN32
     sp = explode(attfile,'\\');
     occ = countCharacterOccurency(attfile,'\\');
     xmlpath = (char *)malloc(sizeof(char)*(strlen(attfile)+22));
     strcpy(xmlpath,"c:\\windows\\temp\\");
     sp2 = explode(sp[occ],'.');
     strcat(xmlpath,sp2[0]);
     strcat(xmlpath,"\\");
     #else
     sp = explode(attfile,'/');
     occ = countCharacterOccurency(attfile,'\\');
     xmlpath = (char *)malloc(sizeof(char)*(strlen(attfile)+11));
     strcpy(xmlpath,"/tmp/");
     sp2 = explode(sp[occ],'.');
     strcat(xmlpath,sp2[0]);
     strcat(xmlpath,"/");     
     #endif
     macropath = (char *)malloc(sizeof(char)*strlen(xmlpath));
     myAttitune->directory = (char *)malloc(sizeof(char)*strlen(xmlpath));
     strcpy(myAttitune->directory,xmlpath);
     strcpy(macropath,xmlpath);
	 strcat(macropath,"macro.txt");
	 strcat(xmlpath,"scene.xml");
     
	if(sp != NULL)
	{
		/*free(sp);
		sp=NULL;*/
	}

	if(sp2 != NULL)
	{
	 	/*free(sp2);
		sp2=NULL;*/
	}
     
     TuxLogger_writeLog(TUX_LOG_DEBUG,"XML PATH ------------------> %s",xmlpath);


	if(!file_exists(xmlpath))
	{
		TuxLogger_writeLog(TUX_LOG_ERROR,"Une erreur à du se produire lors de la décompression de l'attitune !!");

		if(xmlpath != NULL)
		{
			/*free(xmlpath);
			xmlpath=NULL;*/
		}

		if(macropath != NULL)
		{
			/*free(macropath);
			macropath=NULL;*/
		}
		
		if(myAttitune != NULL)
		{
			/*free(myAttitune);
			myAttitune=NULL;*/
		}
		return;
	}
     
     TuxLogger_writeLog(TUX_LOG_DEBUG,"Début du parsing XML");
    
     fp = fopen(macropath,"a+");
	 bool result = TuxAttitune_ParseXMLFile(xmlpath);
     
	if(xmlpath != NULL)
	{
		/*free(xmlpath);
		xmlpath=NULL;*/
	}

	if(fp != NULL)
	{
		/*
     	fclose(fp);
		fp=NULL;*/
	}
     
     
     if(result)
     {
     	TuxLogger_writeLog(TUX_LOG_DEBUG,"Fin du parsing du fichier scene.xml");
     	TuxLogger_writeLog(TUX_LOG_DEBUG,"Total de block: %d",myAttitune->total_blocks);
     
	     printf("Attitune %s version %s par %s est maintenant charger !",myAttitune->name,myAttitune->version,myAttitune->author);
	     
	     TuxLogger_writeLog(TUX_LOG_INFO,"Attitune %s version %s par %s est maintenant charger !",myAttitune->name,myAttitune->version,myAttitune->author);
				
		 int i;
		 for(i = 0; i< myAttitune->total_blocks; i++)
		 {
				if(myAttitune->attitune_block[i] != NULL)
				{
					
					if(myAttitune->attitune_block[i]->wav != NULL)
					{
						pthread_mutex_init(&WAVMutex,NULL);
						pthread_attr_setdetachstate (&mThread_attr, PTHREAD_CREATE_DETACHED);
						pthread_create(&mThread,&mThread_attr,TuxAttitune_startAttituneWav,myAttitune->attitune_block[i]);
					}
					
					if(myAttitune->attitune_block[i]->text != NULL)
					{
						pthread_mutex_init(&TTSMutex,NULL);
						pthread_attr_setdetachstate (&mThread_attr, PTHREAD_CREATE_DETACHED);
						pthread_create(&mThread,&mThread_attr,TuxAttitune_startAttituneTTS,myAttitune->attitune_block[i]);
					}
				}
		 }
		 
		 TuxDrv_PerformMacroFile(macropath);
	}     
}





