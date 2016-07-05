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
#include <stdlib.h>
#include <string.h>
#include <sac.h>

#define MAXLINE 2000

extern int v; /* Verbose ? */
extern int nlines;
extern char version[];

/* This file function prototype */
void jump_coments(FILE *ent,int cmtlines, int print);
void help();
FILE *init (char *filename, int *nt);
char* get_next_trace (FILE * ent);
long fill_sach (char *hdr, sacheader *sach);
void write_sac (sacheader *sach, char *datafile, long dposition, int *mode, int overwrite);
int get_nt (FILE * ent);
