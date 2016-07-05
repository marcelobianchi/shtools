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
#include <parses.h>
#include <time.h>

/* in which char is the first position for values on keys ?*/
int pvalue=5;

/* Time structures EventOrigin and TraceOrigin     */
/* meo && mto are milleseconds for time structures */
struct tm eo; /* Event origin */
struct tm to; /* Trace origin */
struct tm pa; /* P-Arrival */
struct tm sa; /* S-Arrival */
int meo, mto, mpa, msa;
int hasstart=0;
int hasorigin=0;
int hasp=0, hass=0;

/* Filename generation stuff */
char  *modetype[2] = { "Trace start mode", "Event time mode" };
int    maxmodes=2;
 
void parse_l(sacheader *sach, char *key, long *dposition)
{
 long value;
 int position;
  
 if (key[0]!='L') fprintf(stderr," L-ERROR:> Oops ! Wrong key on parse key L\n");
 
 key[pvalue-1]='\0';
 position=atoi(&key[1]);

 switch(position)
 {
  case 0:
   value=atol(&key[pvalue]);
   if (v) fprintf(stderr," L:> Data Offset: %ld\n",value);
   *dposition=value;   
  break;

  case 1:
   value=atol(&key[pvalue]);
   if (v) fprintf(stderr," L:> NPTS:  %ld\n",value);
   sach->npts=(int)value;
  break;

  default: 
   if (v) fprintf(stderr," L-ERROR:> Can't handle ID equal to %d.\n",position);
  break;
 }
}

void parse_c(sacheader *sach, char *key)
{
 int position;
  
 if (key[0]!='C') fprintf(stderr," C-ERROR:> Oops ! Wrong key on parse key L\n");
 
 key[pvalue-1]='\0';
 position=atoi(&key[1]);

 switch(position)
 {
  case 0:
   if (v) fprintf(stderr," C:> Component name: %c\n",key[pvalue]);
   strncpy(sach->kcmpnm,"        ",8);
   sach->kcmpnm[0]=key[pvalue];
  break;

  default: 
   if (v) fprintf(stderr," C-ERROR:> Can't handle ID equal to %d.\n",position);
  break;
 }
}

void parse_s(sacheader *sach, char *key)
{
 int position;
 int ret;

 if (key[0]!='S') fprintf(stderr," S-ERROR:> Oops ! Wrong key on parse key L\n");
 
 key[pvalue-1]='\0';
 position=atoi(&key[1]);
 
 switch(position)
 {
  case 1:
   copyit(sach->kstnm,key,8);
   if (v) fprintf(stderr," S:> Station name: %s\n",sach->kstnm);
  break;

  case 21:
  {
    hasstart = fillstructtmfromchar(&key[pvalue],&to,&mto);
    if (v)
      {
	fprintf (stderr,"  S:> Start: %03dms + %s",mto,asctime(&to));
	if (!hasstart) fprintf (stderr,"  E:> Could not parse start time.\n");
      }
  }
  break;
  
  case 22:
  {
    hasp = fillstructtmfromchar(&key[pvalue],&pa,&mpa);
    if (v)
      {
	fprintf (stderr, "  S:> P-Arrival: %03dms + %s",mpa,asctime(&pa));
	if (!hasp) fprintf(stderr,"  E:> Could not parse P-arrival.\n");
      }
  }
  break;

  case 23:
  {
    hass = fillstructtmfromchar(&key[pvalue],&sa,&msa);
    if (v)
      {
	fprintf (stderr, "  S:> S-Arrival: %03dms + %s",msa,asctime(&sa));
	if (!hass) fprintf(stderr,"  E:> Could not parse S-arrival.\n");
      }
  }
  break;

  case 24:
  {
    hasorigin = fillstructtmfromchar(&key[pvalue],&eo,&meo);
    if (v)
     {
       fprintf (stderr, "  S:> Origin: %03dms + %s",meo,asctime(&eo));
       if (!hasorigin) fprintf(stderr,"  E:> Could not parse Origin time.\n");
     } 
  }
  break;

  default: 
    if (v) fprintf(stderr," S-ERROR:> Can't handle ID equal to %d.\n",position);
  break;
 }
}

