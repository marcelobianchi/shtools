#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sac.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define SKIPCH 3           /* number of chars skipped at each hdr line */
#define MINTRCLIN 4        /* minimum number of lines per trace in q-hdr */
#define QMAGIC 0xABCD      /* magic number of q-files */
#define QFC_HLINELTH 79    /* maximum length of lines in header file */


typedef struct {
  char *infilename;
  char *outfilenameB;
  char *outfilenameH;
  sacheader h;
  struct tm start; // time corrected for B.
  int mstart;      // milli second time correct for B. 
  float *y;
  char *sh;
} data;

void help();
void  fixms(int *ms, int *seconds);
int loadsac(data *s);
int getday (int year, int jday);
int getmo (int year, int jday);
int copyto(char **s,char *string);
int numberOfShLines(char *hdr);
int makeshh(data *s);
void nulilize(data *s);
void cleanSH(data *s);

int v=0;

int loadsac(data *s){
  FILE *ent;
  int i;
  time_t ret;

  ent=fopen(s->infilename,"r");
  if (ent==NULL) {
    if (v) fprintf(stderr," (File %s don't exist).\n",s->infilename);
    return -1;
  }
  
  fread(&s->h,sizeof(sacheader),1,ent);
  if (s->h.nvhdr != 6 && s->h.iftype !=1) {
    if (v) fprintf(stderr," (File not real sac file).\n");
    fclose(ent); 
    return -1;
  } else {
    if (v) fprintf(stderr," (opened) ");
  }


  s->y=malloc(sizeof(float)*s->h.npts);
  if (s->y == NULL) { fclose(ent); return -1;}
  i=fread(s->y,sizeof(float),s->h.npts,ent);
  if (i!=s->h.npts) {
    if (v) fprintf(stderr," (Can't read data part).\n");
    fclose(ent); 
    free(s->y); 
    return -1;
  } else {
    if (v) fprintf(stderr," (readed) ");
  }


  if (s->h.nzyear  != -12345 && 
      s->h.nzjday  != -12345 && 
      s->h.nzhour  != -12345 && 
      s->h.nzmin   != -12345 && 
      s->h.nzsec   != -12345 &&
      s->h.nzmsec  != -12345
      )
    {
      /* time conversion and reading */
      s->start.tm_year = s->h.nzyear - 1900;
      s->start.tm_mday = getday(s->h.nzyear,s->h.nzjday);
      s->start.tm_mon  = getmo(s->h.nzyear,s->h.nzjday)-1;
      s->start.tm_hour = s->h.nzhour;
      s->start.tm_min  = s->h.nzmin;
      s->start.tm_sec  = s->h.nzsec;
      s->start.tm_isdst= -1;
      s->mstart = s->h.nzmsec;
      if ((ret=mktime(&s->start)) == -1) { 
	if (v) fprintf(stderr," (Time reading error).\n");
	fclose(ent); 
	free(s->y);
	return -1;
      } else {
	if (v) fprintf(stderr," (timed) ");
      }
      
      if ((s->start.tm_yday+1) != s->h.nzjday) {
	fprintf(stderr," (Error checking jday date).\n"); 
	fclose(ent); 
	free(s->y); 
	return -1;
      } else {
	if (v) fprintf(stderr," (Jtimed) ");
      }
      
      if (s->h.b != 0.0)
	{
	  if (v) fprintf(stderr," (Applying B)\n");
	  s->start.tm_sec += (int)(s->h.b);
	  s->mstart  += (int) ( 1000 * (float)(s->h.b - (int)(s->h.b)) );
	  fixms(&s->mstart,&s->start.tm_sec);
	}

      if ((ret=mktime(&s->start)) == -1) { 
	fprintf(stderr," (Error checking time after B).\n"); 
	fclose(ent); 
	free(s->y);
	return -1;
      }
    } else {
    if (v) fprintf(stderr," (No trace start information)\n");
  }
  
  fclose(ent);
  return 0;
}

