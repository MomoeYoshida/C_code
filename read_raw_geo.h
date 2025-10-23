/*
 * NAME: 
 *
 *    read_raw_geo.h
 *
 * FUNCTION:
 *
 *    Header for read_raw_geo.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in the hourly GEO files
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_raw_geo  - reads in the hourly GEO data
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef READ_RAW_GEO_HEADER

/* Include type definitions */
#include "types_cnsts.h"

VINT read_raw_geo( const char *pFilename, struct sst_struct *pSST, 
		   const char *pType, struct par_files_str par_files, 
		   unsigned char day_night );

#define READ_RAW_GEO_HEADER 1
#endif
