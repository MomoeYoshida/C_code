/*
 * NAME: 
 *
 *    read_ghrsst.h
 *
 * FUNCTION:
 *
 *    Header for read_ghrsst.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in GHRSST L2P data
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_ghrsst  - reads the ACSPO hdf data
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */

#ifndef READ_GHRSST_HEADER

/* Include type definitions */
#include "types_cnsts.h"

/* Define clear sky probability for cloud clearing */
#define GEO_CLOUD_THRESHOLD 0.98

/* external interface */
VINT read_ghrsst( const char *pFilename, struct int_struct *pTime, 
		  struct float_struct *pSST, struct float_struct *pLat, 
		  struct float_struct *Lon, 
		  struct float_struct *pSSES_Variance,
		  char *pType, struct char_struct  *pCldmask, 
		  VINT day_night, VINT correct_bias, 
		  VFLOAT output_stdev, VINT min_quality, VINT min_algorithm, VFLOAT min_wind_speed );
VINT readNetCDFData_flt( int nc_id, const char *pName, size_t nx, 
			 size_t ny, float *pVar );
VINT get_GHRSST_Time( const VINT Year, const VINT Dayno, const float Hours );

#define READ_GHRSST_HEADER 1
#endif
