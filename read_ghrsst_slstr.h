/*
 * NAME: 
 *
 *    read_ghrsst_slstr.h
 *
 * FUNCTION:
 *
 *    Header for read_ghrsst_slstr.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in SLSTR GHRSST L2P data
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_ghrsst_slstr  - reads the SLSTR GHRSST L2P data
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 *   Adapted by Andy Harris 08/25/2022
 *
 */

#ifndef READ_GHRSST_SLSTR_HEADER

/* Include type definitions */
#include "types_cnsts.h"


/* external interface */
VINT read_ghrsst_slstr( const char *pFilename, struct int_struct *pTime, 
		  struct float_struct *pSST, struct float_struct *pLat, 
		  struct float_struct *Lon, 
		  struct float_struct *pSSES_Variance,
		  char *pType, struct char_struct  *pCldmask, 
		  unsigned char day_night, char correct_bias, 
		  VFLOAT output_stdev, VINT min_quality, VINT min_algorithm, VFLOAT min_windspeed );

#define READ_GHRSST_SLSTR_HEADER 1
#endif