int makeshh(data *s)
{
  char temp[1000];
  char *monthname[13]={ "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
  struct tm Dtemp;
  int Dtempm;
  
  s->sh=NULL;

  /* Npts */
  sprintf(temp,"L001:%d~ ",s->h.npts);
  if (v) fprintf(stderr," NPTS:> %s\n",temp);
  copyto(&s->sh,temp);

  /* offset */
  sprintf(temp,"L000:0~ ");
  if (v) fprintf(stderr," Offset:> %s\n",temp);
  copyto(&s->sh,temp);
  
  /* Delta */
  sprintf(temp,"R000:%.6f~ ",s->h.delta);
  if (v) fprintf(stderr," Delta:> %s\n",temp);
  copyto(&s->sh,temp);

  /* Component */
  if (s->h.cmpinc==0.0){
    sprintf(temp,"C000:Z~ ");
    copyto(&s->sh,temp);
  } else if (s->h.cmpinc==90.0 && s->h.cmpaz==0.0){
    sprintf(temp,"C000:N~ ");
    copyto(&s->sh,temp);
  } else if (s->h.cmpinc==90.0 && s->h.cmpaz==90.0){
    sprintf(temp,"C000:E~ ");
    copyto(&s->sh,temp);
  } else {
    sprintf(temp,"C000:U~ ");
    copyto(&s->sh,temp);
  }
  if (v) fprintf(stderr," CMP:> %s\n",temp);

  /* Event Coordinates */
  if (s->h.evla != -12345.0){
    sprintf(temp,"R016:%f~ ",s->h.evla);
    if (v) fprintf(stderr," EVLA:> %s\n",temp);
    copyto(&s->sh,temp);
  }
  if (s->h.evlo != -12345.0){
    sprintf(temp,"R017:%f~ ",s->h.evlo);
    if (v) fprintf(stderr," EVLO:> %s\n",temp);
    copyto(&s->sh,temp);
  }
  if (s->h.evdp != -12345.0){
    sprintf(temp,"R014:%f~ ",s->h.evdp);
    if (v) fprintf(stderr," EVDP:> %s\n",temp);
    copyto(&s->sh,temp);
  }
  
  /* Distance */ 
  /*
  if (s->h.gcarc != -12345.0){
    sprintf(temp,"R011:%f~ ",s->h.gcarc);
    if (v) fprintf(stderr," GCarc:> %s\n",temp);
    copyto(&s->sh,temp);
  }
  */

  /* Back-Azimuth */
  /*
  if (s->h.baz != -12345.0){
    sprintf(temp,"R012:%f~ ",s->h.baz);
    if (v) fprintf(stderr," BAZ:> %s\n",temp);
    copyto(&s->sh,temp);
  }
  */

  if (s->h.nzyear  != -12345 && 
      s->h.nzjday  != -12345 && 
      s->h.nzhour  != -12345 && 
      s->h.nzmin   != -12345 && 
      s->h.nzsec   != -12345 &&
      s->h.nzmsec  != -12345
      )
    {
      /* Origin time */
      // 6-JUN-2007_12:11:04.938
      sprintf(temp,"S021:%2d-%s-%4d_%02d:%02d:%02d.%03d~ ",
	      s->start.tm_mday,
	      monthname[s->start.tm_mon+1],
	      s->start.tm_year+1900,
	      s->start.tm_hour,
	      s->start.tm_min,
	      s->start.tm_sec,
	      s->mstart
	      );
      if (v) fprintf(stderr," START:> %s\n",temp);
      copyto(&s->sh,temp);
    }

  /* P Onset */
/*
  Dtemp = s->start;
  Dtempm = s->mstart;
  if (s->h.a != -12345.0){
    float value = s->h.a - s->h.b;
    Dtemp.tm_sec += (int) value;
    Dtempm       += (int) ( 1000 * (float)(value - (int)(value)) );
    fixms(&Dtempm, &Dtemp.tm_sec);
    mktime(&Dtemp);
    
    sprintf(temp,"S022:%2d-%s-%4d_%02d:%02d:%02d.%03d~ ",
	    Dtemp.tm_mday,
	    monthname[Dtemp.tm_mon+1],
	    Dtemp.tm_year+1900,
	    Dtemp.tm_hour,
	    Dtemp.tm_min,
	    Dtemp.tm_sec,
	    Dtempm
	    );
    if (v) fprintf(stderr," START:> %s\n",temp);
    copyto(&s->sh,temp);      
  }
*/

  /* O Onset */
  Dtemp = s->start;
  Dtempm = s->mstart;
  if (s->h.o != -12345.0){
    float value = s->h.o - s->h.b;
    Dtemp.tm_sec += (int) value;
    Dtempm       += (int) ( 1000 * (float)(value - (int)(value)) );
    fixms(&Dtempm, &Dtemp.tm_sec);
    mktime(&Dtemp);
    
    sprintf(temp,"S024:%2d-%s-%4d_%02d:%02d:%02d.%03d~ ",
	    Dtemp.tm_mday,
	    monthname[Dtemp.tm_mon+1],
	    Dtemp.tm_year+1900,
	    Dtemp.tm_hour,
	    Dtemp.tm_min,
	    Dtemp.tm_sec,
	    Dtempm
	    );
    if (v) fprintf(stderr," Origin:> %s\n",temp);
    copyto(&s->sh,temp);      
  }



  /* Station name */
  {
    int min,max;
    char *pvar;
    int i;
    int j;

    pvar=s->h.kstnm;
    for(i=0,min=0;i<8;i++)
      if (pvar[i]!=' '&& pvar[i] != '\0') 
	{ 
	  min=i;
	  break;
	}
    
    for(i=0,max=0;i<8;i++)
      if (pvar[i] != ' ' && pvar[i] != '\0') max=i;
    
    strcpy(temp,"S001:");
    for (i=min,j=5;i<max+1;i++,j++)
      temp[j]=pvar[i];
    
    fprintf(stderr,"%d",j);
    temp[j]='~';
    temp[j+1]=' ';
    temp[j+2]='\0';
    if (v) fprintf(stderr," STATION:> %s\n",temp);
    copyto(&s->sh,temp);
  }

  return 0;
}

int fillfilename(data *s)
{
  int i;

  if (s->infilename==NULL) return -1;

  s->outfilenameB=NULL;
  s->outfilenameB=malloc(sizeof(char)*(strlen(s->infilename)+4+1));
  if (s->outfilenameB==NULL) return -1;
  sprintf(s->outfilenameB,"%s.QBN",s->infilename);
  for (i=0;i<strlen(s->outfilenameB);i++) s->outfilenameB[i]=toupper(s->outfilenameB[i]);

  s->outfilenameH=NULL;
  s->outfilenameH=malloc(sizeof(char)*(strlen(s->infilename)+4+1));
  if (s->outfilenameH==NULL) return -1;
  sprintf(s->outfilenameH,"%s.QHD",s->infilename);
  for (i=0;i<strlen(s->outfilenameH);i++) s->outfilenameH[i]=toupper(s->outfilenameH[i]);
  
  if (v) fprintf(stderr," QBN:> %s\n QHD:> %s\n",s->outfilenameB,s->outfilenameH);
  return 0;
}


int writeSH(data *s)
{
  FILE *out;
  int fcounter=1;
  int lasts,nextchar;
  int lines,lcounter;

  /* Binary part */
  out=fopen(s->outfilenameB,"w");
  if (out!=NULL)
    fwrite(s->y,sizeof(float),s->h.npts,out);
  fclose(out);

  /* Header part */
  out=fopen(s->outfilenameH,"w");
  if (out!=NULL)
    {
      lines=numberOfShLines(s->sh);
      fprintf(out,"%d 1 %d",QMAGIC,lines);

      lasts=0;
      nextchar=0;
      lcounter=0;
      while(nextchar<strlen(s->sh)){
	if (lasts==0) {
	  fprintf(out,"\n%02d|",fcounter);
	  lasts=QFC_HLINELTH-3-2;
	  lcounter++;
	}
	fputc(s->sh[nextchar],out);
	nextchar++;
	lasts--;
      }
      
      for(;lcounter<lines;lcounter++)
	fprintf(out,"\n%02d|",fcounter);

      fprintf(out,"\n");
    }
  fclose(out);
  
  return 0;
}

int main(int argc, char **argv)
{
  data s;
  int ifile;
  int i,j;
  extern int v;
  
  if (argc == 1) {
    help();
    return 0;
  }

  v=0;
  nulilize(&s);

  for(ifile=1;ifile<argc;ifile++){
    if ((j=strncmp(argv[ifile],"-v",2))==0) v=1;
  }
  
  for(ifile=1;ifile<argc;ifile++){
    if ((j=strncmp(argv[ifile],"-v",2))==0)  continue;

    if (v) fprintf(stderr,"\n");

    /* Input filename */
    copyto(&s.infilename,argv[ifile]);
    if (v) fprintf(stderr,"Reading %s:\n",s.infilename); 
    i=loadsac(&s);
    if (i != 0) {
      fprintf(stderr,"Error on sac file reading (%s).\n",s.infilename);
      cleanSH(&s);
      continue;
    }
    
    /* preparing filename */
    if (v) fprintf(stderr,"Preparing filenames:\n"); 
    fillfilename(&s);
    
    /* getting new header */
    if (v) fprintf(stderr,"Filling header:\n"); 
    makeshh(&s);
    
    /* Write the sh files */
    if (v) fprintf(stderr,"\n");
    fprintf(stderr,"%s -> (%4d-%2d %8.1fs %8d %6.4f) -> %s %s\n",
	    s.infilename,
	    s.h.nzyear,
	    s.h.nzjday,
	    (s.h.npts-1)*s.h.delta,
	    s.h.npts,
	    s.h.delta,
	    s.outfilenameB,
	    s.outfilenameH
	    );
    writeSH(&s);

    /* prepare for next file */
    cleanSH(&s);
  }

  return 0;
}

int numberOfShLines(char *hdr)
{
  int n=strlen(hdr);
  int perline=QFC_HLINELTH-SKIPCH-2;
  int line;

  line=0;
  while (n>0)
    {
      line++;
      n=n-perline;
    }
  
  if (line<MINTRCLIN) line=MINTRCLIN;
  return line;
}

int getmo (int year, int jday)
{
  int leap, mo;
  static char daytab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} 
  };

  leap = year % 4 == 0 && year % 100 != 0 || year % 400 == 0;

  mo=1;
  while ((jday > daytab[leap][mo])&&(mo<13))
    {
      jday -= daytab[leap][mo];
      mo++;
    }
    
  if (mo>=13) return -1;
  return mo;
}
 
