#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid_reader.h"

int read_mesh_file(char *fname, double **x, double **y, int *nn, int *nt, int ***tri_conn, int *nb, int **nbs, int ****bs)
{
     FILE *fp;
     int bdim = 80;
     char buff[bdim];
     int i, n, t, b;
     int nblocks = 0;

     /* Begin reading 2D grid file */

     /* Open 2D grid file for reading */
     if ((fp=fopen(fname,"r")) == NULL)
     {
	  printf("\nCould not open file <%s>\n",fname);
	  exit(0);
     }

     /* Read number of grid points */
     fgets(buff,bdim,fp); // Header text from file
     fgets(buff,bdim,fp); // Line containing number of grid points
     sscanf(buff,"%d",nn);
     printf("\nNumber of grid points = %d",*nn);
     if (((*x) = (double*)malloc((*nn)*sizeof(double))) == NULL)
     {
	  printf("\nCould not allocate memory for X");
	  exit(0);
     }
     if (((*y) = (double*)malloc((*nn)*sizeof(double))) == NULL)
     {
	  printf("\nCould not allocate memory for Y");
	  exit(0);
     }
     /* Read 2D grid nodes */
     for (n=0; n < (*nn); n++)
     {
	  fgets(buff,bdim,fp);
	  sscanf(buff,"%lg %lg",&(*x)[n],&(*y)[n]);
     }
  
     /* Read number of blocks */
     fgets(buff,bdim,fp); // Header text from file
     fgets(buff,bdim,fp); // Line containing number of blocks
     sscanf(buff,"%d",&nblocks);
     if (nblocks != 1)
     {
	  printf("\nNumber of blocks should be 1");
	  exit(0);
     }

     /* Read number of triangular elements */
     fgets(buff,bdim,fp); // Header text from file
     fgets(buff,bdim,fp); // Line containing number of tri-elements
     sscanf(buff,"%d",nt);
     printf("\nNumber of triangles = %d",*nt);
     if (((*tri_conn) = (int**)malloc((*nt)*sizeof(int*))) == NULL)
     {
	  printf("\nCould not allocate memory for triangle connectivity");
	  exit(0);
     }
     for (t=0; t < (*nt); t++)
     {
	  if (((*tri_conn)[t] = (int*)malloc(3*sizeof(int))) == NULL)
	  {
	       printf("\nCould not allocate memory for triangle connectivity");
	       exit(0);
	  }
	  /* Read in connectivity */
	  /* Indexing should be FORTRAN-like(i.e. numbering starts at 1, instead of 0) */
	  fgets(buff,bdim,fp);
	  sscanf(buff,"%d %d %d",&(*tri_conn)[t][0],&(*tri_conn)[t][1],&(*tri_conn)[t][2]);
	  /* decrement numbers for c indexing */
	  --(*tri_conn)[t][0];
	  --(*tri_conn)[t][1];
	  --(*tri_conn)[t][2];
     }

     // checking if there is quad elemets.
     int nquad = 0;
     fgets(buff,bdim,fp);
     fgets(buff,bdim,fp);
     sscanf(buff,"%d",&nquad);
     if(nquad != 0)
       {
	 printf("quad elements are in the grid! this algorithm is only working for triangle. exiting ...");
	 exit(0);
       }

     // allocate for points per boundary
     fgets(buff,bdim,fp);
     fgets(buff,bdim,fp);
     sscanf(buff,"%d",nb);
     printf("\nNumber of boundaries = %d",*nb);
//     (*nbs) = new int[*nb];
     (*nbs) = (int *)malloc((*nb) * sizeof(int));     
     (*bs) = (int***)malloc((*nb)*sizeof(int**));
     for (b=0; b < (*nb); b++)
       {
	 fgets(buff,bdim,fp);
	 fgets(buff,bdim,fp);
	 sscanf(buff,"%d",&(*nbs)[b]);
  
	 printf("\nBoundary %d has %d segments",b,(*nbs)[b]);

	 (*bs)[b] = (int**)malloc((*nbs)[b]*sizeof(int*));
	 for (i=0; i < (*nbs)[b]; i++)
	   {
	     (*bs)[b][i] = (int*)malloc(2*sizeof(int));
	     fgets(buff,bdim,fp);
	     sscanf(buff,"%d %d",&(*bs)[b][i][0], &(*bs)[b][i][1]);
	     (*bs)[b][i][0]--;
	     (*bs)[b][i][1]--;
	   }
       }
    
     //closing the input file
     fclose(fp);
     //comepete successfully!
     return 0;
}

int write_mesh_gnu_files(char *fname, double *x, double *y, int nn, int nt, int tri[][3], int nb, int *nbs, int ***bs)
{

  FILE *fp;
  int bdim = 80;
  char buff[bdim];
  int i, j;

  printf("\nFilename = <%s>",fname);
  // Open file for write
  if ((fp = fopen(fname,"w")) == 0)
  {
    printf("\nError opening file <%s>.",fname);
    exit(0);
  }

  // Write out nodes
  fprintf(fp,"#Number of grid points\n");
  fprintf(fp,"%d\n",nn);
  for (i=0; i < nn; i++)
    fprintf(fp,"%19.10e %19.10e\n",x[i],y[i]);

  fprintf(fp,"#Number of blocks\n");
  fprintf(fp,"1\n");

  fprintf(fp,"#Number of triangular elements\n");
  fprintf(fp,"%d\n",nt);
  for (i=0; i < nt; i++)
    fprintf(fp,"%d %d %d\n",tri[i][0]+1,tri[i][1]+1,tri[i][2]+1);

  fprintf(fp,"#Number of quadrilateral elements\n");
  fprintf(fp,"%d\n",0);

  fprintf(fp,"#Number of boundaries\n");
  fprintf(fp,"%d\n",nb);

  for (i=0; i < nb; i++)
  {
    fprintf(fp,"#Number of edges for boundary %d\n",i+1);
    fprintf(fp,"%d\n",nbs[i]);

    for (j=0; j < nbs[i]; j++)
      fprintf(fp,"%d %d\n",bs[i][j][0]+1,bs[i][j][1]+1);
  }

  fclose(fp);

  //
  //
  // write GNUPLOT file
  //
  buff[0]='\0';
  strcpy(buff,fname);
  strcat(buff,".dat");
  printf("\nFilename = <%s>\n",buff);
  // Open file for write
  if ((fp = fopen(buff,"w")) == 0)
  {
    printf("\nError opening file <%s>.",buff);
    exit(0);
  }
  for (i=0; i < nt; i++)
  {
    int n0 = tri[i][0];
    int n1 = tri[i][1];
    int n2 = tri[i][2];
    fprintf(fp,"%19.10e %19.10e 0.0\n",  x[n0],y[n0]);
    fprintf(fp,"%19.10e %19.10e 0.0\n",  x[n1],y[n1]);
    fprintf(fp,"%19.10e %19.10e 0.0\n",  x[n2],y[n2]);
    fprintf(fp,"%19.10e %19.10e 0.0\n\n",x[n0],y[n0]);
  }
  fclose(fp);

  //completed successfully
  return 0;

}