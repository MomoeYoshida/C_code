/*
 * NAME: 
 *
 *    read_ghrsst
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
 *    Contains the routine to read in a GHRSST L2P file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_ghrsst
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
#include "read_ghrsst.h"

/* ACSPO old (HDF) format day mask bit position and cld mask bit position */
/* Cloud mask bits */
/* bit 1 - invalid (1) / valid (0) mask
 * bit 2 - day (1) / night (0)
 * bit 3 - land (1) / sea (0)
 * bit 4 - coast (1) / not coast (0)
 * bit 5 - glint (1) / no glint (0)
 * bit 6 - snow (1) / no snow (0)
 * bit 7 - cloud mask
 * bit 8 - ditto
 */  
static int acspo_day_mask = 1;
static int acspo_cld_mask = 6;
/* #define START_JULDAY 2444606.5 Value below is correct */
#define START_JULDAY 2444605.5

/* Routines for internal use */
static VINT readNetCDFData_l2pflags( int nc_id, const char *pName, 
				     size_t nx, size_t ny, signed char *pVar );
static VINT readNetCDFData_2Byte_float( int nc_id, const char *pName, 
					size_t nx, size_t ny, float *pVar );
static VINT readNetCDFData_byte( int nc_id, const char *pName, 
				 size_t nx, size_t ny, signed char *pVar );
static VINT readNetCDFData_short( int nc_id, const char *pName, 
				 size_t nx, size_t ny, signed short *pVar );
static VINT readNetCDFData_Byte_float( int nc_id, const char *pName, 
				       size_t nx, size_t ny, float *pVar );
static VINT get_Julian_Day( VINT mm, VINT id, VINT iyyy );
static VINT get_GHRSST_Time_from_Date( const VINT Year, const VINT Month, 
				       const VINT Day, const VINT Hour,
				       const VINT Mins, const VINT Secs );