int getday (int year, int jday)
{
  int leap, mo;
  static char daytab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} 
  };

  leap = year % 4 == 0 && year % 100 != 0 || year % 400 == 0;


  mo=1;
  while ((jday > daytab[leap][mo])&&(mo<13))
    {
      jday -= daytab[leap][mo];
      mo++;
    }
     
  if (mo>=13) return -1;
  if (jday>daytab[leap][mo]) return -1;
  return jday;
}


void  fixms(int *ms, int *seconds)
{
  if (*ms >= 1000){
    *ms -= 1000;
    *seconds += 1;
    fixms(ms,seconds);
  } else if (*ms < 0){
    *ms += 1000;
    *seconds -= 1;
    fixms(ms,seconds);
  } else {
    return;
  }
}

void nulilize(data *s)
{
  s->infilename = NULL;
  s->outfilenameB = NULL;
  s->outfilenameH = NULL;
  s->y  = NULL;
  s->sh = NULL;
}

void cleanSH(data *s)
{
  if (s->infilename != NULL) free (s->infilename);
  if (s->outfilenameB != NULL) free(s->outfilenameB);
  if (s->outfilenameH != NULL) free(s->outfilenameH);
  if (s->y != NULL) free(s->y);
  if (s->sh !=NULL) free(s->sh);
  nulilize(s);
}

int copyto(char **s,char *string)
{
  int atual;
  int ss;
  
  ss=strlen(string); 
  if (*s == NULL) {
    atual=0;
    *s=realloc(*s,sizeof(char)*(atual+ss+1));
    *s[0]='\0';
  } else {
    atual=strlen(*s);
    *s=realloc(*s,sizeof(char)*(atual+ss+1));
  }

  strcat(*s,string);
  return 0;
}

void help()
{
  fprintf(stderr,"sac2sh software\n  Usage:\n  sac2sh [-v] <file1.sac> <file2.sac> ... <filen.sac>\n");
}
