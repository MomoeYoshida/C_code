/*
 * NAME: 
 *
 *    process_raw_amsr_c
 *
 * FUNCTION:
 *
 *    Header for process_raw_amsr_c.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine from reading in AMSR data c version 
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_amsr_c  - process the raw AMSR data
 *
 * ROUTINES USED INTERNALLY:
 *
 *    get_day_night_cld_mask
 *    add_to_storage
 *    init_storage
 *    setup_storage
 *    free_storage
 *    cleanup_plus_mean_stdev_sst_data
 *    clean_stdev
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef PROCESS_RAW_AMSR_HEADER

/* Include type definitions */
#include "types_cnsts.h"

/* structure for SST storage - stored all AMSR SSTs in single place as
* linked lists */
struct SST_Storage {
  VINT nx;
  VINT ny;
  VINT *pNMax;
  VINT *pNStored;
  struct float_struct1d *pSST;
  struct float_struct1d *pSST_Variance;
};

/* external routine */
VINT process_raw_amsr_c( const char *amsr_name, const VINT year, const VINT day, 
				const struct sst_struct *pRef_SST, 
				const struct sst_struct *pSST_Variability, 
				const VINT sst_check, const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				struct int_struct *pDumparray,
				VINT *pTotal_badclim,
				VINT *pTotal_badstd,
				const struct par_files_str par_files );

#define PROCESS_RAW_AMSR_HEADER 1
#endif

