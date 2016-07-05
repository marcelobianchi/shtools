/*
    This file is part of sh2sac.
    
    sh2sac a tool for converting from Seismic Handler Qfiles to SAC file format
    Copyright (C) 2008  Marcelo B. de Bianchi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <sh2sac.h>
#include <load_stationdata.h>

void get_sh_info(station_information *sta, char *line);
void get_col_info(station_information *sta, char *line);
int id_line_type(FILE *ent);

int fill_station_coordinates(sacheader *sach, station *sta)
{
 int i=0;
 int pos=-1;
 
 if ( strncmp(sach->kstnm,"-12345  ",8)==0 || sach->kstnm[0] == ' ' || sach->kstnm[0] == '\0' )
 {
  if (v)
   fprintf(stderr," I:> No station name.\n");
  else
   fprintf(stderr,"* Ooops no station name *");
   
  return -1;
 }
 
 for (i=0; i < sta->n;i++)
  if ( strncmp(sach->kstnm,sta->stinf[i].name,strlen(sach->kstnm))==0)
  {
   pos=i;
   break;
  }

 if (pos>=0)
 {
  if (v) fprintf(stderr," I:> Filling %s at pos %d coordinates: %f %f\n",sach->kstnm,i,sta->stinf[i].lat,sta->stinf[i].lon);
  sach->stla=sta->stinf[pos].lat;
  sach->stlo=sta->stinf[pos].lon;
  sach->stel=sta->stinf[pos].el;
 } else
   {
    fprintf(stderr,"* Station Not Found *");
    return -2;
   }
   
 return 0;
}


void get_sh_info(station_information *sta, char *line)
{
 int posi=0;
 int i,max;
 
 max=strlen(line);
 
 /* Remove tab */
 for(posi=0;posi<max;posi++)
   if (line[posi] == '\t' ) line[posi]=' ';
 
 /* search for begin of first word ! */
 for(posi=0;line[posi]==' ';posi++);
 
 /* Get station name */
 i=0;
 do
 {
  sta->name[i]=line[posi];
  sta->name[i+1]='\0';
  posi++;
  i++;
 } while( line[posi] != ' ' && i < (MAXSTACHAR-1));
 
 /* search for lat */
 while ( (strncmp(&line[posi],"lat:",4) != 0) && posi < (max-4) )
  posi++;
 
 posi=posi+4;
 sscanf(&line[posi],"%f",&sta->lat);
 
 /* search for lon */
 while ( (strncmp(&line[posi],"lon:",4) != 0) && posi < (max-4) )
  posi++;
 
 posi=posi+4;
 sscanf(&line[posi],"%f",&sta->lon);

 /* search for elevation */
 while ( (strncmp(&line[posi],"elevation:",10) != 0) && posi < (max-10) )
  posi++;
 
 posi=posi+10;
 if (posi < max)
   sscanf(&line[posi],"%f",&sta->el);
 else
   sta->el=0.0;

 /* search for name */
 while ( (strncmp(&line[posi],"name:",5) != 0) && posi < (max-5) )
  posi++;

 posi=posi+5;
 i=0;
 sta->network[0]='\0';
 if (posi<max)
 {
  /* Get NET name */
  do
  {
   sta->network[i]=line[posi];
   sta->network[i+1]='\0';
   posi++;
   i++;
  } while( line[posi] != ' ' && line[posi] != ',' && i < (MAXSTACHAR-2) && posi < (max-1));
 }
}

