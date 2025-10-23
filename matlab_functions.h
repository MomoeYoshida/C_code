/*
 * NAME: 
 *
 *    matlab_functions
 *
 * FUNCTION:
 *
 *    Header for C versions of some low level MATLAB functions
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains routines to emulate some of the MATLAB built in functions which
 *    are used by the POES-GOES Blended software (originally written in MATLAB).
 *    These routines are an attempt to make the POES-GOES Blended code similar
 *    to the MATLAB code for ease of current maintenance/support.  
 *
 *    Also defines int and float types for flexibility and portability 
 *
 *    Defines float and list structures which contain the number of elements
 *    and the values to more closely mimic the high level functionality of 
 *    MATLAB
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *        date_to_days_matlab     - gets number of days since 01/1950
 *        daynum_to_days_matlab   - gets number of days since 01/1950
 *        date_from_days_matlab   - gets date from ndays since 01/1950
 *        date_from_daynum_matlab - gets day number from date
 *        std_matlab              - returns standard deviation 
 *        std_matlab_weighted     - returns standard deviation (weighted)
 *        bubblesort_matlab       - outputs the bubble sorted array 
 *        find_float_matlab       - returns positions that match criteria
 *        find_float_array_matlab - returns positions that match criteria
 *        find_byte_matlab        - returns positions that match criteria
 *        copy_float_malab        - copies array structure (floats)
 *        copy_float_malab_noalloc - copies array structure (no allocation)
 *        copy_int_malab_noalloc - copies array structure (no allocation)
 *        copy_float_1d_malab     - copies array structure (1d floats)
 *        copy_float_1d_malab_noalloc - copies array structure (no allocation)
 *        array_float_oper_matlab - operates on float arrays 
 *        array_float_oper_cnst_matlab - operates on float arrays 
 *        set_find_float_matlab   - set values based in a prev. find
 *        free_float_matlab       - free's memory for float_struct
 *        free_float_1d_matlab    - free's memory for float_struct1d
 *        free_float_3d_matlab    - free's memory for float_struct3d
 *        free_int_matlab         - free's memory for int_struct
 *        free_elem_matlab        - free's memory for find_elem
 *        free_sst_matlab         - free's memory for sst_struct
 *        free_bigsst_matlab      - free's memory for sst_struct
 *        free_out_sst_matlab     - free's memory for out_sst_str
 *        set_find_float_matlab   - sets values based on a previous find
 *        set_float_array_cnst_matlab - makes and sets values in a float array 
 *        mult_float_array_matlab - multiply an array str by a constant
 *        add_float_array_matlab  - add a constant to an array str
 *        subtract_float_array_matlab - subtract a constant tfrom an array str
 *        allocate_float_3d_array_matlab - allocate an array str 
 *        allocate_float_array_matlab - allocate an array str 
 *        allocate_float_1d_array_matlab - allocate an array str 
 *        allocate_int_array_matlab - allocate an array structure
 *        allocate_byte_array_matlab - allocate an array structure
 *        set_min_array_float_matlab - set each element to MAX(value,min_value)
 *        set_max_array_float_matlab - set each element to MIN(value,max_value)
 *        transpose_flip_float_matlab - transpose and flip float array
 *        transpose_flip_int_matlab   - transpose and flip int array
 *        size_zero_float             - set size elements to zero in structure
 *        size_zero_float3d           - set size elements to zero in structure
 *        squeeze_2d_float_matlab     - squeeze line/column to 1d array
 *        rebin_data_matlab           - nearest neighbor rebinning of data
 *        init_float_array_matlab     - initialize  an array structure
 *        init_float_1d_array_matlab  - initialize an array structure
 *        init_float_3d_array_matlab  - initialize an array structure
 *        init_int_array_matlab       - initialize an array structure
 *        init_byte_array_matlab      - initialize an array structure
 *        init_sst_matlab             - initialize an SST structure
 *        init_elem_matlab            - initialize an find_elem structure
 *        copy_flt_to_str_matlab      - copy float array to structure
 *        copy_byte_to_str_matlab     - copy byte array to structure
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef MATLAB_FUNCTION_HEADER

/* Include for typedef */
#include <stdint.h>

