/* =============== GPL HEADER =====================
 * TuxStrings.c
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
#include <malloc.h>
#include <string.h>
#include <TuxStrings.h>
#include <stdbool.h>
#include <ctype.h>


char *replace(char *st, char *orig, char *repl) 
{
	static char buffer[4096];
	char *ch;
	
	if (!(ch = strstr(st, orig)))
		return st;
	
	strncpy(buffer, st, ch-st);  
	buffer[ch-st] = 0;
	sprintf(buffer+(ch-st), "%s%s", repl, ch+strlen(orig));
	
	return buffer;
}
  
  
/*
 Fonction permet de séparer une chaine de caractère par un caractère en particulier
 et de ranger les résultat dans un tableau de char*
*/
char  **explode(char *str, char separator)
{
    char **res = NULL;
    int  nbstr = 1;
    int  len;
    int  from = 0;
    int  i;
    int  j;
    res = (char **) malloc(sizeof (char *));
    len = strlen(str);

    for (i = 0; i <= len; ++i)
    {
        if ((i == len) || (str[i] == separator))
        {
            res = (char **) realloc(res, ++nbstr * sizeof (char *));
            res[nbstr - 2] = (char *) malloc((i - from + 1) * sizeof (char));
            
            for (j = 0; j < (i - from); ++j)
                res[nbstr - 2][j] = str[j + from];
            
            res[nbstr - 2][i - from] = '\0';
            from = i + 1;
            ++i;
        }
    }
    res[nbstr - 1] =  NULL;
    return res;
}

/* compte le nombre d'occurence d'un caractère dans une chaine de caractère */
int countCharacterOccurency(char str[], char car)
{
    int count = 0, k = 0;
    while (str[k] != '\0')
    {
          if (str[k] == car)
              count++;
          k++;
    }
    
    return count;
}


/* converti une chaine de caractères en une chaine majuscule */
char *strtoupper(char *s)		
{
     char *cp;
     if ( !(cp=s) )
        return NULL;
    
     while ( *s != 0 ) 
     {
           *s = toupper( *s );
           s++;
     }
     
     return cp;
}

/* convertit une chaine de caractère en une chaine de caractères minuscule */
char *strtolower(char *s)		
{
     char *cp;
     if ( !(cp=s) )
        return NULL;
    
     while ( *s != 0 ) 
     {
           *s = tolower( *s );
           s++;
     }
     
     return cp;
}

/* retire les espaces en début et fin d'une chaine de caractères */
char *trim(char *str)
{
      char *ibuf = str, *obuf = str;
      int i = 0, cnt = 0;

      if (str)
      {
            for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf);

            if (str != ibuf)
                  memmove(str, ibuf, ibuf - str);

            while (*ibuf)
            {
                  if (isspace(*ibuf) && cnt)
                        ibuf++;
                  else
                  {
                        if (!isspace(*ibuf))
                              cnt = 0;
                        else
                        {
                              *ibuf = ' ';
                              cnt = 1;
                        }
                        obuf[i++] = *ibuf++;
                  }
            }
            obuf[i] = 0; /* NULL */

            while (--i >= 0)
            {
                  if (!isspace(obuf[i]))
                        break;
            }
            obuf[++i] = 0; /* NULL */
      }

      return str;
}
