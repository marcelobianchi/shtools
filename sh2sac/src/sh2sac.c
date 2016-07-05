/*
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
#include <load_stationdata.h>
#include <sacempty.h>

/* Version Number */
char version[]="0.5.3";

/* Verbose flag */
int v;

/* Numbers of lines per trace header */
int nlines;

void help()
{
 int i;
 
 fprintf(stderr,"\nNecessary paramter:\n");
 fprintf(stderr,"\t-h <file>: Header QHD file.\n");

 fprintf(stderr,"\nOptionals:\n");
 fprintf(stderr,"\t-b <file>: Binary QBN file.\n");
 fprintf(stderr,"\t-m <mode>: Filename Generation Mode (see table below).\n");
 fprintf(stderr,"\t-stash: Get station coordinates from the Seismic handler instalation directory.\n");
 fprintf(stderr,"\t-sta <file>: Get station coordinates from file <file>. (See format below)\n");
 fprintf(stderr,"\t-v: Verbose on.\n");
 fprintf(stderr,"\t-V: Print version.\n");
 fprintf(stderr,"\t-o: Force overwrite of existing files.\n");

 fprintf(stderr,"\nExtra information:\n");

 fprintf(stderr,"\n  Mode Generation table:\n");
 fprintf(stderr,"\tMode | Type\n");
 fprintf(stderr,"\t--------------------------------\n");
 for(i=0;i<maxmodes;i++)
   fprintf(stderr,"\t%4d | %s\n",i,modetype[i]);

 fprintf(stderr,"\n  Input file format for station coordinate file:\n");
 fprintf(stderr,"\t You can supply two different file formats. The first one is a fixed 5-column mode format,\n");
 fprintf(stderr,"\t with the folowing columns:\n");
 fprintf(stderr,"\t\n");
 fprintf(stderr,"\t  1. Station name (no spaces or tabs in string)\n");
 fprintf(stderr,"\t  2. Latitude\n");
 fprintf(stderr,"\t  3. Longitude\n");
 fprintf(stderr,"\t  4. Elevation\n");
 fprintf(stderr,"\t  5. Network Name (no spaces or tabs in string)\n");
 fprintf(stderr,"\t\n");
 fprintf(stderr,"\t the other supported file format is the valid seismic handler STATINF.DAT file.\n");

}

int
get_nt (FILE * ent)
{
  char aux[MAXLINE];
  char old[4];
  int nt = 0;

  fgets (aux, MAXLINE, ent);
  strncpy (old, aux, 3);
  nt=1;  
  while (!feof (ent))
    {
      fgets (aux, MAXLINE, ent);
      if (strncmp (old, aux, 3) != 0)
	{
	  strncpy (old, aux, 3);
	  nt++;
	}
    }

  rewind (ent);
  fgets (aux, MAXLINE, ent);
  return nt;
}

void jump_coments(FILE *ent,int cmtlines, int print)
{
 char cmt[MAXLINE];

 cmtlines--;
 while (cmtlines > 0)
   {
     fgets (cmt, MAXLINE, ent);
     if (print != 0) fprintf (stderr, "I:> CMT-%03d|%s", cmtlines, cmt);
     cmtlines--;
   }
}

FILE *
init (char *filename, int *nt)
{
  FILE *ent;
  int magic, cmtlines;

  if ((ent = fopen (filename, "r")) == NULL)
    return NULL;

  fscanf (ent, "%d%d%d\n", &magic, &cmtlines, &nlines);
  if (magic == QMAGIC)
    {
      fprintf (stderr,
	       "I:> Q-file opened and checked with %d comment lines and %d lines/entry.\n",
	       cmtlines, nlines);

      jump_coments(ent,cmtlines,1);
      
      *nt = get_nt (ent);
      fprintf (stderr, "I:> Q-file show %d traces.\n", *nt);

      jump_coments(ent,cmtlines,0);
    }
  else
    {
      ent = NULL;
    }

  return ent;
}

char *
 get_next_trace (FILE * ent)
{
  int i;
  char *st = NULL;
  char aux[MAXLINE];
  int size = 0;
  char order[4];

  st=malloc(sizeof(char));
  st[0]='\0';
  for (i = 0; i < nlines; i++)
  {
      /* Reading */
      fgets (aux, MAXLINE, ent);
      if (feof (ent))
	{
	  fprintf (stderr, "End of File");
	  return NULL;
	}
      /* Removing /n from last char */
      aux[strlen (aux) - 1] = '\0';
      
      // fprintf (stderr, "  L:%d -> %s:: MOre: ", i,aux);
      fprintf (stderr, "%d", i);
      
      /* Sequence checking ! */
      if (i == 0)
	strncpy (order, aux, 3);
      else if (strncmp (order, aux, 3) != 0)
	{
	  fprintf (stderr, "ops");
	  if (st != NULL) free (st);
	  return NULL;
	}

      size = strlen(st) + strlen (&aux[3]);
      
      /* Get more memory */
      st = realloc (st, sizeof (char) * size + 1);
      if (st == NULL) return NULL;

      /* Save the line and go on */
      strcat (st, &aux[3]);
  }

  return st;
}


