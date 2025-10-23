/*
 * NAME: 
 *
 *    read_ghrsst_slstr
 *
 * FUNCTION:
 *
 *    C reads in NetCDF GHRRST data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in a GHRSST L2P SLSTR file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_ghrsst_slstr
 *
 * ROUTINES USED INTERNALLY:
 *
 *    readNetCDFData_float       - read in NetCDF Float data
 *    readNetCDFData_2byte_float - read in NetCDF 2byte data converted to Float data
 *    readNetCDFData_l2pflags    - read in NetCDF l2p_flags data
 *    readNetCDFData_byte        - read in NetCDF byte data
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netcdf.h>

/* header for this routine */
#include "types_cnsts.h"
#include "matlab_functions.h"
#include "logfile.h"
#include "read_ghrsst_slstr.h"


/* #define START_JULDAY 2444606.5 Value below is correct */
#define START_JULDAY 2444605.5

/* Routines for internal use */
static VINT readNetCDFData_l2pflags( int nc_id, const char *pName, 
				     size_t nx, size_t ny, signed char *pByte );
static VINT readNetCDFData_2Byte_float( int nc_id, const char *pName, 
					size_t nx, size_t ny, float *pVar );
static VINT readNetCDFData_byte( int nc_id, const char *pName, 
				 size_t nx, size_t ny, signed char *pByte );
static VINT readNetCDFData_Byte_float( int nc_id, const char *pName, 
				       size_t nx, size_t ny, float *pVar );
static VINT get_Julian_Day( VINT mm, VINT id, VINT iyyy );
static VINT get_GHRSST_Time_from_Date( const VINT Year, const VINT Month, 
				       const VINT Day, const VINT Hour,
				       const VINT Mins, const VINT Secs );

