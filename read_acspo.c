/*
 * NAME: 
 *
 *    read_acspo
 *
 * FUNCTION:
 *
 *    C reads in ACSPO data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in the ACSPO (hdf) file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_acspo
 *
 * ROUTINES USED INTERNALLY:
 *
 *    read_acspo_hdf             - read in ACSPO (HDF) format data
 *    readHdfData_flt            - read in float data
 *    readHdfData_byte           - read in byte data
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *   Modified to add GHRSST NetCDF ACSPO format - J.Mittaz 11/04/2013
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mfhdf.h>

/* header for this routine */
#include "types_cnsts.h"
#include "matlab_functions.h"
#include "logfile.h"
#include "read_acspo.h"
#include "read_ghrsst.h"
#include "read_netcdf.h"
#include "init_par_info.h"
#include "diurnal_warming.h"

/* Routines for internal use */
static VINT read_acspo_hdf( const char *pFilename, struct int_struct *pTime,
			    struct float_struct *pSST, 
			    struct float_struct *pLat, 
			    struct float_struct *pLon, 
			    struct char_struct  *pCldmask );
static VINT readHdfData_flt( const int32_t sd_id, const char *pName, VINT *nx, VINT *ny, 
                            float **ppVar ); /* Fri17/10/2025: int32 > int32_t */
static VINT readHdfData_byte( const int32_t sd_id, const char *pName, VINT *nx, VINT *ny, 
			      char **ppVar );
/*
 * NAME: 
 *
 *    read_acpso
 *
 * FUNCTION:
 *
 *    C Code to read in ACSPO in either HDF or GHRSST GDS 2.0 L2P
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in data from either HDF4 or GHRSST HDS 2.0 L2P (NetCDF 4.0) format 
 *    ACSPO file.  
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, sst, lat, lon, cld_mask, parinfo
 *
 *       IF parinfo.format == HDF THEN
 *          Read HDF format ACSPO file
 *       ELSE
 *          Read GHRSST L2P (GDS2.0 format) ACSPO file
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    read_acspo_ghrsst
 *    read_acspo_hdf
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_acspo( filename, acspo_format, sst, lat, lon, cld_mask, 
 *                     par_info )
 *
 * INPUTS:  
 *    
 *   filename     - ACSPO filename
 *   acspo_format - ACSPO format (HDF, NETCDF, GHRSST)
 *
 * OUTPUTS:  
 * 
 *   sst      - float_struct array with the SSTs
 *   lat      - float_struct array with latitude
 *   lon      - float_struct array with longitude
 *   cld_mask - packed cloud mask
 *   par_info - par_info structure to get which format of ASCPO data to read
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
VINT read_acspo( const char *pFilename, const char *pAcspo_format,
		 struct float_struct *pSST, 
		 struct float_struct *pLat, struct float_struct *pLon, 
		 struct char_struct  *pCldmask, 
		 struct float_struct *pSSES_Variance,
		 const struct par_info_type par_info,
		 const VINT correct_bias, const VFLOAT sses_stdev ) /* char > VINT */
{
  /* Local variables */
  VINT ok = 0;
  VINT i = 0;
  struct int_struct Time;
  VFLOAT sses_stdev_sq = 0.;

  init_int_array_matlab(&Time);

  if( NULL != strstr(pAcspo_format,"HDF") ){
    /* If Original ACSPO HDF format requested */
    ok = read_acspo_hdf( pFilename, &Time, pSST, pLat, pLon, pCldmask );
    if( 1 == ok ){
      free_int_matlab(&Time);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      free_byte_matlab(pCldmask);
      return(ok);
    }
    /* Set SSES_Stdev to a constant (0.6^2)*/
    allocate_float_array_matlab(pSST->nx,pSST->ny,pSSES_Variance);
    sses_stdev_sq = sses_stdev*sses_stdev;
    for(i=0;i<pSST->size;i++){
      if( NaN != *(pSST->array+i) ){
	*(pSSES_Variance->array+i) = sses_stdev_sq;
      } else {
	*(pSSES_Variance->array+i) = NaN;
      }
    }
  } else if( NULL != strstr(pAcspo_format,"NETCDF") ){
    /* If new GHRSST NetCDF4 L2P format requested */
    /* If acspo_sses_bias = 1 also removes SSES Bias from SST */
    ok = read_acspo_netcdf( pFilename, &Time, pSST, pLat, pLon, pCldmask );
    if( 1 == ok ){
      free_int_matlab(&Time);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      free_byte_matlab(pCldmask);
      return(ok);
    }
    /* Set SSES_Stdev to a constant */
    allocate_float_array_matlab(pSST->nx,pSST->ny,pSSES_Variance);
    sses_stdev_sq = sses_stdev*sses_stdev;
    for(i=0;i<pSST->size;i++){
      if( NaN != *(pSST->array+i) ){
	*(pSSES_Variance->array+i) = sses_stdev_sq;
      } else {
	*(pSSES_Variance->array+i) = NaN;
      }
    }
  } else if( NULL != strstr(pAcspo_format,"GHRSST") ){
    /* If new GHRSST NetCDF4 L2P format requested */
    /* If acspo_sses_bias = 1 also removes SSES Bias from SST */
    ok = read_ghrsst( pFilename, &Time, pSST, pLat, pLon, pSSES_Variance,
		      "ACSPO", pCldmask, 0,
		      correct_bias, sses_stdev, 
		      par_info.acspo_ghrsst_min_quality, 0, 0. ); /* Momoe copied from the read_acspo.c in OneDrive */
    if( 1 == ok ){
      free_int_matlab(&Time);
      free_float_matlab(pSST);
      free_float_matlab(pLat);
      free_float_matlab(pLon);
      free_byte_matlab(pCldmask);
      free_float_matlab(pSSES_Variance);
      return(ok);
    }
  } else {
    /* Can't recognize ACPSO format type */
    message(1,"ERROR: Unrecognized ACSPO format (not HDF, netCDF or GHRSST)");
    return(1);
  }

  /* Correct for DW if requested */
  if( 1 == par_info.correct_diurnal_warming ){
    remove_diurnal_warming( &Time, pLon, pLat, pSST, pSSES_Variance );
  }

  /* Deallocate Time array */
  free_int_matlab(&Time);
      
  /* Return exit status */
  return(ok);
}

