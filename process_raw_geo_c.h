/*
 * NAME: 
 *
 *    process_raw_geo
 *
 * FUNCTION:
 *
 *    Header for process_raw_geo_c.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine from reading in geo data c version 
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_geo_c  - process the raw GEO data
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef PROCESS_RAW_GEO_HEADER

/* Include type definitions */
#include "types_cnsts.h"

VINT process_raw_geo_c( const char *pType, const VINT year, const VINT day,
			struct sst_struct *pRef_SST, 
			struct sst_struct *pSST_Variability, 
			const VINT sst_check, const struct float_struct *pBiasDay,
			const struct float_struct *pBiasNight,
			struct out_sst_str *pOutdata, 
			const struct par_files_str par_files );


#define PROCESS_RAW_GEO_HEADER 1
#endif
