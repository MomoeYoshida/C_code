/*
 * NAME: 
 *
 *    init_par_info
 *
 * FUNCTION:
 *
 *    Header for init_par_info routines and types
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Defines the par_info type and has associated read functions to read in 
 *    paraameters from parameters.param file
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */

#ifndef INIT_PAR_INFO_HEADER

#include "types_cnsts.h"

/* par_info structure to hold all data */
struct par_info_type {
  VINT correct_diurnal_warming;
  char noaa16_acspo_format[MAX_STRING_LENGTH];
  char noaa17_acspo_format[MAX_STRING_LENGTH];
  char noaa18_acspo_format[MAX_STRING_LENGTH];
  char noaa19_acspo_format[MAX_STRING_LENGTH];
  char metopa_acspo_format[MAX_STRING_LENGTH];
  char metopb_acspo_format[MAX_STRING_LENGTH];
  char metopc_acspo_format[MAX_STRING_LENGTH];
  char viirs_acspo_format[MAX_STRING_LENGTH];
  char jpss_acspo_format[MAX_STRING_LENGTH];
  char slstr_format[MAX_STRING_LENGTH];
  char geo_format[MAX_STRING_LENGTH];
  char amsr_format[MAX_STRING_LENGTH];
  VINT acspo_ghrsst_min_quality;
  VINT slstr_ghrsst_min_quality;
  VINT geo_ghrsst_min_quality;
  VINT amsr_ghrsst_min_quality;
  VINT slstr_ghrsst_min_algorithm;
  VFLOAT slstr_ghrsst_min_windspeed;
  VINT noaa16_sses_bias;
  VFLOAT noaa16_sses_stdev;
  VINT noaa16_use_sses_stdev;
  VINT noaa17_sses_bias;
  VFLOAT noaa17_sses_stdev;
  VINT noaa17_use_sses_stdev;
  VINT noaa18_sses_bias;
  VFLOAT noaa18_sses_stdev;
  VINT noaa18_use_sses_stdev;
  VINT noaa19_sses_bias;
  VFLOAT noaa19_sses_stdev;
  VINT noaa19_use_sses_stdev;
  VINT metopa_sses_bias;
  VFLOAT metopa_sses_stdev;
  VINT metopa_use_sses_stdev;
  VINT metopb_sses_bias;
  VFLOAT metopb_sses_stdev;
  VINT metopb_use_sses_stdev;
  VINT metopc_sses_bias;
  VFLOAT metopc_sses_stdev;
  VINT metopc_use_sses_stdev;
  VINT viirs_sses_bias;
  VFLOAT viirs_sses_stdev;
  VINT viirs_use_sses_stdev;
  VINT jpss_sses_bias;
  VFLOAT jpss_sses_stdev;
  VINT jpss_use_sses_stdev;
  VINT slstra_sses_bias;
  VFLOAT slstra_sses_stdev;
  VINT slstra_use_sses_stdev;
  VINT slstrb_sses_bias;
  VFLOAT slstrb_sses_stdev;
  VINT slstrb_use_sses_stdev;
  VINT goese_sses_bias;
  VFLOAT goese_sses_stdev;
  VINT goese_use_sses_stdev;
  VINT goesw_sses_bias;
  VFLOAT goesw_sses_stdev;
  VINT goesw_use_sses_stdev;
  VINT goesp_sses_bias;
  VFLOAT goesp_sses_stdev;
  VINT goesp_use_sses_stdev;
  VINT mtsat_sses_bias;
  VFLOAT mtsat_sses_stdev;
  VINT mtsat_use_sses_stdev;
  VINT msg_sses_bias;
  VFLOAT msg_sses_stdev;
  VINT msg_use_sses_stdev;
  VINT mio_sses_bias;
  VFLOAT mio_sses_stdev;
  VINT mio_use_sses_stdev;
  VINT amsr_sses_bias;
  VFLOAT amsr_sses_stdev;
  VINT amsr_use_sses_stdev;
  VFLOAT abs_zero;
  VFLOAT bad_val;
  VFLOAT max_obs_deviation;
  VFLOAT correlation_scaling;
  VFLOAT correlation_min;
  VFLOAT correlation_max;
  VFLOAT oi_corr_parm_001[3];
  VFLOAT oi_corr_parm_002[1];
  VFLOAT oi_corr_parm_003[1];
  VINT oi_function_type_001;
  VINT oi_function_type_002;
  VINT oi_function_type_003;
  VFLOAT sst_variability_scaling;
  VFLOAT obs_variation_max;
  VFLOAT sst_variability_min;
  VFLOAT sst_variability_max;
  VFLOAT sst_variability_weighting[2];
  VFLOAT sst_analysis_min;
  VFLOAT sst_analysis_max;
  VINT oi_function_type;
  VFLOAT oi_density;
  VINT oi_nweight;
  VFLOAT analysis_smoothing_factor;
  VFLOAT error_smoothing_factor;
  VFLOAT bias_smoothing_factor;
  VFLOAT bias_weighting_factor[2];
  VINT spatial_resolution[2];
  VINT cells_per_degree;
  VFLOAT ref_latmin;
  VFLOAT ref_lonmin;
  VFLOAT rtg_fac[2];
  VFLOAT error_val_max;
  char tenth_or_twentieth[MAX_STRING_LENGTH];
  VFLOAT clim_threshold_1;
  VFLOAT clim_threshold_2;
  VFLOAT clim_threshold_var_factor;
  VFLOAT default_goes_std_p;
  VFLOAT default_goes_std_e;
  VFLOAT default_goes_std_w;
  VFLOAT default_msg_std;
  VFLOAT default_mio_std;
  VFLOAT default_mtsat_std;
  VFLOAT default_amsr_std;
  VFLOAT amsr_thin_factor;	/*  For possible future use  */
  VFLOAT goes_threshold_mult;
  VFLOAT goes_threshold_constant;
  VFLOAT msg_threshold_mult;
  VFLOAT msg_threshold_constant;
  VFLOAT mio_threshold_mult;
  VFLOAT mio_threshold_constant;
  VFLOAT mtsat_threshold_mult;
  VFLOAT mtsat_threshold_constant;
  VFLOAT amsr_day_threshold_mult;
  VFLOAT amsr_day_threshold_constant;
  VFLOAT amsr_night_threshold_mult;
  VFLOAT amsr_night_threshold_constant;
  VFLOAT noaa16_day_c0_threshold_mult;
  VFLOAT noaa16_day_c0_threshold_constant;
  VFLOAT noaa16_day_c1_threshold_mult;
  VFLOAT noaa16_day_c1_threshold_constant;
  VFLOAT noaa16_night_c0_threshold_mult;
  VFLOAT noaa16_night_c0_threshold_constant;
  VFLOAT noaa16_night_c1_threshold_mult;
  VFLOAT noaa16_night_c1_threshold_constant;
  VFLOAT noaa17_day_c0_threshold_mult;
  VFLOAT noaa17_day_c0_threshold_constant;
  VFLOAT noaa17_day_c1_threshold_mult;
  VFLOAT noaa17_day_c1_threshold_constant;
  VFLOAT noaa17_night_c0_threshold_mult;
  VFLOAT noaa17_night_c0_threshold_constant;
  VFLOAT noaa17_night_c1_threshold_mult;
  VFLOAT noaa17_night_c1_threshold_constant;
  VFLOAT noaa18_day_c0_threshold_mult;
  VFLOAT noaa18_day_c0_threshold_constant;
  VFLOAT noaa18_day_c1_threshold_mult;
  VFLOAT noaa18_day_c1_threshold_constant;
  VFLOAT noaa18_night_c0_threshold_mult;
  VFLOAT noaa18_night_c0_threshold_constant;
  VFLOAT noaa18_night_c1_threshold_mult;
  VFLOAT noaa18_night_c1_threshold_constant;
  VFLOAT noaa19_day_c0_threshold_mult;
  VFLOAT noaa19_day_c0_threshold_constant;
  VFLOAT noaa19_day_c1_threshold_mult;
  VFLOAT noaa19_day_c1_threshold_constant;
  VFLOAT noaa19_night_c0_threshold_mult;
  VFLOAT noaa19_night_c0_threshold_constant;
  VFLOAT noaa19_night_c1_threshold_mult;
  VFLOAT noaa19_night_c1_threshold_constant;
  VFLOAT METOPA_day_c0_threshold_mult;
  VFLOAT METOPA_day_c0_threshold_constant;
  VFLOAT METOPA_day_c1_threshold_mult;
  VFLOAT METOPA_day_c1_threshold_constant;
  VFLOAT METOPA_night_c0_threshold_mult;
  VFLOAT METOPA_night_c0_threshold_constant;
  VFLOAT METOPA_night_c1_threshold_mult;
  VFLOAT METOPA_night_c1_threshold_constant;
  VFLOAT METOPB_day_c0_threshold_mult;
  VFLOAT METOPB_day_c0_threshold_constant;
  VFLOAT METOPB_day_c1_threshold_mult;
  VFLOAT METOPB_day_c1_threshold_constant;
  VFLOAT METOPB_night_c0_threshold_mult;
  VFLOAT METOPB_night_c0_threshold_constant;
  VFLOAT METOPB_night_c1_threshold_mult;
  VFLOAT METOPB_night_c1_threshold_constant;
  VFLOAT METOPC_day_c0_threshold_mult;
  VFLOAT METOPC_day_c0_threshold_constant;
  VFLOAT METOPC_day_c1_threshold_mult;
  VFLOAT METOPC_day_c1_threshold_constant;
  VFLOAT METOPC_night_c0_threshold_mult;
  VFLOAT METOPC_night_c0_threshold_constant;
  VFLOAT METOPC_night_c1_threshold_mult;
  VFLOAT METOPC_night_c1_threshold_constant;
  VFLOAT VIIRS_day_c0_threshold_mult;
  VFLOAT VIIRS_day_c0_threshold_constant;
  VFLOAT VIIRS_day_c1_threshold_mult;
  VFLOAT VIIRS_day_c1_threshold_constant;
  VFLOAT VIIRS_night_c0_threshold_mult;
  VFLOAT VIIRS_night_c0_threshold_constant;
  VFLOAT VIIRS_night_c1_threshold_mult;
  VFLOAT VIIRS_night_c1_threshold_constant;
  VFLOAT JPSS_day_c0_threshold_mult;
  VFLOAT JPSS_day_c0_threshold_constant;
  VFLOAT JPSS_day_c1_threshold_mult;
  VFLOAT JPSS_day_c1_threshold_constant;
  VFLOAT JPSS_night_c0_threshold_mult;
  VFLOAT JPSS_night_c0_threshold_constant;
  VFLOAT JPSS_night_c1_threshold_mult;
  VFLOAT JPSS_night_c1_threshold_constant;
  VFLOAT slstra_day_threshold_mult;
  VFLOAT slstra_day_threshold_constant;
  VFLOAT slstra_night_threshold_mult;
  VFLOAT slstra_night_threshold_constant;
  VFLOAT slstrb_day_threshold_mult;
  VFLOAT slstrb_day_threshold_constant;
  VFLOAT slstrb_night_threshold_mult;
  VFLOAT slstrb_night_threshold_constant;
  VFLOAT noaa16_day_c0_std_default;
  VFLOAT noaa16_night_c0_std_default;
  VFLOAT noaa16_day_c1_std_default;
  VFLOAT noaa16_night_c1_std_default;
  VFLOAT noaa17_day_c0_std_default;
  VFLOAT noaa17_night_c0_std_default;
  VFLOAT noaa17_day_c1_std_default;
  VFLOAT noaa17_night_c1_std_default;
  VFLOAT noaa18_day_c0_std_default;
  VFLOAT noaa18_night_c0_std_default;
  VFLOAT noaa18_day_c1_std_default;
  VFLOAT noaa18_night_c1_std_default;
  VFLOAT noaa19_day_c0_std_default;
  VFLOAT noaa19_night_c0_std_default;
  VFLOAT noaa19_day_c1_std_default;
  VFLOAT noaa19_night_c1_std_default;
  VFLOAT METOPA_day_c0_std_default;
  VFLOAT METOPA_night_c0_std_default;
  VFLOAT METOPA_day_c1_std_default;
  VFLOAT METOPA_night_c1_std_default;
  VFLOAT METOPB_day_c0_std_default;
  VFLOAT METOPB_night_c0_std_default;
  VFLOAT METOPB_day_c1_std_default;
  VFLOAT METOPB_night_c1_std_default;
  VFLOAT METOPC_day_c0_std_default;
  VFLOAT METOPC_night_c0_std_default;
  VFLOAT METOPC_day_c1_std_default;
  VFLOAT METOPC_night_c1_std_default;
  VFLOAT VIIRS_day_c0_std_default;
  VFLOAT VIIRS_night_c0_std_default;
  VFLOAT VIIRS_day_c1_std_default;
  VFLOAT VIIRS_night_c1_std_default;
  VFLOAT JPSS_day_c0_std_default;
  VFLOAT JPSS_night_c0_std_default;
  VFLOAT JPSS_day_c1_std_default;
  VFLOAT JPSS_night_c1_std_default;
  VFLOAT slstra_day_std_default;
  VFLOAT slstra_night_std_default;
  VFLOAT slstrb_day_std_default;
  VFLOAT slstrb_night_std_default;
  VFLOAT amsr_day_std_default;
  VFLOAT amsr_night_std_default;
  VFLOAT std_threshold;
  VINT min_geo_obs_per_cell;
  VINT min_avhrr_obs_per_cell;
  VINT goes_e_xsize;
  VINT goes_e_ysize;
  VFLOAT goes_e_minlon;
  VFLOAT goes_e_maxlon;
  VFLOAT goes_e_minlat;
  VFLOAT goes_e_maxlat;
  VINT goes_w_xsize;
  VINT goes_w_ysize;
  VFLOAT goes_w_minlon;
  VFLOAT goes_w_maxlon;
  VFLOAT goes_w_minlat;
  VFLOAT goes_w_maxlat;
  VINT goes_p_xsize;
  VINT goes_p_ysize;
  VFLOAT goes_p_minlon;
  VFLOAT goes_p_maxlon;
  VFLOAT goes_p_minlat;
  VFLOAT goes_p_maxlat;
  VINT mtsat_xsize;
  VINT mtsat_ysize;
  VFLOAT mtsat_minlon;
  VFLOAT mtsat_maxlon;
  VFLOAT mtsat_minlat;
  VFLOAT mtsat_maxlat;
  VINT msg_xsize;
  VINT msg_ysize;
  VFLOAT msg_minlon;
  VFLOAT msg_maxlon;
  VFLOAT msg_minlat;
  VFLOAT msg_maxlat;
  VINT mio_xsize;
  VINT mio_ysize;
  VFLOAT mio_minlon;
  VFLOAT mio_maxlon;
  VFLOAT mio_minlat;
  VFLOAT mio_maxlat;
  VINT start_day_night;
  VINT end_day_night;
};

/* This has visibility outside this module */
extern struct par_info_type par_info;

/* prototypes */
void init_par_info( const char *parInfoFile );

#define INIT_PAR_INFO_HEADER 1
#endif
