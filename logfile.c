/*
 * NAME: 
 *
 *    logfile
 *
 * FUNCTION:
 *
 *    C version of logfile routines
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains routines to deal with logfile output.  Uses global variables
 *    to contain file pointer and if to write to file.
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    close_logfile             - closes the logfile
 *    create_logfile            - creates or opens for append a logfile
 *    message                   - writes a message to stdout and the logfile
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Include logfile header */
#include "logfile.h"
/* Include file parameters - has logfile name */
#include "init_file_info.h"
#include "get_dates.h"

/* Internal global variables */
static FILE *fp_logfile = NULL;
static VINT write_logfile = 0;

/*
 * NAME: 
 *
 *   close_logfile
 *
 * FUNCTION:
 *
 *   C Code to close the logfile
 *
 * TYPE/LANGUAGE: 
 *
 *   C routine
 *
 * DESCRIPTION:  
 *
 *   closes the logfile if it was used
 *
 * PSEUDO CODE
 *
 *   FUNCTION
 *
 *      IF logfile open THEN
 *         set logfile open to zero
 *         RETURN status of close logfile
 *      ENIF
 *      RETURN zero status
 *
 *   END
 *
 * FILES NEEDED: 
 *    
 *   Logfile if open
 *
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *   ok = close_logfile()
 *
 * INPUTS:  
 *    
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *   Returns a status
 *
 * SYSTEM CALLS:
 *
 *   fclose
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
VINT close_logfile( void )
{

  /* If logfile written to, close file but return the status */
  if( 1 == write_logfile ){
    write_logfile = 0;
    /* EXIT return status of fclose */
    return(fclose(fp_logfile));
  }
  return(0);
}

/*
 * NAME: 
 *
 *    open_logfile
 *
 * FUNCTION:
 *
 *    C Code to open the logfile
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    creates the logfile if it is needed.  If the file already exists then 
 *    will append to the current file.  It uses the parameter 
 *    file_info.name_logfile for the name stem and file_info.logfile_required
 *    determines if the logfile is needed.  Filename will be
 *
 *    e.g. noaa_op_sst_blend_2012_083.log
 *
 *    file_stem + stream + date
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day stream
 *
 *       IF logfile required THEN
 *
 *          concatenate strings to make filename
 *          Filename = analdir+logname+stream+"_"+datestring+".log"
 *          Open file with error checking
 *
 *       ENDIF
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    get_datestring
 *
 * CALLING SEQUENCE:  
 *
 *    ok = open_logfile(year,day,stream)
 *
 * INPUTS:  
 *    year     - year for logfile filename
 *    day      - day in year for logfile filename
 *    stream   - string to add to logfile filename
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    strncpy
 *    strncat
 *    fopen
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
VINT open_logfile( const VINT year, const VINT day, const char *pStream )
{
  /* Local variables */
  char filename[MAX_STRING_LENGTH];

  /* If logfile required */
  if( 1 == file_info.logfile_required ){
    /* Make logfile name */
    /* Directory */
    strncpy(filename,file_info.dir_analysis,MAX_STRING_LENGTH);
    /* logfile name */
    strncat(filename,file_info.name_logfile,MAX_STRING_LENGTH);
    /* Add stream */
    strncat(filename,pStream,MAX_STRING_LENGTH);
    /* Add underscore */
    strncat(filename,"_",MAX_STRING_LENGTH);
    /* Add datestring */
    strncat(filename,get_datestring(year,day),MAX_STRING_LENGTH);
    /* Add .log */
    strncat(filename,".log",MAX_STRING_LENGTH);

    /* Now open logfile and do some error checking */
    if( NULL == (fp_logfile = fopen(filename,"a")) ){
      write_logfile = 0;
      message(1,"*** Output error: cannot write logfile.");
      /* EXIT problem with write */
      return(1);
    } else {
      write_logfile = file_info.logfile_required;
      /* EXIT eveything is OK with open */
      return(0);
    }
  } else {
    /* If no logfile required, say so */
    message(1,"*** No logfile requested.");
  }
  return(0);
}

/*
 * NAME: 
 *
 *    message
 *
 * FUNCTION:
 *
 *    C Code to write messages either to logfile or stderr
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Writes to the logfile if available and to the screen 
 *    Uses the vararg.h macros to impliment variable number of arguments.
 *    e.g.
 *
 *    message(4,"This ","is ","a ","message");
 *
 *    will print
 *
 *    This is a message
 *
 * PSEUDO CODE
 *  
 *    FUNCTION nargs vargs
 *
 *       Setup getting input arguments
 *
 *       LOOP round number of arguments
 *          concatentate input strings
 *       END LOOP
 *       
 *       print to stdout
 *
 *       IF logfile open THEN
 *          print to logfile
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    ok = message(nargs,string1,string2,...)
 *
 * INPUTS:  
 *    
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    va_start
 *    va_arg
 *    strncpy
 *    strncat
 *    printf
 *    fprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
VINT message( VINT nargs, ... )
{
  /* Local variables */
  int i = 0;
  char msg_string[MAX_STRING_LENGTH];

  /* variable for variable argument lists */
  va_list input_args;

  /* If no arguments - return */
  if( 1 > nargs ){
    return(1);
  }

  /* Start getting arguments */
  va_start( input_args, nargs );

  /* Loop round and add to message string */
  /* First copy first argument */
  strncpy(msg_string,va_arg(input_args,char*),MAX_STRING_LENGTH);
  /* Loop round rest of arguments to get final string */
  for(i=1;i<nargs;i++){
    strncat(msg_string,va_arg(input_args,char*),MAX_STRING_LENGTH);
  }

  /* Write message string to output */
  /* First to screen */
  printf("%s\n",msg_string);

  /* If needed, write to log file */
  if( 1 == write_logfile ){
    fprintf(fp_logfile,"%s\n",msg_string);
  }

  /* Return */
  return(0);
}