long
fill_sach (char *hdr, sacheader *sach)
{
  int pos = 0, i;
  char key[MAXKEY];
  long dposition = -1;

  /* Zero global headers steps */
  hasstart = 0;
  hasorigin = 0;
  hasp = 0;
  hass = 0;
  dposition = -1;

  /* Get a new clean header */
  memcpy (sach, &sach_null, sizeof (sach_null));
  sach->b = 0.0;
  sach->nvhdr = 6;
  sach->iftype = 1;
  sach->leven = 1;
  sach->lcalda = 0;

  /* Go thru all header variables */
  pos = 0;
  while (strlen (&hdr[pos]) > 2)
    {
      for (i = 0; hdr[pos + i - 1] != '~'; i++)
	key[i] = hdr[pos + i];
      key[i] = '\0';
      pos += strlen (key) + 1;

      switch (key[0])
	{
	case 'I':
	  parse_i (sach, key);
	  break;

	case 'L':
	  parse_l (sach, key, &dposition);
	  break;

	case 'R':
	  parse_r (sach, key);
	  break;

	case 'C':
	  parse_c (sach, key);
	  break;

	case 'S':
	  parse_s (sach, key);
	  break;

	default:
	  fprintf (stderr, " I:> Can't handle: %c\n", key[0]);
	  break;
	}
    }

  return dposition;
}

void
write_sac (sacheader *sach, char *datafile, long dposition, int *mode, int overwrite)
{
  FILE *sai=NULL;
  FILE *dent;
  float *y=NULL;
  char *sfilename=NULL;
  
  /* generate the filename */
  sfilename=generate_filename (sach, mode);
  if (sfilename == NULL )
  {
    fprintf (stderr, "I:> Error generating filename.\n");
    return;
  }
   
  /* Allocation of memory */
  y = calloc (sizeof (float),sach->npts);
  if (y == NULL)
  {
   fprintf(stderr, "I:> Error getting memory.\n");
   return;
  }

  /* Open and read binary part of data */
  if ((dent=fopen(datafile,"r"))==NULL)
  {
    fprintf (stderr, "I:> Error openning input data file.\n");
    free(y);
    return;
  }

  fseek (dent, sizeof (float) * dposition, SEEK_SET);
  fread (y, sizeof (float), sach->npts, dent);
  fclose(dent);

  if (v) 
    fprintf (stderr, " F:> Dumping data to %s\n", sfilename);
  else
    fprintf (stderr, "%s", sfilename);

  /* Opening the output file */
  sai = fopen (sfilename, "r");
  if (sai != NULL ) 
  {
   fclose(sai);
   
   if (!overwrite)
   {
    if (v) 
     fprintf (stderr, "  I:> File %s exist, will not over-write.\n",sfilename);
    else 
     fprintf (stderr, " :: File exist, aborted.");
    free(y);
    return;
   } else { 
     if (v) 
      fprintf (stderr, "  I:> File %s exist, WILL over-write as requested.\n",sfilename);
     else 
      fprintf (stderr, ":: File exist, WILL over-write.");
   }
  }
  
  sai=NULL;
  sai = fopen (sfilename, "w");
  if (sai == NULL)
  {
   if (v) 
    fprintf (stderr, "  I:> Error openning output file. Directory permission wrong?\n");
   else     
    fprintf (stderr, ":: Failed, can't open outfile.");
   free(y);
   return;
  }

 
  /* Writing SAC file */
  fwrite (sach, sizeof (sacheader), 1, sai);
  fwrite (y, sizeof (float), sach->npts, sai);
  fclose (sai);

  /* Clean up */
  if (sfilename!=NULL)
  {
   free(sfilename);
   sfilename=NULL;
  }
  
  if (y!=NULL)
  {
   free (y);
   y = NULL;
  }
}