/*
 * NAME: 
 *
 *    read_acpso_hdf
 *
 * FUNCTION:
 *
 *    C Code to read in ACSPO original HDF format
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in data from HDF4 ACSPO file
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename, time, sst, lat, lon, cld_mask
 *
 *       open HDF4 file
 *
 *       get GHRSST Time
 *       get SST data
 *       get latitude
 *       get longitude
 *       get cloud mask
 *
 *       close HDF4 file
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    copy_flt_to_str_matlab
 *    readHdfData_flt
 *    copy_byte_to_str_matlab
 *    readHdfData_byte
 *    message
 *    SDstart
 *    SDend
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_acspo_hdf( filename, time, sst, lat, lon, cld_mask )
 *
 * INPUTS:  
 *    
 *   filename - ACSPO filename
 *
 * OUTPUTS:  
 * 
 *   time     - GHRSST Time
 *   sst      - float_struct array with the SSTs
 *   lat      - float_struct array with latitude
 *   lon      - float_struct array with longitude
 *   cld_mask - packed cloud mask
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
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *   Modified to make HDF ACSPO specific - J.Mittaz 11/04/2013
 *
 */
static VINT read_acspo_hdf( const char *pFilename, struct int_struct *pTime, 
			    struct float_struct *pSST, 
			    struct float_struct *pLat, 
			    struct float_struct *pLon, 
			    struct char_struct  *pCldmask )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT nx = 0;
  VINT ny = 0;
  VINT Year = 0;
  VINT Day = 0;
  float *pVar = NULL;
  char *pByte = NULL;
  float Hours = 0.;
  VINT start_time = 0;
  VINT stop_time = 0;
  VFLOAT deltaTime = 0.;

  int32_t sd_id = 0;
  int32_t status = 0;
  int32_t attr_index = 0;

  /* Open file */
  sd_id = SDstart(pFilename,DFACC_READ);
  if( FAIL == sd_id ){
    message(2,"ERROR: Cannot open HDF ACSPO file ",pFilename);
    return(1);
  }

  /* Get time from global attributes */
  attr_index = SDfindattr(sd_id,"START_YEAR");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get START_YEAR attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Year);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get START_YEAR attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  attr_index = SDfindattr(sd_id,"START_DAY");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get START_DAY attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Day);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get START_DAY attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  attr_index = SDfindattr(sd_id,"START_TIME");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get START_TIME attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Hours);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get START_TIME attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  /* Convert Year, Dayno, Hours to GHRSST Time (from 01-01-1981) */
  start_time = get_GHRSST_Time(Year,Day,Hours);

  attr_index = SDfindattr(sd_id,"END_YEAR");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get STOP_YEAR attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Year);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get END_YEAR attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  attr_index = SDfindattr(sd_id,"END_DAY");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get END_DAY attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Day);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get END_DAY attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  attr_index = SDfindattr(sd_id,"END_TIME");
  if( FAIL == attr_index ){
    message(2,
    "ERROR: Cannot get END_TIME attribute (index) for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }
  status = SDreadattr(sd_id,attr_index,&Hours);
  if( FAIL == status ){
    message(2,
    "ERROR: Cannot get END_TIME attribute for ACSPPO NetCDF file ",
	    pFilename);
    /* EXIT open fail */
    return(1);
  }

  /* Convert Year, Dayno, Hours to GHRSST Time (from 01-01-1981) */
  stop_time = get_GHRSST_Time(Year,Day,Hours);

  /* Get sst_regression */
  if( 1 == readHdfData_flt(sd_id,"sst_regression",&nx,&ny,&pVar) ){
    message(2,"ERROR: Error reading HDF ACSPO file ",pFilename);
    status = SDend(sd_id);
    /* Free what has already been allocated */
    if( NULL != pVar ){
      free(pVar);
    }
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pSST);
  /* Free pVar */
  free(pVar);

  /* Get latitude */
  if( 1 == readHdfData_flt(sd_id,"latitude",&nx,&ny,&pVar) ){
    message(2,"ERROR: Error reading HDF ACSPO file ",pFilename);
    status = SDend(sd_id);
    /* Free what has already been allocated */
    if( NULL != pVar ){
      free(pVar);
    }
    free_float_matlab(pSST);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pLat);
  /* Free pVar */
  free(pVar);

  /* Get longitude */
  if( 1 == readHdfData_flt(sd_id,"longitude",&nx,&ny,&pVar) ){
    message(2,"ERROR: Error reading HDF ACSPO file ",pFilename);
    status = SDend(sd_id);
    /* Free what has already been allocated */
    if( NULL != pVar ){
      free(pVar);
    }
    free_float_matlab(pSST);
    free_float_matlab(pLat);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_flt_to_str_matlab(nx,ny,pVar,pLon);
  /* Free pVar */
  free(pVar);

  /* Make sure longitude is in -180,180 range */
  for(i=0;i<pLon->size;i++){
    if( 180. < *(pLon->array+i) ){
      *(pLon->array+i) -= 360.;
    }
  }

  /* Get cloud mask */
  if( 1 == readHdfData_byte(sd_id,"acspo_mask",&nx,&ny,&pByte) ){
    message(2,"ERROR: Error reading HDF ACSPO file ",pFilename);
    status = SDend(sd_id);
    if( NULL != pVar ){
      free(pVar);
    }
    /* Free what has already been allocated */
    free_float_matlab(pSST);
    free_float_matlab(pLat);
    free_float_matlab(pLon);
    /* EXIT error in read */
    return(1);
  }
  /* Copy to output structure */
  copy_byte_to_str_matlab(nx,ny,pByte,pCldmask);
  /* Free pByte */
  free(pByte);

  /* Close HDF file */
  status = SDend(sd_id);

  /* Make time array */
  allocate_int_array_matlab(pSST->nx,pSST->ny,pTime);
  deltaTime = (stop_time-start_time)*1./pSST->ny;
  for(i=0;i<pSST->ny;i++){
    for(j=0;j<pSST->nx;j++){
      *(pTime->array+j+i*pSST->nx) = (VINT) (start_time + i*deltaTime);
    }
  }

  return(0);
}

/*
 * NAME: 
 *
 *    readHdfData_flt
 *
 * FUNCTION:
 *
 *    C Code to read in float variable from ACSPO datafile
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in float variable from HDF4 ACSPO file
 *
 * PSEUDO CODE
 *
 *    FUNCTION sd_id, Name, nx, ny, Var 
 *
 *       get SDS index    
 *       get SDS Id
 *       get SDS infomation (for variable size)
 *       allocate output
 *       read in data
 *       end access to SDS
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 *    ACSPO data in HDF4 format
 *
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    SDnametoindex
 *    SDselect
 *    SDgetinfo
 *    SDreaddata
 *    SDendaccess
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    readHdfData_flt( sds_id, variable_name, nx, ny, array )
 *
 * INPUTS:  
 *    
 *   sds_id        - sds ID for HDF4 file
 *   variable_name - name of variable to retrieve
 *
 * OUTPUTS:  
 * 
 *   nx    - X size
 *   ny    - Y size
 *   array - output float array
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
static VINT readHdfData_flt( const int32_t sd_id, const char *pName, VINT *nx, VINT *ny, 
			     float **ppVar )
{ 
  intn status = 0;
  int32_t dim_size[2];
  int32_t rank = 0;
  int32_t data_type = 0;
  int32_t n_attr = 0;
  int32_t sds_index = 0;
  int32_t sds_id = 0;
  int32_t start[2];
  int32_t edge[2];
  char outName[MAX_STRING_LENGTH];

  /* Get sds_index given name */
  sds_index = SDnametoindex( sd_id, pName );
  if( FAIL == sds_index ){
    message(2,"ERROR: Cannot get sds_index on ",pName);
    /* EXIT error in getting index */
    return(1);
  }
  /* Get sds_id given index */
  sds_id = SDselect(sd_id, sds_index);
  if( FAIL == sds_id ){
    message(2,"ERROR: Cannot get sds_id on ",pName);
    /* EXIT error in getting index */
    return(1);
  }

  /* Get information about named variable */
  status = SDgetinfo(sds_id,outName,&rank,dim_size,&data_type,&n_attr);
  if( FAIL == status ){
    message(2,"ERROR: Cannot get information on ",pName);
    /* EXIT error in infor */
    return(1);
  }
  /* Check data type/size */
  if( DFNT_FLOAT32 != data_type ){
    message(2,"ERROR: Not a float for ",pName);
    /* EXIT error in type */
    return(1);
  }
  if( 2 != rank ){
    message(2,"ERROR: Wrong rank for ",pName);
    /* EXIT error in rank */
    return(1);
  }
  *nx = dim_size[0];
  *ny = dim_size[1];

  /* Allocate output array */
  if( NULL == (*ppVar = (float *) calloc(*nx * *ny,sizeof(float))) ){
    message(2,"ERROR: Failed to allocate memory for ",pName);
    /* EXIT error in allocation */
    exit(-1);
  }

  /* Setup array ranges to read in */
  start[0] = 0;
  start[1] = 0;
  edge[0] = *nx;
  edge[1] = *ny;

  /* Read in data */
  status = SDreaddata(sds_id,start,NULL,edge,(VOIDP) *ppVar);
  if( FAIL == status ){
    message(2,"ERROR: Cannot get read data from ",pName);
    free(*ppVar);
    *ppVar = NULL;
    /* EXIT error in read */
    return(1);
  }

  /* Release access to sds_id */
  status = SDendaccess(sds_id);
  if( FAIL == status ){
    message(2,"ERROR: Cannot close HDF file ",pName);
    /* EXIT error in close */
    return(1);
  }

  /* Return success */
  return(0);
}

/*
 * NAME: 
 *
 *    readHdfData_byte
 *
 * FUNCTION:
 *
 *    C Code to read in byte variable from ACSPO datafile
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in byte variable from HDF4 ACSPO file
 *
 * PSEUDO CODE
 *
 *    FUNCTION sd_id, Name, nx, ny, Var 
 *
 *       get SDS index    
 *       get SDS Id
 *       get SDS infomation (for variable size)
 *       allocate output
 *       read in data
 *       end access to SDS
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 *    ACSPO data in HDF4 format
 *
 * SUBROUTINES CONTAINED: 
 *
 *    SDnametoindex
 *    SDselect
 *    SDgetinfo
 *    SDreaddata
 *    SDendaccess
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    readHdfData_byte( sds_id, variable_name, nx, ny, array )
 *
 * INPUTS:  
 *    
 *   sds_id        - sds ID for HDF4 file
 *   variable_name - name of variable to retrieve
 *
 * OUTPUTS:  
 * 
 *   nx    - X size
 *   ny    - Y size
 *   array - output byte array
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
static VINT readHdfData_byte( const int32_t sd_id, const char *pName, VINT *nx, VINT *ny, 
			      char **ppVar )
{ 
  intn status = 0;
  int32_t dim_size[2];
  int32_t rank = 0;
  int32_t data_type = 0;
  int32_t n_attr = 0;
  int32_t sds_id = 0;
  int32_t sds_index = 0;
  int32_t start[2];
  int32_t edge[2];
  char outName[MAX_STRING_LENGTH];

  /* Get sds_index given name */
  sds_index = SDnametoindex( sd_id, pName );
  if( FAIL == sds_index ){
    message(2,"ERROR: Cannot get sds_index on ",pName);
    /* EXIT error in read */
    return(1);
  }
  /* Get sds_id given index */
  sds_id = SDselect(sd_id, sds_index);
  if( FAIL == sds_id ){
    message(2,"ERROR: Cannot get sds_id on ",pName);
    /* EXIT error in sds_id */
    return(1);
  }

  /* Get information about named variable */
  status = SDgetinfo(sds_id,outName,&rank,dim_size,&data_type,&n_attr);
  if( FAIL == status ){
    message(2,"ERROR: Cannot get information on ",pName);
    /* EXIT error in info */
    return(1);
  }
  /* Check data type/size */
  if( DFNT_UINT8 != data_type ){
    message(2,"ERROR: Not a byte for ",pName);
    /* EXIT error in type */
    return(1);
  }
  if( 2 != rank ){
    message(2,"ERROR: Wrong rank for ",pName);
    /* EXIT error in rank */
    return(1);
  }
  *nx = dim_size[0];
  *ny = dim_size[1];

  /* Allocate output array */
  if( NULL == (*ppVar = (char *) calloc(*nx * *ny,sizeof(char))) ){
    message(2,"ERROR: Failed to allocate memory for ",pName);
    /* EXIT error in allocation */
    exit(-1);
  }

  /* Setup array ranges to read in */
  start[0] = 0;
  start[1] = 0;
  edge[0] = *nx;
  edge[1] = *ny;

  /* Read in data */
  status = SDreaddata(sds_id,start,NULL,edge,(VOIDP) *ppVar);
  if( FAIL == status ){
    message(2,"ERROR: Cannot get read data from ",pName);
    free(*ppVar);
    *ppVar = NULL;
    /* EXIT error in read */
    return(1);
  }

  /* Release access to sds_id */
  status = SDendaccess(sds_id);
  if( FAIL == status ){
    message(2,"ERROR: Cannot close HDF file ",pName);
    /* EXIT error in close */
    return(1);
  }

  /* Return success */
  return(0);
}
