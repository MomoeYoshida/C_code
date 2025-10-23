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
//#include <mfhdf.h>

/* header for this routine */
#include "types_cnsts.h"
#include "matlab_functions.h"
#include "logfile.h"
#include "read_acspo.h"
#include "read_amsr_ghrsst.h"
#include "read_ghrsst.h"
#include "read_netcdf.h"
#include "init_par_info.h"
#include "diurnal_warming.h"

/* Routines for internal use *//*
static VINT read_acspo_hdf( const char *pFilename, struct int_struct *pTime,
			    struct float_struct *pSST, 
			    struct float_struct *pLat, 
			    struct float_struct *pLon, 
			    struct char_struct  *pCldmask );
static VINT readHdfData_flt( const int32 sd_id, const char *pName, VINT *nx, VINT *ny, 
			     float **ppVar );
static VINT readHdfData_byte( const int32 sd_id, const char *pName, VINT *nx, VINT *ny, 
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
VINT read_amsr_ghrsst( const char *pFilename, const char *pAcspo_format,
		 struct float_struct *pSST, 
		 struct float_struct *pLat, struct float_struct *pLon, 
		 struct char_struct  *pCldmask, 
		 struct float_struct *pSSES_Variance,
		 const struct par_info_type par_info,
		 const VINT correct_bias, const VFLOAT sses_stdev )
{
  /* Local variables */
  VINT ok = 0;
  VINT i = 0;
  struct int_struct Time;
  VFLOAT sses_stdev_sq = 0.;

  init_int_array_matlab(&Time);

    /* If new GHRSST NetCDF4 L2P format requested */
    /* If amsr_sses_bias = 1 also removes SSES Bias from SST */
  ok = read_ghrsst( pFilename, &Time, pSST, pLat, pLon, pSSES_Variance,
		    "AMSR", pCldmask, 0,
		    correct_bias, sses_stdev, 
		    par_info.amsr_ghrsst_min_quality, 0, 0. );  /* Couple of extra parameters (min_algorithm, min_windspeed) - only for SLSTR @present */
  if( 1 == ok ){
    free_int_matlab(&Time);
    free_float_matlab(pSST);
    free_float_matlab(pLat);
    free_float_matlab(pLon);
    free_byte_matlab(pCldmask);
    free_float_matlab(pSSES_Variance);
    return(ok);
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


