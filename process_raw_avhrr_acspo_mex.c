/*
 * NAME: 
 *
 *    process_raw_avhrr_acspo_mex
 *
 * FUNCTION:
 *
 *    C interface to MATLAB for process_raw_avhrr_acspo_c.c - reads in AVHRR data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the MATLAB interface (MEX) code to transfer data from MATLAB to 
 *    the underlying C code
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_avhrr_acspo_mex
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 05/12/2011
 *
 */
/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* MATLAB mex headers */
#include "matrix.h"
#include "mex.h"

/* header for this routine */
#include "types_cnsts.h"
#include "logfile.h"
#include "init_file_info.h"
#include "init_par_info.h"
#include "matlab_functions.h"
#include "get_dates.h"
#include "mex_functions.h"
#include "process_raw_avhrr_acspo_c.h"

/*
 * NAME: 
 *
 *    mexFunction
 *
 * FUNCTION:
 *
 *    C Entry point to C code from MATLAB
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Get input arrays from MATLAB and run process_raw_avhrr_acspo_mex which
 *    reads in day+night or day or night data from the AVHRR ACPSO datasets
 *    and regrids the data onto the reference grid including some quality
 *    control/filtering of the data
 *
 * PSEUDO CODE
 *
 *    mexFunction number_of_lhs_args lhs_args number_of_rhs_args rhs_args
 *
 *       check MATLAB number of input arguments
 *       check MATLAB number of output arguments
 *
 *       check MATLAB input arguments
 *
 *       initialize arrays
 *
 *       setup par_info/file_info
 *
 *       get C variables from MATLAB (MEX) arrays
 *
 *       open logfile
 *
 *       run process_raw_avhrr_acspo_c
 *
 *       IF run status OK THEN
 *       
 *          IF daytime data requested THEN
 *             set output daytime flat to TRUE
 *             copy daytime data to MATLAB variables
 *          ELSE
 *             set output daytime variables to logical FALSE
 *          ENDIF
 *
 *          IF nighttime data requested THEN
 *             set output nighttime flat to TRUE
 *             copy nighttime data to MATLAB variables
 *          ELSE
 *             set output nighttime variables to logical FALSE
 *          ENDIF
 *
 *       ELSE
 *          set output daytime variables to logical FALSE
 *          set output nighttime variables to logical FALSE
 *       ENDIF
 *
 *       clear memory
 *
 *       close logfile
 *
 *    END
 *
 * FILES NEEDED: 
 *
 *     ACSPO data files
 *    
 * SUBROUTINE/FUNCTION CALLS:
 *
 *     mxIsDouble
 *     mxIsNumeric
 *     mxIsClass
 *     mxGetNumberOfDimensions
 *     mexErrMsgIdAndTxt
 *     mxGetDimensions
 *     mxGetPr
 *     mxGetString
 *     mxGetScalar
 *
 *     message
 *     init_sst_matlab
 *     init_sst_matlab
 *     init_float_array_matlab
 *     init_float_array_matlab
 *     init_out_sst_matlab
 *     init_out_sst_matlab
 *     init_int_array_matlab
 *     init_int_array_matlab
 *     init_par_info
 *     init_file_info
 *     process_raw_avhrr_acspo_c
 *     copy_to_MATLAB_flt
 *     copy_to_MATLAB_int
 *     free_float_matlab
 *     free_out_sst_matlab
 *     free_int_matlab
 *
 * CALLING SEQUENCE:  
 *
 *     [run_ok day_there sst_day stdvals_day ...
 *      gridcount_day dumparray_day total_bad_day_clim ...
 *      total_bad_day_stdev night_there sst_night ...
 *      stdvals_night gridcount_night dumparray_night ...
 *      total_bad_night_clim total_bad_night_stdev] = ...
 *          process_raw_avhrr_acspo_mex(noaa_name,year,day,...
 *                                      ref_sst,...
 *                                      sst_variability,...
 *                                      sst_check,bias_day,...
 *                                      bias_night,logfile_stream,...
 *                                      matlab_home)
 *
 * INPUTS
 *
 *    noaa_name         - make of AVHRR type to read
 *                        note - for noaa must be lower case e.g. 'noaa19'
 *                        for metop should be uppercase e.g. 'METOPA'
 *    year              - year 
 *    day               - day number in year
 *    ref_sst           - reference SST array
 *    sst_variability   - SST varaibility array
 *    sst_check         - scalar, 1 == perform checks on SST
 *    biasDay           - daytime SST bias
 *    biasNight         - nighttime SST bias
 *    logfile_stream    - string to append to logfile name
 *    matlab_home       - string for where matlab code is
 *
 * OUTPUTS
 *
 * RETURNS
 *
 *    run_ok                - logical run status
 *    day_there             - logical if daytime data is output
 *    sst_day               - daytime SST 
 *    stdvals_day           - daytime standard deviations
 *    gridcount_day         - daytime grid count
 *    dumparray_day         - daytime dumparray
 *    total_bad_day_clim    - daytime number of rejected pixels (climatology)
 *    total_bad_day_stdev   - daytime number of rejected pixels (clean)      
 *    night_there           - logical if nighttime data is output      
 *    sst_night             - nighttime SST 				     
 *    stdvals_night         - nighttime standard deviations		     
 *    gridcount_night       - nighttime grid count			     
 *    dumparray_night       - nighttime dumparray		  
 *    total_bad_night_clim  - nighttime number of rejected pixels (climatology)
 *    total_bad_night_stdev - nighttime number of rejected pixels (clean)
 *
 * SYSTEM CALLS:
 *
 *     strcpy
 *     memset 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/05/2012
 *
 */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  /* Local variables */
  VINT year = 0;
  VINT day = 0;
  VINT ok = 0;
  VINT bias_day = 0;
  VINT bias_night = 0;
  VINT nx = 0;
  VINT ny = 0;

  VINT sst_check = 0;

  char noaa_name[MAX_STRING_LENGTH];
  char logfile_stream[MAX_STRING_LENGTH];
  char matlab_home[MAX_STRING_LENGTH];

  struct sst_struct Ref_SST;
  struct sst_struct SST_Variability;

  /* Data loaded from MATLAB file */
  struct float_struct BiasDay;
  struct float_struct BiasNight;

  /* Output data */
  struct out_sst_str OutData[2];
  struct int_struct DumpArray[2];
  VINT Total_badclim[2] = {0,0};
  VINT Total_badstd[2] = {0,0};
  unsigned char start_day_night = 0;
  unsigned char end_day_night = 0;
  struct par_files_str par_files;

  /* Check the MATLAB inputs */
  if( 10 != nrhs ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Incorrect number of input arguments");
  }

  /* Check the MATLAB inputs */
  if( 15 != nlhs ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Incorrect number of output arguments");
  }

  /* check that noaa_name is a string */
  if( 0 == mxIsClass(prhs[0],"char") ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "First argument (noaa_no) must be scalar integer");
  }

  /* check that year is integer number */
  if( (0 == mxIsDouble(prhs[1])) || (0 == mxIsNumeric(prhs[1])) ||
      (1 != mxGetNumberOfElements(prhs[1])) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Second argument (year) must be scalar integer");
  }
  /* check that day is integer number */
  if( (0 == mxIsDouble(prhs[2])) || (0 == mxIsNumeric(prhs[2])) ||
      (1 != mxGetNumberOfElements(prhs[2])) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Third argument (day) must be scalar integer");
  }
  /* check that ref_sst is double 2-d array */
  if( (0 == mxIsDouble(prhs[3])) || (0 == mxIsNumeric(prhs[3])) || 
      (2 != mxGetNumberOfDimensions(prhs[3])) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Fourth argument (ref_sst) must be double array");
  }
  /* check that sst_variability is double 2-d array */
  if( (0 == mxIsDouble(prhs[4])) || (0 == mxIsNumeric(prhs[4])) || 
      (2 != mxGetNumberOfDimensions(prhs[4])) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Fifth argument (sst_variability) must be double array");
  }
  /* check that sst_check is integer number */
  if( (0 == mxIsDouble(prhs[5])) || (0 == mxIsNumeric(prhs[5])) ||
      (1 != mxGetNumberOfElements(prhs[5])) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Sixth argument (sst_check) must be scalar integer");
  }

  /* check that bias_day is double 2-d array */
  /* If not make array and set to zero */
  if( (0 == mxIsDouble(prhs[6])) || (0 == mxIsNumeric(prhs[6])) || 
      (2 != mxGetNumberOfDimensions(prhs[6])) ){
    bias_day = 1;
  } else {
    bias_day = 0;
  }

  /* check that bias_night is double 2-d array */
  /* If not make array and set to zero */
  if( (0 == mxIsDouble(prhs[7])) || (0 == mxIsNumeric(prhs[7])) || 
      (2 != mxGetNumberOfDimensions(prhs[7])) ){
    bias_night = 1;
  } else {
    bias_night = 0;
  }

  /* check that type is a string (logfile_stream) */
  if( 0 == mxIsClass(prhs[8],"char") ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Ninth argument (logfile_stream) must be char");
  }

  /* check that type is a string (matlab_home) */
  if( 0 == mxIsClass(prhs[9],"char") ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "Tenth argument (matlab_home) must be char");
  }

  /* Initialize structures */
  init_sst_matlab( &Ref_SST );
  init_sst_matlab( &SST_Variability );
  init_float_array_matlab( &BiasDay );
  init_float_array_matlab( &BiasNight );
  init_out_sst_matlab( &OutData[0] );
  init_out_sst_matlab( &OutData[1] );
  init_int_array_matlab( &DumpArray[0] );
  init_int_array_matlab( &DumpArray[1] );

  /* From input MATLAB arguments get matlab_home */
  if( 1 == mxGetString( prhs[9], matlab_home, MAX_STRING_LENGTH ) ){
    message(1, "ERROR: Getting MATLAB home");
    exit(-1);
  }

  /* Setup par_info files */
  strcpy(par_files.par_info, matlab_home);
  strcat(par_files.par_info, "/Software/init_par_info.m");
  strcpy(par_files.file_info, matlab_home);
  strcat(par_files.file_info, "/Software/init_file_info.m");

  /* Get par_info/file_info data */
  init_par_info( par_files.par_info );
  init_file_info( par_files.file_info );

  /* From input MATLAB arguments get noaa_name, year, day, ref_sst,
   * sst_variability and sst_check */
  if( 1 == mxGetString( prhs[0], noaa_name, MAX_STRING_LENGTH ) ){
    message(1,"ERROR: Getting noaa name");
    exit(-1);
  }
  year = mxGetScalar( prhs[1] );
  day = mxGetScalar( prhs[2] );

  /* Get logfile_stream from 9th input argument */
  if( 1 == mxGetString(prhs[8],logfile_stream,MAX_STRING_LENGTH) ){
    message(1,"ERROR: Getting geo type string");
    exit(-1);
  }

  /* Open log file */
  /* This is the first place where we have access to year,day,logfile_stream */
  open_logfile(year,day,logfile_stream);

  /* Create ref_sst structure */
  nx = *(mxGetDimensions( prhs[3] ));
  ny = *(mxGetDimensions( prhs[3] )+1);
  /* Copy data over to Ref_SST.sst structure */
  copy_from_MATLAB_flt(nx,ny,prhs[3],&Ref_SST.sst);
  
  /* Get locations on min/max lat/lon and deltalat/lon (pixel step) */
  Ref_SST.minlat = par_info.ref_latmin;
  Ref_SST.minlon = par_info.ref_lonmin;
  Ref_SST.deltalat = 1./par_info.cells_per_degree;
  Ref_SST.deltalon = 1./par_info.cells_per_degree;
  Ref_SST.maxlat = Ref_SST.minlat + par_info.spatial_resolution[0] * 
    Ref_SST.deltalat;
  Ref_SST.maxlon = Ref_SST.minlon + par_info.spatial_resolution[1] * 
    Ref_SST.deltalon;

  /* Create sst_varaibility structure */
  nx = *(mxGetDimensions( prhs[4] ));
  ny = *(mxGetDimensions( prhs[4] )+1);
  /* Copy data over to SST_Variability.sst structure */
  copy_from_MATLAB_flt(nx,ny,prhs[4],&SST_Variability.sst);

  /* Get locations on min/max lat/lon and deltalat/lon (pixel step) */
  SST_Variability.minlat = par_info.ref_latmin;
  SST_Variability.minlon = par_info.ref_lonmin;
  SST_Variability.deltalat = 1./par_info.cells_per_degree;
  SST_Variability.deltalon = 1./par_info.cells_per_degree;
  SST_Variability.maxlat = Ref_SST.minlat + par_info.spatial_resolution[0] * 
    Ref_SST.deltalat;
  SST_Variability.maxlon = Ref_SST.minlon + par_info.spatial_resolution[1] * 
    Ref_SST.deltalon;

  /* Check that SST_Variability and Ref_SST are the same size */
  if( (SST_Variability.sst.nx != Ref_SST.sst.nx) || 
      (SST_Variability.sst.nx != Ref_SST.sst.nx) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "sst_variability and ref_sst have incompatible sizes");
  }

  /* Get sst_check value (1 = do check) */
  sst_check = mxGetScalar( prhs[5] );

  /* Get daytime bias data (from MATLAB file) */
  /* If not a 2-d array then set to zero.  ** N.B. biay_day is set to 1 further up if there is an error in the input array dims ** */
  if( 0 == bias_day ){		 	        /*  Previously had 1 == bias_day which would have been the error condition...  */
    nx = *(mxGetDimensions( prhs[6] ));
    ny = *(mxGetDimensions( prhs[6] )+1);
    copy_from_MATLAB_flt(nx,ny,prhs[6],&BiasDay);
  } else {
    allocate_float_array_matlab(Ref_SST.sst.nx,Ref_SST.sst.ny,&BiasDay);
    /* Make sure that it is zeroed */
    memset(BiasDay.array,0,sizeof(VFLOAT)*Ref_SST.sst.size);
  }

  /* Check bias (daytime) sizes */
  if( (BiasDay.nx != Ref_SST.sst.nx) || 
      (BiasDay.nx != Ref_SST.sst.nx) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "BiasDay and ref_sst have incompatible sizes");
  }

  /* Get Nighttime bias data (from MATLAB file) */
  /* If not a 2-d array then set to zero.  ** N.B. biay_day is set to 1 further up if there is an error in the input array dims ** */
  if( 0 == bias_night ){			/*  Previously had 1 == bias_night which would have been the error condition...  */
    nx = *(mxGetDimensions( prhs[7] ));
    ny = *(mxGetDimensions( prhs[7] )+1);
    copy_from_MATLAB_flt(nx,ny,prhs[7],&BiasNight);
  } else {
    allocate_float_array_matlab(Ref_SST.sst.nx,Ref_SST.sst.ny,&BiasNight);
    /* Make sure that it is zeroed */
    memset(BiasNight.array,0,sizeof(VFLOAT)*Ref_SST.sst.size);
  }

  /* Check bias (nighttime) sizes */
  if( (BiasNight.nx != Ref_SST.sst.nx) || 
      (BiasNight.nx != Ref_SST.sst.nx) ){
    mexErrMsgIdAndTxt("MEX:process_raw_avhrr_acspo_mex",
		      "BiasDay and ref_sst have incompatible sizes");
  }

  /* Call C code */
  ok = process_raw_avhrr_acspo_c( noaa_name, year, day, &Ref_SST, 
				  &SST_Variability, sst_check, &BiasDay,
				  &BiasNight, &OutData[0], &DumpArray[0], 
				  &Total_badclim[0], &Total_badstd[0],
				  par_files );
  /* If call OK */
  if( 0 == ok ){

    /* Set run_ok to true */
    plhs[0] = mxCreateLogicalScalar(1);

    /* If daytime selected */
    if( (0 == par_info.start_day_night) || (0 == par_info.end_day_night) ){
      /* Set daytime_there to true */
      plhs[1] = mxCreateLogicalScalar(1);
      /* copy to sst_day */
      copy_to_MATLAB_flt( OutData[0].SST.nx, OutData[0].SST.ny, 
			  OutData[0].SST.array, &plhs[2] );    
      /* copy to stdev_day */
      copy_to_MATLAB_flt( OutData[0].Stdev.nx, OutData[0].Stdev.ny, 
			  OutData[0].Stdev.array, &plhs[3] );    
      /* copy to gridcnt_day */
      copy_to_MATLAB_int( OutData[0].Gridcnt.nx, OutData[0].Gridcnt.ny, 
			  OutData[0].Gridcnt.array, &plhs[4] );    
      /* copy to dumparray_day */
      copy_to_MATLAB_int( DumpArray[0].nx, DumpArray[0].ny, 
			  DumpArray[0].array, &plhs[5] );    
      /* set total_bad_day_clim */
      plhs[6] = mxCreateDoubleScalar(Total_badclim[0]);
      /* set total_bad_day_stdev */
      plhs[7] = mxCreateDoubleScalar(Total_badstd[0]);
    } else {
      /* Set all daytime outputs to logical false */
      plhs[1] = mxCreateLogicalScalar(0);
      plhs[2] = mxCreateLogicalScalar(0);
      plhs[3] = mxCreateLogicalScalar(0);
      plhs[4] = mxCreateLogicalScalar(0);
      plhs[5] = mxCreateLogicalScalar(0);
      plhs[6] = mxCreateLogicalScalar(0);
      plhs[7] = mxCreateLogicalScalar(0);
    }

    /* If nighttime selected */
    if( (1 == par_info.start_day_night) || (1 == par_info.end_day_night) ){
      /* Set nighttime_there to true */
      plhs[8] = mxCreateLogicalScalar(1);
      /* copy to sst_night */
      copy_to_MATLAB_flt( OutData[1].SST.nx, OutData[1].SST.ny, 
			  OutData[1].SST.array, &plhs[9] );    
      /* copy to stdev_night */
      copy_to_MATLAB_flt( OutData[1].Stdev.nx, OutData[1].Stdev.ny, 
			  OutData[1].Stdev.array, &plhs[10] );    
      /* copy to gridcnt_night */
      copy_to_MATLAB_int( OutData[1].Gridcnt.nx, OutData[1].Gridcnt.ny, 
			  OutData[1].Gridcnt.array, &plhs[11] );    
      /* copy to dumparray_night */
      copy_to_MATLAB_int( DumpArray[1].nx, DumpArray[1].ny, 
			  DumpArray[1].array, &plhs[12] );    
      /* set total_bad_night_clim */
      plhs[13] = mxCreateDoubleScalar(Total_badclim[1]);
      /* set total_bad_night_stdev */
      plhs[14] = mxCreateDoubleScalar(Total_badstd[1]);
    } else {
      /* Set all to nighttime data to logical bad  */
      plhs[8] = mxCreateLogicalScalar(0);
      plhs[9] = mxCreateLogicalScalar(0);
      plhs[10] = mxCreateLogicalScalar(0);
      plhs[11] = mxCreateLogicalScalar(0);
      plhs[12] = mxCreateLogicalScalar(0);
      plhs[13] = mxCreateLogicalScalar(0);
      plhs[14] = mxCreateLogicalScalar(0);
    }

    /* Free day/night arrays if used */
    if( (0 == par_info.start_day_night) || (0 == par_info.end_day_night) ){
      free_out_sst_matlab( &OutData[0] );
      free_int_matlab( &DumpArray[0] );
    }
    if( (1 == par_info.start_day_night) || (1 == par_info.end_day_night) ){
      free_out_sst_matlab( &OutData[1] );
      free_int_matlab( &DumpArray[1] );
    }
  } else {    
    /* Set all to daytime and nighttime data to logical bad  */
    plhs[0] = mxCreateLogicalScalar(0);
    plhs[1] = mxCreateLogicalScalar(0);
    plhs[2] = mxCreateLogicalScalar(0);
    plhs[3] = mxCreateLogicalScalar(0);
    plhs[4] = mxCreateLogicalScalar(0);
    plhs[5] = mxCreateLogicalScalar(0);
    plhs[6] = mxCreateLogicalScalar(0);
    plhs[7] = mxCreateLogicalScalar(0);
    plhs[8] = mxCreateLogicalScalar(0);
    plhs[9] = mxCreateLogicalScalar(0);
    plhs[10] = mxCreateLogicalScalar(0);
    plhs[11] = mxCreateLogicalScalar(0);
    plhs[12] = mxCreateLogicalScalar(0);
    plhs[13] = mxCreateLogicalScalar(0);
    plhs[14] = mxCreateLogicalScalar(0);
  }

  /* Free used memory */
  free_float_matlab( &Ref_SST.sst );
  free_float_matlab( &SST_Variability.sst );
  free_float_matlab( &BiasDay );
  free_float_matlab( &BiasNight );

  /* Close logfile */
  close_logfile();

  /* free file info memory (list of files) */
  free_file_info();
}