/*
 * NAME: 
 *
 *    read_ghrrst
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
 *    ok = read_ghrsst( filename, time, sst, variance, lat, lon, type, 
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
VINT read_ghrsst( const char *pFilename, struct int_struct *pTime, 
		  struct float_struct *pSST, struct float_struct *pLat, 
		  struct float_struct *pLon, 
		  struct float_struct *pSSES_Variance,
		  char *pType, struct char_struct  *pCldmask, 
		  unsigned char day_night, char correct_bias, 
		  VFLOAT output_stdev, VINT min_quality )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  float *pVar = NULL;
  signed char *pByte = NULL;
  
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
    /* EXIT Byte variable allocateion fail */
    exit(-1);
  }

  /* Get latitude and longitude */
  /* Latitude */
  if( 1 == readNetCDFData_flt(nc_id,"lat",nx,ny,pVar) ){
    message(2,"ERROR: Error reading GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pLat);

  /* Longitude */
  if( 1 == readNetCDFData_flt(nc_id,"lon",nx,ny,pVar) ){
    message(2,"ERROR: Error reading GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
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

  /* Get SST */
  if( 1 == readNetCDFData_2Byte_float(nc_id,"sea_surface_temperature",
				      nx,ny,pVar) ){
    message(2,"ERROR: Error reading sea_surface_temperature from GHRSST file ",pFilename);
    status = nc_close(nc_id);
    /* Free what has already been allocated */
    free(pVar);
    free(pByte);
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
  /* If ACSPO data use L2P_FLAGS to get cloud mask and daynight flag */
  /* If GEO data use solar zenith angle and clear sky probability */
  if( 0 == strncmp(pType,"ACSPO",5) ){
    /* Note converts L2P_FLAGS to ACSPO heritage acspo_mask format as part of read */
    if( 1 == readNetCDFData_l2pflags(nc_id,"l2p_flags",nx,ny,pByte) ){
      message(2,"ERROR: Error reading l2p_flags from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Copy to output structure */
    copy_byte_to_str_matlab(nx,ny,(char *) pByte,pCldmask);

    /* Change in processing of ACSPO SST - now get & use GHRSST quality flag */
    if( 1 == readNetCDFData_byte(nc_id,"quality_level",nx,ny,pByte) ){
      message(2,
	      "ERROR: Error reading quality_level from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    /* Set bad values to bad based on min_quality */
    for(i=0;i<nx*ny;i++){
      if( min_quality > *(pByte+i) ){
	*(pSST->array+i) = NaN;
      }
    }

  } else if( 0 == strncmp(pType,"GEO",3) ){
    /* Get quality_flag for cloud masking */
    if( 1 == readNetCDFData_Byte_float(nc_id,"probability_of_clear_sky",
				       nx,ny,pVar) ){
      message(2,
	      "ERROR: Error reading probability_of_clear_sky from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Set data bad if probability < threshold */
    for(i=0;i<nx*ny;i++){
      if( GEO_CLOUD_THRESHOLD > *(pVar+i) ){
	*(pSST->array+i) = NaN;
      }
    }
    
    /* Get solar zenith angle to choose day/night */
    if( 1 == readNetCDFData_Byte_float(nc_id,"solar_zenith_angle",nx,ny,pVar) ){
      message(2,
	      "ERROR: Error reading solar_zenith_angle from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Get GHRSST quality flag */
    if( 1 == readNetCDFData_byte(nc_id,"quality_level",nx,ny,pByte) ){
      message(2,
	      "ERROR: Error reading quality_level from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    /* Set bad values to bad based on min_quality */
    for(i=0;i<nx*ny;i++){
      if( min_quality > *(pByte+i) ){
	*(pSST->array+i) = NaN;
      }
    }

    /* Now simply reuse to flag day/night */
    for(i=0;i<nx*ny;i++){
      if( (0. <= *(pVar+i)) && (90. > *(pVar+i)) ){	/*  Fix due to wrapping above 127 in byte var  */
      /* Daytime */
	*(pByte+i) = 0;
      } else {
      /* Nighttime */					/*  Angles >127 were wrapping to -128,-127, etc. */
	*(pByte+i) = 1;					/*  Exclude data with -128 as that is badVal...? */
      }
    }

    /* Copy to output structure - may as well return for GEO as well as ACSPO - used in new Geo GHRSST reader */
    copy_byte_to_str_matlab(nx,ny,(char *) pByte,pCldmask);

  } else if( 0 == strncmp(pType,"AMSR",4) ){
    
    /* Get solar zenith angle to choose day/night */
    if( 1 == readNetCDFData_Byte_float(nc_id,"solar_zenith_angle",nx,ny,pVar) ){
      message(2,
	      "ERROR: Error reading solar_zenith_angle from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }
    /* Get GHRSST quality flag */
    if( 1 == readNetCDFData_byte(nc_id,"quality_level",nx,ny,pByte) ){
      message(2,
	      "ERROR: Error reading quality_level from GHRSST file ",
	      pFilename);
      status = nc_close(nc_id);
      /* Free what has already been allocated */
      free(pVar);
      free(pByte);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      /* EXIT error in read */
      return(1);
    }

    /* Set bad values to bad based on min_quality */
    for(i=0;i<nx*ny;i++){
      if( min_quality > *(pByte+i) ){
	*(pSST->array+i) = NaN;
      }
    }

    /* Now simply reuse to flag day/night */
    for(i=0;i<nx*ny;i++){
      if( 90. > *(pVar+i) ){
      /* Daytime */
	*(pByte+i) = 0;
      } else {
      /* Nighttime */
	*(pByte+i) = 1;
      }
    }

    /* Copy to output structure - may as well return for AMSR as well as ACSPO - used in new GHRSST reader */
    copy_byte_to_str_matlab(nx, ny, (char *) pByte, pCldmask);

  } else {
    message(2,"ERROR: Cannot recognize read_ghrsst type : ", pType);
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

  /* Free pVar */
  free(pVar);

  /* Free pByte */
  free(pByte);

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

/*
 * NAME: 
 *
 *    read_NetCDFData_flt
 *
 * FUNCTION:
 *
 *    C Code to read in a floating point variable from a NetCDF file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in float array from NetCDF4 GHRSST file. 
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Read in data
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
 *    ok = readNetCDFData_float( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (float) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
VINT readNetCDFData_flt( int nc_id, const char *pName, size_t nx, 
			 size_t ny, float *pVar )
{ 
  /* Local variables */
  int status = 0;
  int varid = 0;
  size_t start[2] = {0,0};
  size_t count[2] = {0,0};

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  count[0] = ny;
  count[1] = nx;

  status = nc_get_vara_float( nc_id, varid, start, count, pVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get float variable for ",pName);
    return(1);
  }

  /* Return good status */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_l2pflags
 *
 * FUNCTION:
 *
 *    C Code to read in a byte point variable from a NetCDF file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in byte array from NetCDF4 GHRSST ACSPO file.  
 *    Note converts L2P LP_FLAGS parameter to ACSPO heritage acspo_mask 
 *    format for consistenct with HDF format.
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Read in data
 *       convert L2P_FLAGS to acspo_mask format
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
 *    ok = readNetCDFData_l2pflags( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (byte) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT readNetCDFData_l2pflags( int nc_id, const char *pName, size_t nx, 
				     size_t ny, signed char *pVar )
{ 
  /* Local variables */
  int i = 0;
  int status = 0;
  int varid = 0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};
  short *pFlags = NULL;
  char flag_meanings[MAX_STRING_LENGTH];
  char *pStrtok_save = NULL;
  int nflags = 0;
  short *pFlag_masks = NULL;
  int flag_day_pos = 0;
  int flag_cld_pos = 0;
  int cloud_bit_pos = 0;
  char *pStr = NULL;
  short value = 0;
  short cld_mask = 0;

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Allocate short array for flags */
  if( NULL == (pFlags = (short *) calloc(nx*ny,sizeof(short))) ){
    message(2,"ERROR: Cannot allocate pFlags for ",pName);
    /* EXIT cannot get var id from file */
    exit(-1);
  }

  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = 1;
  count[1] = ny;
  count[2] = nx;

  /* Read in L2P_FLAGS (short array) */
  status = nc_get_vara_short( nc_id, varid, start, count, pFlags );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get l2p_flags variable for ",pName);
    return(1);
  }

  /* Get flag_meanings / flag_masks to work out where relevant bits are */
  status = nc_get_att_text( nc_id, varid, "flag_meanings", &flag_meanings[0] );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get flag_meanings attribute for ",pName);
    return(1);
  }

  /* Set position of day flag and cld flag to -1 */
  flag_day_pos = -1;
  flag_cld_pos = -1;

  /* Find number of tokens with strtok - note this modifies the input 
   * string */
  /* Start search */
  pStr = strtok_r(flag_meanings," ",&pStrtok_save);
  if( NULL != pStr ){
    if( 0 == strncmp(pStr,"day",3) ){
      flag_day_pos = 0;
    }
    if( 0 == strncmp(pStr,"probably_clear_or_cloudy_or_undefined",37) ){
      flag_cld_pos = 0;
    }
    nflags = 1;
  } else {
    message(2,"ERROR: Cannot parse (strtok) flag_meanings attribute for ",
	    pName);
    return(1);
  }  
  /* Loop until no more tokens */
  while( NULL != pStr ){
    pStr = strtok_r(NULL," ",&pStrtok_save);
    if( NULL != pStr ){
      if( 0 == strncmp(pStr,"day",3) ){
	flag_day_pos = nflags;
      }
      if( 0 == strncmp(pStr,"probably_clear_or_cloudy_or_undefined",37) ){
	flag_cld_pos = nflags;
      }
      nflags++;
    }
  }

  /* Check to see if we have positions of day/cld mask */
  if( -1 == flag_day_pos || -1 == flag_cld_pos ){
    message(2,"ERROR: Cannot find day/cld mask in flag_meanings for ",pName);
    return(1);
  }

  /* Allocate flag_masks array */
  if( NULL == (pFlag_masks = (short *) calloc(nflags,sizeof(short))) ){
    message(2,"ERROR: Cannot allocate flag_masks attribute for ",pName);
    return(1);
  }

  /* Read flag masks */
  status = nc_get_att_short( nc_id, varid, "flag_masks", pFlag_masks );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get flag_masks variable for ",pName);
    return(1);
  }

  /* For cloud mask work out location of bits */
  /* Two bit mask */
  for(i=0;i<31;i++){
    value = (3<<i);
    if( value == *(pFlag_masks+flag_cld_pos) ){
      cloud_bit_pos = i;
      break;
    }
  }

  /* Convert pFlags to heritage acspo_mask bit mask */
  /* Only need to set cloud mask and day/night bits */
  for(i=0;i<nx*ny;i++){
    /* Reset mask */
    *(pVar+i) = 0;
    /* If day flag set - set in old acspo mask position*/
    if( 0 != (*(pFlags+i) & *(pFlag_masks+flag_day_pos)) ){
      *(pVar+i) = (*(pVar+i) | (1<<acspo_day_mask));
    }
    /* Now cld mask - note this is two bits */
    cld_mask = ((*(pFlags+i)>>cloud_bit_pos) & 0x3);
    *(pVar+i) = (*(pVar+i) | (cld_mask<<acspo_cld_mask));
  }

  /* Free flags */
  free(pFlags);
  free(pFlag_masks);

  /* Return good status */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_byte
 *
 * FUNCTION:
 *
 *    C Code to read in a byte variable from a NetCDF file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in a byte array from NetCDF4 GHRSST file. 
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Read in data
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
 *    ok = readNetCDFData_byte( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (char) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT readNetCDFData_byte( int nc_id, const char *pName, size_t nx, 
				 size_t ny, signed char *pVar )
{ 
  /* Local variables */
  int status = 0;
  int varid = 0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = 1;
  count[1] = ny;
  count[2] = nx;

  status = nc_get_vara_schar( nc_id, varid, start, count, pVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get byte variable for ",pName);
    return(1);
  }

  /* Return good status */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_short
 *
 * FUNCTION:
 *
 *    C Code to read in a short variable from a NetCDF file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in a byte array from NetCDF4 GHRSST file. 
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Read in data
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
 *    ok = readNetCDFData_short( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (char) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *   Adapted from readNetCDFData_byte() to get short by Andy Harris, 11/29/2023
 *
 */
static VINT readNetCDFData_short( int nc_id, const char *pName, size_t nx, 
				 size_t ny, signed short *pVar )
{ 
  /* Local variables */
  int status = 0;
  int varid = 0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = 1;
  count[1] = ny;
  count[2] = nx;

  status = nc_get_vara_short( nc_id, varid, start, count, pVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get short variable for ",pName);
    return(1);
  }

  /* Return good status */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_2Byte_float
 *
 * FUNCTION:
 *
 *    C Code to read in a 2 byte variable from a NetCDF file and convert to floating point
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in 2 byte array from NetCDF4 GHRSST file and use scale and offset to get
 *    floating point variable
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Get attributes (scale, offset, fill value)
 *       Read in data
 *       scale data to float
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
 *    ok = readNetCDFData_2Byte_float( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (float) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT readNetCDFData_2Byte_float( int nc_id, const char *pName, 
					size_t nx, size_t ny, float *pVar )
{ 
  /* Local variables */
  int i = 0;
  int status = 0;
  int varid = 0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};

  short *pByteVar = NULL;
  short *pByteVar_ptr = NULL;
  short fill_value = 0;
  short byteval = 0;

  float scale = 0;
  float offset = 0;

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Get offset/scale and fill value */
  status = nc_get_att_float( nc_id, varid, "scale_factor", &scale );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get scale_factor for ",pName);
    return(1);
  }
  status = nc_get_att_float( nc_id, varid, "add_offset", &offset );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get add_offset for ",pName);
    return(1);
  }
  status = nc_get_att_short( nc_id, varid, "_FillValue", &fill_value );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get _FillValue for ",pName);
    return(1);
  }

  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = 1;
  count[1] = ny;
  count[2] = nx;

  /* Allocate 2 byte array */
  if( NULL == (pByteVar = (short *) calloc(nx*ny,sizeof(short))) ){
    message(1,"ERROR: Cannot allocate pByteVar for ",pName);
    /* EXIT cannot get variable id from file */
    exit(-1);
  }

  /* Read in 2 byte data */
  status = nc_get_vara_short( nc_id, varid, start, count, pByteVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get float from 2 byte variable for ",pName);
    return(1);
  }

  /* Scale to floating point */
  pByteVar_ptr = pByteVar;
  for(i=0;i<nx*ny;i++){
    byteval = *(pByteVar_ptr++);
    if( fill_value == byteval ){
      *(pVar+i) = NaN;
    } else {
      /* Convert to centigrade */
      *(pVar+i) = byteval*scale + offset;
    }
  }

  /* Free byte array */
  free(pByteVar);

  /* Return good status */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_Byte_float
 *
 * FUNCTION:
 *
 *    C Code to read in a byte variable from a NetCDF file and convert to floating point
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in byte array from NetCDF4 GHRSST file and use scale and offset to get
 *    floating point variable
 *
 * PSEUDO CODE
 *
 *    FUNCTION ncid, name, nx, ny, outvar
 *
 *       Get variable ID
 *       Get attributes (scale, offset, fill value)
 *       Read in data
 *       scale data to float
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
 *    ok = readNetCDFData_Byte_float( nc_id, name, nx, ny, outvar )
 *
 * INPUTS:  
 *    
 *   nc_id   - NetCDF file ID
 *   name    - name of array to be read
 *   nx      - size of X dimension to be read
 *   ny      - size of Y dimension to be read
 *
 * OUTPUTS:  
 * 
 *   outvar  - output (float) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT readNetCDFData_Byte_float( int nc_id, const char *pName, 
				       size_t nx, size_t ny, float *pVar )
{ 
  /* Local variables */
  int i = 0;
  int status = 0;
  int varid = 0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};

  signed char *pByteVar = NULL;
  signed char *pByteVar_ptr = NULL;
  signed char fill_value = 0;
  signed char byteval = 0;

  float scale = 0;
  float offset = 0;

  /* Get variable ID */
  status = nc_inq_varid( nc_id, pName, &varid );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varID for ",pName);
    return(1);
  }
  
  /* Get offset/scale and fill value */
  status = nc_get_att_float( nc_id, varid, "scale_factor", &scale );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get scale_factor for ",pName);
    return(1);
  }
  status = nc_get_att_float( nc_id, varid, "add_offset", &offset );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get add_offset for ",pName);
    return(1);
  }
  status = nc_get_att_schar( nc_id, varid, "_FillValue", &fill_value );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get _FillValue for ",pName);
    return(1);
  }

  /* Setup read */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = 1;
  count[1] = ny;
  count[2] = nx;

  /* Allocate 1 byte array */
  if( NULL == (pByteVar = (signed char *) calloc(nx*ny,sizeof(signed char))) ){
    message(1,"ERROR: Cannot allocate pByteVar for ",pName);
    /* EXIT cannot get variable id from file */
    exit(-1);
  }

  /* Read in 1 byte data */
  status = nc_get_vara_schar( nc_id, varid, start, count, pByteVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get float variable for ",pName);
    return(1);
  }

  /* Scale to floating point */
  pByteVar_ptr = pByteVar;
  for(i=0;i<nx*ny;i++){
    byteval = *(pByteVar_ptr++);
    if( fill_value == byteval ){
      *(pVar+i) = NaN;
    } else {
      *(pVar+i) = byteval*scale + offset;
    }
  }

  /* Free byte array */
  free(pByteVar);

  /* Return good status */
  return(0);

}

/*
 * NAME: 
 *
 *    get_GHRSST_Time
 *
 * FUNCTION:
 *
 *    C Code to return GHRSST Time 
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns the GHRSST time (time in integer seconds from 01-01-1981)
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, year, dayno, hours
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
 *    time = get_GHRSST_Time( year, dayno, hours )
 *
 * INPUTS:  
 *    
 *   year    - Year
 *   dayno   - Day number
 *   hours   - hours
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 *   time    - GHRSST Time
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
VINT get_GHRSST_Time( const VINT Year, const VINT Dayno, const float Hours )
{
  VINT YYYY = 0;
  VINT Month = 0;
  VINT Day = 0;
  double julDay = 0;
  double totDays = 0;

  /* Make sure Year is correct range */
  if( Year < 100 ){
    if( Year < 50 )
      YYYY = 2000+Year;
    else
      YYYY = 1900+Year;
  } else {
    YYYY = Year;
  }

  /* Get month day from day number */
  date_from_daynum_matlab(YYYY,Dayno,&Month,&Day);

  /* Get julian day */
  julDay = ((double) get_Julian_Day( Month, Day, YYYY )) - 0.5;

  /* Convert to seconds from 01-01-1981 */
  totDays = (julDay - START_JULDAY)*86400. + Hours*3600.;
  return (int) totDays;
}

/*
 * NAME: 
 *
 *    get_GHRSST_Time_from_Date
 *
 * FUNCTION:
 *
 *    C Code to return GHRSST Time 
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns the GHRSST time (time in integer seconds from 01-01-1981) from
 *    input year,month,day,hour,min,secs
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, year, month, day, hour, min, secs
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
 *    time = get_GHRSST_Time_from_Date( year, month, day, hour, min, sec )
 *
 * INPUTS:  
 *    
 *   year    - Year
 *   month   - Month
 *   day     - Day
 *   hour    - Hour
 *   min     - Minutes
 *   sec     - Seconds
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 *   time    - GHRSST Time
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT get_GHRSST_Time_from_Date( const VINT Year, const VINT Month, 
				       const VINT Day, const VINT Hour,
				       const VINT Mins, const VINT Secs )
{
  VINT YYYY = 0;
  double julDay = 0;
  double totDays = 0;

  /* Make sure Year is correct range */
  if( Year < 100 ){
    if( Year < 50 )
      YYYY = 2000+Year;
    else
      YYYY = 1900+Year;
  } else {
    YYYY = Year;
  }

  /* Get julian day */
  julDay = ((double) get_Julian_Day( Month, Day, YYYY )) - 0.5;

  /* Convert to seconds from 01-01-1981 */
  totDays = (julDay - START_JULDAY)*86400. + Hour*3600. + Mins*60. + Secs;
  return (int) totDays;
}

/*
 * NAME: 
 *
 *    get_Julian_Day
 *
 * FUNCTION:
 *
 *    C Code to return the julian day
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns the julian day - code taken from NOAA GHRSST Level 2P code
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, year, month, day
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
 *    jday = get_Julian_Day( year, month, day )
 *
 * INPUTS:  
 *    
 *   year    - Year
 *   month   - Month
 *   day     - Day
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 *   time    - Julian Day
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/26/2013
 *
 */
#define IGREG (15+31*(10+12*1582))
static VINT get_Julian_Day( VINT mm, VINT id, VINT iyyy )
{
  VINT jy = 0;
  VINT jm = 0;
  VINT ja = 0;
  VINT julday = 0;

  jy = iyyy;
  if( mm > 2 )
    jm = mm+1;
  else {
    jy = jy-1;
    jm = mm+13;
  }
  julday = ((int) (365.25*jy)) + ((int) (30.6001*jm)) + id + 1720995;
  if( id+31*(mm+12*iyyy) >= IGREG ){
    ja = ((int) (0.01*jy));
    julday += 2 - ja + ((int) (0.25*ja));
  }
  return julday;
} 

