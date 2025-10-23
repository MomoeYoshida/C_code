/*
 * NAME: 
 *
 *    read_netcdf
 *
 * FUNCTION:
 *
 *    C reads in NetCDF ACSPO data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routines to read in ACSPO original NetCDF file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_netcdf
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
 *   Original version by Jonathan P.D. Mittaz on 11/25/2013
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

/* Routines for internal use */
static VINT readNetCDFData_ubyte( int nc_id, const char *pName, size_t nx, 
				  size_t ny, unsigned char *pVar );

/*
 * NAME: 
 *
 *    read_acspo_netcdf
 *
 * FUNCTION:
 *
 *    C Code to read in original ACSPO NetCDF format file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in data from ACPSO NetCDF format 
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, sst, lat, lon, cld_mask
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
 *    ok = read_acspo_netcdf( filename, sst, lat, lon, cld_mask )
 *
 * INPUTS:  
 *    
 *   filename - ACSPO NetCDF file
 *
 * OUTPUTS:  
 * 
 *   sst      - float_struct array with the SSTs
 *   lat      - float_struct array with latitude
 *   lon      - float_struct array with longitude
 *   cld_mask - packed cloud mask for ACSPO processing
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
VINT read_acspo_netcdf( const char *pFilename, struct int_struct *pTime,
			struct float_struct *pSST, struct float_struct *pLat, 
			struct float_struct *pLon, 
			struct char_struct  *pCldmask )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  float *pVar = NULL;
  unsigned char *pByte = NULL;
  
  VINT nc_id = 0;
  VINT status = 0;
  VINT dimid_nx = 0;
  VINT dimid_ny = 0;
  VINT Year = 0;
  VINT Day = 0;
  size_t nx = 0;
  size_t ny = 0;
  float Hours = 0.;
  VINT start_time = 0;
  VINT stop_time = 0;
  VFLOAT deltaTime = 0.;

  /* Open file */
  status = nc_open(pFilename,NC_NOWRITE,&nc_id);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot open ACSPO NetCDF file ",pFilename);
    /* EXIT open fail */
    return(1);
  }

  /* Read global attributes to get time */
  status = nc_get_att_int(nc_id,NC_GLOBAL,"START_YEAR",&Year);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get START_YEAR attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  status = nc_get_att_int(nc_id,NC_GLOBAL,"START_DAY",&Day);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get START_DAY attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  status = nc_get_att_float(nc_id,NC_GLOBAL,"START_TIME",&Hours);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get START_TIME attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  /* Convert Year, Dayno, Hours to GHRSST Time (from 01-01-1981) */
  start_time = get_GHRSST_Time(Year,Day,Hours);

  status = nc_get_att_int(nc_id,NC_GLOBAL,"END_YEAR",&Year);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get END_YEAR attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  status = nc_get_att_int(nc_id,NC_GLOBAL,"END_DAY",&Day);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get END_DAY attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  status = nc_get_att_float(nc_id,NC_GLOBAL,"END_TIME",&Hours);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get END_TIME attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  stop_time = get_GHRSST_Time(Year,Day,Hours);

  /* Get dimensions */

  /* First get dim ids */
  status = nc_inq_dimid( nc_id, "pixels_across_track", &dimid_nx );
  if( NC_NOERR != status ){
    message(2,
    "ERROR: Cannot get dimid pixels_across_track from ACSPO NetCDF file ",
	    pFilename);
    /* EXIT dimid access fail */
    return(1);
  }

  status = nc_inq_dimid( nc_id, "scan_lines_along_track", &dimid_ny );
  if( NC_NOERR != status ){
    message(2,
    "ERROR: Cannot get dimid scan_lines_along_track from ACSPO NetCDF file ",
	    pFilename);
    /* EXIT dimid access fail */
    return(1);
  }

  /* Now get dimensions */
  status = nc_inq_dimlen( nc_id, dimid_nx, &nx );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dim nx from ACSPO NetCDF file ",pFilename);
    /* EXIT dimlen access fail */
    return(1);
  }

  status = nc_inq_dimlen( nc_id, dimid_ny, &ny );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get dim ny from ACSPO NetCDF file ",pFilename);
    /* EXIT dimlen access fail */
    return(1);
  }  

  /* Allocate arrays for reading */
  if( NULL == (pVar = (float *) calloc(nx*ny,sizeof(float))) ){
    message(1,"ERROR: Cannot allocate pVar (flt) array");
    /* EXIT float variable allocateion fail */
    exit(-1);
  }
  if( NULL == (pByte = (unsigned char *) calloc(nx*ny,sizeof(unsigned char))) ){
    message(1,"ERROR: Cannot allocate pByte (byte) array");
    /* EXIT Byte variable allocateion fail */
    exit(-1);
  }

  /* Get latitude and longitude */
  /* Latitude */
  if( 1 == readNetCDFData_flt(nc_id,"latitude",nx,ny,pVar) ){
    message(2,"ERROR: Error reading ACSPO NetCDF file (latitude)",pFilename);
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
  if( 1 == readNetCDFData_flt(nc_id,"longitude",nx,ny,pVar) ){
    message(2,"ERROR: Error reading ACSPO NetCDF file (longitude)",pFilename);
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

  /* Get sst_regression */
  if( 1 == readNetCDFData_flt(nc_id,"sst_regression",nx,ny,pVar) ){
    message(2,"ERROR: Error reading ACSPO NetCDF file ",pFilename);
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

  /* Get cloud mask */
  if( 1 == readNetCDFData_ubyte(nc_id,"acspo_mask",nx,ny,pByte) ){
    message(2,"ERROR: Error reading ACSPO NetCDF file ",pFilename);
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
  copy_byte_to_str_matlab(nx,ny,(char *)pByte,pCldmask);

  /* Free pVar */
  free(pVar);

  /* Free pByte */
  free(pByte);

  /* Close NetCDF file */
  status = nc_close(nc_id);
  if( NC_NOERR != status ){
    message(2,"ERROR: Error closing ACSPO NetCDF file ",pFilename);
    return(1);
  }

  /* Make time array */
  allocate_int_array_matlab(pSST->nx,pSST->ny,pTime);
  deltaTime = (stop_time-start_time)*1./pSST->ny;
  for(i=0;i<pSST->ny;i++){
    for(j=0;j<pSST->nx;j++){
      *(pTime->array+j+i*pSST->nx) = (VINT)(start_time + i*deltaTime);
    }
  }

  /* Succesful exit */
  return(0);
}

/*
 * NAME: 
 *
 *    read_NetCDFData_ubyte
 *
 * FUNCTION:
 *
 *    C Code to read in a unsigned byte variable from a NetCDF file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in a unsigned byte array from ACSPO NetCDF4 file. 
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
 *    ok = readNetCDFData_ubyte( nc_id, name, nx, ny, outvar )
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
 *   outvar  - output (unsigned char) array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/15/2013
 *
 */
static VINT readNetCDFData_ubyte( int nc_id, const char *pName, size_t nx, 
				  size_t ny, unsigned char *pVar )
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

  status = nc_get_vara_uchar( nc_id, varid, start, count, pVar );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get ubyte variable for ",pName);
    return(1);
  }

  /* Return good status */
  return(0);
}

