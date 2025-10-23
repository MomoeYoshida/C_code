/*
 * NAME: 
 *
 *    read_netcdf.h
 *
 * FUNCTION:
 *
 *    Header for read_netcdf.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in ACSPO NetCDF data
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_acspo_netcdf  - reads the ACSPO netCDF data
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/15/2013
 *
 */

#ifndef READ_NETCDF_HEADER

/* Include type definitions */
#include "types_cnsts.h"

/* external interface */
VINT read_acspo_netcdf( const char *pFilename, struct int_struct *pTime,
			struct float_struct *pSST, struct float_struct *pLat, 
			struct float_struct *Lon, 
			struct char_struct  *pCldmask );

#define READ_NETCDF_HEADER 1
#endif
