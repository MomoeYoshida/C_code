/*
 * NAME: 
 *
 *    get_dates
 *
 * FUNCTION:
 *
 *    C code to deal with dates and datestrings
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to generate the datestring
 *    Note all routines are written to deal with cases where 
 *    the input day is specified as "day-yesterday" where 
 *    yesterday can be any number of days (though normally 1)
 *    This means that the input day can be out of range (<1)
 *    so the codes always recalculate the year day to make sure
 *    that if the input is (for example) 2011 -1 the code 
 *    returns 2010 265.
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    date2mjd - date to days from 1950 variable arguments
 *    get_datestring - gets the date string
 *    get_ghrsst_datestring - gets the date string for GHRSST L2P 
 *    get_avhrr_datestring - gets the date string for AVHRR
 *    get_avhrr_datestring2 - gets the date string for AVHRR2 update
 *    get_goes_datestring - gets the date string for goes 
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
#include <stdarg.h>

/* header for this routine */
#include "matlab_functions.h"
#include "get_dates.h"

/*
 * NAME: 
 *
 *    date2mjd
 *
 * FUNCTION:
 *
 *    C Code to get the mjd bases on either year,day,month or year,day
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    returns the number of days from the star year (currently 01/01/1950) 
 *    using either the year and daynumber, or year,month,day (uses C 
 *    variable arguments)
 *
 *    Based on the following MATLAB code
 *
 *    function [mjd]=date2mjd(year, day, month);
 *
 *    if(nargin==2)
 *       jan_1st=datenum(year,1,1);
 *       mjd=jan_1st+day-1;
 *    elseif(nargin==3)
 *       mjd=datenum(year, month, day);
 *    end
 *
 * PSEUDO CODE
 *
 *    FUNCTION nargs....
 * 
 *       setup variable arguments
 *
 *       IF number of arguments is 2 THEN
 *           get year and day number
 *           get number of days to start of year from 1/1/1950
 *           get total number of days
 *       ELSE
 *           get year month day
 *           get number of days from 1/1/1950
 *       ENDIF 
 *  
 *       RETURN number of days
 * 
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    date_to_days_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    mjd = date2mjd(year,daynum)
 *
 *   or
 * 
 *    mjd = date2mjd(year,daynum,month)
 *
 * INPUTS:  
 *    
 *    year - Julian year
 *     day - day of year or day of month if 3 arguments
 *   month - month of year (optional)
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the mjd
 *
 * SYSTEM CALLS:
 *
 *    va_start
 *    va_args
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
VINT date2mjd( VINT nargs, ... )
{
  va_list inputArgs;

  /* Local variables */
  VINT mjd = 0;
  VINT year = 0;
  VINT month = 0;
  VINT day = 0;

  va_start( inputArgs, nargs );

  if( 2 == nargs ){
    /* 2nd argument is day number in year */
    /* Get year and day number in year */
    year = va_arg( inputArgs, VINT );
    day = va_arg( inputArgs, VINT );
    /* Get mjd to begining of year */
    mjd = date_to_days_matlab(year,1,1);
    /* Add on number of days from 1/1/Year - note -1 to take into account
     * 1st Jan */
    mjd += day-1;
  } else {
    /* 2nd argument is month 3rd is day */
    /* Get year, month, day from input argument list */
    year = va_arg( inputArgs, VINT );
    month = va_arg( inputArgs, VINT );
    day = va_arg( inputArgs, VINT );
    /* Get mjd */
    mjd = date_to_days_matlab(year,month,day);
  }
  /* REturn value */
  return(mjd);
}

