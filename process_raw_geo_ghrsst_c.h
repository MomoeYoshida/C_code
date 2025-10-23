/*
 * NAME: 
 *
 *    process_raw_avhrr_acspo_c
 *
 * FUNCTION:
 *
 *    Header for process_raw_avhrr_acspo_c.c
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Contains the header routine from reading in ACSPO data c version 
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_avhrr_acspo_c  - process the raw ACSPO data
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

#ifndef PROCESS_RAW_GEO_GHRSST_HEADER

/* Include type definitions */
#include "types_cnsts.h"

/* structure for SST storage - stored all ACSPO SSTs in single place as
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
VINT process_raw_geo_ghrsst_c( const char *pType, const VINT year, const VINT day, 
				const struct sst_struct *pRef_SST, 
				const struct sst_struct *pSST_Variability, 
				const VINT sst_check, const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				const struct par_files_str par_files );

#define PROCESS_RAW_GEOGHRSST_HEADER 1
#endif

