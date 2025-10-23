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

#ifndef PROCESS_RAW_AVHRR_ACSPO_HEADER

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
/*Riley*/
void cleanup_plus_mean_stdev_sst_data( const struct SST_Storage *pStored_SST, 
					      struct out_sst_str *pOutdata, 
					      const VFLOAT stdev_threshold, 
					      const VFLOAT stdev_default, 
					      VINT *pNBadStd, VINT use_sses_stdev );

void add_to_storage(const VFLOAT sst, const VFLOAT variance, 
			   const VINT pos, struct SST_Storage *pStorage);
void init_storage(struct SST_Storage *pStorage);
void setup_storage(const VINT nx, const VINT ny, struct SST_Storage *pStorage);
void free_storage(struct SST_Storage *pStorage);
VFLOAT clean_stdev( struct float_struct1d *pArray,
			   struct float_struct1d *pArray_Var,
			   const VINT nstored,
			   const VFLOAT stdev_threshold, 
			   const VFLOAT stdev_default, 
			   struct float_struct1d *pSorted,
			   struct float_struct1d *pSorted_Var,
			   VINT *pNDump, 
			   VFLOAT *pMean, VINT use_sses_stdev );
void get_day_night_cld_mask( const struct char_struct *pCldmask, 
				    const struct float_struct *pSST,
				    const VINT day_or_night, char cmask, 
				    const VFLOAT sst_min,
				    struct char_struct *pNewMask,
				    VINT *pNgood );
/* external routine */
VINT process_raw_avhrr_acspo_c( const char *noaa_name, const VINT year, const VINT day, 
				const struct sst_struct *pRef_SST, 
				const struct sst_struct *pSST_Variability, 
				const VINT sst_check, const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				struct int_struct *pDumparray,
				VINT *pTotal_badclim,
				VINT *pTotal_badstd,
				const struct par_files_str par_files );

#define PROCESS_RAW_AVHRR_ACSPO_HEADER 1
#endif