/*
 * NAME: 
 *
 *    get_datestring
 *
 * FUNCTION:
 *
 *    C Code to get a string based on the date
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    returns a string based on year and day of year e.g.
 *    yyyy_ddd where year is year, and ddd is day 
 *    '2004_032' is 2004, day 32, (1st February 2004).  
 *    It does deal with cases where the day is < 1 or > 365/366 and makes
 *    appropriate modification to the year in those cases
 *
 *    Based on the following MATLAB code
 *
 *    function [datestring]=get_datestring(year,day);
 *    % Get Modifie Julian Date from year and day of year, and convert back
 *    % to year and dayand day of year. This produces correct date when day 
 *    % is not in range 1-365(6). For example to (2004,0) will give the
 *    % date string '2003_365'.
 *
 *    mjd=date2mjd(year,day);
 *    [year,day]=mjd2date(mjd);
 *
 *    yearstring=num2str_pad_zeros(year,4);
 *
 *    % Pad day to three digits if necessary.
 *    daystring=num2str_pad_zeros(day,3);
 *
 *    % Return date string.
 *    datestring=[yearstring '_' daystring];
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day
 *
 *       get mjd from year,day 
 *
 *       convert mjd back to year,day - this deals with -ve or >365/366 input 
 *       days
 *
 *       write date string
 *
 *       RETURN date string
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    daynum_to_days_matlab
 *    date_from_days_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    date_string = get+datestring(year,day)
 *
 * INPUTS:  
 *    
 *    year - Julian year
 *     day - day of year (can be out of normal day number range)
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the date string
 *
 * SYSTEM CALLS:
 *
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
char *get_datestring( const VINT year, const VINT day )
{
  /* Local variables */
  VINT daynum=0; 
  VINT yr = 0;
  VINT dy = 0;
  /* Static forces this block of memroy to be permanently there */
  static char datestring[MAX_STRING_LENGTH];

  /* Get number of days from NUM_START_YEAR (currently 01/01/1950 */
  /* defined in matlab_functions.h                                */
  daynum = daynum_to_days_matlab(year,day);

  /* Now convert back to year day.  This solves the problem if the input */
  /* day is out of range 1-365/366 e.g. if day = day-1 in call           */
  date_from_days_matlab(daynum,&yr,&dy);
  
  /* Get date string */
  sprintf(datestring,"%4.4d_%3.3d",yr,dy);
  
  /* Return datestring */
  return(&datestring[0]);
}

/*
 * NAME: 
 *
 *    get_ghrsst_datestring
 *
 * FUNCTION:
 *
 *    C Code to get a string based on the date for GHRSST GDS 2.0
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    returns a string based on year, month and day of year e.g.
 *    In GHRSST GDS 2.0 date format which is e.g.
 *
 *       20130126
 *
 *    or
 *
 *      YYYYMMDD
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day
 *
 *       get mjd from year day_number
 *       get year day_number from mjd
 *       get month day from day_number
 *
 *       make date string
 *
 *       RETURN date string
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    daynum_to_days_matlab
 *    date_from_days_matlab
 *    date_from_daynum_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    date_string = get_ghrsst_datestring(year,day)
 *
 * INPUTS:  
 *    
 *    year - Julian year
 *     day - day of year (can be out of normal day number range)
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the date string
 *
 * SYSTEM CALLS:
 *
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
char *get_ghrsst_datestring( const VINT year, const VINT daynum )
{
  /* Local variables */
  VINT yearout = 0;
  VINT days = 0;
  VINT month = 0;
  VINT day = 0;
  VINT mjd = 0;

  /* Static forces this block of memory to be permanently there */
  static char string[MAX_STRING_LENGTH];

  /* Get MJD - deals with daynum outside 1,364/365 */
  mjd = daynum_to_days_matlab(year,daynum);
  /* Get day number */
  date_from_days_matlab( mjd, &yearout, &days );
  /* Get date (day,month) */
  date_from_daynum_matlab(yearout,days,&month,&day);
  /* Write datestring */
  sprintf(string,"%4.4d%2.2d%2.2d",yearout,month,day);

  /* Return date string */
  return(string);
}

