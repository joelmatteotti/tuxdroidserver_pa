/* =============== GPL HEADER =====================
 * TuxDownloader.c
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
#include <string.h>
#include <malloc.h>
#include <TuxDownloader.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <TuxStrings.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void TuxDownloader_DownloadFile(char *url, char *destination)
{
    CURL *curl;
    FILE *fp;
    CURLcode res;

    char **ex = explode(url,'/');
    int argc = countCharacterOccurency(url,'/') - 1;
     
    char *file = (char *)malloc(sizeof(char)*(strlen(destination)+1+strlen(ex[argc])));
    sprintf(file,"%s%s",destination,ex[argc]);
  
    curl = curl_easy_init();
    if (curl) 
    {
        fp = fopen(file,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}