#include "types_cnsts.h"

/* Routines */
/* Find routine */
VINT find_float_matlab(const struct float_struct  *pVar, const char *pOper, 
		       const VFLOAT limit, struct find_elems *pOutarray );
VINT find_float_1d_matlab(const struct float_struct1d  *pVar, const char *pOper, 
			  const VFLOAT limit, struct find_elems *pOutarray );
VINT find_byte_1d_matlab(const struct char_struct1d *pVar, const char *pOper, 
			 const unsigned char limit, struct find_elems *pElems );
VINT find_float_array_matlab(const struct float_struct  *pVar, const char *pOper, 
			     const struct float_struct *pLimit, 
			     struct find_elems *pOutarray );
VINT find_byte_matlab(const struct char_struct  *pVar, 
		      const char *pOper, const char limit,
		      struct find_elems *pOutarray );
/* Copy arrays */
void copy_float_matlab_noalloc( const struct float_struct *pVar_in, 
				struct float_struct *pVar_out );
void copy_int_matlab_noalloc( const struct int_struct *pVar_in, 
			      struct int_struct *pVar_out );
void copy_float_matlab( const struct float_struct *pVar_in, 
		        struct float_struct *pVar_out );
void copy_float_1d_matlab( const struct float_struct1d *pVar_in, 
			   struct float_struct1d *pVar_out );
void copy_float_1d_matlab_noalloc( const struct float_struct1d *pVar_in, 
				   struct float_struct1d *pVar_out );
/* Operate on arrays */
void array_float_oper_matlab( const struct float_struct *pVar1, 
			      const char *Oper,
			      const struct float_struct *pVar2,
			      struct float_struct *pOutarray);
void array_float_oper_cnst_matlab( const struct float_struct *pVar1, 
				   const char *Oper,
				   const VFLOAT constantval,
				   struct float_struct *pOutVar );
/* Multiply arrays by a constant */
void mult_float_array_matlab( struct float_struct *pArray, const float constant );
/* Add a constant to an array */
void add_float_array_matlab( struct float_struct *pArray, const float constant );
/* Subtract a constant from an array */
void subtract_float_array_matlab( struct float_struct *pArray, const float constant );
/* set array to a constant value */
void set_float_array_cnst_matlab( const VINT nx, const VINT ny,
				  const VFLOAT constant,
				  struct float_struct *pOutarray );
/* Set size elements to zero */
void size_zero_float(struct float_struct *pArray);
void size_zero_float3d(struct float_struct3d *pArray);
/* set array to absolute values */
void array_float_abs_matlab( struct float_struct *pVar );
/* Set array elements to <=maximum value */
void set_max_array_float_matlab(struct float_struct *pArray, const float max_value );
/* Set array elements to >= minimum value */
void set_min_array_float_matlab(struct float_struct *pArray, const float min_value );
/* Transpose and flip arrays */
void transpose_flip_float_matlab( struct float_struct *pArray );
void transpose_flip_int_matlab( struct int_struct *pArray );
/* Allocate array structure */
void allocate_float_3d_array_matlab(const VINT nx, const VINT ny, const VINT nz,
				    struct float_struct3d *pArray );
void allocate_float_array_matlab(const VINT nx, const VINT ny, 
				 struct float_struct *pArray );
void allocate_float_1d_array_matlab(const VINT nx, struct float_struct1d *pArray );
void reallocate_float_1d_array_matlab(const VINT nx, 
				      struct float_struct1d *pArray );
void allocate_byte_array_matlab(const VINT nx, const VINT ny, 
				struct char_struct *pArray );
void allocate_byte_1d_array_matlab(const VINT nx, struct char_struct1d *pArray );
void reallocate_byte_1d_array_matlab(const VINT nx, struct char_struct1d *pArray );
void allocate_int_array_matlab(const VINT nx, const VINT ny, 
			       struct int_struct *pArray );