/*
 * NAME: 
 *
 *    read_ghrsst_slstr
 *
 * FUNCTION:
 *
 *    C Code to read in GHRSST GDS 2.0 L2P data
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in data from either GHRSST GDS 2.0 L2P (NetCDF 4.0) format 
 *    Reads in ACSPO cloud mask (from l2p_flags) or GEO cloud mask (from 
 !    quality flag)
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, time, sst, lat, lon, type, cld_mask, day_night
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_ghrsst_slstr( filename, time, sst, variance, lat, lon, type,
 *                      cld_mask, day_night )
 *
 * INPUTS:  
 *    
 *   filename - GOES hourly filename
 *
 * OUTPUTS:  
 * 
 *   time     - number of seconds from 1 Jan 1980 (GHRRST time)
 *   sst      - float_struct array with the SSTs
 *   variance - SSES variance (square error), constant if no SSES used
 *   lat      - float_struct array with latitude
 *   lon      - float_struct array with longitude
 *   type     - Type of GHRSST file ("ACSPO" or "GEO")
 *   cld_mask - packed cloud mask for ACSPO processing
 *   day_night- day night flag for geo processing
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
VINT read_ghrsst_slstr( const char *pFilename, struct int_struct *pTime,
		  struct float_struct *pSST, struct float_struct *pLat, 
		  struct float_struct *pLon, 
		  struct float_struct *pSSES_Variance,
		  char *pType, struct char_struct  *pCldmask, 
		  unsigned char day_night, char correct_bias, 
		  VFLOAT output_stdev, VINT min_quality, VINT min_algorithm, VFLOAT min_wind_speed )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  float *pVar = NULL;
  signed char *pByte = NULL;
  signed char *pByte2 = NULL;

  int nc_id = 0;
  int status = 0;
  int dimid_nx = 0;
  int dimid_ny = 0;
  size_t nx = 0;
  size_t ny = 0;
  VFLOAT output_stdev_sq = 0.;
  char start_time_string[17];
  char stop_time_string[17];
  VINT Year = 0;
  VINT Month = 0;
  VINT Day = 0;
  VINT Hour = 0;
  VINT Mins = 0;
  VINT Secs = 0;
  VINT start_time = 0;
  VINT stop_time = 0;
  VFLOAT deltaTime = 0.;

  /* Open file */
  status = nc_open(pFilename,NC_NOWRITE,&nc_id);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot open GHRSST file ",pFilename);
    /* EXIT open fail */
    return(1);
  }

  /* Get time from global attributes */
  status = nc_get_att_text(nc_id,NC_GLOBAL,"start_time",start_time_string);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get start_time from GHRSST file ",pFilename);
    /* EXIT open fail */
    return(1);
  }
  sscanf(start_time_string,"%4d%2d%2dT%2d%2d%2dZ",
	 &Year,&Month,&Day,&Hour,&Mins,&Secs);
  start_time = get_GHRSST_Time_from_Date( Year, Month, Day, Hour, Mins, Secs );
  status = nc_get_att_text(nc_id,NC_GLOBAL,"stop_time",stop_time_string);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get start_time from GHRSST file ",pFilename);
    /* EXIT open fail */
    return(1);
  }
  sscanf(stop_time_string,"%4d%2d%2dT%2d%2d%2dZ",
	 &Year,&Month,&Day,&Hour,&Mins,&Secs);
  stop_time = get_GHRSST_Time_from_Date( Year, Month, Day, Hour, Mins, Secs );

  /* Get dimensions */

  /* First get dim ids */
  status = nc_inq_dimid( nc_id, "ni", &dimid_nx );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dimid ni from GHRSST file ",pFilename);
    /* EXIT dimid access fail */
    return(1);
  }

  status = nc_inq_dimid( nc_id, "nj", &dimid_ny );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dimid nj from GHRSST file ",pFilename);
    /* EXIT dimid access fail */
    return(1);
  }

  /* Now get dimensions */
  status = nc_inq_dimlen( nc_id, dimid_nx, &nx );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dim nx from GHRSST file ",pFilename);
    /* EXIT dimlen access fail */
    return(1);
  }

  status = nc_inq_dimlen( nc_id, dimid_ny, &ny );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dim ny from GHRSST file ",pFilename);
    /* EXIT dimlen access fail */
    return(1);
  }  

  /* Allocate arrays for reading */
  if( NULL == (pVar = (float *) calloc(nx*ny,sizeof(float))) ){
    message(1,"ERROR: Cannot allocate pVar (flt) array");
    /* EXIT float variable allocateion fail */
    exit(-1);
  }
  if( NULL == (pByte = (signed char *) calloc(nx*ny,sizeof(signed char))) ){
    message(1,"ERROR: Cannot allocate pByte (byte) array");
    /* EXIT Byte variable allocation fail */
    free(pVar);
    exit(-1);
  }

  if( NULL == (pByte2 = (signed char *) calloc(nx*ny,sizeof(signed char))) ){
    message(1,"ERROR: Cannot allocate pByte2 (byte) array");
    /* EXIT Byte2 variable allocation fail */
    free(pVar);
    free(pByte);
    exit(-1);
  }

  /* Get latitude and longitude */
  /* Latitude */
  if( 1 == readNetCDFData_flt(nc_id,"lat",nx,ny,pVar) ){
    message(2,"ERROR: Error reading lat from GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
    free(pByte2);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pLat);

  /* Longitude */
  if( 1 == readNetCDFData_flt(nc_id,"lon",nx,ny,pVar) ){
    message(2,"ERROR: Error reading lon from GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
    free(pByte2);
    free_float_matlab(pLat);
    /* EXIT error in read */
    return(1);
  }

  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pLon);

  /* Make sure longitude is in -180,180 range */
  for(i=0;i<pLon->size;i++){
    if( 180. < *(pLon->array+i) ){
      *(pLon->array+i) -= 360.;
    }
  }

  /* Get SLSTR skin SST */
  if( 1 == readNetCDFData_2Byte_float(nc_id,"sea_surface_temperature",
				      nx,ny,pVar) ){
    message(2,"ERROR: Error reading SST from GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
    free(pByte2);
    free_float_matlab(pLat);
    free_float_matlab(pLon);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pSST);

  /* If correct using SSES Bias, read in and correct */
  if( 1 == correct_bias ){
    if( 1 == readNetCDFData_Byte_float(nc_id,"sses_bias",nx,ny,pVar) ){
      message(2,
	      "ERROR: Error reading sses_bias from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free(pByte2);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Correct using bias - if not present set SST to bad */
    for(i=0;i<nx*ny;i++){
      if( NaN != *(pVar+i) ){
	*(pSST->array+i) -= *(pVar+i);
      } else {
	*(pSST->array+i) = NaN;
      }
    }
  }

  /* Get cloud mask */
  /* If SLSTR data use quality level & alg. type */
  if( 0 == strncmp(pType,"SLSTR",5) ){
    /* Get GHRSST quality flag */
    if( 1 == readNetCDFData_byte(nc_id,"quality_level",nx,ny,pByte) ){
      message(2,
          "ERROR: Error reading quality_level from GHRSST file ",
          pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free(pByte2);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    if( 1 == readNetCDFData_byte(nc_id,"sst_algorithm_type",nx,ny,pByte2) ){
      message(2,
          "ERROR: Error reading sst_algorithm_type from SLSTR L2P file ",
          pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free(pByte2);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    /* Set bad values to bad based on min_quality & min_algorithm */
    for(i=0;i<nx*ny;i++){
      if( (min_quality > *(pByte+i)) && (min_algorithm > *(pByte2+i)) ){
	*(pSST->array+i) = NaN;
      }
    }

    /* Now simply reuse to flag day/night */
    for(i=0;i<nx*ny;i++){
      /* algorithm_type = 1 2 3 4 5 -> N2 N3R N3 D2 D3 */
      if( (1 == *(pByte2+i)) || (4 == *(pByte2+i)) ){
      /* Daytime */
	*(pByte+i) = 0;
      } else {
      /* Nighttime */
	*(pByte+i) = 1;
      }
    }

    /* Copy to output structure - may as well return for GEO as well as ACSPO - used in new Geo GHRSST reader */
    copy_byte_to_str_matlab(nx,ny,(char *) pByte, pCldmask);

  } else {
    message(2,"ERROR: Cannot recognize read_ghrsst_slstr type : ",pType);
    /* EXIT error in type */
    exit(-1);
  }

  /* If using GHRSST SSES Standard deviation, get it*/
  if( 0. < output_stdev ){
    if( 1 == readNetCDFData_Byte_float(nc_id,"sses_standard_deviation",nx,ny,
				       pVar) ){
      message(2,
	      "ERROR: Error reading sses_standard_deviation from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free(pByte2);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Copy to output structure - make sure it's variance */
    for(i=0;i<nx*ny;i++){
      if( NaN != *(pVar+i) ){
	*(pVar+i) = *(pVar+i) * *(pVar+i);
      }
    }
    copy_flt_to_str_matlab(nx,ny,pVar,pSSES_Variance);
  } else {
    /* Set to a constant value defined in parameter file */
    output_stdev_sq = output_stdev*output_stdev;
    allocate_float_array_matlab(nx,ny,pSSES_Variance);
    for(i=0;i<nx*ny;i++){
      if( NaN != *(pSST->array+i) ){
	*(pSSES_Variance->array+i) = output_stdev_sq;
      } else {
	*(pSSES_Variance->array+i) = NaN;
      }
    }
  }

  /* Get wind speed & apply to daytime data */
    if( 1 == readNetCDFData_Byte_float(nc_id,"wind_speed",nx,ny,pVar) ){
      message(2,
          "ERROR: Error reading wind_speed from SLSTR L2P file ",
          pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free(pByte2);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    /* Set SST values to bad based on daytime & minimum wind speed (pByte now holds day/night flag) */
    for(i=0;i<nx*ny;i++){
      if( (0 == *(pByte+i)) && (min_wind_speed > *(pVar+i)) ){
	*(pSST->array+i) = NaN;
      }
    }


  /* Free pVar */
  free(pVar);

  /* Free pByte */
  free(pByte);
  free(pByte2);

  /* Close NetCDF file */
  status = nc_close(nc_id);
  if( NC_NOERR != status ){
    message(2,"ERROR: Error closing GHRSST file ",pFilename);
    return(1);
  }

  /* Create time array from start/stop time */
  allocate_int_array_matlab(pSST->nx,pSST->ny,pTime);
  deltaTime = (stop_time-start_time)*1./pSST->ny;
  for(i=0;i<pTime->ny;i++){
    for(j=0;j<pTime->nx;j++){
      *(pTime->array+j+i*pTime->nx) = (VINT) (start_time + deltaTime*i);
    }
  }

  /* Succesful exit */
  return(0);
}

