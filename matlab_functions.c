/*
 *    matlab_functions
 *
 * FUNCTION:
 *
 *    C versions of some low level MATLAB functions
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains routines to emulate some of the MATLAB built in functions which
 *    are used by the POES-GOES Blended software (originally written in MATLAB).
 *    These routines are an attempt to make the POES-GOES Blended code similar
 *    to the MATLAB code for ease of current maintenance/support.  
 *
 *    Note that some routines are considered external for use outside this module
 *    and some are internal (defined as static)
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *        date_to_days_matlab     - gets number of days since 01/1950
 *        daynum_to_days_matlab   - gets number of days since 01/1950
 *        date_from_days_matlab   - gets date from ndays since 01/1950
 *        date_from_daynum_matlab - gets day number from date
 *        std_matlab              - returns standard deviation 
 *        sort_matlab             - outputs the sorted array
 *        find_float_matlab       - returns positions that match criteria
 *        find_float_1d_matlab    - returns positions that match criteria
 *        find_float_array_matlab - returns positions that match criteria
 *        find_byte_matlab        - returns positions that match criteria
 *        copy_float_matlab       - copies array structure (floats)
 *        copy_float_matlab_noalloc - copies array structure (no allocation)
 *        copy_float_1d_malab     - copies array structure (floats)
 *        copy_float_1d_malab_noalloc - copies array structure (no allocation)
 *        array_float_oper_matlab - operates on float arrays 
 *        free_float_1d_matlab    - free's memory for float_struct
 *        free_float_matlab       - free's memory for float_struct
 *        free_float_3d_matlab    - free's memory for float_struct
 *        free_int_matlab         - free's memory for int_struct              
 *        free_byte_matlab        - free's memory for byte_struct             
 *        free_elem_matlab        - free's memory for find_elem
 *        free_sst_matlab         - free's memory for sst_struct
 *        free_bigsst_matlab      - free's memory for find_elem
 *        set_find_float_matlab   - sets values based on a previous find
 *        set_float_array_cnst_matlab - makes and sets values in a float array
 *        mult_float_array_matlab - multiply an array str by a constant
 *        add_float_array_matlab     - adds a constant to an array
 *        subract_float_array_matlab - subtract a constant from an array
 *        allocate_float_array_matlab - allocate an array structure
 *        allocate_float_1d_array_matlab - allocate an array structure
 *        allocate_float_3d_array_matlab - allocate an array structure
 *        allocate_int_array_matlab - allocate an array structure
 *        allocate_byte_array_matlab - allocate an array structure
 *        set_min_array_float_matlab - set each element to MIN(value,min_value)
 *        set_max_array_float_matlab - set each element to MIN(value,min_value)
 *        transpose_flip_float_matlab - transpose and flip float array
 *        transpose_flip_int_matlab   - transpose and flip int array
 *        size_zero_float             - set size elements to zero in structure
 *        size_zero_float3d           - set size elements to zero in structure
 *        find_files                  - gets a list of filenames
 *        free_files                  - frees filenames
 *        squeeze_2d_float_matlab     - squeeze line/column to 1d array
 *        rebin_data_matlab           - nearest neighbor rebinning of data
 *        init_sst_matlab             - initialize an sst structure
 *        init_big_geo_grid_matlab    - initialize an big_geo_grid structure
 *        init_out_sst_matlab         - initialize an out_sst_str structure
 *        init_float_array_matlab     - initialize an array structure
 *        init_float_1d_array_matlab  - initialize an array structure
 *        init_float_3d_array_matlab  - initialize an array structure
 *        init_int_array_matlab       - initialize an array structure
 *        init_byte_array_matlab      - initialize an array structure
 *        init_elem_matlab            - initialize an array structure
 *        copy_flt_to_str_matlab      - copy float array to strucure
 *        copy_byte_to_str_matlab     - copy byte array to strucure
 *
 * ROUTINES USED INTERNALLY:
 *
 *        isLeap                      - return 1 if year is leap year
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

/* Standard C library includes */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <glob.h>

/* Include header file for this module */
#include "matlab_functions.h"
#include "logfile.h"

/* routines for internal use only */
static VINT isLeap( const VINT year );

/* variables for internal use in this module */
#define N_MONTHS 13
static VINT dayNumber[N_MONTHS] = 
  {0,31,59,90,120,151,181,212,243,273,304,334,365};
static VINT dayNumberLeap[N_MONTHS] = 
  {0,31,60,91,121,152,182,213,244,274,305,335,366};

/* Start/End year for date/daynum routines - set to 1950/2100 */
#define DAYNUM_START_YEAR 1950
#define DAYNUM_END_YEAR 2100

/*
 * NAME: 
 *
 *    isLeap
 *
 * FUNCTION:
 *
 *    C Code to return 1 if year is a leap year
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns 1 of input year is a leap year, 0 else
 *
 * PSEUDO CODE
 * 
 *    FUNCTION( input_year )
 *
 *      IF year satisfies Leap Year criteria THEN
 *         RETURN 1
 *      ELSE
 *         RETURN 0
 *      ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    leap = isLeap( year )
 *
 * INPUTS:  
 *    
 *     year - input year
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns 1 if a leap year, 0 if not
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
static VINT isLeap( const VINT year )
{
  if( (0 == year%4 && 0 != year%100) || (0 == year%400) ){
    /* EXIT If leap year return 1 */
    return(1);
  } else {
    /* EXIT If not leap year return 0 */
    return(0);
  }
}

/*
 * NAME: 
 *
 *    date_to_days_matlab
 *
 * FUNCTION:
 *
 *    C Code to return days since 1950 from date
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function datenum.  
 *    On input of year, month, day will return the number of days since
 *    1st Jan 1950.  Used in the POES-GOES Blended software for filenames
 *    which are date specific
 *
 * PSEUDO CODE
 * 
 *    FUNCTION( input_year input_month input_day)
 *
 *      IF input_year is of form %% i.e. 98 equals 1998 and 02 equals 2002 THEN
 *         IF input_year less than 50 THEN
 *            set year to 2000 + input_year 
 *         ELSE
 *            set year to 1900 + input_year  
 *         ENDIF
 *      ELSE
 *         set year to input_year
 *      ENDIF
 *
 *      LOOP round years from 1950 
 *         IF year is Leap Year THEN
 *            Add 366 to sum
 *         ELSE
 *            Add 365 to sum
 *         ENDIF
 *      END LOOP
 *      IF input_year is a Leap Year THEN
 *         Add Day Number from month and day defined for Leap Year to sum 
 *      ELSE
 *         Add Day Number from month and day defined for non Leap Year to sum
 *      ENDIF
 *
 *      RETURN sum
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    isLeap
 *
 * CALLING SEQUENCE:  
 *
 *    daynum = date_to_days_matlab(year,month,day)
 *
 * INPUTS:  
 *    
 *     year - input year
 *    month - input month
 *      day - input day
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the number of days since 01/01/1950
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT date_to_days_matlab( const VINT year, const VINT month, const VINT day )
{
  /* Local variables */
  VINT i = 0;
  VINT daynum = 0;
  VINT year_use = 0;

  /* Make sure year is of form 2007/1998 even if input is 7/98 */
  /* DAYNUM_START_YEAR is currently set to 1950 */
  if( DAYNUM_START_YEAR > year ){
    /* Year is in form e.g. 98 for 1998 */
    /* Are we in 19%% or 20%% years */
    if( 50 > year ){
      year_use = 2000+year;
    } else {
      year_use = 1900+year;
    }
  } else {
    /* Year already in long from e.g. 2012 */
    year_use = year;
  }

  /* Accumulate number of days from DAYNUM_START_YEAR (1950) */
  daynum = 0;
  for(i=DAYNUM_START_YEAR;i<year_use;i++){
    if( 1 == isLeap(i) ){
      daynum += 366;
    } else {
      daynum += 365;
    }
  }

  if( 1 == isLeap(year) ){
    /* If current year is a leap year use leap year day number */
    daynum += dayNumberLeap[month-1] + day;
  } else {
    /* If current year is not a leap year use non-leap year day number */
    daynum += dayNumber[month-1] + day;
  }
  return(daynum);
}

/*
 * NAME: 
 *
 *    daynum_to_days_matlab
 *
 * FUNCTION:
 *
 *    C Code to return days since 1950 from date (year,daynumber)
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function datenum.  
 *    On input of year, day will return the number of days since
 *    1st Jan 1950.  Used in the POES-GOES Blended software for filenames
 *    which are date specific
 *
 * PSEUDO CODE
 * 
 *    FUNCTION( input_year input_day)
 *
 *      IF input_year is of form %% i.e. 98 equals 1998 and 02 equals 2002 THEN
 *         IF input_year less than 50 THEN
 *            set year to 2000 + input_year 
 *         ELSE
 *            set year to 1900 + input_year  
 *         ENDIF
 *      ELSE
 *         set year to input_year
 *      ENDIF
 *
 *      LOOP round years from 1950 
 *         IF year is Leap Year THEN
 *            Add 366 to sum
 *         ELSE
 *            Add 365 to sum
 *         ENDIF
 *      END LOOP
 *      IF input_year is a Leap Year THEN
 *         Add Day Number defined for Leap Year to sum
 *      ELSE
 *         Add Day Number defined for non Leap Year to sum
 *      ENDIF
 *
 *      RETURN sum
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    isLeap
 *
 * CALLING SEQUENCE:  
 *
 *    daynum = daynum_to_days_matlab(year,day)
 *
 * INPUTS:  
 *    
 *     year - input year
 *      day - input day number in year
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the day number from 01/01/1950
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT daynum_to_days_matlab( const VINT year, const VINT day )
{
  /* Local variables */
  VINT i = 0;
  VINT daynum = 0;
  VINT year_use = 0;

  /* Make sure year is of form 2007/1998 even if input is 7/98 */
  /* DAYNUM_START_YEAR is currently set to 1950 */
  if( DAYNUM_START_YEAR > year ){
    /* Year is in form e.g. 98 for 1998 */
    /* Are we in 19%% or 20%% years */
    if( 50 > year ){
      year_use = 2000+year;
    } else {
      year_use = 1900+year;
    }
  } else {
    /* Year already in long from e.g. 2012 */
    year_use = year;
  }

  /* Accumulate number of days from DAYNUM_START_YEAR (1950) */
  daynum = 0;
  for(i=DAYNUM_START_YEAR;i<year_use;i++){
    if( 1 == isLeap(i) ){
      daynum += 366;
    } else {
      daynum += 365;
    }
  }

  /* Add number of days in current year */
  daynum += day;
  return(daynum);
}

/*
 * NAME: 
 *
 *    date_from_days_matlab
 *
 * FUNCTION:
 *
 *    C Code to return date from days since 1950
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function datenum.  
 *    On input of day  number from 01/01/1950 returns the date (year, month, 
 *    day). 
 *
 * PSEUDO CODE
 * 
 *    SUBROUTINE( day_number_from_1950 output_year output_daynum )
 *
 *      LOOP from 1950 to 2100 (beyond what is expected)
 *
 *         IF Leap Year THEN
 *            IF day_number_from_1950 lt accumulated_daynum+366 THEN
 *               This is the year we need - output_year = year in loop
 *               output_daynum is difference from accumulated_daynum from 
 *               day_number_from_1950
 *            ELSE
 *               increment accumulated_daynum by 366
 *            ENDIF
 *         ELSE Not Leap Year
 *            IF day_number_from_1950 lt accumulated_daynum+365 THEN
 *               This is the year we need - output_year = year in loop
 *               output_daynum is difference from accumylated_daynum from 
 *               day_number_from_1950
 *            ELSE
 *               increment accumulated_daynum by 365
 *            ENDIF
 *         ENDIF
 *
 *      END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    isLeap
 *
 * CALLING SEQUENCE:  
 *
 *    date_from_days_matlab(daynum,&year,&daynumber)
 *
 * INPUTS:  
 *    
 *    daynum - input day number from 01/01/1950
 *
 * OUTPUTS:  
 *
 *      year - output year (4 digit)
 *       day - output daynum in year
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void date_from_days_matlab( const VINT datenum, VINT *pYear, VINT *pDaynum )
{
  /* Local variables */
  VINT i = 0;
  VINT daynum = 0;

  daynum = 0;
  /* Find year between DAYNUM_START_YEAR (1950) and DAYNUM_END_YEAR (2100) */
  for(i=DAYNUM_START_YEAR;i<DAYNUM_END_YEAR;i++){
    /* If leap year max days in year = 366 */
    if( 1 == isLeap(i) ){
      /* If datenum within this year */
      if( daynum+366 >= datenum ){
	*pYear = i;
	*pDaynum = datenum - daynum;
	break;
      } else {
	/* Accumumate total number of days */
	daynum += 366;
      }
    } else {
      /* If not leap year max days in year = 365 */
      /* If datenum within this year */
      if( daynum+365 >= datenum ){
	*pYear = i;
	*pDaynum = datenum - daynum;
	break;
      } else {
	/* Accumumate total number of days */
	daynum += 365;
      }
    }
  }

}

/*
 * NAME: 
 *
 *    date_from_daynum_matlab
 *
 * FUNCTION:
 *
 *    C Code to get day/month from daynum (in year)
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns the month day given a daynumber (in year)
 *
 * PSEUDO CODE
 * 
 *    SUBROUTINE( input_year input_day_number output_month output_day )
 *
 *       IF Leap Year THEN
 *          LOOP round months in year
 *             IF input_day_number less than day_number_for_month for Leap Year
 *                Return output_month
 *                Get output_day from difference from input_day_number and
 *                day_number_for_month
 *                RETURN
 *             ENDIF 
 *          END LOOP
 *       ELSE Not Leap Year
 *          LOOP round months in year
 *             IF input_day_number less than day_number_for_month 
 *                Return output_month
 *                Get output_day from difference from input_day_number and
 *                day_number_for_month
 *                RETURN
 *             ENDIF 
 *          END LOOP
 *       ENDIF
 *
 *       If we are here there's been an error - report error
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    isLeap
 *    message 
 *
 * CALLING SEQUENCE:  
 *
 *    date_from_daynum_matlab(year,daynum,&month,&day)
 *
 * INPUTS:  
 *    
 *      year - year
 *    daynum - input day number within year
 *
 * OUTPUTS:  
 *
 *     month - output month
 *       day - output day in month
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void date_from_daynum_matlab( const VINT year, const VINT daynum, VINT *pMonth, VINT *pDay )
{
  /* Local variables */
  VINT i = 0;

  /* If year is leap year use leap year day numbers in year */
  if( 1 == isLeap(year) ){
    /* Loop round number of months to find correct month */
    for(i=1;i<N_MONTHS;i++){
      if( daynum <= dayNumberLeap[i] ){
	/* Get month and day and return */
	*pMonth = i;
	*pDay = daynum - dayNumberLeap[i-1];
	return;
      }
    }
  } else {
    /* Not a leap year */
    /* Loop round number of months to find correct month */
    for(i=1;i<N_MONTHS;i++){
      if( daynum <= dayNumber[i] ){
	/* Get month and day and return */
	*pMonth = i;
	*pDay = daynum - dayNumber[i-1];
	return;
      }
    }
  }

  /* Should not get here */
  message(1,
  "ERROR: Cannot get month/day from day number in date_from_daynum_matlab");
  /* EXIT */
  exit(-1);

}