void parse_i(sacheader *sach, char *key)
{
 int value;
 int position;
  
 if (key[0]!='I') fprintf(stderr," I-ERROR:> Oops ! Wrong key on parse key I\n");
 
 key[pvalue-1]='\0';
 position=atoi(&key[1]);

 switch(position)
 {
  case 12:
   value=atoi(&key[pvalue]);
   if (v) fprintf(stderr," L:> EventNO: %d\n",value);
   sach->nevid=value;
  break;

  default: 
   if (v) fprintf(stderr," L-ERROR:> Can't handle ID equal to %d.\n",position);
  break;
 }

}

void parse_r(sacheader *sach, char *key)
{
 float value;
 int position;

 if (key[0]!='R') fprintf(stderr," R-ERROR:> Oops ! Wrong key on parse key L\n");
 
 key[pvalue-1]='\0';
 position=atoi(&key[1]);
 value=atof(&key[pvalue]);

 switch(position)
 {
  case 0:
   if (v) fprintf(stderr," R:> Delta: %f s\n",value);
   sach->delta=(float)value;
  break;

  case 11:
   if (v) fprintf(stderr," R:> Distance:  %f deg\n",value);
   sach->gcarc=(float)value;
  break;

  case 12:
   if (v) fprintf(stderr," R:> (Back)Azimuth:  %f deg\n",value);
   sach->baz=(float)value;
  break;

  case 13:
   if (v) fprintf(stderr," R:> Inciangle (user1):  %f deg\n",value);
   sach->user1=(float)value;
   strncpy(sach->kuser1,"InciAng",7);
  break;

  case 14:
   /* if (value>2000)
   {
    value=value/1000.0;
    fprintf(stderr," I:> Evdp in meters:  %f m -> %f km\n",value*1000.0,value);
   } */

   if (v) {
    fprintf(stderr," R:> Evdp:  %f km\n",value);
    fprintf(stderr,"  I:> **** Warning writing non-standard SAC files,\n           depth standard is in meters, writting kilometers ! ****\n");
   }
   sach->evdp=(float)value;
  break;

  case 15:
   if (v) fprintf(stderr," R:> Magnitude (MAG SAC field):  %f\n",value);
   sach->mag=(float)value;
  break;

  case 16:
   if (v) fprintf(stderr," R:> Evla:  %f deg\n",value);
   sach->evla=(float)value;
  break;
  
  case 17:
   if (v) fprintf(stderr," R:> Evlo:  %f deg\n",value);
   sach->evlo=(float)value;
  break;

  case 18:
   if (v) fprintf(stderr," R:> Slowness (User7):  %f\n",value);
   sach->user7=(float)value;
   // strncpy(sach->kuser7,"Slowness",8);
  break;

  case 26:
   if (v) fprintf(stderr," R:> Calib (ignoring):  %f\n",value);
   /*sach->????=(float)value;*/
  break;

  default: 
   if (v) fprintf(stderr," R-ERROR:> Can't handle ID equal to %d\n",position);
  break;
 }
 
}

void zeroit(char *p,int size)
{
 int i;
 
 for(i=0;i<size;i++)
    p[i]='\0';
}

void copyit(char *p,char *s,int size)
{
 int i;
 
 zeroit(p,size);
 for(i=0;i<size && s[pvalue+i]!='~';i++)
      p[i]=s[pvalue+i];
}

int
getmonth (char *sm)
{
  static char mtab[12][3] =
    { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
  int t, i, j;
  int field=0;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else
	if (field==1 && j<10) st[j++]=sm[i];
    }

  st[j]='\0'; 

  if (isdigit(st[0]))
    {
      return atoi(st);
    } else {
    for (i = 0; i < 12; i++)
      {
	t = strncasecmp (st, mtab[i],3);
	if (t == 0) return (i + 1);
      }
  }

  return -1;
}

int getyear(char *sm) 
{
  int i,j;
  int field=0;
  int value;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else 
	if (field==2 && j<10) st[j++]=sm[i];
    }

  st[j]='\0';
  value = atoi(st);
  return value;
}

int getday(char *sm)
{
  int i,j;
  int field=0;
  int value;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else 
	if(field==0 && j<10) st[j++]=sm[i];
    }

  st[j]='\0'; 
  value=atoi(st);

  return value;
}

int gethour(char *sm)
{
  int i,j;
  int field=0;
  int value;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else 
	if (field==3 && j<10) st[j++]=sm[i];
    }

  st[j]='\0'; 
  value=atoi(st);

  return value;
}

int getmin(char *sm)
{
  int i,j;
  int field=0;
  int value;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else 
	if (field==4 && j<10) st[j++]=sm[i];
    }

  st[j]='\0'; 
  value=atoi(st);

  return value;

}

