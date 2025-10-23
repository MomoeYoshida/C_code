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
 *    Contains routines to copy data to MATLAB array (mxArray)
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *        copy_from_MATLAB_flt          - copy from MATLAB double variable
 *        copy_to_MATLAB_flt            - copy to MATLAB double variable
 *        copy_to_MATLAB_int            - copy to MATLAB int32 variable
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 03/12/2012
 *
 */
#include <stdlib.h>

/* MATLAB mex header */
#include "matrix.h"
#include "mex.h"

/* prototypes */
#include "mex_functions.h"

/*
 * NAME: 
 *
 *    copy_from_MATLAB_flt
 *
 * FUNCTION:
 *
 *    C Copies data from a MATLAB variabile of type mxREAL
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine/MEX
 *
 * DESCRIPTION:  
 *
 *    Copys 2-d data to a C array from MATLAB variable for input
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny inputArray outputArray
 *
 *       get pointer from mxArray
 *       allocate VFLOAT structure
 *
 *       copy data to output structure
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    mxGetPr
 *
 * CALLING SEQUENCE:  
 *
 *    copy_to_MATLAB_flt(nx,ny,inputArray,outputMEXArray)
 *
 * INPUTS:  
 *
 *     nx               - X dimension of input array
 *     ny               - Y dimension of input array
 *     inputArray       - pointer to (VFLOAT *) inputArray
 *
 * OUTPUTS:  
 * 
 *     outputMEXArray   - pointer to output MEX array (mexArray)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 03/12/2011
 *
 */
void copy_from_MATLAB_flt( const VINT nx, const VINT ny, const mxArray *pMxData,
			   const struct float_struct *pArray )
{
  /* Local variables */
  VINT i = 0;
  double *p_inmat = NULL;

  /* Create MATLAB variable */
  p_inmat = mxGetPr( pMxData );
  
  /* Allocate memory */
  allocate_float_array_matlab(nx,ny,pArray);

  /* Copy data over */
  for(i=0;i<nx*ny;i++){
    *(pArray->array+i) = (VFLOAT) *(p_inmat+i);
  } 

}

/*
 * NAME: 
 *
 *    copy_to_MATLAB_flt
 *
 * FUNCTION:
 *
 *    C Copys data to a MATLAB variabile of type mxREAL
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine/MEX
 *
 * DESCRIPTION:  
 *
 *    Copys 2-d data from C array to MATLAB variable for output
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny inputArray outputArray
 *
 *       create pointer to mxArray (double) 
 *       get pointer
 *
 *       copy data to pointer
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    mxCreateDoubleMatrix
 *    mxGetPr
 *
 * CALLING SEQUENCE:  
 *
 *    copy_to_MATLAB_flt(nx,ny,inputArray,outputMEXArray)
 *
 * INPUTS:  
 *
 *     nx               - X dimension of input array
 *     ny               - Y dimension of input array
 *     inputArray       - pointer to (VFLOAT *) inputArray
 *
 * OUTPUTS:  
 * 
 *     outputMEXArray   - pointer to output MEX array (mexArray)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 03/12/2011
 *
 */
void copy_to_MATLAB_flt( const VINT nx, const VINT ny, const VFLOAT *pArray, 
			 mxArray **ppMxData )
{
  /* Local variables */
  VINT i = 0;
  double *p_outmat = NULL;

  /* Create MATLAB variable */
  *ppMxData = mxCreateDoubleMatrix( nx, ny, mxREAL );

  /* Get pointer to copy data to */
  p_outmat = mxGetPr( *ppMxData );
  
  /* Copy data over */
  for(i=0;i<nx*ny;i++){
    *(p_outmat+i) = *(pArray+i);
  } 

}

/*
 * NAME: 
 *
 *    copy_to_MATLAB_int
 *
 * FUNCTION:
 *
 *    C Copys data to a MATLAB variabile of type mxINT32
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine/MEX
 *
 * DESCRIPTION:  
 *
 *    Copys 2-d data from C array to MATLAB variable for output
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny inputArray outputArray
 *
 *       create pointer to mxArray (int) 
 *       get pointer
 *
 *       copy data to pointer
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    mxCreateNumericMatrix
 *    mxGetData
 *
 * CALLING SEQUENCE:  
 *
 *    copy_to_MATLAB_int(nx,ny,inputArray,outputMEXArray)
 *
 * INPUTS:  
 *
 *     nx               - X dimension of input array
 *     ny               - Y dimension of input array
 *     inputArray       - pointer to (VINT *) inputArray
 *
 * OUTPUTS:  
 * 
 *     outputMEXArray   - pointer to output MEX array (mexArray)
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 03/12/2011
 *
 */
void copy_to_MATLAB_int( const VINT nx, const VINT ny, const VINT *pArray, 
			 mxArray **ppMxData )
{
  /* Local variables */
  VINT i = 0;
  VINT *p_outmat = NULL;

  /* Create MATLAB variable */
  *ppMxData = mxCreateNumericMatrix( nx, ny, mxINT32_CLASS, mxREAL );

  /* Get pointer to copy data to */
  p_outmat = (VINT *) mxGetData( *ppMxData );
  
  /* Copy data over */
  for(i=0;i<nx*ny;i++){
    *(p_outmat+i) = *(pArray+i);
  } 

}