/*
 * NAME: 
 *
 *    get_avhrr_datestring
 *
 * FUNCTION:
 *
 *    C Code to get AVHRR datestring
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets the datestring based on year, day in year and returns a string
 *    in the AVHRR format.  Make sure that subtracting from the day works
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day_number
 *
 *       get mjd from year day_number
 *       get year day_number from mjd
 *
 *       convert year (4 digits) to 2 digit form
 *
 *       make date string
 *
 *       RETURN date string
 * 
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    daynum_to_days_matlab
 *    date_from_days_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    pStr = get_avhrr_datestring(year,day)
 *
 * INPUTS:  
 *    
 *    year - year
 *     day - day in year
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the datestring
 *
 * SYSTEM CALLS:
 *
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
char *get_avhrr_datestring( const VINT year, const VINT daynum )
{
  /* Local variables */
  VINT mjd = 0;
  VINT yearout = 0;
  VINT days = 0;
  static char string[MAX_STRING_LENGTH];
  
  /* Make sure we get the right year - corrects for daynum < 1 */
  mjd = daynum_to_days_matlab(year,daynum);
  /* Return year days where year is 4 digit number */
  date_from_days_matlab( mjd, &yearout, &days );

  /* Truncate year to 2 digits - deals with years before 2000 for possible
  * POES-GOES reprocessing of archive data */
  if( 2000 > yearout ){
    yearout -= 1900;
  } else {
    yearout -= 2000;
  }

  /* Write datestring */
  sprintf(string,"%2.2d%3.3d",yearout,days);

  /* Return date string */
  return(string);
}

/*
 * NAME: 
 *
 *    get_avhrr_datestring2
 *
 * FUNCTION:
 *
 *    C Code to get NOAA/AVHRR datestring
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets the datestring based on year, day in year and returns a string
 *    in the NOAA/AVHRR format.  Works with subtraction of the day
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day_number
 *
 *       get mjd from year day_number
 *       get year day_number from mjd
 *       get month day from day_number
 *
 *       make date string
 *
 *       RETURN date string
 * 
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    daynum_to_days_matlab
 *    date_from_days_matlab
 *    date_from_daynum_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    pStr = get_avhrr_datestring2(year,day)
 *
 * INPUTS:  
 *    
 *    year - year
 *     day - day in year
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the datestring
 *
 * SYSTEM CALLS:
 *
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
char *get_avhrr_datestring2( const VINT year, const VINT daynum )
{
  /* Local variables */
  VINT month = 0;
  VINT day = 0;
  VINT mjd = 0;
  VINT days = 0;
  VINT yearout = 0;
  static char string[MAX_STRING_LENGTH];
  
  /* Get MJD - deals with daynum outside 1,364/365 */
  mjd = daynum_to_days_matlab(year,daynum);
  /* Get day number */
  date_from_days_matlab( mjd, &yearout, &days );
  /* Get date (day,month) */
  date_from_daynum_matlab(yearout,days,&month,&day);
  /* Write datestring */
  sprintf(string,"%4.4d-%2.2d-%2.2d",yearout,month,day);

  /* Return date sstring */
  return(string);
}

/*
 * NAME: 
 *
 *    get_goes_datestring
 *
 * FUNCTION:
 *
 *    C Code to get GOES datestring
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets the datestring based on year, day in year and returns a string
 *    in the GOES format
 *
 * PSEUDO CODE
 *
 *    FUNCTION year day_number
 *
 *       get mjd from year day_number
 *       get year day_number from mjd
 *
 *       make date string
 *
 *       RETURN date string
 * 
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    daynum_to_days_matlab
 *    date_from_days_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    pStr = get_goes_datestring(year,day)
 *
 * INPUTS:  
 *    
 *    year - year
 *     day - day in year
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the datestring
 *
 * SYSTEM CALLS:
 *
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
char *get_goes_datestring( const VINT year, const VINT daynum )
{
  /* Local variables */
  VINT yr = 0;
  VINT dy = 0;
  VINT days = 0;
  static char string[MAX_STRING_LENGTH];
  
  /* Get mjd to deal with daynum outside 1,364/365 */
  days = daynum_to_days_matlab(year,daynum);
  /* convert to new year, daynumber */
  date_from_days_matlab(days,&yr,&dy);
  
  /* Write datestring */
  sprintf(string,"%4.4d-%3.3d",yr,dy);

  /* Return data string */
  return(string);
}
