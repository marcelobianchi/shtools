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
/* Struct SAC - nvhdr == 6 */

typedef struct { 
float delta,depmin,depmax,scale,odelta;
float b,e,o,a,int1;
float t0,t1,t2,t3,t4;
float t5,t6,t7,t8,t9;
float f,resp0,resp1,resp2,resp3;
float resp4,resp5,resp6,resp7,resp8;
float resp9,stla,stlo,stel,stdp;
float evla,evlo,evel,evdp,mag;
float user0,user1,user2,user3,user4;
float user5,user6,user7,user8,user9;
float dist,az,baz,gcarc,int2;
float int3,depmen,cmpaz,cmpinc,xminimum;
float xmaximum,yminimum,ymaximum,unf0,unf1;
float unf2,unf3,unf4,unf5,unf6;

int nzyear,nzjday,nzhour,nzmin,nzsec;
int nzmsec,nvhdr,norid,nevid,npts;
int int4,nwfid,nxsize,nysize,uni0;
int iftype,idep,iztype,uni1,iinst;
int istreg,ievreg,ievtyp,iqual,isynth;
int imagtyp,imagsrc,uni2,uni3,uni4;
int uni5,uni6,uni7,uni8,uni9;

int leven,lpspol,lovrok,lcalda,unl0;

char kstnm[8],kevnm[16];
char khole[8],ko[8],ka[8];
char kt0[8],kt1[8],kt2[8];
char kt3[8],kt4[8],kt5[8];
char kt6[8],kt7[8],kt8[8];
char kt9[8],kf[8],kuser0[8];
char kuser1[8],kuser2[8],kcmpnm[8];
char knetwk[8],kdatrd[8],kinst[8];
}  sacheader;

