/*
 * NAME: 
 *
 *    mex_functions
 *
 * FUNCTION:
 *
 *    Header for C versions of MATLAB (MEX) array copy functions
 *
 * TYPE/LANGUAGE: 
 *
 *    C header/MEX
 *
 * DESCRIPTION:  
 *
 *    Contains routines to copy data to MATLAB (MEX) output arrays
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    copy_from_MATLAB_flt - copy from MATLAB float array
 *    copy_to_MATLAB_flt - copy to MATLAB float array
 *    copy_to_MATLAB_int - copy to MATLAB int array
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 03/12/2012
 *
 */

#ifndef MEX_FUNCTION_HEADER

/* Include for MATLAB */
#include "matrix.h"
#include "mex.h"

#include "types_cnsts.h"

/* Routines */
void copy_from_MATLAB_flt( const VINT nx, const VINT ny, const mxArray *pMxData,
			   const struct float_struct *pArray );
void copy_to_MATLAB_flt( const VINT nx, const VINT ny, const VFLOAT *pArray, 
			 mxArray **ppMxData );
void copy_to_MATLAB_int( const VINT nx, const VINT ny, const VINT *pArray, 
			 mxArray **ppMxData );

#define MEX_FUNCTION_HEADER 1
#endif
