/*
 * NAME: 
 *
 *    logfile.h
 *
 * FUNCTION:
 *
 *    Header for logfile.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Header for routines to deal with logfile output.  
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    close_logfile  - closes the logfile
 *    open_logfile   - opens the logfile
 *    message        - writes message to stdout and logfile (if open)
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef LOGFILE_HEADER

#include "types_cnsts.h"

/* prototypes */
VINT close_logfile( void );
VINT open_logfile( const VINT year, const VINT day, const char *stream );
VINT message( VINT nargs, ... );

#define LOGFILE_HEADER 1
#endif