int getsec(char *sm)
{
  int i,j;
  int field=0;
  int value;
  char st[11];

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',') 
	field++;
      else
	if (field==5 && j<10) st[j++]=sm[i];
    }

  st[j]='\0'; 
  value=atoi(st);

  return value;
}

int getmsec(char *sm)
{
  int i,j;
  int field=0;
  int value;
  char st[11],sep;

  for (i=0,j=0;sm[i]!='~';i++)
    {
      if (sm[i]=='-' || sm[i]=='_' || sm[i]==':' || sm[i]=='.' || sm[i]==',')
	{
	  field++;
	  sep=sm[i];
	}
      else
	if(field==6 && j<10) st[j++]=sm[i];
    }
  
  st[j]='\0'; 
  value=atoi(st);
  
  /*
    Correct by the number of digits in the milliseconds after the dot (.)
    In this case we have to account for missing numbers.
  */
  if (sep == '.') // Last separator == '.'
    {
      i=strlen(st);
      if (i==1) value=value*100;
      if (i==2) value=value*10;
      if (i==3) value=value*1;
    }

  return value;
}

void
fill_time (sacheader *trace)
{
  int aux;
  time_t t_to, t_eo, t_pa, t_sa;
  double dseconds;

  if (hasstart != 1)
  {
    fprintf(stderr," ERROR:> Can't set time !\n");
    return;
  }

  aux = getjulday (to.tm_year + 1900, to.tm_mon + 1, to.tm_mday);
  if (aux != (to.tm_yday + 1))
    fprintf (stderr, "  ERROR:> Error, Julian day miss-match\n");
  trace->nzyear = to.tm_year + 1900;
  trace->nzjday = to.tm_yday + 1;
  trace->nzhour = to.tm_hour;
  trace->nzmin = to.tm_min;
  trace->nzsec = to.tm_sec;
  trace->nzmsec = mto;

  if (hasorigin == 1)
    {
      t_eo = mktime (&eo);
      t_to = mktime (&to);
      dseconds = (t_eo + meo / 1000.0) - (t_to + mto / 1000.0);
      if (v) fprintf (stderr, "  I:> ORIGIN-START == %f\n", dseconds);
      trace->o = (float) dseconds;
      strncpy(trace->ko,"Origin ",8);
    }

  if (hasp == 1)
    {
      t_pa = mktime (&pa);
      t_to = mktime (&to);
      dseconds = (t_pa + mpa / 1000.0) - (t_to + mto / 1000.0);
      if (v) fprintf (stderr, "  I:> P-Arrival-START == %f\n", dseconds);
      trace->a = (float) dseconds;
      strncpy(trace->ka,"P-wave ",8);
    }

  if (hass == 1)
    {
      t_sa = mktime (&sa);
      t_to = mktime (&to);
      dseconds = (t_sa + msa / 1000.0) - (t_to + mto / 1000.0);
      if (v) fprintf (stderr, "  I:> S-Arrival-START == %f\n", dseconds);
      trace->t0 = (float) dseconds;
      strncpy(trace->kt0,"S-wave ",8);
    }
}

int
getjulday (int year, int month, int day)
{
  int julday;
  int i, leap;
  static char daytab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
  };

  julday = day;
  leap = year % 4 == 0 && year % 100 != 0 || year % 400 == 0;

  if ((month < 1) || (month > 12))
    {
      fprintf (stderr, " ERROR:>  Month = %d is not valid month\n", month);
      exit (1);
    }

  if ((day < 1) || (day > (daytab[leap][month])))
    {
      fprintf (stderr, " ERROR:> Date %d %d %d is not valid date\n",
	       month, day, year);
      exit (1);
    }

  for (i = 1; i < month; i++)
    julday += daytab[leap][i];

  return julday;
}

