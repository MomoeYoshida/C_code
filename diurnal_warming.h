/*
 * NAME: 
 *
 *    diurnal_warming.h
 *
 * FUNCTION:
 *
 *    Header for diurnal_warming.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in and use a diurnal warming model
 *    output NetCDF file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_diurnal_file  - reads the DW model data from netCDF
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 12/03/2013
 *
 */

#ifndef DIURNAL_WARMING_HEADER

/* Include type definitions */
#include "types_cnsts.h"

/* external interface */
VINT read_diurnal_file( const char *pFilename );
void remove_diurnal_warming( struct int_struct *pTime,
			     struct float_struct *pLon,
			     struct float_struct *pLat,
			     struct float_struct *pSST,
			     struct float_struct *pVariance );
void free_diurnal_file( );
VINT make_diurnal_filename( char *filestem, VINT Year, VINT Day, char *pOutfile );

#define DIURNAL_WARMING_HEADER 1
#endif
