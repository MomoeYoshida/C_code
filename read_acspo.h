/*
 * NAME: 
 *
 *    read_acspo.h
 *
 * FUNCTION:
 *
 *    Header for read_acspo.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine to read in acspo data
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_acspo  - reads the ACSPO hdf data
 *
 * ROUTINES USED INTERNALLY:
 *
 *    readHdfData_flt  - read in float data
 *    readHdfData_byte - read in byte data
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef READ_ACSPO_HEADER

/* Include type definitions */
#include "types_cnsts.h"
#include "init_par_info.h"

/* external interface */
VINT read_acspo( const char *pFilename, const char *pAcspo_format,
		 struct float_struct *pSST, 
		 struct float_struct *pLat, struct float_struct *Lon, 
		 struct char_struct  *pCldmask,
		 struct float_struct *pSSES_Variance,
		 const struct par_info_type par_info,
		 const VINT correct_bias, const VFLOAT sses_stdev );

#define READ_ACSPO_HEADER 1
#endif
