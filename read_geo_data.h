/*
 * NAME: 
 *
 *    read_geo_data.h
 *
 * FUNCTION:
 *
 *    Header for read_geo_data.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read one days worth of GEO files
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_day_geo_data  - reads in one days worth of GEO data
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef READ_GEO_DATA_HEADER

/* Include type definitions */
#include "types_cnsts.h"

VINT read_geo_data( const char *pType, const VINT year, const VINT day, const VINT frequency, 
		    struct sst_struct *pRef_SST, 
		    struct sst_struct *pSST_Variability, 
		    const VINT sst_check, struct big_geo_grid *pBigGridGeoSST, 
		    struct pixel_extent_str *pPixel_Extent,
		    const unsigned char day_night_type, 
		    const struct par_files_str par_files, char use_sses_stdev );

#define READ_GEO_DATA_HEADER 1
#endif