/* Init array structure */
void init_float_3d_array_matlab( struct float_struct3d *pArray );
void init_float_array_matlab( struct float_struct *pArray );
void init_float_1d_array_matlab( struct float_struct1d *pArray );
void init_byte_array_matlab( struct char_struct *pArray );
void init_byte_1d_array_matlab( struct char_struct1d *pArray );
void init_int_array_matlab( struct int_struct *pArray );
void init_sst_matlab( struct sst_struct *pArray );
void init_big_geo_grid_matlab( struct big_geo_grid *pGrid );
void init_out_sst_matlab( struct out_sst_str *pSST );
void init_elem_matlab( struct find_elems *pElems );
/* Sort arrays */
void bubblesort_matlab( const struct float_struct1d *pArray, 
			const struct float_struct1d *pAssoc, 
			struct float_struct1d *pOutArray,
			struct float_struct1d *pOutAssoc );
/* GEt standard deviation */
VFLOAT std_matlab( VFLOAT *array, const VINT start, const VINT stop, VFLOAT *pMean );
VFLOAT std_matlab_weighted( VFLOAT *pArray, VFLOAT *pVariance, 
			    const VINT start, const VINT stop, VFLOAT *pMean );
/* Date year,daynum from datenum (ndays from 1950) */
void date_from_days_matlab( const VINT datenum, VINT *pYear, VINT *pDaynum );
/* Get datenum from date */
VINT date_to_days_matlab( const VINT year, const VINT month, const VINT day );
/* Get daynum from year and day of year */
VINT daynum_to_days_matlab( const VINT year, const VINT day );
/* Get momth/day from day in year */
void date_from_daynum_matlab( const VINT year, const VINT daynum, VINT *pMonth, 
			      VINT *pDay );
/* Set values in float_struct */
void set_find_float_matlab(const struct float_struct *pVar, struct find_elems *pBad,
  			   const VFLOAT badVal );
/* Set array with constant value */
void set_float_array_matlab(const VINT nx, const VINT ny, const VFLOAT constant,
			    struct float_struct *pOutarray);
/* Copy from float/byte array to structure */
void copy_flt_to_str_matlab( const VINT nx, const VINT ny, const float *pVar, 
			     struct float_struct *pStr );
void copy_byte_to_str_matlab( const VINT nx, const VINT ny, const char *pVar, 
			      struct char_struct *pStr );
char *add_string_matlab( VINT nargs, ... );
/* Free memory */
void free_float_3d_matlab(struct float_struct3d *pArray);
void free_float_matlab(struct float_struct *pArray);
void free_float_1d_matlab(struct float_struct1d *pArray);
void free_int_matlab(struct int_struct *pArray);
void free_elem_matlab(struct find_elems *pArray);
void free_byte_matlab(struct char_struct *pArray);
void free_byte_1d_matlab(struct char_struct1d *pArray);
void free_sst_matlab(struct sst_struct *pArray);
void free_out_sst_matlab(struct out_sst_str *pArray);
void free_bigsst_matlab(struct big_geo_grid *pArray);
/* Finding lists of files */
void free_filelist( VINT *pNFiles, char ***pFilelist );
VINT find_files( const char *searchString, VINT *pNFiles, char ***pFilelist );
/* Queeze out a line/column from arrays */
void squeeze_2d_float_matlab( const struct float_struct *pInArray, const VINT elem,
			      const VINT line, struct float_struct1d *pOutArray );
/* Rebin data using a nearest neighbor routine */
void rebin_data_matlab(struct sst_struct *pRefSST, 
		       struct sst_struct *pSST,
		       struct float_struct *pRebinSST);
/* Weighted averaging for rebinning */
void rebin_data_matlab_weight(struct sst_struct *pRefSST, 
			      struct sst_struct *pSST,
			      struct float_struct *pRebinSST);

#define MATLAB_FUNCTION_HEADER 1
#endif