/*
 * NAME: 
 *
 *    std_matlab
 *
 * FUNCTION:
 *
 *    C Code to return standard deviation from array data
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function std().  
 *    Calculates standard deviation for a section of input array.  
 *
 *    Note, uses the Kunth (Wilton) method which is faster than Sum(X-Xmean)^2 
 *    and less prone to rounding problems obtained by Sum X^2 - n (Sum X)^2
 *
 * PSEUDO CODE
 *
 *    FUNCTION( input_array, array_start, array_stop, mean_value )
 *
 *       Check we have a sensible number of points to get standard deviation
 *
 *       LOOP from array_start to array_stop
 *
 *          Use the Kunth (Wilton) Method
 *
 *          Ndata = Ndata + 1
 *          Delta = input_array - mean_value
 *          mean_value = Delta / Ndata
 *          M2 = M2 + Delta*(input_array - mean_value)
 *
 *       END LOOP
 *
 *       mean_value is already calculated from above loop
 *  
 *       standard_deviation = SQRT( M2 / (Ndata - 1 ) )
 *
 *       RETURN standard_deviation
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    standard_dev = std_matlab(inarray,0,array_size,mean)
 *
 * INPUTS:  
 *    
 *   inarray - real array to get standard deviation from
 *     start - start position for calculation
 *      stop - end position for calculation
 *
 * OUTPUTS:  
 *
 *      mean - returns the mean value
 *
 * RETURNS:
 *
 *   Returns the standard deviation
 *
 * SYSTEM CALLS:
 *
 *   sqrt
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VFLOAT std_matlab( VFLOAT *pArray, const VINT start, const VINT stop, VFLOAT *pMean )
{
  /* Local variables */
  VINT i = 0;
  VINT ndata = 0;
  double value = 0.;
  double meanval = 0.;
  double delta = 0.;
  double M2 = 0.;
  VFLOAT stdev = 0.;
  VFLOAT *pArr = NULL;

  /* Number of data values for calculation */
  ndata = stop-start;
  /* If not enough data points to make a calculation */
  if( 1 >= ndata ){
    message(1,
"ERROR: Not enough data points to generate a standard deviation in std_matlab");
    exit(-1);
  }

  /* Initialize data */
  ndata = 0;
  meanval = 0.;
  M2 = 0.;
 
  /* Loop round array from start to stop - zero indexed */
  pArr = pArray;
  for(i=start;i<stop;i++){
    value = (double) *(pArr++);
    ndata++;
    delta = value - meanval;
    meanval += delta/ndata;
    M2 += delta*(value - meanval);
  }
  /* Get standard deviation and mean */
  stdev = (VFLOAT)(M2/(ndata-1));
  *pMean = meanval;

  /* Return standard deviation */
  return(sqrt(stdev));
}

/*
 * NAME: 
 *
 *    std_matlab_weighted
 *
 * FUNCTION:
 *
 *    C Code to return standard deviation from array data for the weighted case
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function std().  
 *    Calculates standard deviation for a section of input array.  
 *
 *    Note, uses 1/variance as a weight and calculates the unbiased 
 *    standard deviation for the weighted case.
 *
 * PSEUDO CODE
 *
 *    FUNCTION( input_array, array_start, array_stop, mean_value )
 *
 *       Check we have a sensible number of points to get standard deviation
 *
 *       LOOP from array_start to array_stop
 *
 *          Calculate weighted average
 *
 *       END LOOP
 *
 *       mean_value is already calculated from above loop
 *  
 *       LOOP from array_start to array_stop
 *
 *          Calculate weighted variance
 *
 *       END LOOP
 *
 *       standard_deviation = SQRT( V1/(V1^2-V2)*weighted_variance )
 *
 *       RETURN standard_deviation
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    standard_dev = std_matlab_weighted(inarray,vararray,0,array_size,mean)
 *
 * INPUTS:  
 *    
 *   inarray  - real array to get standard deviation from
 *   vararray - input variance
 *     start  - start position for calculation
 *      stop  - end position for calculation
 *
 * OUTPUTS:  
 *
 *      mean - returns the mean value
 *
 * RETURNS:
 *
 *   Returns the standard deviation
 *
 * SYSTEM CALLS:
 *
 *   sqrt
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VFLOAT std_matlab_weighted( VFLOAT *pArray, VFLOAT *pVariance, 
			    const VINT start, const VINT stop, VFLOAT *pMean )
{
  /* Local variables */
  VINT i = 0;
  VINT ndata = 0;
  double value = 0.;
  double meanval = 0.;
  double w = 0.;
  double weight = 0.;
  double weight2 = 0.;
  double variance = 0.;
  VFLOAT *pArr = NULL;
  VFLOAT *pVar = NULL;
  VFLOAT stdev = 0.;

  /* Number of data values for calculation */
  ndata = stop-start;
  /* If not enough data points to make a calculation */
  if( 1 >= ndata ){
    message(1,
"ERROR: Not enough data points to generate a standard deviation in std_matlab");
    exit(-1);
  }

  /* Initialize data */
  ndata = 0;
  meanval = 0.;
  weight = 0.;
  weight2 = 0.;
  variance = 0.;
 
  /* Loop round array from start to stop - zero indexed */
  pArr = pArray;
  pVar = pVariance;
  for(i=start;i<stop;i++){
    value = (double) *(pArr++);
    w = (double) (1. / *(pVar++));
    meanval += w * value;
    weight += w;
  }
  if( 0 >= weight ){
    message(1,"ERROR: weights for Mean and Stdev <= 0.");
    /* EXIT bad weights - something wrong */
    exit(-1);
  }
  meanval /= weight;

  /* Get standard deviation and mean */  
  pArr = pArray;
  pVar = pVariance;
  for(i=start;i<stop;i++){
    value = (double) *(pArr++);
    w = (double) (1. / *(pVar++));
    variance += w * (value - meanval);
    weight2 += w*w;
  }
  stdev = (VFLOAT)((weight/(weight*weight-weight2))*variance);
  *pMean = meanval;

  /* Return standard deviation */
  return(sqrt(stdev));
}

/*
 * NAME: 
 *
 *    transpose_flip_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to transpose and flip a float array structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Transposes and flips the array in a float array structure
 *    OI code seems to work in lat,lon space where S is at beginning of file
 *    (data stored so north is line 1)
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE input_array
 *
 *       allocate internal storage array
 *
 *       LOOP round x and y dimensions
 *
 *          Store transpose and flipped data in storage array
 *
 *       END LOOP
 *      
 *       Copy storage array back to input array (same size)
 *
 *       Free storage array
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    init_float_array_matlab
 *    allocate_float_array_matlab
 *    copy_float_matlab_noalloc
 *    free_float_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    transpose_flip_float_matlab( array )
 *
 * INPUTS:  
 *    
 *     array - float_struct array to be transposed and flipped
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void transpose_flip_float_matlab( struct float_struct *pArray )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  struct float_struct flipped;

  /* Initialize array */
  init_float_array_matlab(&flipped);
  /* Allocate pFlipped ready for transpose */
  allocate_float_array_matlab( pArray->ny, pArray->nx, &flipped );

  /* Do the transpose and flip */
  for(i=0;i<pArray->ny;i++){
    for(j=0;j<pArray->nx;j++){
      /* Tranpose and Flipped */
      *(flipped.array + ((pArray->ny-1-i) + j*pArray->ny)) = 
	*(pArray->array+j+i*pArray->nx);      
    }
  }

  /* Copy to pArray */
  copy_float_matlab_noalloc( &flipped, pArray );

  /* Free memory */
  free_float_matlab( &flipped );
}

/*
 * NAME: 
 *
 *    transpose_flip_int_matlab
 *
 * FUNCTION:
 *
 *    C Code to transpose and flip a int array structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Transposes and flips the array in a int array structure
 *    OI code seems to work in lat,lon space where S is at beginning of file
 *    (data stored so north is line 1)
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE input_array
 *
 *       allocate internal storage array
 *
 *       LOOP round x and y dimensions
 *
 *          Store transpose and flipped data in storage array
 *
 *       END LOOP
 *      
 *       Copy storage array back to input array (same size)
 *
 *       Free storage array
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    init_int_array_matlab
 *    allocate_int_array_matlab
 *    copy_int_matlab_noalloc
 *    free_int_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    transpose_flip_int_matlab( array )
 *
 * INPUTS:  
 *    
 *     array - int_struct array to be transposed and flipped
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 30/05/2014
 *
 */
void transpose_flip_int_matlab( struct int_struct *pArray )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  struct int_struct flipped;

  /* Initialize array */
  init_int_array_matlab(&flipped);
  /* Allocate pFlipped ready for transpose */
  allocate_int_array_matlab( pArray->ny, pArray->nx, &flipped );

  /* Do the transpose and flip */
  for(i=0;i<pArray->ny;i++){
    for(j=0;j<pArray->nx;j++){
      /* Tranpose and Flipped */
      *(flipped.array + ((pArray->ny-1-i) + j*pArray->ny)) = 
	*(pArray->array+j+i*pArray->nx);      
    }
  }

  /* Copy to pArray */
  copy_int_matlab_noalloc( &flipped, pArray );

  /* Free memory */
  free_int_matlab( &flipped );
}

/*
 * NAME: 
 *
 *    bubblesort_matlab
 *
 * FUNCTION:
 *
 *    C Code to sort an array and an associated array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Emulates some of the functionality of the MATLAB function sort().  
 *    Uses the bubble sort algorithm and sorts from high to low
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE input_array output_array associated_array
 *
 *       IF output array no allocated THEN
 *          create output_array and copy input_array
 *       ELSE
 *          copy data to output array without allocation
 *       ENDIF
 *
 *       ok = 0
 *       LOOP while ok == 0
 *
 *          ok = 1  - if nothing needs swapping exit main loop
 *          LOOP over array
 *
 *             IF array at i < array at i+1 and needs swapping THEN
 *                swap elements
 *                ok = 0 - make sure main loop loops again
 *              ENDIF
 *
 *          END
 *
 *       END
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *    
 *    copy_float_1d_matlab
 *    copy_float_1d_matlab_noalloc
 *
 * CALLING SEQUENCE:  
 *
 *    bubblesort_matlab(&array,&outarray,&out_associated)
 *
 * INPUTS:  
 *    
 *    array     - float_struct array to be sorted
 *    assocated - float_struct associated to be sorted the same way as array
 *
 * OUTPUTS:  
 *
 *    outarray   - Returns a float_struct array with the sorted values
 *    out_associated - Returns a float_struct array with the sorted 
 *                     assocated values sorted in the same way as array
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *   Modified to add associated array on 06/12/2013
 *
 */
void bubblesort_matlab( const struct float_struct1d *pArray, 
			const struct float_struct1d *pAssoc, 
			struct float_struct1d *pOutArray,
			struct float_struct1d *pOutAssoc )
{
  /* Local variables */
  VINT i = 0;
  VINT ok = 0;
  VFLOAT temp = 0.;

  /* create output variable - note check to see if outarray is already 
   * allocated */
  if( NULL == pOutArray->array ){
    /* Need to allocate memory */
    copy_float_1d_matlab(pArray,pOutArray);
  } else {
    /* Just copy data over to already allocated array */
    copy_float_1d_matlab_noalloc(pArray,pOutArray);
  }

  if( NULL == pOutAssoc->array ){
    /* Need to allocate memory */
    copy_float_1d_matlab(pAssoc,pOutAssoc);
  } else {
    /* Just copy data over to already allocated array */
    copy_float_1d_matlab_noalloc(pAssoc,pOutAssoc);
  }

  /* Do bubble sort */
  ok = 0;
  while(0 == ok){
    /* Lo0op round until there is nothing left to swap */
    ok = 1;
    for(i=0;i<pOutArray->size-1;i++){
      /* If array[i] lt array[i+1] then swap over */
      if( *(pOutArray->array+i) < *(pOutArray->array+i+1) ){
	/* Swap variables */
	temp = *(pOutArray->array+i);
	*(pOutArray->array+i) = *(pOutArray->array+i+1);
	*(pOutArray->array+i+1) = temp;
	temp = *(pOutAssoc->array+i);
	*(pOutAssoc->array+i) = *(pOutAssoc->array+i+1);
	*(pOutAssoc->array+i+1) = temp;
	/* Make sure that we continue the loop */
	ok = 0;
      }
    }
  }
}

/*
 * NAME: 
 *
 *    set_find_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to set values in an array on the basis of a list of array 
 *    locations
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Replaces array locations stored in a list (from find) by another value
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array list value
 *
 *       LOOP round number of list positions
 *
 *          Set array to value
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    set_find_flaot_matlab(array,bad,badval)
 *
 * INPUTS:  
 *    
 *    pVar   - input array structure to be modified
 *    pBad   - list of array positions to be modified
 *    badVal - replacement value
 *
 * OUTPUTS:  
 *
 *    pVar (input) is modified
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void set_find_float_matlab(const struct float_struct *pVar, struct find_elems *pBad,
			   const VFLOAT badVal )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* pArray has indexes that need to be set */
  pArray = pBad->elems;
  for(i=0;i<pBad->nelems;i++){
    /* Set values from find list */
    *(pVar->array + *(pArray++) ) = badVal;
  }
}

/*
 * NAME: 
 *
 *    add_string_matlab
 *
 * FUNCTION:
 *
 *    C Code to add strings together
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Adds a variable number of strings together
 *
 * PSEUDO CODE
 *
 *    FUNCTION number_of_strings input_list_of_strings
 *
 *       Get strings from variable argument list
 *
 *       LOOP input strings
 *
 *          add strings together in output_string
 *
 *       END LOOP
 *
 *       RETURN output_string
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    exmaples:
 *     pointer to string = add_string_matlab(2,"string1","string2")
 *     pointer to string = add_string_matlab(3,"string1","string2","string3")
 *
 * INPUTS:  
 *    
 *    nargs - number of following arguments
 *     args - input strings, number of them is nargs
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    final string
 *
 * SYSTEM CALLS:
 *
 *    strncpy
 *    strncat
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
char *add_string_matlab( VINT nargs, ... )
{
  va_list inputArgs;
  VINT i = 0;
  static char string[MAX_STRING_LENGTH];

  va_start( inputArgs, nargs );

  /* Make string */
  strncpy(string,va_arg(inputArgs,char*),MAX_STRING_LENGTH);
  /* Add strings (concatenate) together */
  for(i=1;i<nargs;i++){
    strncat(string,va_arg(inputArgs,char*),MAX_STRING_LENGTH);
  }
  return(string);
}

/*
 * NAME: 
 *
 *    set_float_array_cnst_matlab
 *
 * FUNCTION:
 *
 *    C Code to setup a float array with a constant value
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for and sets up the array with a constant
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny constant out_array
 *
 *       Setup array dimensions
 *
 *       Allocate output array
 *
 *       LOOP round array
 *
 *          set array to constant
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    set_float_array_cnst_matlab(nx,ny,const,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *    ny    - y dimension size
 *    const - constant value for array
 *    array - output float_struct array structure
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void set_float_array_cnst_matlab( const VINT nx, const VINT ny, const VFLOAT constant, 
				  struct float_struct *pArray )
{
  VINT i = 0;
  VFLOAT *pArr = NULL;

  /* Set dimensions */
  pArray->nx = nx;
  pArray->ny = ny;
  pArray->size = nx*ny;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (VFLOAT *) calloc(nx*ny,sizeof(VFLOAT))) ){
    message(1,"ERROR: Allocating array in set_flaot_array_matlab");
    /* EXIT */
    exit(-1);
  }

  /* Copy constant into array */
  pArr = pArray->array;
  for(i=0;i<pArray->size;i++){
    *(pArr++) = constant;
  }
}

