#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "types_cnsts.h"
#include "init_par_info.h"
#include "init_file_info.h"
#include "matlab_functions.h"
#include "process_raw_avhrr_acspo_c.h"
#include "process_raw_geo_c.h"

int main( int argc, char *argv[] );
void run_leo_processing(char *pNOAA_Name, VINT year, VINT day );

int main( int argc, char *argv[] )
{
  VINT year = 0;
  VINT day = 0;
  char noaa_name[MAX_STRING_LENGTH];
  
  if( 4 != argc ){
    fprintf(stderr,"USAGE: ./test_readers NOAA_NAME year dayno\n");
    exit(-1);
  }

  strcpy(noaa_name,argv[1]);
  if( 1 != sscanf(argv[2],"%d",&year) ){
    fprintf(stderr,"ERROR: Parsing year from command line\n");
    exit(-1);
  }
  if( 1 != sscanf(argv[3],"%d",&day) ){
    fprintf(stderr,"ERROR: Parsing dayno from command line\n");
    exit(-1);
  }

  run_leo_processing(noaa_name, year, day);
}

void run_leo_processing(char *pNOAA_Name, VINT year, VINT day )
{
  VINT i = 0;
  VINT Total_badclim[2] = {0,0};
  VINT Total_badstd[2] = {0,0};
  VINT ok = 0;

  struct sst_struct Ref_SST;
  struct sst_struct SST_Variability;
  struct float_struct biasDay;
  struct float_struct biasNight;
  struct out_sst_str outData[2];
  struct int_struct DumpArray[2];
  struct par_files_str par_files;

  /* Initialize structures */
  init_sst_matlab(&Ref_SST);
  init_sst_matlab(&SST_Variability);
  init_float_array_matlab(&biasDay);
  init_float_array_matlab(&biasNight);
  init_out_sst_matlab(&outData[0]);
  init_out_sst_matlab(&outData[1]);
  init_int_array_matlab(&DumpArray[0]);
  init_int_array_matlab(&DumpArray[1]);

  /* Read in parameter files */
  strcpy(par_files.file_info,"init_file_info.m");
  strcpy(par_files.par_info,"init_par_info.m");
  init_file_info(par_files.file_info);
  init_par_info(par_files.par_info);  

  /* Allocate references */
  allocate_float_array_matlab(par_info.spatial_resolution[0],
			      par_info.spatial_resolution[1],&Ref_SST.sst);
  allocate_float_array_matlab(par_info.spatial_resolution[0],
			      par_info.spatial_resolution[1],
			      &SST_Variability.sst);

  /* Setup reference values */
  for(i=0;i<Ref_SST.sst.size;i++){
    *(Ref_SST.sst.array+i) = 0.;
  }
  Ref_SST.minlat = par_info.ref_latmin;
  Ref_SST.minlon = par_info.ref_lonmin;
  Ref_SST.deltalat = 1./par_info.cells_per_degree;
  Ref_SST.deltalon = 1./par_info.cells_per_degree;
  Ref_SST.maxlat = Ref_SST.minlat + par_info.spatial_resolution[0] * 
    Ref_SST.deltalat;
  Ref_SST.maxlon = Ref_SST.minlon + par_info.spatial_resolution[1] * 
    Ref_SST.deltalon;

  for(i=0;i<SST_Variability.sst.size;i++){
    *(SST_Variability.sst.array+i) = 10000.;
  }
  SST_Variability.minlat = par_info.ref_latmin;
  SST_Variability.minlon = par_info.ref_lonmin;
  SST_Variability.deltalat = 1./par_info.cells_per_degree;
  SST_Variability.deltalon = 1./par_info.cells_per_degree;
  SST_Variability.maxlat = SST_Variability.minlat + 
    par_info.spatial_resolution[0] * 
    SST_Variability.deltalat;
  SST_Variability.maxlon = SST_Variability.minlon + 
    par_info.spatial_resolution[1] * 
    SST_Variability.deltalon;
  
  allocate_float_array_matlab(par_info.spatial_resolution[0],
			      par_info.spatial_resolution[1],
			      &biasDay);
  allocate_float_array_matlab(par_info.spatial_resolution[0],
			      par_info.spatial_resolution[1],
			      &biasNight);
  for(i=0;i<biasDay.size;i++){
    *(biasDay.array+i) = 0.;
  }
  for(i=0;i<biasNight.size;i++){
    *(biasNight.array+i) = 0.;
  }

  /* Run AVHRR reader */
  ok = process_raw_avhrr_acspo_c( pNOAA_Name, year, day, &Ref_SST,
				  &SST_Variability, 1, &biasDay,
				  &biasNight, &outData[0], &DumpArray[0],
				  &Total_badclim[0], &Total_badstd[0],
				  par_files );
  /* Free memory in the same way as operational code */
  if( 0 == ok ){
    if( (0 == par_info.start_day_night) || (0 == par_info.end_day_night) ){
      free_out_sst_matlab( &outData[0] );
      free_int_matlab( &DumpArray[0] );
    }
    if( (1 == par_info.start_day_night) || (1 == par_info.end_day_night) ){
      free_out_sst_matlab( &outData[1] );
      free_int_matlab( &DumpArray[1] );
    }
  }
  free_float_matlab( &Ref_SST.sst );
  free_float_matlab( &SST_Variability.sst );
  free_float_matlab( &biasDay );
  free_float_matlab( &biasNight );

  free_file_info();
}
