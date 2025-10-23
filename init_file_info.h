/*
 * NAME: 
 *
 *    init_file_info
 *
 * FUNCTION:
 *
 *    Header file for init_file_info routines
 *
 * TYPE/LANGUAGE: 
 *
 *    C header file
 *
 * DESCRIPTION:  
 *
 *    Contains headers and parameters to define where and which files
 *    the code reads/writes and logs to
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef INIT_FILE_INFO_HEADER

#include "types_cnsts.h"

/* Structure that contains all file_par data */
struct file_info_str {
  VINT logfile_required;
  char name_logfile[MAX_STRING_LENGTH];
  char blended_sst_home[MAX_STRING_LENGTH];
  /* */
  char dir_metopa_sst[MAX_STRING_LENGTH];
  char dir_metopa_nav[MAX_STRING_LENGTH];     
  char dir_metopb_sst[MAX_STRING_LENGTH];
  char dir_metopb_nav[MAX_STRING_LENGTH];     
  char dir_metopc_sst[MAX_STRING_LENGTH];
  char dir_metopc_nav[MAX_STRING_LENGTH];     
  char dir_viirs_sst[MAX_STRING_LENGTH];
  char dir_viirs_nav[MAX_STRING_LENGTH];     
  char dir_jpss_sst[MAX_STRING_LENGTH];
  char dir_jpss_nav[MAX_STRING_LENGTH];     
  char dir_slstra_sst[MAX_STRING_LENGTH];
  char dir_slstra_nav[MAX_STRING_LENGTH];     
  char dir_slstrb_sst[MAX_STRING_LENGTH];
  char dir_slstrb_nav[MAX_STRING_LENGTH];     
  char dir_noaa_16_sst[MAX_STRING_LENGTH];   
  char dir_noaa_16_nav[MAX_STRING_LENGTH];   
  char dir_noaa_17_sst[MAX_STRING_LENGTH];   
  char dir_noaa_17_nav[MAX_STRING_LENGTH];   
  char dir_noaa_18_sst[MAX_STRING_LENGTH];   
  char dir_noaa_18_nav[MAX_STRING_LENGTH];   
  char dir_noaa_19_sst[MAX_STRING_LENGTH];   
  char dir_noaa_19_nav[MAX_STRING_LENGTH];   
  char dir_goes[MAX_STRING_LENGTH];
  char dir_msg[MAX_STRING_LENGTH];
  char dir_mio[MAX_STRING_LENGTH];
  char dir_mtsat[MAX_STRING_LENGTH];
  char dir_amsr[MAX_STRING_LENGTH];
  /* */
  char dir_ms_read_grib[MAX_STRING_LENGTH];
  char dir_ms_overlap[MAX_STRING_LENGTH];
  char dir_ms_smoother[MAX_STRING_LENGTH];
  char dir_ms_statecorr[MAX_STRING_LENGTH];
  char dir_ms_executable[MAX_STRING_LENGTH];
  /* */
  char dir_rtg_lowres[MAX_STRING_LENGTH];
  char dir_rtg_hires[MAX_STRING_LENGTH];
  char name_rtg_lowres[MAX_STRING_LENGTH];
  char name_rtg_hires[MAX_STRING_LENGTH];
  /* */
  char dir_input_ssts[MAX_STRING_LENGTH];
  char dir_analysis[MAX_STRING_LENGTH];
  char dir_ancillary[MAX_STRING_LENGTH];
  char dir_coastwatch[MAX_STRING_LENGTH];
  char dir_GHRSST[MAX_STRING_LENGTH];
  char dir_Diurnal[MAX_STRING_LENGTH];
  /* */
  char name_Diurnal[MAX_STRING_LENGTH];
  char name_sst_analysis[MAX_STRING_LENGTH];
  char name_error_analysis[MAX_STRING_LENGTH];
  char name_sst_variability[MAX_STRING_LENGTH];
  char name_ice_mask[MAX_STRING_LENGTH];
  char name_correlation_map[MAX_STRING_LENGTH];
  char name_biases[MAX_STRING_LENGTH];
  char name_land_mask[MAX_STRING_LENGTH];
  char name_oi_oceans_coupling[MAX_STRING_LENGTH];
  char name_oi_state_values[MAX_STRING_LENGTH];
  char name_oi_scales[MAX_STRING_LENGTH];
  char name_coastwatch_file[MAX_STRING_LENGTH];
  char name_sst_biases[MAX_STRING_LENGTH];
  /* */
  VINT var_n_datasets;
  /* Note the memory is assigned in the reader */
  char **name_dataset;
  /* */
  char dir_ice_lowres[MAX_STRING_LENGTH];
  char dir_ice_hires[MAX_STRING_LENGTH];
  char name_ice_lowres[MAX_STRING_LENGTH];
  char name_ice_hires[MAX_STRING_LENGTH];
  /* */
  char dir_ostia[MAX_STRING_LENGTH];
  char name_ostia[MAX_STRING_LENGTH];
};

/* file_info has visibility outside this routine */
extern struct file_info_str file_info;

/* Functions */
void init_file_info( const char *fileInfoFile );
VINT find_key(FILE *fp, const char *pKey, char *pString);
void free_file_info( void );

#define INIT_FILE_INFO_HEADER 1
#endif