/*
 * NAME: 
 *
 *    allocate_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_float_array_matlab(nx,ny,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *    ny    - y dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (float_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    free
 *    calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_float_array_matlab(const VINT nx, const VINT ny, 
				 struct float_struct *pArray )
{
  /* Check inputs */
  if( (0 >= nx) || (0 >= ny) ){
    message(1,"ERROR: nx/ny <= 0 in allocate_float_array_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Set dimensions */
  pArray->nx = nx;
  pArray->ny = ny;
  pArray->size = nx*ny;

  /* Allocate array */
  if( NULL == (pArray->array = (VFLOAT *) calloc(nx*ny,sizeof(VFLOAT))) ){
    message(1,"ERROR: Allocating array in allocate_float_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    init_big_geo_grid_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a big_geo_grid structure
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE Grid
 *
 *       Set number of grid to zero
 *
 *       LOOP round hours in day
 *
 *          Set array numbers to zero and array pointer to NULL
 *
 *       END LOOP
 *
 *    END
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes a big_geo_grid structure to make sure things are 
 *    zeroed/NULLED
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_big_geo_grid_matlab(biggeo)
 *
 * INPUTS:  
 *    
 *    biggeo - big_geo_grid structure to be initialized (sst_struct sst)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_big_geo_grid_matlab(struct big_geo_grid *pGrid )
{
  VINT i = 0;

  /* Initalize values to 0 or NULL */
  pGrid->nData = 0;
  for(i=0;i<NHOURS_DAY;i++){
    pGrid->SST[i].array = NULL;
    pGrid->SST[i].nx = 0;
    pGrid->SST[i].ny = 0;
    pGrid->SST[i].size = 0;
    pGrid->Variance[i].array = NULL;
    pGrid->Variance[i].nx = 0;
    pGrid->Variance[i].ny = 0;
    pGrid->Variance[i].size = 0;
  }
}
/*
 * NAME: 
 *
 *    init_elem_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a find_elem structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes a find_elem structure to make sure things are 
 *    zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set nelems to zero
 *
 *       Set array pointer to NULL
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_elem_matlab(elems)
 *
 * INPUTS:  
 *    
 *    elems - find_elem structure to be initialized 
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_elem_matlab(struct find_elems *pElems )
{
  /* Initalize values to 0 or NULL */
  pElems->nelems = 0;
  pElems->elems = NULL;
}

/*
 * NAME: 
 *
 *    init_out_sst_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a out_sst_str structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes a out_sst_str structure to make sure things are 
 *    zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE sst
 *
 *       Set number of elems to zero
 *
 *       Set SST structure number of elems and array pointer to NULL
 *       Set Stdev structure number of elems and array pointer to NULL
 *       Set Gridcnt structure number of elems and array pointer to NULL
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_out_sst_matlab(sst)
 *
 * INPUTS:  
 *    
 *    sst - out_sst_str structure to be initialized (out_sst_str sst)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_out_sst_matlab(struct out_sst_str *pSST )
{
  /* Initalize values to 0 or NULL */
  pSST->nx = 0;
  pSST->ny = 0;
  pSST->SST.array = NULL;
  pSST->SST.nx = 0;
  pSST->SST.ny = 0;
  pSST->SST.size = 0;
  pSST->Stdev.array = NULL;
  pSST->Stdev.nx = 0;
  pSST->Stdev.ny = 0;
  pSST->Stdev.size = 0;
  pSST->Gridcnt.array = NULL;
  pSST->Gridcnt.nx = 0;
  pSST->Gridcnt.ny = 0;
  pSST->Gridcnt.size = 0;

}

/*
 * NAME: 
 *
 *    init_sst_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize an SST structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an SST structure to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE sst
 *
 *       Set SST structure number of elems and array pointer to NULL
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_sst_matlab(sst)
 *
 * INPUTS:  
 *    
 *    array - SST structure to be initialized (sst_struct sst)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_sst_matlab(struct sst_struct *pSST )
{
  /* Initalize values to 0 or NULL */
  pSST->sst.array = NULL;
  pSST->sst.nx = 0;
  pSST->sst.ny = 0;
  pSST->sst.size = 0;
  pSST->est_variance.array = NULL;
  pSST->est_variance.nx = 0;
  pSST->est_variance.ny = 0;
  pSST->est_variance.size = 0;
  pSST->lat.array = NULL;
  pSST->lat.nx = 0;
  pSST->lat.ny = 0;
  pSST->lat.size = 0;
  pSST->lon.array = NULL;
  pSST->lon.nx = 0;
  pSST->lon.ny = 0;
  pSST->lon.size = 0;
}

/*
 * NAME: 
 *
 *    init_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_float_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (float_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_float_array_matlab(struct float_struct *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->nx = 0;
  pArray->ny = 0;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    allocate_float_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a 1d float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for a 1d array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_float_1d_array_matlab(nx,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (float_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    free
 *    calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_float_1d_array_matlab(const VINT nx, struct float_struct1d *pArray )
{ 
  /* Check inputs */
  if( 0 >= nx ){
    message(1,"ERROR: nx <= 0 in allocate_float_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }
 
  /* Set dimensions */
  pArray->size = nx;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (VFLOAT *) calloc(nx,sizeof(VFLOAT))) ){
    message(1,"ERROR: Allocating array in allocate_float_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    init_float_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a 1d float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_float_1d_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (float_struct1d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_float_1d_array_matlab( struct float_struct1d *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    reallocate_float_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to reallocate a 1d float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reallocates memory for an array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx array
 *
 *       Check input size
 *
 *       Set new size
 *
 *       Reallocate array
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    reallocate_float_1d_array_matlab(nx,array)
 *
 * INPUTS:  
 *    
 *    nx    - new x dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be reallocated (float_struct1d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void reallocate_float_1d_array_matlab(const VINT nx, 
				      struct float_struct1d *pArray )
{ 
  /* Check inputs */
  if( 0 >= nx ){
    message(1,"ERROR: nx <= 0 in reallocate_float_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }
  if( NULL == pArray->array ){
    message(1,
    "ERROR: pArray->array == NULL in reallocate_float_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }
 
  /* Set dimensions */
  pArray->size = nx;

  /* Rellocate array */
  if( NULL == (pArray->array = (VFLOAT *) realloc(pArray->array,
						  nx*sizeof(VFLOAT))) ){
    message(1,"ERROR: Allocating array in reallocate_float_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    allocate_float_3d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an array (3-dimensional)
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_float_3d_array_matlab(nx,ny,nz,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *    ny    - y dimension size
 *    nz    - z dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (float_struct3d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_float_3d_array_matlab(const VINT nx, const VINT ny, const VINT nz,
				    struct float_struct3d *pArray )
{
  /* Check inputs */
  if( (0 >= nx) || (0 >= ny) || (0 >= nz) ){
    message(1,"ERROR: nx/ny/nz <= 0 in allocate_float_3d_array_matlab");
    /* EXIT */
    exit(-1);
  }
  /* Set dimensions */
  pArray->nx = nx;
  pArray->ny = ny;
  pArray->nz = ny;
  pArray->size = nx*ny*nz;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (VFLOAT *) calloc(nx*ny*nz,sizeof(VFLOAT))) ){
    message(1,"ERROR: Allocating array in allocate_float_3d_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    init_float_3d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a 3d float array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_float_3d_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (float_struct3d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_float_3d_array_matlab( struct float_struct3d *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->nx = 0;
  pArray->ny = 0;
  pArray->nz = 0;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    allocate_byte_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a byte array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_byte_array_matlab(nx,ny,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *    ny    - y dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (byte_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_byte_array_matlab(const VINT nx, const VINT ny, 
				struct char_struct *pArray )
{
  /* Check inputs */
  if( (0 >= nx) || (0 >= ny) ){
    message(1,"ERROR: nx/ny <= 0 in allocate_byte_array_matlab");
    /* EXIT */
    exit(-1);
  }
  /* Set dimensions */
  pArray->nx = nx;
  pArray->ny = ny;
  pArray->size = nx*ny;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (unsigned char *) 
	       calloc(nx*ny,sizeof(unsigned char))) ){
    message(1,"ERROR: Allocating array in allocate_byte_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    allocate_byte_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a 1d byte array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an 1d array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_byte_1d_array_matlab(nx,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (byte_struct1d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    free
 *    calloc  
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_byte_1d_array_matlab( const VINT nx, struct char_struct1d *pArray )
{
  /* Check inputs */
  if( 0 >= nx ){
    message(1,"ERROR: nx <= 0 in allocate_byte_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }
  /* Set dimensions */
  pArray->size = nx;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (unsigned char *) 
	       calloc(nx,sizeof(unsigned char))) ){
    message(1,"ERROR: Allocating array in allocate_byte_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    reallocate_byte_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to reallocate a 1d byte array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an 1d array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx array
 *
 *       Check input size
 *
 *       Set new size
 *
 *       Reallocate array
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    reallocate_byte_1d_array_matlab(nx,array)
 *
 * INPUTS:  
 *    
 *    nx    - new dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be reallocated (byte_struct1d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void reallocate_byte_1d_array_matlab( const VINT nx, struct char_struct1d *pArray )
{
  /* Check inputs */
  if( 0 >= nx ){
    message(1,"ERROR: nx <= 0 in reallocate_byte_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }
  /* Set dimensions */
  pArray->size = nx;

  /* Rellocate array */
  if( NULL == (pArray->array = (unsigned char *) 
	       realloc(pArray->array,nx*sizeof(unsigned char))) ){
    message(1,"ERROR: Reallocating array in reallocate_byte_1d_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    init_byte_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a byte array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_byte_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (char_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_byte_array_matlab( struct char_struct *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->nx = 0;
  pArray->ny = 0;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    init_byte_1d_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a 1d byte array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_byte_1d_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (char_struct1d array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_byte_1d_array_matlab( struct char_struct1d *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    allocate_int_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to allocate a int array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Allocates memory for an array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny array
 *
 *       Check inputs
 *
 *       IF array allocated THEN
 *          free array
 *       ENDIF
 *
 *       set array dimensions
 *
 *       allocate array 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message 
 *
 * CALLING SEQUENCE:  
 *
 *    allocate_int_array_matlab(nx,ny,array)
 *
 * INPUTS:  
 *    
 *    nx    - x dimension size
 *    ny    - y dimension size
 *
 * OUTPUTS:  
 *
 *    array - array to be allocated (int_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    free
 *    calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void allocate_int_array_matlab( const VINT nx, const VINT ny, 
			        struct int_struct *pArray )
{
  /* Check inputs */
  if( (0 >= nx) || (0 >= ny) ){
    message(1,"ERROR: nx/ny <= 0 in allocate_int_array_matlab");
    /* EXIT */
    exit(-1);
  }

  /* Set dimensions */
  pArray->nx = nx;
  pArray->ny = ny;
  pArray->size = nx*ny;

  /* If already allocated - deallocate */
  if( NULL != pArray->array ){
    free(pArray->array);
    pArray->array = NULL;
  }

  /* Allocate array */
  if( NULL == (pArray->array = (VINT *) calloc(nx*ny,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in allocate_int_array_matlab");
    /* EXIT */
    exit(-1);
  }

 }

/*
 * NAME: 
 *
 *    init_int_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to initialize a int array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes an array to make sure things are zeroed/NULLED
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       Set number of elements to zero and pointer to array to NULL
 *
 *    END
 *    
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_int_array_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - array to be allocated (int_struct array)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void init_int_array_matlab( struct int_struct *pArray )
{
  /* Initalize values to 0 or NULL */
  pArray->array = NULL;
  pArray->nx = 0;
  pArray->ny = 0;
  pArray->size = 0;
}

/*
 * NAME: 
 *
 *    mult_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to multiply an array by a constant
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    multiplies a float_struct array by a constant
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array constant
 *
 *       LOOP round array
 *
 *          multiply elements by constant
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    mult_float_array_matlan(array,constant)
 *
 * INPUTS:  
 *    
 *    array  - input float_struct array
 *    const  - constant value which array is multiplied by
 *
 * OUTPUTS:  
 *
 *    array is modified by the routine
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void mult_float_array_matlab( struct float_struct *pArray, const float constant )
{
  VINT i = 0;
  VFLOAT *pArr = NULL;

  /* Loop round array and multiply by a constant */
  pArr = pArray->array;
  for(i=0;i<pArray->size;i++){
    *(pArr++) *= constant;
  }
}

/*
 * NAME: 
 *
 *    add_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to add a constant to an array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    adds a constant to a float_struct array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array constant
 *
 *       LOOP round array
 *
 *          add a constant to elements
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    add_float_array_matlan(array,constant)
 *
 * INPUTS:  
 *    
 *    array  - input float_struct array
 *    const  - constant value which array is added to array
 *
 * OUTPUTS:  
 *
 *    array is modified by the routine
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void add_float_array_matlab( struct float_struct *pArray, const float constant )
{
  VINT i = 0;
  VFLOAT *pArr = NULL;

  /* Loop round array and add a constant */
  pArr = pArray->array;
  for(i=0;i<pArray->size;i++){
    *(pArr++) += constant;
  }
}

/*
 * NAME: 
 *
 *    subtract_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to subtract a constant from an array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    subtract a constant from a float_struct array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array constant
 *
 *       LOOP round array
 *
 *          subtract a constant from elements
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    subtract_float_array_matlan(array,constant)
 *
 * INPUTS:  
 *    
 *    array  - input float_struct array
 *    const  - constant value which is subtracted from array
 *
 * OUTPUTS:  
 *
 *    array is modified by the routine
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void subtract_float_array_matlab( struct float_struct *pArray, const float constant )
{
  VINT i = 0;
  VFLOAT *pArr = NULL;

  /* Loop round array and subtract a constant */
  pArr = pArray->array;
  for(i=0;i<pArray->size;i++){
    *(pArr++) -= constant;
  }
}

/*
 * NAME: 
 *
 *    set_max_array_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to set elements to maximum value
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    sets each array element to the its value if value < max_value or to 
 *    max_value otherwise.  Mimics the min(A,B) MATLAB function where B is
 *    a constant
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array max_value
 *
 *       LOOP round array
 *
 *          Set array to MIN(array,max_value)
 *
 *       EMD LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    set_max_array_float_matlan(array,max_value)
 *
 * INPUTS:  
 *    
 *    array   - input float_struct array
 *    max_val - constant value which to have as a the maximum value
 *
 * OUTPUTS:  
 *
 *    array is modified by the routine
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void set_max_array_float_matlab( struct float_struct *pArray, const float max_value )
{
  VINT i = 0;

  /* Loop round array and get the minimum */
  for(i=0;i<pArray->size;i++){
    *(pArray->array+i) = MIN(*(pArray->array+i),max_value);
  }
}

/*
 * NAME: 
 *
 *    set_min_array_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to set elements to minimum value
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    sets each array element to the its value if value > min_value or to 
 *    min_value otherwise.  Mimics the min(A,B) MATLAB function where B is
 *    a constant
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array min_value
 *
 *       LOOP round array
 *
 *          Set array to MAX(array,min_value)
 *
 *       EMD LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    set_min_array_float_matlan(array,min_value)
 *
 * INPUTS:  
 *    
 *    array   - input float_struct array
 *    min_val - constant value which to have as a the largest minimum value
 *
 * OUTPUTS:  
 *
 *    array is modified by the routine
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void set_min_array_float_matlab( struct float_struct *pArray, const float min_value )
{
  VINT i = 0;

  /* Loop round array and get the minimum */
  for(i=0;i<pArray->size;i++){
    *(pArray->array+i) = MAX(*(pArray->array+i),min_value);
  }
}

/*
 * NAME: 
 *
 *    size_zero_float
 *
 * FUNCTION:
 *
 *    C Code to set size variables in a float_struct to zero
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Sets size variables to zero in a float_struct structure
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       set array sizes to zero
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    size_zero_float( array )
 *
 * INPUTS:  
 *    
 *    array - float_struct to be zeroed
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

void size_zero_float(struct float_struct *pArray)
{
  /* Set array sizes to zero */
  pArray->size = 0;
  pArray->nx = 0;
  pArray->ny = 0;
}

/*
 * NAME: 
 *
 *    size_zero_float3d
 *
 * FUNCTION:
 *
 *    C Code to set size variables in a float_struct3d to zero
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Sets size variables to zero in a float_struct3d structure
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       set array sizes to zero
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    size_zero_float3d( array )
 *
 * INPUTS:  
 *    
 *    array - float_struct to be zeroed
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

void size_zero_float3d(struct float_struct3d *pArray)
{
  /* Set array sizes to zero */
  pArray->size = 0;
  pArray->nx = 0;
  pArray->ny = 0;
  pArray->nz = 0;
}

/*
 * NAME: 
 *
 *    free_byte_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the byte_struct structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the byte structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_byte_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - byte_struct to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

void free_byte_matlab(struct char_struct *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
    pArray->nx = 0;
    pArray->ny = 0;
  }
}

/*
 * NAME: 
 *
 *    free_byte_1d_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the byte_struct1d structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the byte 1d structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_byte_1d_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - byte_struct1d to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

void free_byte_1d_matlab(struct char_struct1d *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
  }
}

/*
 * NAME: 
 *
 *    free_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the float_struct structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the float structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF allocated array THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_float_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - float_struct to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_float_matlab(struct float_struct *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
    pArray->nx = 0;
    pArray->ny = 0;
  }
}

/*
 * NAME: 
 *
 *    free_float_1d_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the float1d_struct structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the float structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_float_1d_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - float_struct to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_float_1d_matlab(struct float_struct1d *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
  }
}

/*
 * NAME: 
 *
 *    free_float_3d_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the float_struct3d structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the float structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN  
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_float_3d_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - float_struct3d to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_float_3d_matlab(struct float_struct3d *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
    pArray->nx = 0;
    pArray->ny = 0;
    pArray->nz = 0;
  }
}

/*
 * NAME: 
 *
 *    free_int_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the int_struct structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the int structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_int_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - int_struct to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_int_matlab(struct int_struct *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->array);
    pArray->array = NULL;
    pArray->size = 0;
    pArray->nx = 0;
    pArray->ny = 0;
  }

}

/*
 * NAME: 
 *
 *    free_elem_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the find_elems structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the find_elems structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set size to zero
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
 *    free_elem_matlab(array)
 *
 * INPUTS:  
 *    
 *    array - find_elems to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_elem_matlab(struct find_elems *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->elems ){
    /* Deallocate array and set sizes to zero */
    free(pArray->elems);
    pArray->elems = NULL;
    pArray->nelems = 0;
  }
}

/*
 * NAME: 
 *
 *    free_sst_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the sst_struct structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the sst_struct structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF array allocated THEN
 *          free array pointer
 *          set array pointer to NULL
 *          set sizes to zero
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
 *    free_sst_matlab(sst)
 *
 * INPUTS:  
 *    
 *    array - sst_struct to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_sst_matlab(struct sst_struct *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->sst.array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->sst.array);
    pArray->sst.array = NULL;
    pArray->sst.nx = 0;
    pArray->sst.ny = 0;
    pArray->sst.size = 0;
    free(pArray->est_variance.array);
    pArray->est_variance.array = NULL;
    pArray->est_variance.nx = 0;
    pArray->est_variance.ny = 0;
    pArray->est_variance.size = 0;
    if( NULL != pArray->lat.array ){
      free(pArray->lat.array);
      pArray->lat.array = NULL;
    }
    if( NULL != pArray->lon.array ){
      free(pArray->lon.array);
      pArray->lon.array = NULL;
    }
  }
}

/*
 * NAME: 
 *
 *    free_out_sst_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the out_sst_str structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the out_sst_str structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE array
 *
 *       IF SST array allocated THEN
 *          free SST array pointer
 *          set SST array pointer to NULL
 *          set sizes to zero
 *       ENDIF
 *       IF Stdev array allocated THEN
 *          free Stdev array pointer
 *          set Stdev array pointer to NULL
 *          set sizes to zero
 *       ENDIF
 *       IF Gridcnt array allocated THEN
 *          free Gridcnt array pointer
 *          set Gridcnt array pointer to NULL
 *          set sizes to zero
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
 *    free_out_sst_matlab(outdata)
 *
 * INPUTS:  
 *    
 *    outdata - out_sst_str to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_out_sst_matlab(struct out_sst_str *pArray)
{
  /* Check that array is allocated */
  if( NULL != pArray->SST.array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->SST.array);
    pArray->SST.array = NULL;
    pArray->SST.nx = 0;
    pArray->SST.ny = 0;
    pArray->SST.size = 0;
  }
  /* Check that array is allocated */
  if( NULL != pArray->Stdev.array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->Stdev.array);
    pArray->Stdev.array = NULL;
    pArray->Stdev.nx = 0;
    pArray->Stdev.ny = 0;
    pArray->Stdev.size = 0;
  }
  /* Check that array is allocated */
  if( NULL != pArray->Gridcnt.array ){
    /* Deallocate array and set sizes to zero */
    free(pArray->Gridcnt.array);
    pArray->Gridcnt.array = NULL;
    pArray->Gridcnt.nx = 0;
    pArray->Gridcnt.ny = 0;
    pArray->Gridcnt.size = 0;
  }
}

/*
 * NAME: 
 *
 *    free_bigsst_matlab
 *
 * FUNCTION:
 *
 *    C Code to free the big_geo_grid structure
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the big_geo_grid structure memory
 *
 * PSEUDO CODE
 *    
 *    SUBROUTINE grid
 *
 *       LOOP number of grids
 *
 *          IF array allocated THEN
 *             free SST array
 *          ENDIF
 *
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    free_float_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    free_bigsst_matlab(sst)
 *
 * INPUTS:  
 *    
 *    array - big_geo_grid to be free'd
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void free_bigsst_matlab(struct big_geo_grid *pArray)
{
  VINT i = 0;

  /* Loop round allocated data */
  for(i=0;i<pArray->nData;i++){
    free_float_matlab(&pArray->SST[i]);
    free_float_matlab(&pArray->Variance[i]);
  }
  /* Set nData to zero */
  pArray->nData = 0;
}

/*
 * NAME: 
 *
 *    find_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to mimic the MATLAB function find for floating point numbers
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns an array of positions which match a search function 
 *
 *    MATLAB syntax is, for example
 *
 *      ice=find(ice_mask>0)
 * 
 *    which returns positions where ice_mask is > 0
 *
 * PSEUDO CODE
 *
 *    FUNCTION variable operator value find_list
 *
 *       allocate find_list to maximum size
 *
 *       IF operator is greater than THEN
 *          LOOP round variable
 *             IF variable is greater than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than THEN
 *          LOOP round variable
 *             IF variable is less than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is equal to THEN
 *          LOOP round variable
 *             IF variable is equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is greater than or equal to THEN
 *          LOOP round variable
 *             IF variable is greater than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than or equal to THEN
 *          LOOP round variable
 *             IF variable is less than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF find_list has entries THEN
 *          reallocate find_list to correct size
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    find_float_matlab(ice_mask,'gt',0.,ice)
 *
 * INPUTS:  
 *    
 *    variable  - pointer to variable of type float_struct
 *    operator  - string ('gt','lt' etc.) to say what test needs to be done
 *    limit     - value to test agains 
 *
 * OUTPUTS:  
 *
 *    list       - list of postions that agree with logical
 *
 * RETURNS:
 *
 *   Returns the an element array containing the positions in the array 
 *   where the test is satisfied
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *   strcmp
 *   realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT find_float_matlab(const struct float_struct *pVar, const char *pOper, 
		       const VFLOAT limit, struct find_elems *pElems )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* Check find_elems array */
  if( NULL != pElems->elems ){
    /* Free memory */
    free(pElems->elems);
    pElems->elems = NULL;
  }

  /* Allocate output array of elements - set to complete input array size */
  if( NULL == (pElems->elems = (VINT *) calloc(pVar->size,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* initialize array */
  pElems->nelems = 0;

  /* Get pointer */
  pArray = pElems->elems;

  /* if .gt. */
  if( 0 == strcmp(pOper,"gt") ){    
    /* Loop round array and find where array > limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) > limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"lt") ){
    /* if .lt. */
    /* Loop round array and find where array < limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) < limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"eq") ){
    /* if .eq. */
    /* Loop round array and find where array == limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) == limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"ge") ){
    /* if .ge. */
    /* Loop round array and find where array >= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) >= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"le") ){
    /* if .le. */
    /* Loop round array and find where array <= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) <= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else {
    /* Incorrect search test - exit */
    message(1,"ERROR: Oper not gt,lt,ge,le in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If data found */
  if( 0 < pElems->nelems ){
    /* Reallocate array to correct size (original size was size of input array
     * which is the maximum size */
    if( NULL == (pElems->elems = (VINT *) realloc(pElems->elems,
			  pElems->nelems*sizeof(VINT))) ){
      message(1,"ERROR: Reallocating array in find_matlab");
      /* EXIT */
      exit(-1);
    }
    /* EXIT everything OK */
    return(0);
  } else {
    /* Nothing found - free up memory */
    free(pElems->elems);
    pElems->elems = NULL;
    /* EXIT return no entries found status */
    return(1);
  }
}

/*
 * NAME: 
 *
 *    find_byte_1d_matlab
 *
 * FUNCTION:
 *
 *    C Code to mimic the MATLAB function find for floating point numbers
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns an array of positions which match a search function 
 *
 *    MATLAB syntax is, for example
 *
 *      ice=find(ice_mask>0)
 * 
 *    which returns positions where ice_mask is > 0
 *
 * PSEUDO CODE
 *
 *    FUNCTION variable operator value find_list
 *
 *       allocate find_list to maximum size
 *
 *       IF operator is greater than THEN
 *          LOOP round variable
 *             IF variable is greater than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than THEN
 *          LOOP round variable
 *             IF variable is less than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is equal to THEN
 *          LOOP round variable
 *             IF variable is equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is greater than or equal to THEN
 *          LOOP round variable
 *             IF variable is greater than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than or equal to THEN
 *          LOOP round variable
 *             IF variable is less than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF find_list has entries THEN
 *          reallocate find_list to correct size
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    find_byte_1d_matlab(ice_mask,'gt',0.,ice)
 *
 * INPUTS:  
 *    
 *    variable  - pointer to variable of type char_struct1d
 *    operator  - string ('gt','lt' etc.) to say what test needs to be done
 *    limit     - value to test agains 
 *
 * OUTPUTS:  
 *
 *    list       - list of postions that agree with logical
 *
 * RETURNS:
 *
 *   Returns the an element array containing the positions in the array 
 *   where the test is satisfied
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *   strcmp
 *   realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT find_byte_1d_matlab(const struct char_struct1d *pVar, const char *pOper, 
			 const unsigned char limit, struct find_elems *pElems )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* Check find_elems array */
  if( NULL != pElems->elems ){
    /* Free memory */
    free(pElems->elems);
    pElems->elems = NULL;
  }

  /* Allocate output array of elements */
  if( NULL == (pElems->elems = (VINT *) calloc(pVar->size,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* initialize array */
  pElems->nelems = 0;

  /* Get pointer */
  pArray = pElems->elems;

  /* if .gt. */
  if( 0 == strcmp(pOper,"gt") ){    
    /* Loop round array and find where array > limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) > limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"lt") ){
    /* if .lt. */
    /* Loop round array and find where array < limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) < limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"eq") ){
    /* if .eq. */
    /* Loop round array and find where array == limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) == limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"ge") ){
    /* if .ge. */
    /* Loop round array and find where array >= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) >= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"le") ){
    /* if .le. */
    /* Loop round array and find where array <= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) <= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else {
    message(1,"ERROR: Oper not gt,lt,ge,le in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If data found */
  if( 0 < pElems->nelems ){
    /* Reallocate array to correct size (original size was size of input array
     * which is the maximum size */
    if( NULL == (pElems->elems = (VINT *) realloc(pElems->elems,
			  pElems->nelems*sizeof(VINT))) ){
      message(1,"ERROR: Reallocating array in find_matlab");
      /* EXIT */
      exit(-1);
    }
    /* EXIT Everythink OK */
    return(0);
  } else {
    /* Nothing found - free up memory */
    free(pElems->elems);
    pElems->elems = NULL;
    /* EXIT Return nothing found status */
    return(1);
  }
}

/*
 * NAME: 
 *
 *    find_float_1d_matlab
 *
 * FUNCTION:
 *
 *    C Code to mimic the MATLAB function find for floating point numbers
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns an array of positions which match a search function 
 *
 *    MATLAB syntax is, for example
 *
 *      ice=find(ice_mask>0)
 * 
 *    which returns positions where ice_mask is > 0
 *
 * PSEUDO CODE
 *
 *    FUNCTION variable operator value find_list
 *
 *       allocate find_list to maximum size
 *
 *       IF operator is greater than THEN
 *          LOOP round variable
 *             IF variable is greater than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than THEN
 *          LOOP round variable
 *             IF variable is less than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is equal to THEN
 *          LOOP round variable
 *             IF variable is equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is greater than or equal to THEN
 *          LOOP round variable
 *             IF variable is greater than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than or equal to THEN
 *          LOOP round variable
 *             IF variable is less than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF find_list has entries THEN
 *          reallocate find_list to correct size
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    find_float_1d_matlab(ice_mask,'gt',0.,ice)
 *
 * INPUTS:  
 *    
 *    variable  - pointer to variable of type float_struct1d
 *    operator  - string ('gt','lt' etc.) to say what test needs to be done
 *    limit     - value to test agains 
 *
 * OUTPUTS:  
 *
 *    list       - list of postions that agree with logical
 *
 * RETURNS:
 *
 *   Returns the an element array containing the positions in the array 
 *   where the test is satisfied
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *   strcmp
 *   realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT find_float_1d_matlab(const struct float_struct1d *pVar, const char *pOper, 
			  const VFLOAT limit, struct find_elems *pElems )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* Check find_elems array */
  if( NULL != pElems->elems ){
    /* Free memory */
    free(pElems->elems);
    pElems->elems = NULL;
  }

  /* Allocate output array of elements */
  if( NULL == (pElems->elems = (VINT *) calloc(pVar->size,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* initialize array */
  pElems->nelems = 0;

  /* Get pointer */
  pArray = pElems->elems;

  /* if .gt. */
  if( 0 == strcmp(pOper,"gt") ){    
    /* Loop round array and find where array > limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) > limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"lt") ){
    /* if .gt. */
    /* Loop round array and find where array < limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) < limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"eq") ){
    /* if .gt. */
    /* Loop round array and find where array == limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) == limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"ge") ){
    /* if .gt. */
    /* Loop round array and find where array >= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) >= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"le") ){
    /* if .gt. */
    /* Loop round array and find where array <= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) <= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else {
    message(1,"ERROR: Oper not gt,lt,ge,le in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If data found */
  if( 0 < pElems->nelems ){
    /* Reallocate array to correct size (original size was size of input array
     * which is the maximum size */
    if( NULL == (pElems->elems = (VINT *) realloc(pElems->elems,
			  pElems->nelems*sizeof(VINT))) ){
      message(1,"ERROR: Reallocating array in find_matlab");
      /* EXIT */
      exit(-1);
    }
    /* EXIT Return everything OK */
    return(0);
  } else {
    /* Nothing found - free up memory */
    free(pElems->elems);
    pElems->elems = NULL;
    /* EXIT Return nothing found status */
    return(1);
  }
}

/*
 * NAME: 
 *
 *    find_float_array_matlab
 *
 * FUNCTION:
 *
 *    C Code to mimic the MATLAB function find for floating point numbers
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns an array of positions which match a search function 
 *
 *    MATLAB syntax is, for example
 *
 *      ice=find(ice_mask>0)
 * 
 *    which returns positions where ice_mask is > 0
 *
 * PSEUDO CODE
 *
 *    FUNCTION variable operator value find_list
 *
 *       allocate find_list to maximum size
 *
 *       IF operator is greater than THEN
 *          LOOP round variable
 *             IF variable is greater than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than THEN
 *          LOOP round variable
 *             IF variable is less than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is equal to THEN
 *          LOOP round variable
 *             IF variable is equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is greater than or equal to THEN
 *          LOOP round variable
 *             IF variable is greater than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than or equal to THEN
 *          LOOP round variable
 *             IF variable is less than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF find_list has entries THEN
 *          reallocate find_list to correct size
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    find_float_matlab(ice_mask,'gt',threshold,ice)
 *
 * INPUTS:  
 *    
 *    variable  - pointer to variable of type float_struct
 *    operator  - string ('gt','lt' etc.) to say what test needs to be done
 *    threshold - array of thresholds 
 *
 * OUTPUTS:  
 *
 *    list       - list of postions that agree with logical
 *
 * RETURNS:
 *
 *   Returns the an element array containing the positions in the array 
 *   where the test is satisfied
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *   strcmp
 *   realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT find_float_array_matlab(const struct float_struct *pVar, const char *pOper, 
			     const struct float_struct *pLimit, 
			     struct find_elems *pElems )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* Check find_elems array */
  if( NULL != pElems->elems ){
    /* Free memory */
    free(pElems->elems);
    pElems->elems = NULL;
  }

  /* Check input structures */
  if( pVar->size != pLimit->size ){
    message(1,"ERROR: Mismatch in pVar/pLimit in find_float_array_matlab");
    /* EXIT */
    exit(-1);
  }

  /* Allocate output array of elements */
  if( NULL == (pElems->elems = (VINT *) calloc(pVar->size,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* initialize array */
  pElems->nelems = 0;

  /* Get pointer */
  pArray = pElems->elems;

  /* if .gt. */
  if( 0 == strcmp(pOper,"gt") ){    
    /* Loop round array and find where array > limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) > *(pLimit->array+i) ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"lt") ){
    /* if .lt. */
    /* Loop round array and find where array < limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) < *(pLimit->array+i) ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"eq") ){
    /* if .eq. */
    /* Loop round array and find where array == limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) == *(pLimit->array+i) ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"ge") ){
    /* if .ge. */
    /* Loop round array and find where array >= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) >= *(pLimit->array+i) ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"le") ){
    /* if .le. */
    /* Loop round array and find where array <= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) <= *(pLimit->array+i) ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else {
    message(1,"ERROR: Oper not gt,lt,ge,le in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If data found */
  if( 0 < pElems->nelems ){
    /* Reallocate array to correct size (original size was size of input array
     * which is the maximum size */
    if( NULL == (pElems->elems = (VINT *) realloc(pElems->elems,
				   pElems->nelems*sizeof(VINT))) ){
      message(1,"ERROR: Reallocating array in find_matlab");
      /* EXIT */
      exit(-1);
    }
    /* EXIT Everything OK */
    return(0);
  } else {
    /* Nothing found - free up memory */
    free(pElems->elems);
    pElems->elems = NULL;
    /* EXIT Nothing found status */
    return(1);
  }
}

/*
 * NAME: 
 *
 *    find_byte_matlab
 *
 * FUNCTION:
 *
 *    C Code to mimic the MATLAB function find for byte numbers
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Returns an array of positions which match a search function 
 *
 *    MATLAB syntax is, for example
 *
 *      ice=find(ice_mask>0)
 * 
 *    which returns positions where ice_mask is > 0
 *
 * PSEUDO CODE
 *
 *    FUNCTION variable operator value find_list
 *
 *       allocate find_list to maximum size
 *
 *       IF operator is greater than THEN
 *          LOOP round variable
 *             IF variable is greater than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than THEN
 *          LOOP round variable
 *             IF variable is less than value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is equal to THEN
 *          LOOP round variable
 *             IF variable is equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is greater than or equal to THEN
 *          LOOP round variable
 *             IF variable is greater than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is less than or equal to THEN
 *          LOOP round variable
 *             IF variable is less than or equal to value THEN
 *                Add to find_list
 *             ENDIF
 *          END LOOP
 *       ENDIF
 *
 *       IF find_list has entries THEN
 *          reallocate find_list to correct size
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ice = find_byte_matlab(mask,'gt',0)
 *
 * INPUTS:  
 *    
 *    variable  - pointer to variable of type byte_struct
 *    operator  - string ('gt','lt' etc.) to say what test needs to be done
 *    limit     - value to test agains 
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *   Returns the an element array containing the positions in the array 
 *   where the test is satisfied in form of struct find_elems
 *
 * SYSTEM CALLS:
 *
 *   free
 *   calloc
 *   strcmp
 *   realloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
VINT find_byte_matlab(const struct char_struct *pVar, const char *pOper, 
		      const char limit, struct find_elems *pElems )
{
  /* Local variables */
  VINT i = 0;
  VINT *pArray = NULL;

  /* Check find_elems array */
  if( NULL != pElems->elems ){
    /* Free memory */
    free(pElems->elems);
    pElems->elems = NULL;
  }

  /* Allocate output array of elements */
  if( NULL == (pElems->elems = (VINT *) calloc(pVar->size,sizeof(VINT))) ){
    message(1,"ERROR: Allocating array in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* initialize array */
  pElems->nelems = 0;

  /* Get pointer */
  pArray = pElems->elems;

  /* if .gt. */
  if( 0 == strcmp(pOper,"gt") ){    
    /* Loop round array and find where array > limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) > limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"lt") ){
    /* if .lt. */
    /* Loop round array and find where array < limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) < limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"eq") ){
    /* if .eq. */
    /* Loop round array and find where array == limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) == limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"ge") ){
    /* if .ge. */
    /* Loop round array and find where array >= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) >= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else if( 0 == strcmp(pOper,"le") ){
    /* if .le. */
    /* Loop round array and find where array <= limit */
    for(i=0;i<pVar->size;i++){
      if(*(pVar->array+i) <= limit ){
	*(pArray++) = i;
	pElems->nelems++;
      }	
    }
  } else {
    message(1,"ERROR: Oper not gt,lt,ge,le in find_matlab");
    /* EXIT */
    exit(-1);
  }

  /* If data found */
  if( 0 < pElems->nelems ){
    /* Reallocate array to correct size (original size was size of input array
     * which is the maximum size */
    if( NULL == (pElems->elems = (VINT *) realloc(pElems->elems,
				  pElems->nelems*sizeof(VINT))) ){
      message(1,"ERROR: Reallocating array in find_matlab");
      /* EXIT */
      exit(-1);
    }
    /* EXIT Everything OK */
    return(0);
  } else {
    /* Nothing found - free up memory */
    free(pElems->elems);
    pElems->elems = NULL;
    /* EXIT Nothing found status */
    return(1);
  }

}

/*
 * NAME: 
 *
 *    copy_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to copy a floating point structure to another - like MATLAB = on 
 *    arrays
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies from one structure to another with memory allocation
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE in_array out_array
 *
 *       IF out_array already allocated THEN
 *          reallocate out_array 
 *       ELSE
 *          allocate out_array
 *       ENDIF
 *
 *       set out_array sizes
 *
 *       LOOP round arrays
 *          copy data from in_array to out_array
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    copy_float_matlab(array1,array2)
 *
 * INPUTS:  
 *    
 *    array1 - input array structure
 *
 * OUTPUTS:  
 *
 *    array2 - array with copied data
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    realloc
 *    calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void copy_float_matlab( const struct float_struct *pVar_in, 
			struct float_struct *pVar_out )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar1 = NULL;
  VFLOAT *pVar2 = NULL;

  /* If pVar_out is already allocated, reallocate */
  if( NULL != pVar_out->array ){
    /* Reallocate memory */
    if( NULL == (pVar_out->array = (VFLOAT *) 
		 realloc(pVar_out->array,pVar_in->size*sizeof(VFLOAT))) ){
      message(1,"ERROR: reallocating array in copy_matlab");
      /* EXIT */
      exit(-1);
    }
  } else {
    /* Allocate memory */
    if( NULL == (pVar_out->array = (VFLOAT *) 
		 calloc(pVar_in->size,sizeof(VFLOAT))) ){
      message(1,"ERROR: allocating array in copy_matlab");
      /* EXIT */
      exit(-1);
    }
  }

  /* Set output sizes to input sizes */
  pVar_out->size = pVar_in->size;
  pVar_out->nx = pVar_in->nx;
  pVar_out->ny = pVar_in->ny;
  
  /* Get pointers */
  pVar1 = pVar_in->array;
  pVar2 = pVar_out->array;

  /* Loop round array and copy */
  for(i=0;i<pVar_out->size;i++){
    *(pVar2++) = *(pVar1++);
  }
}

/*
 * NAME: 
 *
 *    copy_float_matlab_noalloc
 *
 * FUNCTION:
 *
 *    C Code to copy a floating point structure to another - like MATLAB = on 
 *    arrays - no allocation of output array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies from one structure to another without memory allocation
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE in_array out_array
 *
 *       IF out_array not allocated THEN
 *          error condition
 *       ENDIF
 *
 *       IF in_array size not thes same as out_array size THEN
 *          error condition
 *       ENDIF
 *
 *       set out_array sizes
 *
 *       LOOP round arrays
 *          copy data from in_array to out_array
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    copy_float_matlab_noalloc(array1,array2)
 *
 * INPUTS:  
 *    
 *    array1 - input array structure
 *
 * OUTPUTS:  
 *
 *    array2 - array with copied data
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void copy_float_matlab_noalloc( const struct float_struct *pVar_in, 
				struct float_struct *pVar_out )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar1 = NULL;
  VFLOAT *pVar2 = NULL;

  /* Make sure that output array is allocated */
  if( NULL == pVar_out->array ){
    message(1,
    "ERROR: Output array not allocated in copy_float_matlab_noalloc\n");
    exit(-1);
  }

  /* check size */
  if( pVar_in->size != pVar_out->size ){
    message(1,"ERROR: Input size discrepancy in copy_float_matlab_noalloc\n");
    exit(-1);
  }
  
  /* In cases where the shape has changed (transpose/flip) copy ny/ny values */
  pVar_out->nx = pVar_in->nx;
  pVar_out->ny = pVar_in->ny;

  /* Get pointers */
  pVar1 = pVar_in->array;
  pVar2 = pVar_out->array;

  /* Loop round and copy */
  for(i=0;i<pVar_out->size;i++){
    *(pVar2++) = *(pVar1++);
  }

}

/*
 * NAME: 
 *
 *    copy_int_matlab_noalloc
 *
 * FUNCTION:
 *
 *    C Code to copy a int point structure to another - like MATLAB = on 
 *    arrays - no allocation of output array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies from one structure to another without memory allocation
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE in_array out_array
 *
 *       IF out_array not allocated THEN
 *          error condition
 *       ENDIF
 *
 *       IF in_array size not thes same as out_array size THEN
 *          error condition
 *       ENDIF
 *
 *       set out_array sizes
 *
 *       LOOP round arrays
 *          copy data from in_array to out_array
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    copy_int_matlab_noalloc(array1,array2)
 *
 * INPUTS:  
 *    
 *    array1 - input array structure
 *
 * OUTPUTS:  
 *
 *    array2 - array with copied data
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void copy_int_matlab_noalloc( const struct int_struct *pVar_in, 
			      struct int_struct *pVar_out )
{ 
  /* Local variables */
  VINT i = 0;
  VINT *pVar1 = NULL;
  VINT *pVar2 = NULL;

  /* Make sure that output array is allocated */
  if( NULL == pVar_out->array ){
    message(1,
    "ERROR: Output array not allocated in copy_int_matlab_noalloc\n");
    exit(-1);
  }

  /* check size */
  if( pVar_in->size != pVar_out->size ){
    message(1,"ERROR: Input size discrepancy in copy_int_matlab_noalloc\n");
    exit(-1);
  }
  
  /* In cases where the shape has changed (transpose/flip) copy ny/ny values */
  pVar_out->nx = pVar_in->nx;
  pVar_out->ny = pVar_in->ny;

  /* Get pointers */
  pVar1 = pVar_in->array;
  pVar2 = pVar_out->array;

  /* Loop round and copy */
  for(i=0;i<pVar_out->size;i++){
    *(pVar2++) = *(pVar1++);
  }

}

/*
 * NAME: 
 *
 *    copy_float_1d_matlab
 *
 * FUNCTION:
 *
 *    C Code to copy a 1d floating point structure to another - like MATLAB = 
 *    on vector
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies from one structure to another with memory allocation
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE in_array out_array
 *
 *       IF out_array already allocated THEN
 *          reallocate out_array 
 *       ELSE
 *          allocate out_array
 *       ENDIF
 *
 *       set out_array sizes
 *
 *       LOOP round arrays
 *          copy data from in_array to out_array
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    copy_float_1d_matlab(array1,array2)
 *
 * INPUTS:  
 *    
 *    array1 - input array structure
 *
 * OUTPUTS:  
 *
 *    array2 - array with copied data
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void copy_float_1d_matlab( const struct float_struct1d *pVar_in, 
			   struct float_struct1d *pVar_out )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar1 = NULL;
  VFLOAT *pVar2 = NULL;

  /* Allocate memory */
  if( NULL == (pVar_out->array = (VFLOAT *) 
	       calloc(pVar_in->size,sizeof(VFLOAT))) ){
    message(1,"ERROR: allocating array in copy_1d_matlab");
    /* EXIT */
    exit(-1);
  }

  /* Set output size to input size */
  pVar_out->size = pVar_in->size;
  
  /* Get pointers */
  pVar1 = pVar_in->array;
  pVar2 = pVar_out->array;

  /* Loop round array and copy */
  for(i=0;i<pVar_out->size;i++){
    *(pVar2++) = *(pVar1++);
  }
}

/*
 * NAME: 
 *
 *    copy_float_1d_matlab_noalloc
 *
 * FUNCTION:
 *
 *    C Code to copy a 1d floating point structure to another - like MATLAB = 
 *    on vector.  This version does not allocate memory for output - it is 
 *    assumed that it is correct from calling routine
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies from one structure to another with memory allocation
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE in_array out_array
 *
 *       IF NOT out_array already allocated THEN
 *          ERROR message
 *       ENDIF
 *
 *       set out_array sizes
 *
 *       LOOP round arrays
 *          copy data from in_array to out_array
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    copy_float_1d_matlab_noalloc(array1,array2)
 *
 * INPUTS:  
 *    
 *    array1 - input array structure
 *
 * OUTPUTS:  
 *
 *    array2 - array with copied data
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void copy_float_1d_matlab_noalloc( const struct float_struct1d *pVar_in, 
				   struct float_struct1d *pVar_out )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar1 = NULL;
  VFLOAT *pVar2 = NULL;

  /* Allocate memory */
  if( NULL == pVar_out->array ){
    message(1,"ERROR: array not allocated in copy_1d_matlab_noalloc");
    /* EXIT */
    exit(-1);
  }

  /* Set output size to input size */
  pVar_out->size = pVar_in->size;
  
  /* Get pointers */
  pVar1 = pVar_in->array;
  pVar2 = pVar_out->array;

  /* Loop round array and copy */
  for(i=0;i<pVar_out->size;i++){
    *(pVar2++) = *(pVar1++);
  }
}

/*
 * NAME: 
 *
 *    array_float_oper_matlab
 *
 * FUNCTION:
 *
 *    C Code to operate on one variable against another
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Does an operation on two variables (float_struct type) and returns the 
 *    result
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array1 operator array2 out_array
 *
 *       check input array sizes 
 *
 *       allocate output array
 *
 *       IF operator is minus THEN
 *          LOOP round arrays
 *             subtract array2 from array1
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is plus THEN
 *          LOOP round arrays
 *             add array2 to array1
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is multiply THEN
 *          LOOP round arrays
 *             multiply array2 by array1
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is divide THEN
 *          LOOP round arrays
 *             divide array1 by array2
 *          END LOOP
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    allocate_float_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    array_float_oper_matlab(&array1,"+",&array2,&outarray)
 *
 * INPUTS:  
 *    
 *    array1 - pointer to first input array structure
 *      oper - string with operator - + - / *
 *    array2 - pointer to second input array structure
 *
 * OUTPUTS:  
 *
 *    outvar - float_struct array with result of operation - is allocated
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    strcmp
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void array_float_oper_matlab( const struct float_struct *pVar1, 
			      const char *pOper,
			      const struct float_struct *pVar2,
			      struct float_struct *pOutVar )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar_temp1 = NULL;
  VFLOAT *pVar_temp2 = NULL;
  VFLOAT *pVar_temp3 = NULL;

  /* Check sizes of arrays - make sure they are the same */
  if( pVar1->size != pVar2->size ){
    message(1,"ERROR: Var2 and Var2 different sizes in array_oper_matlab");
    /* EXIT */
    exit(-1);
  }

  /* Allocate memory for output */
  allocate_float_array_matlab( pVar1->nx, pVar1->ny, pOutVar );
  
  /* Do operations */
  /* Get pointers */
  pVar_temp1 = pVar1->array;
  pVar_temp2 = pVar2->array;
  pVar_temp3 = pOutVar->array;

  /* If subtract */
  if( 0 == strcmp(pOper,"-") ){
    /* Loop round arrays and subtract */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) - *(pVar_temp2++));      
    }
  } else if( 0 == strcmp(pOper,"+") ){
    /* If add */
    /* Loop round arrays and add */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) + *(pVar_temp2++));      
    }
  } else if( 0 == strcmp(pOper,"*") ){
    /* If multiply */
    /* Loop round arrays and multiply */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) * *(pVar_temp2++));      
    }
  } else if( 0 == strcmp(pOper,"/") ){
    /* If divide */
    /* Loop round arrays and divide */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) / *(pVar_temp2++));      
    }
  } else {
    message(1,"ERROR: Oper not +,-,*,/ in array_float_oper_matlab");
    /* EXIT */
    exit(-1);
  }

}

/*
 * NAME: 
 *
 *    array_float_oper_cnst_matlab
 *
 * FUNCTION:
 *
 *    C Code to operate on one variable with a constant another
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Does an operation on variables (float_struct type) and constant
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array operator constant out_array
 *
 *       allocate output array
 *
 *       IF operator is minus THEN
 *          LOOP round arrays
 *             subtract constant from array
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is plus THEN
 *          LOOP round arrays
 *             add constant to array
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is multiply THEN
 *          LOOP round arrays
 *             multiply array by constant
 *          END LOOP
 *       ENDIF
 *
 *       IF operator is divide THEN
 *          LOOP round arrays
 *             divide array by constant
 *          END LOOP
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    allocate_float_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    array_float_oper_cnst_matlab(&array1,"+",constant,&outarray)
 *
 * INPUTS:  
 *    
 *    array1 - pointer to first input array structure
 *      oper - string with operator - + - / *
 *  constant - constant value
 *
 * OUTPUTS:  
 *
 *    outvar - float_struct array with result of operation - is allocated
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    strcmp
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void array_float_oper_cnst_matlab( const struct float_struct *pVar1, 
				   const char *pOper,
				   const VFLOAT constantval,
				   struct float_struct *pOutVar )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pVar_temp1 = NULL;
  VFLOAT *pVar_temp3 = NULL;

  /* Allocate memory for  output */
  allocate_float_array_matlab( pVar1->nx, pVar1->ny, pOutVar );
  
  /* Do operations */
  /* Get pointers */
  pVar_temp1 = pVar1->array;
  pVar_temp3 = pOutVar->array;

  /* If subtract */
  if( 0 == strcmp(pOper,"-") ){
    /* Loop round arrays and subtract */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) - constantval);      
    }
  } else if( 0 == strcmp(pOper,"+") ){
    /* If add */
    /* Loop round arrays and add */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) + constantval);      
    }
  } else if( 0 == strcmp(pOper,"*") ){
    /* If multiply */
    /* Loop round arrays and multiply */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) * constantval);      
    }
  } else if( 0 == strcmp(pOper,"/") ){
    /* If divide */
    /* Loop round arrays and divide */
    for(i=0;i<pOutVar->size;i++){
      *(pVar_temp3++) = (*(pVar_temp1++) / constantval);      
    }
  } else {
    message(1,"ERROR: Oper not +,-,*,/ in array_float_oper_cnst_matlab");
    /* EXIT */
    exit(-1);
  }

}

/*
 * NAME: 
 *
 *    array_float_abs_matlab
 *
 * FUNCTION:
 *
 *    C Code to get the absolute value for elements of an array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Uses ABS on elements of an array
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array
 *
 *       LOOP round elements of array
 *           array = ABS(array)
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    array_float_abs_matlab(&array1)
 *
 * INPUTS:  
 *    
 *    array - pointer to input array structure which will be ABS's
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   fabs
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */
void array_float_abs_matlab( struct float_struct *pVar )
{ 
  /* Local variables */
  VINT i = 0;
  VFLOAT *pArr = NULL;

  /* Get pointer */
  pArr = pVar->array;

  /* Loop round and get absolute values */
  for(i=0;i<pVar->size;i++){
    *(pArr+i) = fabs(*(pArr+i));
  }

}

/*
 * NAME: 
 *
 *    find_files
 *
 * FUNCTION:
 *
 *    C Code to get list of filenames that satisfy a search string
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets files for GOES/MTSAT/MSG data for a complete day
 *
 * PSEUDO CODE
 *
 *    FUNCTION search_string nFiles filelist
 *
 *       find files using search string (glob)
 *
 *       IF no files found THEN
 *          RETURN error
 *       ENDIF
 *
 *       allocate memory for filelist 
 *
 *       LOOP round blob files
 *          copy filenames to output filelist
 *       END LOOP
 *
 *       free glob structure
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = find_files( searchString, &nFiles, pFilelist )
 *
 * INPUTS:  
 *    
 *   searchString    - search string for searching for files
 *
 * OUTPUTS:  
 * 
 *   nFiles          - number of files found
 *   pFilelist       - list of files
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    glob
 *    globfree
 *    calloc
 *    strncpy
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
VINT find_files( const char *searchString, VINT *pNFiles, char ***pppFilelist )
{
  /* Local variables */
  VINT i = 0;
  glob_t glob_list;

  /* Use glob to find files */
  if( 0 != glob(searchString,GLOB_ERR,NULL,&glob_list) ){
    /* EXIT If an error return */
    return(0);
  }

  /* Number of matches */
  if( 0 == glob_list.gl_pathc ){
    /* EXIT a further check */
    return(0);
  }
  *pNFiles = glob_list.gl_pathc;

  /* Allocate memory for filelist */
  if( NULL == (*pppFilelist = (char **) calloc(*pNFiles,sizeof(char *))) ){
    message(1,"ERROR: Cannot allocate memory (1) for filelist in find_files");
    /* EXIT */
    exit(-1);
  }

  /* Loop round files */
  for(i=0;i<*pNFiles;i++){
    /* Allocate memory for filename storage */
    if( NULL == (*(*pppFilelist+i) = 
		 (char *) calloc(MAX_STRING_LENGTH,sizeof(char))) ){
      message(1,
	      "ERROR: Cannot allocate memory (2) for filelist in find_files");
      /* EXIT */
      exit(-1);
    }
    /* Copy filenames from glob list */
    strncpy(*(*pppFilelist+i),*(glob_list.gl_pathv+i),MAX_STRING_LENGTH);
    /* Make sure we have a \0 at end */
    *(*(*pppFilelist+i)+MAX_STRING_LENGTH-1) = '\0';
  }

  /* Free glob */
  globfree(&glob_list);

  return(1);
}

/*
 * NAME: 
 *
 *    free_filelist
 *
 * FUNCTION:
 *
 *    C Code to free filelist memory
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees memory of filelist from file search
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nfiles filelist
 *
 *       LOOP round files
 *          free string memory
 *       END LOOP
 *       free top levels filelist memory
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    free_filelist( &nFiles, pFilelist )
 *
 * INPUTS:  
 *    
 *   nFiles          - number of files in list
 *   pFilelist       - filelist to be free'd
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void free_filelist( VINT *pNFiles, char ***pppFilelist )
{
  VINT i = 0;
  
  /* Loop round all files and free string memory */
  for(i=0;i<*pNFiles;i++){
    free(*(*pppFilelist+i));
  }
  /* Free top level memory */
  free(*pppFilelist);

  /* Set pointer/nos to initial values */
  *pppFilelist = NULL;
  *pNFiles = 0;
}

/*
 * NAME: 
 *
 *    squeeze_2d_float_matlab
 *
 * FUNCTION:
 *
 *    C Code to return 1d array from 2d array
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Strips out an elem/line from a 2d array into a 1d array
 *    Control of which line/element to extra is by setting the 
 *    appropriate elem/line value to -1
 *    If elem = -1 then extract line at line position
 *    If line = -1 then extract column at element position
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE array elem line 1darray
 *
 *       IF elem > 0 THEN
 *          LOOP round lines
 *            copy given element from each line in loop
 *          END LOOP
 *       ELSE IF line > 0 THEN
 *          LOOP round elements
 *            copy given line position from each element in loop
 *          END LOOP
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_1d_array_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    squeeze_2d_float_matlab( 2darray, elem, line, 1darray )
 *
 * INPUTS:  
 *    
 *   2darray         - input float_struxt 2d array
 *   elem            - line to be extract or -1 if column to be extracted
 *   line            - column to be extract or -1 if line to be extracted
 *
 * OUTPUTS:  
 *
 *   1darray         - output float_struct1d array
 * 
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 12/12/2011
 *
 */
void squeeze_2d_float_matlab( const struct float_struct *pInArray, const VINT elem,
			      const VINT line, struct float_struct1d *pOutArray )
{
  /* Local variables */
  VINT i = 0;
  VINT nx = 0;

  nx = pInArray->nx;
  /* Check to see if we need to extract an element */
  if( (elem >= 0) && (elem < nx) ){
    /* Extract lines at an element elem (size ny) */   
    /* Allocate output array */
    allocate_float_1d_array_matlab(pInArray->ny,pOutArray);
    /* Loop over line and copy at element position */
    for(i=0;i<pInArray->ny;i++){
      *(pOutArray->array+i) = *(pInArray->array+elem+i*nx);
    }
  /* Check to see if we need to extract a line */
  } else if( (line >= 0) && (line < pInArray->ny) ){
    /* Extract elements at a line (size nx) */   
    /* Allocate output array */
    allocate_float_1d_array_matlab(pInArray->ny,pOutArray);
    /* Loop over element and copy at line position */
    for(i=0;i<pInArray->nx;i++){
      *(pOutArray->array+i) = *(pInArray->array+i+line*nx);
    }
  } else {
    message(1,"ERROR: elem/line not in range in squeeze_2d_float_matlab");
    /* EXIT */
    exit(-1);
  }
}
/* NAME: 
 *
 *    rebin_data_matlab
 *
 * FUNCTION:
 *
 *    C Code to rebin an input dataset to another grid
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Rebins the dataset to the resolution of the reference input image
 *    Currently only uses nearest neighor binning and only works if both
 *    images are on equal angle grids 
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE reference sst rebin
 *
 *       get delta longitudes and latitudes for reference and sst image
 *       
 *       allocate output (rebin) image
 *       allocate ndata array
 *
 *       IF input(sst) data is on a finer scale that output(rebin) image THEN
 *          setup loop arrays/deltas - loop round sst image
 *       ELSE IF input(sst) data is on a coarser scale that output THEN
 *          setup loop arrays/deltas - loop round reference image
 *       ENDIF
 *
 *       LOOP in Y direction
 *          get YPOS position in relevant image
 *          LOOP in X direction 
 *             get XPOS position in relevant image
 *
 *             Check for good data 
 *             copy from first image to second array based on X->XPOS,Y->YPOS
 *
 *          END LOOP
 *       END LOOP
 *
 *       Check that some data has been rebinned
 *
 *       LOOP round rebin image
 *          IF Ndata > 1 get mean value
 *          IF Ndata <= 0 set to bad value
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    rebin_data_matlab( pRef_SST, pSST, pRebinSST  )
 *
 * INPUTS:  
 *    
 *   pRef_SST        - input reference SST
 *   pSST            - input SST image
 *
 * OUTPUTS:  
 * 
 *   pRebinSST       - rebinned SST dataset
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *   isnan
 *   free 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void rebin_data_matlab(struct sst_struct *pRefSST, 
		       struct sst_struct *pSST,
		       struct float_struct *pRebinSST)
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT pos2 = 0;
  VFLOAT lon = 0.;
  VFLOAT lat = 0.;
  VFLOAT loop_deltalon = 0.;
  VFLOAT loop_deltalat = 0.;
  VFLOAT match_deltalon = 0.;
  VFLOAT match_deltalat = 0.;

  VINT *pNdata = NULL;
  VINT max_ndata = 0;
  VINT large_reference = 0;
  VINT use_stored_latlon = 0;
  VINT sst_lat_size = 0;
  VINT ref_lat_size = 0;
  VINT sst_lon_size = 0;
  VINT ref_lon_size = 0;
  struct sst_struct *pLoopSST = NULL;
  struct sst_struct *pMatchSST = NULL;

  /* Note that longitude is now stored as y - MATLAB effect */
  /* Allocate output to same size as refernce image */
  allocate_float_array_matlab( pRefSST->sst.nx, pRefSST->sst.ny, pRebinSST );

  /* Allocate ndata array */
  if( NULL == (pNdata = (VINT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pNdata in rebin_data");
    /* EXIT */
    exit(-1);
  }

  /* Comes in 2 parts.  If scale of input image > reference then loop round
   * reference and do nearest neighbor from input image.  If scale of input
   * image <= reference then loop round input image and do nearest neighbor
   * from reference.  This enables the rebinning of both an SST image onto
   * the reference grid (e.g. input SST onto the analysis grid) or mapping
   * the reference to the SST grid */

  /* Setup sizes to a precision of 0.001 */
  /* Avoids slight rounding precision problem */
  sst_lat_size = (VINT)(fabs(pSST->deltalat)*1000.);
  ref_lat_size = (VINT)(fabs(pRefSST->deltalat)*1000.);
  sst_lon_size = (VINT)(fabs(pSST->deltalon)*1000.);
  ref_lon_size = (VINT)(fabs(pRefSST->deltalon)*1000.);

  /* Use pointers to reset which array is which */
  if( sst_lon_size <= ref_lon_size && sst_lat_size <= ref_lat_size ){
    /* This is the case when the input data is on a finer scale than
     * the reference, so we loop round the input data to match to 
     * reference */
    pLoopSST = pSST;
    pMatchSST = pRefSST;
    large_reference = 1;
  } else {
    /* This is the case when the input data is on a larger scale than
     * the reference, so we loop round the reference to match to 
     * the input data using nearest neighbor */
    pLoopSST = pRefSST;
    pMatchSST = pSST;
    large_reference = 0;
  }

  /* Get relevant delta longitudes/latitudes for arrays */
  loop_deltalon = pLoopSST->deltalon;
  loop_deltalat = pLoopSST->deltalat;
  match_deltalon = pMatchSST->deltalon;
  match_deltalat = pMatchSST->deltalat;

  /* Note that for large_reference = 1 we can use the real Lat/Lon rather than using min/delta lat/lon
   * For large_reference = 0 it would be too time consuming to use true value, so use min/delta lat/lon
   * For the GEO data the difference is small anyway...
   */
  if( NULL != pLoopSST->lon.array && NULL != pLoopSST->lat.array ){
    use_stored_latlon = 1;
  } else {
    use_stored_latlon = 0;
  }

  /* Loop round and use nearest neighbour to get rebinned data */

  max_ndata = 0;
  for(i=0;i<pLoopSST->sst.ny;i++){ 
    if( 0 == large_reference || 0 == use_stored_latlon ){
      /* Either no stored lat/lon OR */
      /* Expanding out, so just us min/delta values for lat/lon */
      /* Get longitude */
      lon = pLoopSST->minlon + i*loop_deltalon;
      ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
    }
    /* Check that we are not out of bound */
    if( (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
      for(j=0;j<pLoopSST->sst.nx;j++){
	if( 1 == large_reference && 1 == use_stored_latlon ){
	  /* Here we use the satellite projection values */
	  /* There are only true satellite projection values if GHRSST data read in... */
	  /* Need to get both lat and lon values */
	  pos = j+i*pLoopSST->sst.nx;
	  lon = *(pLoopSST->lon.array+pos) ;
	  lat = *(pLoopSST->lat.array+pos) ;

	  /* Put a lat/lon check here - may end up being inelegant! */

	  if( -180. <= lon && 360. >= lon && -90. <= lat &&  90. >= lat ){
	  /* Note MATLAB ordering of data */
	    xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	    ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
	  } else {
	    xpos = -1;	/* Horrible cheat but it allows the ypos loop to continue whilst tripping the xpos test */
	  }
	} else {
	  /* Either no stored lat/lon OR */
	  /* Expanding out, so just us min/delta values for lat/lon */
	  lat = pLoopSST->minlat + j*loop_deltalat;
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	}

	/* Check that we are not out of bound */
	if( (0 <= xpos) && (pMatchSST->sst.nx > xpos) && (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
	  /* Location in the input image */
	  /* xpos and ypos are now the location in the binned image */
	  /* No extra pixels (bigArray scale >= image scale) */
	  /* Simple nearest neighbor */
	  pos = xpos+ypos*pMatchSST->sst.nx;
	  pos2 = j+i*pLoopSST->sst.nx;
	  /* Actually do the rebinning - but test to see if data is valid */
	  /* If reference larger scale than SST */
	  if( 1 == large_reference ){
	    /* Check we have good data */
	    if( 0 == isnan(*(pLoopSST->sst.array+pos2)) ){
	      *(pRebinSST->array+pos) += *(pLoopSST->sst.array+pos2);
	      *(pNdata+pos) += 1;
	      max_ndata = MAX(*(pNdata+pos),max_ndata);
	    }
	  } else {
	    /* If reference smaller scale than SST */
	    if( 0 == isnan(*(pMatchSST->sst.array+pos)) ){
	      *(pRebinSST->array+pos2) += *(pMatchSST->sst.array+pos);
	      *(pNdata+pos2) += 1;
	      max_ndata = MAX(*(pNdata+pos2),max_ndata);
	    }
	  }
	}
      }
    }
  }

  /* If no data has been rebinned - exit */
  if( 0 == max_ndata ){
    message(1,"ERROR: No data rebinned in rebin_reference (function: rebin_data_matlab)");
    /* EXIT */
    exit(-1);
  }

  /* If rebin has added multiple entries, then take mean and set places 
   * with no data to NaN */
  for(i=0;i<pRebinSST->size;i++){
    if( 1 < *(pNdata+i) ){
      *(pRebinSST->array+i) /= *(pNdata+i);
    } else if( 0 >= *(pNdata+i) ){
      *(pRebinSST->array+i) = NaN;
    }
  }

  /* Free memory */
  free(pNdata);
}

/* NAME: 
 *
 *    rebin_data_matlab_weight
 *
 * FUNCTION:
 *
 *    C Code to rebin an input dataset to another grid using variance weighting
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Rebins the dataset to the resolution of the reference input image
 *    Currently only uses nearest neighor binning and only works if both
 *    images are on equal angle grids .  Uses input variances to weight
 *    the average.
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE reference sst rebin
 *
 *       get delta longitudes and latitudes for reference and sst image
 *       
 *       allocate output (rebin) image
 *       allocate output (weight) image
 *       allocate ndata array
 *
 *       IF input(sst) data is on a finer scale that output(rebin) image THEN
 *          setup loop arrays/deltas - loop round sst image
 *       ELSE IF input(sst) data is on a coarser scale that output THEN
 *          setup loop arrays/deltas - loop round reference image
 *       ENDIF
 *
 *       LOOP in Y direction
 *          get YPOS position in relevant image
 *          LOOP in X direction 
 *             get XPOS position in relevant image
 *
 *             Check for good data 
 *             copy from first image to second array based on X->XPOS,Y->YPOS
 *
 *          END LOOP
 *       END LOOP
 *
 *       Check that some data has been rebinned
 *
 *       LOOP round rebin image
 *          IF Ndata > 1 get mean value
 *          IF Ndata <= 0 set to bad value
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    rebin_data_matlab( pRef_SST, pSST, pRebinSST )
 *
 * INPUTS:  
 *    
 *   pRef_SST        - input reference SST
 *   pSST            - input SST image
 *
 * OUTPUTS:  
 * 
 *   pRebinSST       - rebinned SST dataset
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *   isnan
 *   free 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void rebin_data_matlab_weight(struct sst_struct *pRefSST, 
			      struct sst_struct *pSST,
			      struct float_struct *pRebinSST)
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT pos2 = 0;
  VFLOAT lon = 0.;
  VFLOAT lat = 0.;
  VFLOAT loop_deltalon = 0.;
  VFLOAT loop_deltalat = 0.;
  VFLOAT match_deltalon = 0.;
  VFLOAT match_deltalat = 0.;

  VINT *pNdata = NULL;
  VINT max_ndata = 0;
  VINT large_reference = 0;
  VINT use_stored_latlon = 0;
  VINT sst_lat_size = 0;
  VINT ref_lat_size = 0;
  VINT sst_lon_size = 0;
  VINT ref_lon_size = 0;
  VFLOAT *pWeight = NULL;
  VFLOAT weight = 0.;
  struct sst_struct *pLoopSST = NULL;
  struct sst_struct *pMatchSST = NULL;

  /* Note that longitude is now stored as y - MATLAB effect */
  /* Allocate output to same size as refernce image */
  allocate_float_array_matlab( pRefSST->sst.nx, pRefSST->sst.ny, pRebinSST );

  /* Allocate ndata array */
  if( NULL == (pNdata = (VINT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pNdata in rebin_data");
    /* EXIT */
    exit(-1);
  }

  if( NULL == (pWeight = (VFLOAT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VFLOAT))) ){
    message(1,"ERROR: Cannot allocate memory for pWeight in rebin_data");
    /* EXIT */
    exit(-1);
  }

  /* Comes in 2 parts.  If scale of input image > reference then loop round
   * reference and do nearest neighbor from input image.  If scale of input
   * image <= reference then loop round input image and do nearest neighbor
   * from reference.  This enables the rebinning of both an SST image onto
   * the reference grid (e.g. input SST onto the analysis grid) or mapping
   * the reference to the SST grid */

  /* Use pointers to reset which array is which */
  /* Setup sizes to a precision of 0.001 */
  /* Avoids slight rounding precision problem */
  sst_lat_size = (VINT)(fabs(pSST->deltalat)*1000.);
  ref_lat_size = (VINT)(fabs(pRefSST->deltalat)*1000.);
  sst_lon_size = (VINT)(fabs(pSST->deltalon)*1000.);
  ref_lon_size = (VINT)(fabs(pRefSST->deltalon)*1000.);

  /* Use pointers to reset which array is which */
  if( sst_lon_size <= ref_lon_size && sst_lat_size <= ref_lat_size ){
    /* This is the case when the input data is on a finer scale than
     * the refernce, so we loop round the input data to match to 
     * reference */
    pLoopSST = pSST;
    pMatchSST = pRefSST;
    large_reference = 1;
  } else {
    /* This is the case when the input data is on a larger scale than
     * the reference, so we loop round the reference to match to 
     * the input data using nearest neighbor */
    pLoopSST = pRefSST;
    pMatchSST = pSST;
    large_reference = 0;
  }

  /* Get relevant delta longitudes/latitudes for arrays */
  loop_deltalon = pLoopSST->deltalon;
  loop_deltalat = pLoopSST->deltalat;
  match_deltalon = pMatchSST->deltalon;
  match_deltalat = pMatchSST->deltalat;

  /* Note that for large_reference = 1 we can use the real Lat/Lon rather than using min/delta lat/lon
   * For large_reference = 0 it would be too time consuming to use true value, so use min/delta lat/lon
   * For the GEO data the difference is small anyway...
   */
  if( NULL != pLoopSST->lon.array && NULL != pLoopSST->lat.array ){
    use_stored_latlon = 1;
  } else {
    use_stored_latlon = 0;
  }

  /* Loop round and use nearest neighbour to get rebinned data */
  max_ndata = 0;
  for(i=0;i<pLoopSST->sst.ny;i++){ 
    if( 0 == large_reference || 0 == use_stored_latlon ){
      /* Expanding out, so just us min/delta values for lat/lon */
      /* Get longitude */
      lon = pLoopSST->minlon + i*loop_deltalon;
      ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
    }
    /* Check that we are not out of bound */
    if( (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
      for(j=0;j<pLoopSST->sst.nx;j++){
	if( 1 == large_reference && 1 == use_stored_latlon ){
	  /* Here we use the satellite projection values */
	  /* There are only true satellite projection values if GHRSST data read in... */
	  /* Need to get both lat and lon values */
	  pos = j+i*pLoopSST->sst.nx;
	  lon = *(pSST->lon.array+pos) ;
	  lat = *(pSST->lat.array+pos) ;

	  /* Must check lat/lon before assigning to ypos, xpos otherwise the former will be set < 0 and can never reset  */
	  /* Also, checking on ypos when the 'corners' of the geostationary image may be off planet will cause the whole */
	  /* line to be discarded */
	  /* Note MATLAB ordering of data */

	  /* Put a lat/lon check here - may end up being inelegant! */

	  if( -180. <= lon && 360. >= lon && -90. <= lat &&  90. >= lat ){
	    xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	    ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
	  } else {
	    xpos = -1;	/* Horrible cheat but it allows the ypos loop to continue whilst tripping the xpos test */
	  }
	} else {
	  /* Expanding out, so just us min/delta values for lat/lon */
	  lat = pLoopSST->minlat + j*loop_deltalat;
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	}
	/* Check that we are not out of bound */
	if( (0 <= xpos) && (pMatchSST->sst.nx > xpos) ){
	  /* Location in the input image */
	  /* xpos and ypos are now the location in the binned image */
	  /* No extra pixels (bigArray scale >= image scale) */
	  /* Simple nearest neighbor */
	  pos = xpos+ypos*pMatchSST->sst.nx;
	  pos2 = j+i*pLoopSST->sst.nx;
	  /* Actually do the rebinning - but test to see if data is valid */
	  /* If reference larger scale than SST */
	  if( 1 == large_reference ){
	    /* Check we have good data */
	    if( 0 == isnan(*(pLoopSST->sst.array+pos2)) ){
	      weight = *(pLoopSST->est_variance.array+pos2);
	      *(pRebinSST->array+pos) += weight * *(pLoopSST->sst.array+pos2);
	      *(pWeight+pos) += weight;
	      *(pNdata+pos) += 1;
	      max_ndata = MAX(*(pNdata+pos),max_ndata);
	    }
	  } else {
	    /* If reference smaller scale than SST */
	    if( 0 == isnan(*(pMatchSST->sst.array+pos)) ){
	      weight = *(pMatchSST->est_variance.array+pos);
	      *(pRebinSST->array+pos2) += weight * *(pMatchSST->sst.array+pos);
	      *(pWeight+pos2) += weight;
	      *(pNdata+pos2) += 1;
	      max_ndata = MAX(*(pNdata+pos2),max_ndata);
	    }
	  }
	}
      }
    }
  }

  /* If no data has been rebinned - exit */
  if( 0 == max_ndata ){
    message(1,"ERROR: No data rebinned in rebin_reference (function: rebin_data_matlab_weight)");
    /* EXIT */
    exit(-1);
  }

  /* If rebin has added multiple entries, then take mean and set places 
   * with no data to NaN */
  for(i=0;i<pRebinSST->size;i++){
    if( 1 < *(pNdata+i) ){
      *(pRebinSST->array+i) /= *(pWeight+i);
    } else if( 0 >= *(pNdata+i) ){
      *(pRebinSST->array+i) = NaN;
    }
  }

  /* Free memory */
  free(pNdata);
  free(pWeight);
}

/*
 * NAME: 
 *
 *    copy_flt_to_str_matlab
 *
 * FUNCTION:
 *
 *    C Code to copy float array to float_struct
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies float variable to float_struct
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny fltarr out_fltstruct
 *
 *       allocate memory for out_fltstruct
 *
 *       copy data
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    copy_flt_to_str_matlab( nx, ny, fltarr, flt_str )
 *
 * INPUTS:  
 *
 *    nx      - nx
 *    ny      - ny
 *    fltarr  - input float array    
 *
 * OUTPUTS:  
 * 
 *   flt_str  - output float_struct
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void copy_flt_to_str_matlab( const VINT nx, const VINT ny, const float *pVar, 
			     struct float_struct *pStr )
{
  VINT i = 0;

  /* Allocate memory */
  allocate_float_array_matlab( nx, ny, pStr );

  /* Copy */
  for(i=0;i<pStr->size;i++){
    *(pStr->array+i) = (VFLOAT) *(pVar+i);
  }
}

/*
 * NAME: 
 *
 *    copy_byte_to_str
 *
 * FUNCTION:
 *
 *    C Code to copy byte array to char_struct
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Copies byte variable to char_struct
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny fltarr out_fltstruct
 *
 *       allocate memory for out_fltstruct
 *
 *       copy data
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_byte_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    copy_flt_to_str_matlab( nx, ny, fltarr, flt_str )
 *
 * INPUTS:  
 *
 *    nx      - nx
 *    ny      - ny
 *    fltarr  - input byte array    
 *
 * OUTPUTS:  
 * 
 *   flt_str  - output char_struct
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void copy_byte_to_str_matlab( const VINT nx, const VINT ny, const char *pVar, 
			      struct char_struct *pStr )
{
  VINT i = 0;

  /* Allocate memory */
  allocate_byte_array_matlab( nx, ny, pStr );

  /* Copy */
  for(i=0;i<pStr->size;i++){
    *(pStr->array+i) = *(pVar+i);
  }
}


/* NAME: 
 *
 *    rebin_data_matlab
 *
 * FUNCTION:
 *
 *    C Code to rebin an input dataset to another grid
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Rebins the dataset to the resolution of the reference input image
 *    Currently only uses nearest neighor binning and only works if both
 *    images are on equal angle grids 
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE reference sst rebin
 *
 *       get delta longitudes and latitudes for reference and sst image
 *       
 *       allocate output (rebin) image
 *       allocate ndata array
 *
 *       IF input(sst) data is on a finer scale that output(rebin) image THEN
 *          setup loop arrays/deltas - loop round sst image
 *       ELSE IF input(sst) data is on a coarser scale that output THEN
 *          setup loop arrays/deltas - loop round reference image
 *       ENDIF
 *
 *       LOOP in Y direction
 *          get YPOS position in relevant image
 *          LOOP in X direction 
 *             get XPOS position in relevant image
 *
 *             Check for good data 
 *             copy from first image to second array based on X->XPOS,Y->YPOS
 *
 *          END LOOP
 *       END LOOP
 *
 *       Check that some data has been rebinned
 *
 *       LOOP round rebin image
 *          IF Ndata > 1 get mean value
 *          IF Ndata <= 0 set to bad value
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    rebin_data_matlab( pRef_SST, pSST, pRebinSST  )
 *
 * INPUTS:  
 *    
 *   pRef_SST        - input reference SST
 *   pSST            - input SST image
 *
 * OUTPUTS:  
 * 
 *   pRebinSST       - rebinned SST dataset
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *   isnan
 *   free 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void rebin_data_matlab2(struct sst_struct *pRefSST, 
		       struct sst_struct *pSST,
		       struct float_struct *pRebinSST)
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT pos2 = 0;
  VFLOAT lon = 0.;
  VFLOAT lat = 0.;
  VFLOAT loop_deltalon = 0.;
  VFLOAT loop_deltalat = 0.;
  VFLOAT match_deltalon = 0.;
  VFLOAT match_deltalat = 0.;

  VINT *pNdata = NULL;
  VINT max_ndata = 0;
  VINT large_reference = 0;
  VINT use_stored_latlon = 0;
  VINT sst_lat_size = 0;
  VINT ref_lat_size = 0;
  VINT sst_lon_size = 0;
  VINT ref_lon_size = 0;
  struct sst_struct *pLoopSST = NULL;
  struct sst_struct *pMatchSST = NULL;

  /* Note that longitude is now stored as y - MATLAB effect */
  /* Allocate output to same size as refernce image */
  allocate_float_array_matlab( pRefSST->sst.nx, pRefSST->sst.ny, pRebinSST );

  /* Allocate ndata array */
  if( NULL == (pNdata = (VINT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pNdata in rebin_data");
    /* EXIT */
    exit(-1);
  }

  /* Comes in 2 parts.  If scale of input image > reference then loop round
   * reference and do nearest neighbor from input image.  If scale of input
   * image <= reference then loop round input image and do nearest neighbor
   * from reference.  This enables the rebinning of both an SST image onto
   * the reference grid (e.g. input SST onto the analysis grid) or mapping
   * the reference to the SST grid */

  /* Setup sizes to a precision of 0.001 */
  /* Avoids slight rounding precision problem */
  sst_lat_size = (VINT)(fabs(pSST->deltalat)*1000.);
  ref_lat_size = (VINT)(fabs(pRefSST->deltalat)*1000.);
  sst_lon_size = (VINT)(fabs(pSST->deltalon)*1000.);
  ref_lon_size = (VINT)(fabs(pRefSST->deltalon)*1000.);

  /* Use pointers to reset which array is which */
  if( sst_lon_size <= ref_lon_size && sst_lat_size <= ref_lat_size ){
    /* This is the case when the input data is on a finer scale than
     * the reference, so we loop round the input data to match to 
     * reference */
    pLoopSST = pSST;
    pMatchSST = pRefSST;
    large_reference = 1;
  } else {
    /* This is the case when the input data is on a larger scale than
     * the reference, so we loop round the reference to match to 
     * the input data using nearest neighbor */
    pLoopSST = pRefSST;
    pMatchSST = pSST;
    large_reference = 0;
  }

  /* Get relevant delta longitudes/latitudes for arrays */
  loop_deltalon = pLoopSST->deltalon;
  loop_deltalat = pLoopSST->deltalat;
  match_deltalon = pMatchSST->deltalon;
  match_deltalat = pMatchSST->deltalat;

  /* Note that for large_reference = 1 we can use the real Lat/Lon rather than using min/delta lat/lon
   * For large_reference = 0 it would be too time consuming to use true value, so use min/delta lat/lon
   * For the GEO data the difference is small anyway...
   */
  if( NULL != pLoopSST->lon.array && NULL != pLoopSST->lat.array ){
    use_stored_latlon = 1;
  } else {
    use_stored_latlon = 0;
  }

  /* Loop round and use nearest neighbour to get rebinned data */

  max_ndata = 0;
  for(i=0;i<pLoopSST->sst.ny;i++){ 
    if( 0 == large_reference || 0 == use_stored_latlon ){
      /* Either no stored lat/lon OR */
      /* Expanding out, so just us min/delta values for lat/lon */
      /* Get longitude */
      lon = pLoopSST->minlon + i*loop_deltalon;
      ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
    }
    /* Check that we are not out of bound */
    if( (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
      for(j=0;j<pLoopSST->sst.nx;j++){
	if( 1 == large_reference && 1 == use_stored_latlon ){
	  /* Here we use the satellite projection values */
	  /* There are only true satellite projection values if GHRSST data read in... */
	  /* Need to get both lat and lon values */
	  pos = j+i*pLoopSST->sst.nx;
	  lon = *(pLoopSST->lon.array+pos) ;
	  lat = *(pLoopSST->lat.array+pos) ;
	  /* Note MATLAB ordering of data */
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	  ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
	} else {
	  /* Either no stored lat/lon OR */
	  /* Expanding out, so just us min/delta values for lat/lon */
	  lat = pLoopSST->minlat + j*loop_deltalat;
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	}

	/* Check that we are not out of bound */
	if( (0 <= xpos) && (pMatchSST->sst.nx > xpos) && (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
	  /* Location in the input image */
	  /* xpos and ypos are now the location in the binned image */
	  /* No extra pixels (bigArray scale >= image scale) */
	  /* Simple nearest neighbor */
	  pos = xpos+ypos*pMatchSST->sst.nx;
	  pos2 = j+i*pLoopSST->sst.nx;
	  /* Actually do the rebinning - but test to see if data is valid */
	  /* If reference larger scale than SST */
	  if( 1 == large_reference ){
	    /* Check we have good data */
	    if( 0 == isnan(*(pLoopSST->sst.array+pos2)) ){
	      *(pRebinSST->array+pos) += *(pLoopSST->sst.array+pos2);
	      *(pNdata+pos) += 1;
	      max_ndata = MAX(*(pNdata+pos),max_ndata);
	    }
	  } else {
	    /* If reference smaller scale than SST */
	    if( 0 == isnan(*(pMatchSST->sst.array+pos)) ){
	      *(pRebinSST->array+pos2) += *(pMatchSST->sst.array+pos);
	      *(pNdata+pos2) += 1;
	      max_ndata = MAX(*(pNdata+pos2),max_ndata);
	    }
	  }
	}
      }
    }
  }

  /* If no data has been rebinned - exit */
  if( 0 == max_ndata ){
    message(1,"ERROR: No data rebinned in rebin_reference (function: rebin_data_matlab)");
    /* EXIT */
    exit(-1);
  }

  /* If rebin has added multiple entries, then take mean and set places 
   * with no data to NaN */
  for(i=0;i<pRebinSST->size;i++){
    if( 1 < *(pNdata+i) ){
      *(pRebinSST->array+i) /= *(pNdata+i);
    } else if( 0 >= *(pNdata+i) ){
      *(pRebinSST->array+i) = NaN;
    }
  }

  /* Free memory */
  free(pNdata);
}

/* NAME: 
 *
 *    rebin_data_matlab_weight
 *
 * FUNCTION:
 *
 *    C Code to rebin an input dataset to another grid using variance weighting
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Rebins the dataset to the resolution of the reference input image
 *    Currently only uses nearest neighor binning and only works if both
 *    images are on equal angle grids .  Uses input variances to weight
 *    the average.
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE reference sst rebin
 *
 *       get delta longitudes and latitudes for reference and sst image
 *       
 *       allocate output (rebin) image
 *       allocate output (weight) image
 *       allocate ndata array
 *
 *       IF input(sst) data is on a finer scale that output(rebin) image THEN
 *          setup loop arrays/deltas - loop round sst image
 *       ELSE IF input(sst) data is on a coarser scale that output THEN
 *          setup loop arrays/deltas - loop round reference image
 *       ENDIF
 *
 *       LOOP in Y direction
 *          get YPOS position in relevant image
 *          LOOP in X direction 
 *             get XPOS position in relevant image
 *
 *             Check for good data 
 *             copy from first image to second array based on X->XPOS,Y->YPOS
 *
 *          END LOOP
 *       END LOOP
 *
 *       Check that some data has been rebinned
 *
 *       LOOP round rebin image
 *          IF Ndata > 1 get mean value
 *          IF Ndata <= 0 set to bad value
 *       END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    rebin_data_matlab( pRef_SST, pSST, pRebinSST )
 *
 * INPUTS:  
 *    
 *   pRef_SST        - input reference SST
 *   pSST            - input SST image
 *
 * OUTPUTS:  
 * 
 *   pRebinSST       - rebinned SST dataset
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *   isnan
 *   free 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
void rebin_data_matlab_weight2(struct sst_struct *pRefSST, 
			      struct sst_struct *pSST,
			      struct float_struct *pRebinSST)
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT pos2 = 0;
  VFLOAT lon = 0.;
  VFLOAT lat = 0.;
  VFLOAT loop_deltalon = 0.;
  VFLOAT loop_deltalat = 0.;
  VFLOAT match_deltalon = 0.;
  VFLOAT match_deltalat = 0.;

  VINT *pNdata = NULL;
  VINT max_ndata = 0;
  VINT large_reference = 0;
  VINT use_stored_latlon = 0;
  VINT sst_lat_size = 0;
  VINT ref_lat_size = 0;
  VINT sst_lon_size = 0;
  VINT ref_lon_size = 0;
  VFLOAT *pWeight = NULL;
  VFLOAT weight = 0.;
  struct sst_struct *pLoopSST = NULL;
  struct sst_struct *pMatchSST = NULL;

  /* Note that longitude is now stored as y - MATLAB effect */
  /* Allocate output to same size as refernce image */
  allocate_float_array_matlab( pRefSST->sst.nx, pRefSST->sst.ny, pRebinSST );

  /* Allocate ndata array */
  if( NULL == (pNdata = (VINT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pNdata in rebin_data");
    /* EXIT */
    exit(-1);
  }

  if( NULL == (pWeight = (VFLOAT *) 
	       calloc(pRefSST->sst.nx*pRefSST->sst.ny,sizeof(VFLOAT))) ){
    message(1,"ERROR: Cannot allocate memory for pWeight in rebin_data");
    /* EXIT */
    exit(-1);
  }

  /* Comes in 2 parts.  If scale of input image > reference then loop round
   * reference and do nearest neighbor from input image.  If scale of input
   * image <= reference then loop round input image and do nearest neighbor
   * from reference.  This enables the rebinning of both an SST image onto
   * the reference grid (e.g. input SST onto the analysis grid) or mapping
   * the reference to the SST grid */

  /* Use pointers to reset which array is which */
  /* Setup sizes to a precision of 0.001 */
  /* Avoids slight rounding precision problem */
  sst_lat_size = (VINT)(fabs(pSST->deltalat)*1000.);
  ref_lat_size = (VINT)(fabs(pRefSST->deltalat)*1000.);
  sst_lon_size = (VINT)(fabs(pSST->deltalon)*1000.);
  ref_lon_size = (VINT)(fabs(pRefSST->deltalon)*1000.);

  /* Use pointers to reset which array is which */
  if( sst_lon_size <= ref_lon_size && sst_lat_size <= ref_lat_size ){
    /* This is the case when the input data is on a finer scale than
     * the refernce, so we loop round the input data to match to 
     * reference */
    pLoopSST = pSST;
    pMatchSST = pRefSST;
    large_reference = 1;
  } else {
    /* This is the case when the input data is on a larger scale than
     * the reference, so we loop round the reference to match to 
     * the input data using nearest neighbor */
    pLoopSST = pRefSST;
    pMatchSST = pSST;
    large_reference = 0;
  }

  /* Get relevant delta longitudes/latitudes for arrays */
  loop_deltalon = pLoopSST->deltalon;
  loop_deltalat = pLoopSST->deltalat;
  match_deltalon = pMatchSST->deltalon;
  match_deltalat = pMatchSST->deltalat;

  /* Note that for large_reference = 1 we can use the real Lat/Lon rather than using min/delta lat/lon
   * For large_reference = 0 it would be too time consuming to use true value, so use min/delta lat/lon
   * For the GEO data the difference is small anyway...
   */
  if( NULL != pLoopSST->lon.array && NULL != pLoopSST->lat.array ){
    use_stored_latlon = 1;
  } else {
    use_stored_latlon = 0;
  }

  /* Loop round and use nearest neighbour to get rebinned data */
  max_ndata = 0;
  for(i=0;i<pLoopSST->sst.ny;i++){ 
    if( 0 == large_reference || 0 == use_stored_latlon ){
      /* Expanding out, so just us min/delta values for lat/lon */
      /* Get longitude */
      lon = pLoopSST->minlon + i*loop_deltalon;
      ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
    }
    /* Check that we are not out of bound */
    if( (0 <= ypos) && (pMatchSST->sst.ny > ypos) ){
      for(j=0;j<pLoopSST->sst.nx;j++){
	if( 1 == large_reference && 1 == use_stored_latlon ){
	  /* Here we use the satellite projection values */
	  /* There are only true satellite projection values if GHRSST data read in... */
	  /* Need to get both lat and lon values */
	  pos = j+i*pLoopSST->sst.nx;
	  lon = *(pSST->lon.array+pos) ;
	  lat = *(pSST->lat.array+pos) ;
	  /* Note MATLAB ordering of data */
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	  ypos = (VINT) ((lon-pMatchSST->minlon)/match_deltalon);
	} else {
	  /* Expanding out, so just us min/delta values for lat/lon */
	  lat = pLoopSST->minlat + j*loop_deltalat;
	  xpos = (VINT) ((lat-pMatchSST->minlat)/match_deltalat);
	}
	/* Check that we are not out of bound */
	if( (0 <= xpos) && (pMatchSST->sst.nx > xpos) ){
	  /* Location in the input image */
	  /* xpos and ypos are now the location in the binned image */
	  /* No extra pixels (bigArray scale >= image scale) */
	  /* Simple nearest neighbor */
	  pos = xpos+ypos*pMatchSST->sst.nx;
	  pos2 = j+i*pLoopSST->sst.nx;
	  /* Actually do the rebinning - but test to see if data is valid */
	  /* If reference larger scale than SST */
	  if( 1 == large_reference ){
	    /* Check we have good data */
	    if( 0 == isnan(*(pLoopSST->sst.array+pos2)) ){
	      weight = *(pLoopSST->est_variance.array+pos2);
	      *(pRebinSST->array+pos) += weight * *(pLoopSST->sst.array+pos2);
	      *(pWeight+pos) += weight;
	      *(pNdata+pos) += 1;
	      max_ndata = MAX(*(pNdata+pos),max_ndata);
	    }
	  } else {
	    /* If reference smaller scale than SST */
	    if( 0 == isnan(*(pMatchSST->sst.array+pos)) ){
	      weight = *(pMatchSST->est_variance.array+pos);
	      *(pRebinSST->array+pos2) += weight * *(pMatchSST->sst.array+pos);
	      *(pWeight+pos2) += weight;
	      *(pNdata+pos2) += 1;
	      max_ndata = MAX(*(pNdata+pos2),max_ndata);
	    }
	  }
	}
      }
    }
  }

  /* If no data has been rebinned - exit */
  if( 0 == max_ndata ){
    message(1,"ERROR: No data rebinned in rebin_reference (function: rebin_data_matlab_weight)");
    /* EXIT */
    exit(-1);
  }

  /* If rebin has added multiple entries, then take mean and set places 
   * with no data to NaN */
  for(i=0;i<pRebinSST->size;i++){
    if( 1 < *(pNdata+i) ){
      *(pRebinSST->array+i) /= *(pWeight+i);
    } else if( 0 >= *(pNdata+i) ){
      *(pRebinSST->array+i) = NaN;
    }
  }

  /* Free memory */
  free(pNdata);
  free(pWeight);
}

