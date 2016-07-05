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
#include <stdio.h>
#include <time.h>

#define MAXKEY  200
#define QMAGIC 0xABCD

extern int pvalue;
extern int hasstart,hasorigin,hasp,hass;
extern int maxmodes;
extern char *modetype[2];

/* Prototypes ! */
void parse_l(sacheader *sach, char *key,long *dposition);
void parse_c(sacheader *sach, char *key);
void parse_i(sacheader *sach, char *key);
void parse_s(sacheader *sach, char *key);
void parse_r(sacheader *sach, char *key);
void zeroit(char *p,int size);
void copyit(char *p,char *s,int size);
int getmonth (char *sm);
int getjulday (int year, int month, int day);
void fill_time (sacheader *trace);
char *generate_filename (sacheader *trace, int *mode);
void fill_station_component (sacheader *trace);

/* time parse functions */
/* Separators: .,:-_ */
/* Format DD?MM?YYYY?HH?MM?SS?MMM 
   where ? represent one of the separators above.
*/

int getmonth(char *sm);
int getyear(char *sm);
int getday(char *sm);
int gethour(char *sm);
int getmin(char *sm);
int getsec(char *sm);
int getmsec(char *sm);
int fillstructtmfromchar(char *key, struct tm *temp, int *ms);
char getfirstsep(char *str);
