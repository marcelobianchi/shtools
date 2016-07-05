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
#define MAXSTACHAR 128

typedef struct { 
 char name[MAXSTACHAR];
 char network[MAXSTACHAR];
 float lat;
 float lon;
 float el;
} station_information;

typedef struct {
  int n;
  station_information *stinf;
} station;

void  load_stationdata(char *filename, station *sta);
int fill_station_coordinates(sacheader *sach, station *sta);