int 
main (int argc, char **argv)
{
  FILE *ent;
  
  char *hdr = NULL;
  int counter = 1;
  int nt,mode=1,i;
  int overwrite=0;
  sacheader sach;
  long dposition=-1;

  /* files to convert */
  char *qhd=NULL;
  char *qbn=NULL;

  /* Station location file */
  int hassta = 0;
  char *stafilename = NULL;
  char *sh_root=NULL;
  station sta;
  
  sta.n=0;
  sta.stinf=NULL;

  for (i=1;i<argc;i++)
  {
   if (strncmp(argv[i],"-h",2) == 0 && argc > (i+1))
   {
     qhd=argv[i+1];
     i++;
   } else if (strncmp(argv[i],"-b",2)==0 && argc > (i+1)) 
     {
     qbn=argv[i+1];
     i++;
   } else if (strncmp(argv[i],"-o",2)==0) 
     {
       overwrite=1;
   } else if (strncmp(argv[i],"-m",2)==0 && argc > (i+1)) 
     {
      mode=atoi(argv[i+1]);
      i++;
   } else if (strncmp(argv[i],"-V",2)==0) 
     {
      fprintf(stderr,"This is sh2sac v. %s - Seismic Handler Q to SAC converter\n",version);
      fprintf(stderr,"this software is GPL, you must have received a copy of the GPL license along.\n");
      return 0;
   } else if (strncmp(argv[i],"-v",2)==0) 
     {
      v = 1;
   } else if (strncmp(argv[i],"-stash",6)==0)
     {
      sh_root=getenv("SH_ROOT");
      if (sh_root == NULL)
      {
       fprintf(stderr,"Can't find a validy seismic handler instalation. No SH_ROOT variable !\n");
       return -1;
      }
      hassta=1;
   } else if (strncmp(argv[i],"-sta",4)==0 && argc > (i+1)) 
     {
      stafilename=argv[i+1];
      i++;
      hassta=1;
   }
  }

  if (qhd == NULL)
  {
   fprintf(stderr,"\nERROR:> You must supply -h <QHD file>.\n");
   help();
   return -1;
  }
  
   if ( mode < maxmodes && mode >= 0)
   {
      if (v) fprintf(stderr,"I:> Using mode == %d [%s]\n",mode,modetype[mode]);
   }
   else
   {
    help();
    fprintf(stderr,"I:> Mode supplied bigger than max modes. Mode %d supplied. \n",mode);
    return -1;
   }  

  if (qbn == NULL)
     fprintf(stderr,"\nW:> You  didn't pass any binary (real data) files, you will got any sac output.\n");

   if (hassta == 1)
   {
    if (sh_root != NULL && stafilename == NULL)
    {
      stafilename = (char *)malloc(sizeof(char)*(strlen(sh_root)+30));
      sprintf(stafilename,"%s/inputs/STATINF.DAT",sh_root);
    }
    fprintf(stderr,"I:> Using: %s as station file.\n",stafilename);
    load_stationdata(stafilename, &sta);
   } else
    {
     fprintf(stderr,"W:> No station coordinate information.\n");
    }


  ent = init (qhd, &nt);
  if (ent==NULL) 
  {
   fprintf(stderr,"E:> Could not read the file: %s\n",qhd);
   return -1;
  } 

  for (counter = 0; counter < nt; counter++)
    {
      fprintf (stderr, "I:> Trace %4d [", counter);
      hdr = get_next_trace (ent);
      /* fprintf(stderr,"\n\nZ%sZ\n\n",hdr); // DEBUGING */
      if (hdr == NULL)
	{
	  fprintf (stderr, "] Failed !\n");
	  fclose (ent);
	  return -1;
	}
      else
	{
  	  if (!v && qbn!=NULL) 
  	    fprintf (stderr, "] -> ");
          else
            fprintf (stderr,"] ok !\n");

	  dposition = fill_sach (hdr, &sach);
	  if (dposition>=0 && qbn!=NULL)
	  {
	   fill_time (&sach);
	   if (hassta==1) fill_station_coordinates(&sach,&sta);
	   fill_station_component(&sach);
	   write_sac (&sach,qbn,dposition,&mode,overwrite);
	  }
      	  if (!v && qbn!=NULL)fprintf(stderr,"\n");

 
      	  if (hdr!=NULL)
	  {
	   free (hdr);
           hdr=NULL;
	  } else 
	   {
	    fprintf(stderr,"E:> Erro liberando HDR!\n");
	    return -1;
	   }
	   
	}
     if (v) fprintf(stderr,"\n");
    }


  if (hassta==1)
    if (sta.n>0 && sta.stinf!=NULL)
    {
      sta.n=0;
      free(sta.stinf);
      sta.stinf=NULL;
    }

  fclose (ent);
  return 0;
}