char *generate_filename (sacheader *trace, int *mode)
{
  int i;
  char station[10] = "";
  char *filename=NULL;
  
  filename=malloc(sizeof(char)*30);
  if (filename == NULL) 
  {
   return NULL;
  }

  strncpy (station, trace->kstnm, 8);
  for(i=7;i>=0;i--)
    if (station[i] != ' ' && station[i] != '\0') {
      station[i+1] = '\0';
      break;
    } else {
      station[i] = '\0';
    }
  
  
  if ( *mode == 1 && hasorigin==0)
  {
   fprintf(stderr," * setting mode=0 no event origin *\n");
   *mode = 0;
  }
  
  switch (*mode)
    {
    case 0:
      if ((trace->kcmpnm[0] >= 65) && (trace->kcmpnm[0] <= 90))
	{
	  sprintf (filename, "%04d.%03d.%02d.%02d.%02d.%s.%c.sac",
		   trace->nzyear,
		   trace->nzjday,
		   trace->nzhour, trace->nzmin, trace->nzsec, station,
		   trace->kcmpnm[0]);
	}
      else
	{
	  sprintf (filename, "%04d.%03d.%02d.%02d.%02d.%s.%c.sac",
		   trace->nzyear,
		   trace->nzjday,
		   trace->nzhour, trace->nzmin, trace->nzsec, station, 'U');
	}
      break;
    case 1:
      if ((trace->kcmpnm[0] >= 65) && (trace->kcmpnm[0] <= 90))
	{
	  sprintf (filename, "%04d.%03d.%02d.%02d.%02d.%s.%c.sac",
		   eo.tm_year+1900,
		   eo.tm_yday+1,
		   eo.tm_hour, eo.tm_min, eo.tm_sec, station,
		   trace->kcmpnm[0]);
	}
      else
	{
	  sprintf (filename, "%04d.%03d.%02d.%02d.%02d.%s.%c.sac",
		   eo.tm_year+1900,
		   eo.tm_yday+1,
		   eo.tm_hour, eo.tm_min, eo.tm_sec, station, 'U');

	}
      break;
     
      default:
       free(filename);
       filename=NULL;
      break;
    }

 return filename;
}

void
fill_station_component (sacheader *trace)
{
  switch (trace->kcmpnm[0])
    {
    /* Trivial ZNE stuff */
    case 'Z':
      trace->cmpinc = 0.0;
      break;
    case 'N':
      trace->cmpinc = 90.0;
      trace->cmpaz = 0.0;
      break;
    case 'E':
      trace->cmpinc = 90.0;
      trace->cmpaz = 90.0;
      break;
    }
}

char getfirstsep(char *str)
{
  int i;

  for(i=0;str[i]!='~';i++)
      if (ispunct(str[i])) return str[i];
}

int fillstructtmfromchar(char *key, struct tm *temp, int *ms)
{
  int ret;
  int stop=0;
  char c;
  
  c=getfirstsep(&key[pvalue]);
  switch(c)
    {
    case(','):
      if (v) fprintf(stderr," I:> Trying to parse comma (,) time string.\n");
      break;
      
    case('-'):
      if (v) fprintf(stderr," I:> Trying to parse normal (-_:) string.\n");
      break;
      
    default:
      if (v) fprintf(stderr,"  W:> Unknow time string (separator == %c), ignoring.\n",c);
      return 0;
      break;
    }
  
  temp->tm_mon  = getmonth(key)-1;
  temp->tm_mday = getday(key);
  temp->tm_year = getyear(key)-1900;
  temp->tm_hour = gethour(key);
  temp->tm_min  = getmin(key);
  temp->tm_sec  = getsec(key);
  temp->tm_isdst= -1;
  *ms = getmsec(key);
  
  /* Year must be positive (i.e. > 1900) */
  if (temp->tm_year < 0) stop=1;
  
  /* Months between 0 .. 11 (Unix struct definition from 0 to 11) */
  if ((temp->tm_mon < 0) || (temp->tm_mon > 11)) stop=1;
  
  /* Days from 1 .. 31 */
  if ((temp->tm_mday < 1) || (temp->tm_mday > 31)) stop=1;
  
  /* Hours between 0 -> 23 */
  if ((temp->tm_hour < 0) || (temp->tm_hour > 23)) stop=1;
  
  /* Minutes between 0 -> 59 */
  if ((temp->tm_min < 0) || (temp->tm_min > 59)) stop=1;
  
  /* Seconds between 0 -> 59 */
  if ((temp->tm_sec < 0) || (temp->tm_sec > 59)) stop=1;
  
  /* mili Seconds between 0 -> 999 */
  if ((*ms < 0) || (*ms > 999)) stop=1;
  
  if (stop == 1)
    {
      temp->tm_mon  = -1;
      temp->tm_mday = -1;
      temp->tm_year = -1;
      temp->tm_hour = -1;
      temp->tm_min  = -1;
      temp->tm_sec  = -1;
      temp->tm_isdst= -1;
      *ms = -1;
      return 0;
    }
  
  /* Fill in the correct julian day */
  if ((ret=mktime(temp)) == -1) return 0;
  
  return 1;
}