void get_col_info(station_information *sta, char *line)
{
 int posi=0;
 int i,max;
 
 max=strlen(line);
 
 /* Remove tab */
 for(posi=0;posi<max;posi++)
 {
   if (line[posi] == '\t' ) line[posi]=' ';
   if (line[posi] == '\n' ) line[posi]=' ';
 }
 
 /* search for begin of first word ! */
 for(posi=0;line[posi]==' ';posi++);
 
 /* Get station name */
 i=0;
 sta->name[0]='\0';
 do
 {
  sta->name[i]=line[posi];
  sta->name[i+1]='\0';
  posi++;
  i++;
 } while( line[posi] != ' ' && i < (MAXSTACHAR-1));
 
 /* Get lat. */
 for(;line[posi]==' '&&posi<max;posi++);
 if (posi<max)
   sscanf(&line[posi],"%f",&sta->lat);
 else
  sta->lat=0.0;

 /* Get lon. */
 for(;line[posi]!=' '&&posi<max;posi++);
 for(;line[posi]==' '&&posi<max;posi++);
 if (posi<max)
   sscanf(&line[posi],"%f",&sta->lon);
 else
  sta->lon=0.0;
  
 /* Get el. */
 for(;line[posi]!=' '&&posi<max;posi++);
 for(;line[posi]==' '&&posi<max;posi++);
 if (posi<max)
   sscanf(&line[posi],"%f",&sta->el);
 else
  sta->el=0.0;

 /* Get nt. */
 for(;line[posi]!=' '&&posi<max;posi++);
 for(;line[posi]==' '&&posi<max;posi++);
 /* Get network name */
 i=0;
 sta->network[0]='\0';
 do
 {
  sta->network[i]=line[posi];
  sta->network[i+1]='\0';
  posi++;
  i++;
 } while( line[posi] != ' ' && i < (MAXSTACHAR-1));

}

int id_line_type(FILE *ent)
{
 char line[MAXLINE];
 int i;
 long count=0, nl=0;
 float score=0;
 
 do {
  fgets(line,MAXLINE,ent);
  if (strlen(line)> 10)
  {
   for (i=0; i < (strlen(line)-11);i++)
   {
    if ( strncmp(&line[i],"lat:",4) == 0 ) count++;
    if ( strncmp(&line[i],"lon:",4) == 0 ) count++;
    if ( strncmp(&line[i],"elevation:",10) == 0 ) count++;    
   }
   nl++;
  }
 } while (!feof(ent));
 
 score=(float)count/(float)nl;
 rewind (ent);
  
 if (score > 2.0)
   return 0; /* SH type */
 else
   return 1; /* Oder type */
}

void  load_stationdata(char *filename, station *sta)
{
 FILE *ent;
 // int i;
 char line[MAXLINE];
 int type;
  
 sta->n=0;
 sta->stinf = NULL;
 
 if ( (ent=fopen(filename,"r")) == NULL)
  return;
 
 type=id_line_type(ent);
 
 while (!feof(ent))
 {
  line[0]='\0';
  fgets(line,MAXLINE,ent);
  if ( strlen(line) > 1 )
  {
   sta->n++;
   sta->stinf=realloc(sta->stinf,sizeof(station_information)*sta->n);
   switch(type)
   {
    case 0: /* Seismic handler type */
     get_sh_info(&sta->stinf[sta->n-1],line);
     if (v) fprintf(stderr,"I:> Load %04d sts - %12s %8.3f %8.3f %7.1f [%s]\n",
      sta->n,
      sta->stinf[sta->n-1].name,
      sta->stinf[sta->n-1].lat,
      sta->stinf[sta->n-1].lon,
      sta->stinf[sta->n-1].el,
      sta->stinf[sta->n-1].network);
    break;

    case 1: /* 4 column type stanm lat lon el */
     get_col_info(&sta->stinf[sta->n-1],line);
     if (v) fprintf(stderr,"I:> Load %04d sts - %12s\t %8.3f\t %8.3f\t %7.1f [%s]\n",
      sta->n,
      sta->stinf[sta->n-1].name,
      sta->stinf[sta->n-1].lat,
      sta->stinf[sta->n-1].lon,
      sta->stinf[sta->n-1].el,
      sta->stinf[sta->n-1].network);
   break;
   }   
  }
 }
 
 fprintf(stderr,"I:> Total of %d stations loaded.\n",sta->n);
 fclose(ent);
 return;
}
