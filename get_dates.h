/*
 * NAME: 
 *
 *    get_date.h
 *
 * FUNCTION:
 *
 *    Header for get_dates.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to generate the datestring
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    date2mjd - date to days from 1950 variable arguments
 *    get_datestring - gets the date string
 *    get_avhrr_datestring - gets the date string for AVHRR
 *    get_avhrr_datestring2 - gets the date string for AVHRR2 update
 *    get_goes_datestring - gets the date string for goes 
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef GET_DATESTRING_HEADER

/* Include type definitions */
#include "types_cnsts.h"

VINT date2mjd( VINT nargs, ... );
char *get_datestring( const VINT year, const VINT day );
char *get_ghrsst_datestring( const VINT year, const VINT daynum );
char *get_avhrr_datestring( const VINT year, const VINT daynum );
char *get_avhrr_datestring2( const VINT year, const VINT daynum );
char *get_goes_datestring( const VINT year, const VINT daynum );

#define GET_DATESTRING_HEADER 1
#endif
