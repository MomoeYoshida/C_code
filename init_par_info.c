/*
 * NAME: 
 *
 *    init_par_info
 *
 * FUNCTION:
 *
 *    C version of init_par_info routine
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Reads and sets the par_info structure by reading in par_info.m file
 *    Data stored in par_info structure accessable by other routines via 
 *    init_par_info.h.
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    init_par_info
 *
 * ROUTINES USED INTERNALLY:
 *
 *    parse_float_values
 *    parse_int_values
 *    parse_float_values_1
 *    parse_float_values_2
 *    parse_float_values_3
 *    parse_int_values
 *    parse_int_values_1
 *    parse_int_values_2
 *    parse_int_values_3
 *    number_of_commas
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "init_par_info.h"
#include "init_file_info.h"
#include "logfile.h"

/* Functions for internal use only */
static VINT parse_float_values(const char *pString, VFLOAT *pVar, VFLOAT *pVar2, 
			       VFLOAT *pVar3);
static VINT parse_int_values(const char *pString, VINT *pVar, VINT *pVar2, VINT *pVar3);
static VINT parse_float_values_1(const char *pString, VFLOAT *pVar );
static VINT parse_float_values_2(const char *pString, VFLOAT *pVar, VFLOAT *pVar2);
static VINT parse_float_values_3(const char *pString, VFLOAT *pVar, VFLOAT *pVar2, 
				 VFLOAT *pVar3);
static VINT parse_int_values(const char *pString, VINT *pVar, VINT *pVar2, 
			     VINT *pVar3);
static VINT parse_int_values_1(const char *pString, VINT *pVar);
static VINT parse_int_values_2(const char *pString, VINT *pVar, VINT *pVar2);
static VINT parse_int_values_3(const char *pString, VINT *pVar, VINT *pVar2, 
			       VINT *pVar3);
static VINT number_of_commas(const char *pString);

/* variables for internal global use */
static VINT init_run = 0;  /* Flag to make sure we only do this once */

/* variable for external (global) use */
struct par_info_type par_info;

/*
 * NAME: 
 *
 *    init_par_info
 *
 * FUNCTION:
 *
 *    C Code to setup parameters - version of init_par_info.m
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Set up parameters for processing.  Reads in a file containing
 *    parameters (init_par_info.m).  The init_par_info.m file requires some
 *    extra parameters (see PSEUDO CODE) beyond the old MATLAB code.
 *
 *    Note has to deal with a number of cases to parse out information
 *
 *       read in number, both float and integer
 *       read in array of numbers (up to three) e.g. [8, 16, 32]
 *          - uses the parse routines
 *       read in string
 *
 *    Note also that all longitudes are converted to -180,180 range
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE parInfoFile
 *
 *       IF already run THEN
 *          RETURN
 *       ENDIF
 *
 *       open parInfoFile
 *
 *       get correct for diurnal warming
 *       get NOAA16 ACSPO format
 *       get NOAA17 ACSPO format
 *       get NOAA18 ACSPO format
 *       get NOAA19 ACSPO format
 *       get METOPA ACSPO format
 *       get METOPB ACSPO format
 *       get VIIRS ACSPO format
 *       get JPSS ACSPO format
 *       get GEO format
 *       get use NOAA16 SSES bias
 *       get use NOAA16 SSES stdev
 *       get NOAA16 SSES default stdev
 *       get use NOAA17 SSES bias
 *       get use NOAA17 SSES stdev
 *       get NOAA17 SSES default stdev
 *       get use NOAA18 SSES bias
 *       get use NOAA18 SSES stdev
 *       get NOAA18 SSES default stdev
 *       get use NOAA19 SSES bias
 *       get use NOAA19 SSES stdev
 *       get NOAA19 SSES default stdev
 *       get use METOPA SSES bias
 *       get use METOPA SSES stdev
 *       get METOPA SSES default stdev
 *       get use METOPB SSES bias
 *       get use METOPB SSES stdev
 *       get METOPB SSES default stdev
 *       get use METOPC SSES bias
 *       get use METOPC SSES stdev
 *       get METOPC SSES default stdev
 *       get use VIIRS SSES bias
 *       get use VIIRS SSES stdev
 *       get VIIRS SSES default stdev
 *       get use JPSS SSES bias
 *       get use JPSS SSES stdev
 *       get JPSS SSES default stdev
 *       get use goesE SSES bias
 *       get use goesE SSES stdev
 *       get goesE SSES default stdev
 *       get use goesW SSES bias
 *       get use goesW SSES stdev
 *       get goesW SSES default stdev
 *       get use MTSAT SSES bias
 *       get use MTSAT SSES stdev
 *       get MTSAT SSES default stdev
 *       get use MSG SSES bias
 *       get use MSG SSES stdev
 *       get MSG SSES default stdev
 *       get ACSPO minimum GHRSST qaulity value
 *       get GEO minimum GHRSST qaulity value
 *       get abs_zero
 *       get bad_val
 *       get max_obs_deviation
 *       get correlation_scaling
 *       get correlation_min
 *       get correlation_max
 *       get oi_corr_parm_001
 *       get oi_corr_parm_002
 *       get oi_corr_parm_003
 *       get oi_function_type_001
 *       get oi_function_type_002
 *       get oi_function_type_003
 *       get sst_variability_scaling
 *       get obs_variation_max
 *       get sst_variability_min
 *       get sst_variability_max
 *       get sst_variability_weighting
 *       get sst_analysis_min
 *       get sst_analysis_max
 *       get oi_function_type
 *       get oi_density
 *       get oi_nweight
 *       get analysis_smoothing_factor
 *       get error_smoothing_factor
 *       get bias_smoothing_factor
 *       get bias_weighting_factor
 *       get spatial_resolution
 *       calculate cells_per_degree - this is a new parameter
 *       get rtg_fac
 *       get error_val_max
 *       get tenth_or_twentieth
 *       get clim_threshold_1
 *       get clim_threshold_2
 *       get clim_threshold_var_factor
 *       get default_goes_std_p
 *       get default_goes_std_e
 *       get default_goes_std_w
 *       get default_msg_std
 *       get default_mtsat_std
 *       get goes_threshold_mult
 *       get goes_threshold_constant
 *       get msg_threshold_mult
 *       get msg_threshold_constant
 *       get mtsat_threshold_mult
 *       get mtsat_threshold_constant
 *       get noaa17_day_c0_threshold_mult
 *       get noaa17_day_c0_threshold_constant
 *       get METOPA_day_c0_threshold_mult
 *       get METOPA_day_c0_threshold_constant
 *       get VIIRS_day_c0_threshold_mult
 *       get VIIRS_day_c0_threshold_constant
 *       get JPSS_day_c0_threshold_mult
 *       get JPSS_day_c0_threshold_constant
 *       get noaa17_day_c1_threshold_mult
 *       get noaa17_day_c1_threshold_constant
 *       get noaa17_night_c0_threshold_mult
 *       get noaa17_night_c0_threshold_constant
 *       get METOPA_night_c0_threshold_mult
 *       get METOPA_night_c0_threshold_constant
 *       get VIIRS_night_c0_threshold_mult
 *       get VIIRS_night_c0_threshold_constant
 *       get JPSS_night_c0_threshold_mult
 *       get JPSS_night_c0_threshold_constant
 *       get noaa17_night_c1_threshold_mult
 *       get noaa17_night_c1_threshold_constant
 *       get noaa16_day_c0_threshold_mult
 *       get noaa16_day_c0_threshold_constant
 *       get noaa16_day_c1_threshold_mult
 *       get noaa16_day_c1_threshold_constant
 *       get noaa16_night_c0_threshold_mult
 *       get noaa16_night_c0_threshold_constant
 *       get noaa16_night_c1_threshold_mult
 *       get noaa16_night_c1_threshold_constant
 *       get noaa18_day_c0_threshold_mult
 *       get noaa18_day_c0_threshold_constant
 *       get noaa18_day_c1_threshold_mult
 *       get noaa18_day_c1_threshold_constant
 *       get noaa18_night_c0_threshold_mult
 *       get noaa18_night_c0_threshold_constant
 *       get noaa18_night_c1_threshold_mult
 *       get noaa18_night_c1_threshold_constant
 *       get noaa19_day_c0_threshold_mult
 *       get noaa19_day_c0_threshold_constant
 *       get noaa19_day_c1_threshold_mult
 *       get noaa19_day_c1_threshold_constant
 *       get noaa19_night_c0_threshold_mult
 *       get noaa19_night_c0_threshold_constant
 *       get noaa19_night_c1_threshold_mult
 *       get noaa19_night_c1_threshold_constant
 *       get noaa17_day_c0_std_default
 *       get noaa17_night_c0_std_default
 *       get METOPA_day_c0_std_default
 *       get METOPA_night_c0_std_default
 *       get VIIRS_day_c0_std_default
 *       get VIIRS_night_c0_std_default
 *       get JPSS_day_c0_std_default
 *       get JPSS_night_c0_std_default
 *       get noaa17_day_c1_std_default
 *       get noaa17_night_c1_std_default
 *       get noaa18_day_c0_std_default
 *       get noaa18_night_c0_std_default
 *       get noaa18_day_c1_std_default
 *       get noaa18_night_c1_std_default
 *       get noaa19_day_c0_std_default
 *       get noaa19_night_c0_std_default
 *       get noaa19_day_c1_std_default
 *       get noaa19_night_c1_std_default
 *       get std_threshold
 *       get min_geo_obs_per_cell
 *       get min_avhrr_obs_per_cell
 * 
 *       NOTE below are extra parameters for the C version
 *
 *       get ref_latmin
 *       get ref_lonmin
 *       get goes_e_xsize
 *       get goes_e_ysize
 *       get goes_e_minlat
 *       get goes_e_maxlat
 *       get goes_e_minlon
 *       get goes_e_maxlon
 *       get goes_w_xsize
 *       get goes_w_ysize
 *       get goes_w_minlat
 *       get goes_w_maxlat
 *       get goes_w_minlon
 *       get goes_w_maxlon
 *       get goes_p_xsize
 *       get goes_p_ysize
 *       get goes_p_minlat
 *       get goes_p_maxlat
 *       get goes_p_minlon
 *       get goes_p_maxlon
 *       get msg_xsize
 *       get msg_ysize
 *       get msg_minlat
 *       get msg_maxlat
 *       get msg_minlon
 *       get msg_maxlon
 *       get mtsat_xsize
 *       get mtsat_ysize
 *       get mtsat_minlat
 *       get mtsat_maxlat
 *       get mtsat_minlon
 *       get mtsat_maxlon
 *       get start_day_night
 *       get end_day_night
 *
 *       close file
 *
 *       check for missing entries
 *
 *       set init flag
 *
 *    END
 *
 * FILES NEEDED: 
 *
 *    parInfoFIle - e.g. init_par_info.m
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    find_key
 *    parse_float_values
 *
 * CALLING SEQUENCE:  
 *
 *    init_par_info( parInfoFile )
 *
 * INPUTS:  
 *
 *    parInfoFile - name of par_info_file (e.g. init_par_info.m. 
 *                  init_par_info_5KM.m)
 *    
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    fopen
 *    fclose
 *    sizeof
 *    sscanf
 *    strncpy
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
void init_par_info( const char *parInfoFile )
{
  VINT ok = 0;
  VINT ndata = 0;
  VINT i = 0;

  char string[MAX_STRING_LENGTH];
  char format_spec[MAX_STRING_LENGTH];

  FILE *fp = NULL;

  /* If we have run this before, return */
  if( 1 == init_run ){
    /* EXIT already setup parameters */
    return;
  }

  /* set format string depending on VFLOAT type */
  if( 8 == sizeof(VFLOAT) ){
    strncpy(format_spec,"%lf",MAX_STRING_LENGTH);
  } else {
    strncpy(format_spec,"%f",MAX_STRING_LENGTH);
  }

  /* Open top level file information */
  if( NULL == (fp = fopen(parInfoFile,"r")) ){
    fprintf(stdout, "%s\n", parInfoFile);	/*  To see full path  */
    message(1,"ERROR: Cannot open init_par_info.m file ^^");
    /* EXIT */
    exit(-1);
  }
  
  /* Read in required parameters */
  ok = 0;

  /* Apply diurnal correction? 1=yes 0=no */
  if( 1 == find_key(fp,"correct_diurnal_warming",string) ){
    message(1,"correct_diurnal_warming keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.correct_diurnal_warming) ){
      message(1,"correct_diurnal_warming keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.correct_diurnal_warming) && (1 != par_info.correct_diurnal_warming) ){
      message(1,"correct_diurnal_warming keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Get ACPSO format - either HDF or NETCDF or GHRSST */
  /* NOAA 16 */
  if( 1 == find_key(fp,"noaa16_acspo_format",string) ){
    message(1,"noaa16_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.noaa16_acspo_format[i] = toupper(string[i]);
    }
  }

  /* NOAA 17 */
  if( 1 == find_key(fp,"noaa17_acspo_format",string) ){
    message(1,"noaa17_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.noaa17_acspo_format[i] = toupper(string[i]);
    }
  }

  /* NOAA 18 */
  if( 1 == find_key(fp,"noaa18_acspo_format",string) ){
    message(1,"noaa18_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.noaa18_acspo_format[i] = toupper(string[i]);
    }
  }

  /* NOAA 16 */
  if( 1 == find_key(fp,"noaa19_acspo_format",string) ){
    message(1,"noaa19_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.noaa19_acspo_format[i] = toupper(string[i]);
    }
  }

  /* METOPA */
  if( 1 == find_key(fp,"metopa_acspo_format",string) ){
    message(1,"metopa_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.metopa_acspo_format[i] = toupper(string[i]);
    }
  }

  /* METOPB */
  if( 1 == find_key(fp,"metopb_acspo_format",string) ){
    message(1,"metopb_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.metopb_acspo_format[i] = toupper(string[i]);
    }
  }

  /* METOPC */
  if( 1 == find_key(fp,"metopc_acspo_format",string) ){
    message(1,"metopc_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.metopc_acspo_format[i] = toupper(string[i]);
    }
  }

  /* VIIRS */
  if( 1 == find_key(fp,"viirs_acspo_format",string) ){
    message(1,"viirs_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.viirs_acspo_format[i] = toupper(string[i]);
    }
  }

  /* JPSS */
  if( 1 == find_key(fp,"jpss_acspo_format",string) ){
    message(1,"jpss_acspo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.jpss_acspo_format[i] = toupper(string[i]);
    }
  }

  /* SLSTR */
  if( 1 == find_key(fp,"slstr_format",string) ){
    message(1,"slstr_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.slstr_format[i] = toupper(string[i]);
    }
  }

  /* Use ACSPO SSES Bias (GHRSST format only)? 0 = no, 1 = yes */
  if( 1 == find_key(fp,"noaa16_sses_bias",string) ){
    message(1,"noaa16_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa16_sses_bias) ){
      message(1,"noaa16_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa16_sses_bias) && (1 != par_info.noaa16_sses_bias) ){
      message(1,"noaa16_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_sses_bias",string) ){
    message(1,"noaa17_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa17_sses_bias) ){
      message(1,"noaa17_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa17_sses_bias) && (1 != par_info.noaa17_sses_bias) ){
      message(1,"noaa17_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_sses_bias",string) ){
    message(1,"noaa18_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa18_sses_bias) ){
      message(1,"noaa18_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa18_sses_bias) && (1 != par_info.noaa18_sses_bias) ){
      message(1,"noaa18_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_sses_bias",string) ){
    message(1,"noaa19_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa19_sses_bias) ){
      message(1,"noaa19_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa19_sses_bias) && (1 != par_info.noaa19_sses_bias) ){
      message(1,"noaa19_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopa_sses_bias",string) ){
    message(1,"metopa_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopa_sses_bias) ){
      message(1,"metopa_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopa_sses_bias) && (1 != par_info.metopa_sses_bias) ){
      message(1,"metopa_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopb_sses_bias",string) ){
    message(1,"metopb_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopb_sses_bias) ){
      message(1,"metopb_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopb_sses_bias) && (1 != par_info.metopb_sses_bias) ){
      message(1,"metopb_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopc_sses_bias",string) ){
    message(1,"metopc_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopc_sses_bias) ){
      message(1,"metopc_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopc_sses_bias) && (1 != par_info.metopc_sses_bias) ){
      message(1,"metopc_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"viirs_sses_bias",string) ){
    message(1,"viirs_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.viirs_sses_bias) ){
      message(1,"viirs_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.viirs_sses_bias) && (1 != par_info.viirs_sses_bias) ){
      message(1,"viirs_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"jpss_sses_bias",string) ){
    message(1,"jpss_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.jpss_sses_bias) ){
      message(1,"jpss_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.jpss_sses_bias) && (1 != par_info.jpss_sses_bias) ){
      message(1,"jpss_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_sses_bias",string) ){
    message(1,"slstra_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstra_sses_bias) ){
      message(1,"slstra_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.slstra_sses_bias) && (1 != par_info.slstra_sses_bias) ){
      message(1,"slstra_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_sses_bias",string) ){
    message(1,"slstrb_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstrb_sses_bias) ){
      message(1,"slstrb_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.slstrb_sses_bias) && (1 != par_info.slstrb_sses_bias) ){
      message(1,"slstrb_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Get use SSES Stdev (zero don't use, one use) */
  if( 1 == find_key(fp,"noaa16_use_sses_stdev",string) ){
    message(1,"noaa16_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa16_use_sses_stdev) ){
      message(1,"noaa16_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa16_use_sses_stdev) && (1 != par_info.noaa16_use_sses_stdev) ){
      message(1,"noaa16_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_use_sses_stdev",string) ){
    message(1,"noaa17_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa17_use_sses_stdev) ){
      message(1,"noaa17_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa17_use_sses_stdev) && (1 != par_info.noaa17_use_sses_stdev) ){
      message(1,"noaa17_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_use_sses_stdev",string) ){
    message(1,"noaa18_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa18_use_sses_stdev) ){
      message(1,"noaa18_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa18_use_sses_stdev) && (1 != par_info.noaa18_use_sses_stdev) ){
      message(1,"noaa18_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_use_sses_stdev",string) ){
    message(1,"noaa19_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.noaa19_use_sses_stdev) ){
      message(1,"noaa19_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.noaa19_use_sses_stdev) && (1 != par_info.noaa19_use_sses_stdev) ){
      message(1,"noaa19_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopa_use_sses_stdev",string) ){
    message(1,"metopa_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopa_use_sses_stdev) ){
      message(1,"metopa_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopa_use_sses_stdev) && (1 != par_info.metopa_use_sses_stdev) ){
      message(1,"metopa_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopb_use_sses_stdev",string) ){
    message(1,"metopb_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopb_use_sses_stdev) ){
      message(1,"metopb_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopb_use_sses_stdev) && (1 != par_info.metopb_use_sses_stdev) ){
      message(1,"metopb_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopc_use_sses_stdev",string) ){
    message(1,"metopc_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.metopc_use_sses_stdev) ){
      message(1,"metopc_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.metopc_use_sses_stdev) && (1 != par_info.metopc_use_sses_stdev) ){
      message(1,"metopc_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"viirs_use_sses_stdev",string) ){
    message(1,"viirs_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.viirs_use_sses_stdev) ){
      message(1,"viirs_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.viirs_use_sses_stdev) && (1 != par_info.viirs_use_sses_stdev) ){
      message(1,"viirs_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"jpss_use_sses_stdev",string) ){
    message(1,"jpss_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.jpss_use_sses_stdev) ){
      message(1,"jpss_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.jpss_use_sses_stdev) && (1 != par_info.jpss_use_sses_stdev) ){
      message(1,"jpss_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_use_sses_stdev",string) ){
    message(1,"slstra_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstra_use_sses_stdev) ){
      message(1,"slstra_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.slstra_use_sses_stdev) && (1 != par_info.slstra_use_sses_stdev) ){
      message(1,"slstra_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_use_sses_stdev",string) ){
    message(1,"slstrb_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstrb_use_sses_stdev) ){
      message(1,"slstrb_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.slstrb_use_sses_stdev) && (1 != par_info.slstrb_use_sses_stdev) ){
      message(1,"slstrb_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Get SSES Stdev default (added if SSES does not exist */
  if( 1 == find_key(fp,"noaa16_sses_stdev",string) ){
    message(1,"noaa16_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa16_sses_stdev) ){
      message(1,"noaa16_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_sses_stdev",string) ){
    message(1,"noaa17_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa17_sses_stdev) ){
      message(1,"noaa17_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_sses_stdev",string) ){
    message(1,"noaa18_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa18_sses_stdev) ){
      message(1,"noaa18_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_sses_stdev",string) ){
    message(1,"noaa19_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa19_sses_stdev) ){
      message(1,"noaa19_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopa_sses_stdev",string) ){
    message(1,"metopa_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.metopa_sses_stdev) ){
      message(1,"metopa_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopb_sses_stdev",string) ){
    message(1,"metopb_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.metopb_sses_stdev) ){
      message(1,"metopb_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"metopc_sses_stdev",string) ){
    message(1,"metopc_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.metopc_sses_stdev) ){
      message(1,"metopc_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"viirs_sses_stdev",string) ){
    message(1,"viirs_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.viirs_sses_stdev) ){
      message(1,"viirs_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"jpss_sses_stdev",string) ){
    message(1,"jpss_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.jpss_sses_stdev) ){
      message(1,"jpss_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_sses_stdev",string) ){
    message(1,"slstra_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.slstra_sses_stdev) ){
      message(1,"slstra_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_sses_stdev",string) ){
    message(1,"slstrb_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.slstrb_sses_stdev) ){
      message(1,"slstrb_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  /* Get GEO format - either BINARY or GHRSST */
  if( 1 == find_key(fp,"geo_format",string) ){
    message(1,"geo_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.geo_format[i] = toupper(string[i]);
    }
  }

  /* Uee GEO SSES Bias (GHRSST format only)? 0 = no, 1 = yes */
  if( 1 == find_key(fp,"goese_sses_bias",string) ){
    message(1,"goese_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goese_sses_bias) ){
      message(1,"goese_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goese_sses_bias) && (1 != par_info.goese_sses_bias) ){
      message(1,"goese_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesw_sses_bias",string) ){
    message(1,"goesw_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goesw_sses_bias) ){
      message(1,"goesw_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goesw_sses_bias) && (1 != par_info.goesw_sses_bias) ){
      message(1,"goesw_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesp_sses_bias",string) ){
    message(1,"goesp_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goesp_sses_bias) ){
      message(1,"goesp_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goesp_sses_bias) && (1 != par_info.goesp_sses_bias) ){
      message(1,"goesp_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_sses_bias",string) ){
    message(1,"mtsat_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mtsat_sses_bias) ){
      message(1,"mtsat_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.mtsat_sses_bias) && (1 != par_info.mtsat_sses_bias) ){
      message(1,"mtsat_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_sses_bias",string) ){
    message(1,"msg_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.msg_sses_bias) ){
      message(1,"msg_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.msg_sses_bias) && (1 != par_info.msg_sses_bias) ){
      message(1,"msg_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_sses_bias",string) ){
    message(1,"mio_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mio_sses_bias) ){
      message(1,"mio_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.mio_sses_bias) && (1 != par_info.mio_sses_bias) ){
      message(1,"mio_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* GEO SSES default Stdev */
  if( 1 == find_key(fp,"goese_sses_stdev",string) ){
    message(1,"goese_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goese_sses_stdev) ){
      message(1,"goese_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesw_sses_stdev",string) ){
    message(1,"goesw_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goesw_sses_stdev) ){
      message(1,"goesw_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesp_sses_stdev",string) ){
    message(1,"goesp_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goesp_sses_stdev) ){
      message(1,"goesp_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_sses_stdev",string) ){
    message(1,"mtsat_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_sses_stdev) ){
      message(1,"mtsat_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_sses_stdev",string) ){
    message(1,"msg_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_sses_stdev) ){
      message(1,"msg_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_sses_stdev",string) ){
    message(1,"mio_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_sses_stdev) ){
      message(1,"mio_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
  }

  /* Use GEO SSES Stdev */
  if( 1 == find_key(fp,"goese_use_sses_stdev",string) ){
    message(1,"goese_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goese_use_sses_stdev) ){
      message(1,"goese_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goese_use_sses_stdev) && (1 != par_info.goese_use_sses_stdev) ){
      message(1,"goese_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesw_use_sses_stdev",string) ){
    message(1,"goesw_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goesw_use_sses_stdev) ){
      message(1,"goesw_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goesw_use_sses_stdev) && (1 != par_info.goesw_use_sses_stdev) ){
      message(1,"goesw_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goesp_use_sses_stdev",string) ){
    message(1,"goesp_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goesp_use_sses_stdev) ){
      message(1,"goesp_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.goesp_use_sses_stdev) && (1 != par_info.goesp_use_sses_stdev) ){
      message(1,"goesp_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_use_sses_stdev",string) ){
    message(1,"mtsat_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mtsat_use_sses_stdev) ){
      message(1,"mtsat_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.mtsat_use_sses_stdev) && (1 != par_info.mtsat_use_sses_stdev) ){
      message(1,"mtsat_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_use_sses_stdev",string) ){
    message(1,"msg_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.msg_use_sses_stdev) ){
      message(1,"msg_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.msg_use_sses_stdev) && (1 != par_info.msg_use_sses_stdev) ){
      message(1,"msg_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_use_sses_stdev",string) ){
    message(1,"mio_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mio_use_sses_stdev) ){
      message(1,"mio_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.mio_use_sses_stdev) && (1 != par_info.mio_use_sses_stdev) ){
      message(1,"mio_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Get AMSR format - should just be GHRSST */
  if( 1 == find_key(fp,"amsr_format",string) ){
    message(1,"amsr_format keyword not found in parameters file");
    ok = 1;
  } else {
    /* Make sure format string is uppercase */
    for(i=0;i<strlen(string);i++){
      par_info.amsr_format[i] = toupper(string[i]);
    }
  }

  /* Use AMSR SSES Bias (GHRSST format only)? 0 = no, 1 = yes */
  if( 1 == find_key(fp,"amsr_sses_bias",string) ){
    message(1,"amsr_sses_bias keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.amsr_sses_bias) ){
      message(1,"amsr_sses_bias keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.amsr_sses_bias) && (1 != par_info.amsr_sses_bias) ){
      message(1,"amsr_sses_bias keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Use AMSR SSES Stdev */
  if( 1 == find_key(fp,"amsr_use_sses_stdev",string) ){
    message(1,"amsr_use_sses_stdev keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.amsr_use_sses_stdev) ){
      message(1,"amsr_use_sses_stdev keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0 != par_info.amsr_use_sses_stdev) && (1 != par_info.amsr_use_sses_stdev) ){
      message(1,"amsr_use_sses_stdev keyword not 0/1 in parameters file");
      ok = 1;
    }
  }

  /* Get ACSPO minimum quality flag value */
  if( 1 == find_key(fp,"acspo_ghrsst_min_quality",string) ){
    message(1,"acspo_ghrsst_min_quality keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.acspo_ghrsst_min_quality) ){
      message(1,"acspo_ghrsst_min_quality keyword not parsable in parameters file");
      ok = 1;
    }
    if( (1 > par_info.acspo_ghrsst_min_quality) || 
	(5 < par_info.acspo_ghrsst_min_quality) ){
      message(1,"acspo_ghrsst_min_quality keyword not between 1 & 5 in parameters file");
      ok = 1;
    }
  }

  /* Get GEO minimum quality flag value */
  if( 1 == find_key(fp,"geo_ghrsst_min_quality",string) ){
    message(1,"geo_ghrsst_min_quality keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.geo_ghrsst_min_quality) ){
      message(1,"geo_ghrsst_min_quality keyword not parsable in parameters file");
      ok = 1;
    }
    if( (1 > par_info.geo_ghrsst_min_quality) || 
	(5 < par_info.geo_ghrsst_min_quality) ){
      message(1,"geo_ghrsst_min_quality keyword not between 1 & 5 in parameters file");
      ok = 1;
    }
  }

  /* Get AMSR minimum quality flag value */
  if( 1 == find_key(fp,"amsr_ghrsst_min_quality",string) ){
    message(1,"amsr_ghrsst_min_quality keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.amsr_ghrsst_min_quality) ){
      message(1,"amsr_ghrsst_min_quality keyword not parsable in parameters file");
      ok = 1;
    }
    if( (1 > par_info.amsr_ghrsst_min_quality) || 
	(5 < par_info.amsr_ghrsst_min_quality) ){
      message(1,"amsr_ghrsst_min_quality keyword not between 1 & 5 in parameters file");
      ok = 1;
    }
  }

  /* Get SLSTR minimum quality flag value */
  if( 1 == find_key(fp,"slstr_ghrsst_min_quality",string) ){
    message(1,"slstr_ghrsst_min_quality keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstr_ghrsst_min_quality) ){
      message(1,"slstr_ghrsst_min_quality keyword not parsable in parameters file");
      ok = 1;
    }
    if( (1 > par_info.slstr_ghrsst_min_quality) || 
	(5 < par_info.slstr_ghrsst_min_quality) ){
      message(1,"slstr_ghrsst_min_quality keyword not between 1 & 5 in parameters file");
      ok = 1;
    }
  }

  /* Get SLSTR minimum algorithm (algorithm_type = 1 2 3 4 5 -> N2 N3R N3 D2 D3) */
  if( 1 == find_key(fp,"slstr_ghrsst_min_algorithm",string) ){
    message(1,"slstr_ghrsst_min_algorithm keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.slstr_ghrsst_min_algorithm) ){
      message(1,"slstr_ghrsst_min_algorithm keyword not parsable in parameters file");
      ok = 1;
    }
    if( (1 > par_info.slstr_ghrsst_min_algorithm) || 
	(5 < par_info.slstr_ghrsst_min_algorithm) ){
      message(1,"slstr_ghrsst_min_algorithm keyword not between 1 & 5 in parameters file");
      ok = 1;
    }
  }

  /* Get SLSTR minimum windspeed (for screening of low wind daytime obs) */
  if( 1 == find_key(fp,"slstr_ghrsst_min_windspeed",string) ){
    message(1,"slstr_ghrsst_min_windspeed keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.slstr_ghrsst_min_windspeed) ){
      message(1,"slstr_ghrsst_min_windspeed keyword not parsable in parameters file");
      ok = 1;
    }
    if( (0. > par_info.slstr_ghrsst_min_windspeed) || 
	(10. < par_info.slstr_ghrsst_min_windspeed) ){
      message(1,"slstr_ghrsst_min_windspeed keyword not between 0 & 10 in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"abs_zero",string) ){
    message(1,"abs_zero keyword not found in parameters file");
    ok = 1;
  } else {
    sscanf(string,format_spec,&par_info.abs_zero);
  }

  if( 1 == find_key(fp,"bad_val",string) ){
    message(1,"bad_val keyword not found in parameters file");
    ok = 1;
  } else {
    sscanf(string,format_spec,&par_info.bad_val);
  }

  if( 1 == find_key(fp,"max_obs_deviation",string) ){
    message(1,"max_obs_deviation keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.max_obs_deviation) ){
      message(1,"ERROR: Parsing max_obs_deviation (init_par_infp)");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"correlation_scaling",string) ){
    message(1,"correlation_scaling keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.correlation_scaling) ){
      message(1,"correlation_scaling keyword not found in oarameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"correlation_min",string) ){
    message(1,"correlation_min keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.correlation_min) ){
      message(1,"correlation_min keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"correlation_max",string) ){
    message(1,"correlation_max keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.correlation_max) ){
      message(1,"correlation_max keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_corr_parm_001",string) ){
    message(1,"oi_corr_parm_001[3] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			       &par_info.oi_corr_parm_001[0],
			       &par_info.oi_corr_parm_001[1],
			       &par_info.oi_corr_parm_001[2]);
    if( -1 == ndata ){
      message(1,"oi_corr_parm_001 parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_corr_parm_002",string) ){
    message(1,"oi_corr_parm_002[1] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			       &par_info.oi_corr_parm_002[0],
			       &par_info.oi_corr_parm_002[1],
			       &par_info.oi_corr_parm_002[2]);
    if( -1 == ndata ){
      message(1,"oi_corr_parm_002 parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_corr_parm_003",string) ){
    message(1,"oi_corr_parm_003[1] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			       &par_info.oi_corr_parm_003[0],
			       &par_info.oi_corr_parm_003[1],
			       &par_info.oi_corr_parm_003[2]);
    if( -1 == ndata ){
      message(1,"oi_corr_parm_003 parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_function_type_001",string) ){
    message(1,"oi_function_type_001 keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.oi_function_type_001) ){
      message(1,"oi_function_type_001 keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_function_type_002",string) ){
    message(1,"oi_function_type_002 keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.oi_function_type_002) ){
      message(1,"oi_function_type_002 keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_function_type_003",string) ){
    message(1,"oi_function_type_003 keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.oi_function_type_003) ){
      message(1,"oi_function_type_003 keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_variability_scaling",string) ){
    message(1,"sst_variability_scaling keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.sst_variability_scaling) ){
      message(1,"sst_variability_scaling keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"obs_variation_max",string) ){
    message(1,"obs_variation_max keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.obs_variation_max) ){
      message(1,"obs_variation_max keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_variability_min",string) ){
    message(1,"sst_variability_min keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.sst_variability_min) ){
      message(1,"sst_variability_min keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_variability_max",string) ){
    message(1,"sst_variability_max keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.sst_variability_max) ){
      message(1,"sst_variability_max keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_variability_weighting",string) ){
    message(1,"sst_variability_weighting[2] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			       &par_info.sst_variability_weighting[0],
			       &par_info.sst_variability_weighting[1],
			       &par_info.sst_variability_weighting[2]);
    if( -1 == ndata ){
      message(1,"sst_variability_weighting parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_analysis_min",string) ){
    message(1,"sst_analysis_min keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.sst_analysis_min) ){
      message(1,"sst_analysis_min keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"sst_analysis_max",string) ){
    message(1,"sst_analysis_max keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.sst_analysis_max) ){
      message(1,"sst_analysis_max keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_function_type",string) ){
    message(1,"oi_function_type keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.oi_function_type) ){
      message(1,"oi_function_type keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_density",string) ){
    message(1,"oi_density keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.oi_density) ){
      message(1,"oi_density keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"oi_nweight",string) ){
    message(1,"oi_nweight keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.oi_nweight) ){
      message(1,"oi_nweight keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"analysis_smoothing_factor",string) ){
    message(1,"analysis_smoothing_factor keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.analysis_smoothing_factor) ){
      message(1,"analysis_smoothing_factor keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"error_smoothing_factor",string) ){
    message(1,"error_smoothing_factor keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.error_smoothing_factor) ){
      message(1,"error_smoothing_factor keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"bias_smoothing_factor",string) ){
    message(1,"bias_smoothing_factor keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.bias_smoothing_factor) ){
      message(1,"bias_smoothing_factor keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"bias_weighting_factor",string) ){
    message(1,"bias_weighting_factor[2] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			       &par_info.bias_weighting_factor[0],
			       &par_info.bias_weighting_factor[1],
			       &par_info.bias_weighting_factor[2]);
    if( -1 == ndata ){
      message(1,"bias_weighting_factor parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"spatial_resolution",string) ){
    message(1,"spatial_resolution[2] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_int_values(string,
			     &par_info.spatial_resolution[0],
			     &par_info.spatial_resolution[1],
			     &par_info.spatial_resolution[2]);
    if( -1 == ndata ){
      message(1,"spatial_resolution parse failed");
      ok = 1;
    }
  }

  /* Calculate cells_per_degree (note parameter does not exist in 
   * original 11km analysis MATLAB */
  par_info.cells_per_degree = par_info.spatial_resolution[0]/180.;

  if( 1 == find_key(fp,"cells_per_degree",string) ){
    message(1,"cells_per_degree keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.cells_per_degree) ){
      message(1,"cells_per_degree keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"rtg_fac",string) ){
    message(1,"rtg_fac[2] keyword not found in parameters file");
    ok = 1;
  } else {
    ndata = parse_float_values(string,
			     &par_info.rtg_fac[0],
			     &par_info.rtg_fac[1],
			     &par_info.rtg_fac[2]);
    if( -1 == ndata ){
      message(1,"rtg_fac parse failed");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"error_val_max",string) ){
    message(1,"error_val_max keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.error_val_max) ){
      message(1,"error_val_max keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"tenth_or_twentieth",string) ){
    message(1,"tenth_or_twentieth keyword not found in parameters file");
    ok = 1;
  } else {
    strncpy(par_info.tenth_or_twentieth,string,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"clim_threshold_1",string) ){
    message(1,"clim_threshold_1 keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.clim_threshold_1) ){
      message(1,"clim_threshold_1 keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"clim_threshold_2",string) ){
    message(1,"clim_threshold_2 keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.clim_threshold_2) ){
      message(1,"clim_threshold_2 keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"clim_threshold_var_factor",string) ){
    message(1,"clim_threshold_var_factor keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.clim_threshold_var_factor) ){
      message(1,"clim_threshold_var_factor keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_goes_std_p",string) ){
    message(1,"default_goes_std_p keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_goes_std_p) ){
      message(1,"default_goes_std_p keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_goes_std_e",string) ){
    message(1,"default_goes_std_e keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_goes_std_e) ){
      message(1,"default_goes_std_e keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_goes_std_w",string) ){
    message(1,"default_goes_std_w keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_goes_std_w) ){
      message(1,"default_goes_std_w keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_msg_std",string) ){
    message(1,"default_msg_std keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_msg_std) ){
      message(1,"default_msg_std keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_mio_std",string) ){
    message(1,"default_mio_std keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_mio_std) ){
      message(1,"default_mio_std keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_mtsat_std",string) ){
    message(1,"default_mtsat_std keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_mtsat_std) ){
      message(1,"default_mtsat_std keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"default_amsr_std",string) ){
    message(1,"default_amsr_std keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.default_amsr_std) ){
      message(1,"default_amsr_std keyword not found in parameters file");
      ok = 1;
    }
  }

/*  Parse AMSR thinning factor for now, even though it will initially only be used outside MEX routines  */

  if( 1 == find_key(fp,"amsr_thin_factor",string) ){
    message(1,"amsr_thin_factor keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.amsr_thin_factor) ){
      message(1,"amsr_thin_factor keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_threshold_mult",string) ){
    message(1,"goes_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_threshold_mult) ){
      message(1,"goes_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_threshold_constant",string) ){
    message(1,"goes_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_threshold_constant) ){
      message(1,"goes_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_threshold_mult",string) ){
    message(1,"msg_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_threshold_mult) ){
      message(1,"msg_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_threshold_constant",string) ){
    message(1,"msg_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_threshold_constant) ){
      message(1,"msg_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_threshold_mult",string) ){
    message(1,"mio_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_threshold_mult) ){
      message(1,"mio_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_threshold_constant",string) ){
    message(1,"mio_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_threshold_constant) ){
      message(1,"mio_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_threshold_mult",string) ){
    message(1,"mtsat_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_threshold_mult) ){
      message(1,"mtsat_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_threshold_constant",string) ){
    message(1,"mtsat_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_threshold_constant) ){
      message(1,"mtsat_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"amsr_day_threshold_mult",string) ){
    message(1,"amsr_day_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.amsr_day_threshold_mult) ){
      message(1,"amsr_day_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"amsr_day_threshold_constant",string) ){
    message(1,"amsr_day_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.amsr_day_threshold_constant) ){
      message(1,"amsr_day_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"amsr_night_threshold_mult",string) ){
    message(1,"amsr_night_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.amsr_night_threshold_mult) ){
      message(1,"amsr_night_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"amsr_night_threshold_constant",string) ){
    message(1,"amsr_night_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.amsr_night_threshold_constant) ){
      message(1,"amsr_night_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c0_threshold_mult",string) ){
    message(1,"noaa17_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa17_day_c0_threshold_mult) ){
      message(1,"noaa17_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c0_threshold_constant",string) ){
    message(1,"noaa17_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_day_c0_threshold_constant) ){
      message(1,"noaa17_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_day_c0_threshold_mult",string) ){
    message(1,"METOPA_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_day_c0_threshold_mult) ){
      message(1,"METOPA_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_day_c0_threshold_constant",string) ){
    message(1,"METOPA_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_day_c0_threshold_constant) ){
      message(1,"METOPA_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_day_c0_threshold_mult",string) ){
    message(1,"METOPB_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_day_c0_threshold_mult) ){
      message(1,"METOPB_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_day_c0_threshold_constant",string) ){
    message(1,"METOPB_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_day_c0_threshold_constant) ){
      message(1,"METOPB_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_day_c0_threshold_mult",string) ){
    message(1,"METOPC_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_day_c0_threshold_mult) ){
      message(1,"METOPC_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_day_c0_threshold_constant",string) ){
    message(1,"METOPC_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_day_c0_threshold_constant) ){
      message(1,"METOPC_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_day_c0_threshold_mult",string) ){
    message(1,"VIIRS_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_day_c0_threshold_mult) ){
      message(1,"VIIRS_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_day_c0_threshold_constant",string) ){
    message(1,"VIIRS_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_day_c0_threshold_constant) ){
      message(1,"VIIRS_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_day_c0_threshold_mult",string) ){
    message(1,"JPSS_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_day_c0_threshold_mult) ){
      message(1,"JPSS_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_day_c0_threshold_constant",string) ){
    message(1,"JPSS_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_day_c0_threshold_constant) ){
      message(1,"JPSS_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3A SLSTR */

  if( 1 == find_key(fp,"slstra_day_threshold_mult",string) ){
    message(1,"slstra_day_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_day_threshold_mult) ){
      message(1,"slstra_day_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_day_threshold_constant",string) ){
    message(1,"slstra_day_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_day_threshold_constant) ){
      message(1,"slstra_day_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3B SLSTR */

  if( 1 == find_key(fp,"slstrb_day_threshold_mult",string) ){
    message(1,"slstrb_day_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_day_threshold_mult) ){
      message(1,"slstrb_day_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_day_threshold_constant",string) ){
    message(1,"slstrb_day_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_day_threshold_constant) ){
      message(1,"slstrb_day_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c1_threshold_mult",string) ){
    message(1,"noaa17_day_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_day_c1_threshold_mult) ){
      message(1,"noaa17_day_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c1_threshold_constant",string) ){
    message(1,"noaa17_day_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_day_c1_threshold_constant) ){
      message(1,"noaa17_day_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c0_threshold_mult",string) ){
    message(1,"noaa17_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c0_threshold_mult) ){
      message(1,"noaa17_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c0_threshold_constant",string) ){
    message(1,"noaa17_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c0_threshold_constant) ){
      message(1,"noaa17_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_night_c0_threshold_mult",string) ){
    message(1,"METOPA_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_night_c0_threshold_mult) ){
      message(1,"METOPA_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_night_c0_threshold_constant",string) ){
    message(1,"METOPA_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_night_c0_threshold_constant) ){
      message(1,"METOPA_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_night_c0_threshold_mult",string) ){
    message(1,"METOPB_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_night_c0_threshold_mult) ){
      message(1,"METOPB_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_night_c0_threshold_constant",string) ){
    message(1,"METOPB_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_night_c0_threshold_constant) ){
      message(1,"METOPB_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_night_c0_threshold_mult",string) ){
    message(1,"METOPC_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_night_c0_threshold_mult) ){
      message(1,"METOPC_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_night_c0_threshold_constant",string) ){
    message(1,"METOPC_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_night_c0_threshold_constant) ){
      message(1,"METOPC_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_night_c0_threshold_mult",string) ){
    message(1,"VIIRS_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_night_c0_threshold_mult) ){
      message(1,"VIIRS_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_night_c0_threshold_constant",string) ){
    message(1,"VIIRS_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_night_c0_threshold_constant) ){
      message(1,"VIIRS_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_night_c0_threshold_mult",string) ){
    message(1,"JPSS_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_night_c0_threshold_mult) ){
      message(1,"JPSS_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_night_c0_threshold_constant",string) ){
    message(1,"JPSS_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_night_c0_threshold_constant) ){
      message(1,"JPSS_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3A SLSTR */

  if( 1 == find_key(fp,"slstra_night_threshold_mult",string) ){
    message(1,"slstra_night_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_night_threshold_mult) ){
      message(1,"slstra_night_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_night_threshold_constant",string) ){
    message(1,"slstra_night_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_night_threshold_constant) ){
      message(1,"slstra_night_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3B SLSTR */

  if( 1 == find_key(fp,"slstrb_night_threshold_mult",string) ){
    message(1,"slstrb_night_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_night_threshold_mult) ){
      message(1,"slstrb_night_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_night_threshold_constant",string) ){
    message(1,"slstrb_night_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_night_threshold_constant) ){
      message(1,"slstrb_night_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c1_threshold_mult",string) ){
    message(1,"noaa17_night_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c1_threshold_mult) ){
      message(1,"noaa17_night_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c1_threshold_constant",string) ){
    message(1,"noaa17_night_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c1_threshold_constant) ){
      message(1,"noaa17_night_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_day_c0_threshold_mult",string) ){
    message(1,"noaa16_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_day_c0_threshold_mult) ){
      message(1,"noaa16_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_day_c0_threshold_constant",string) ){
    message(1,"noaa16_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_day_c0_threshold_constant) ){
      message(1,"noaa16_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_day_c1_threshold_mult",string) ){
    message(1,"noaa16_day_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_day_c1_threshold_mult) ){
      message(1,"noaa16_day_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_day_c1_threshold_constant",string) ){
    message(1,"noaa16_day_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_day_c1_threshold_constant) ){
      message(1,"noaa16_day_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_night_c0_threshold_mult",string) ){
    message(1,"noaa16_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_night_c0_threshold_mult) ){
      message(1,"noaa16_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_night_c0_threshold_constant",string) ){
    message(1,"noaa16_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_night_c0_threshold_constant) ){
      message(1,"noaa16_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_night_c1_threshold_mult",string) ){
    message(1,"noaa16_night_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_night_c1_threshold_mult) ){
      message(1,"noaa16_night_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa16_night_c1_threshold_constant",string) ){
    message(1,"noaa16_night_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa16_night_c1_threshold_constant) ){
      message(1,"noaa16_night_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c0_threshold_mult",string) ){
    message(1,"noaa18_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_day_c0_threshold_mult) ){
      message(1,"noaa18_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c0_threshold_constant",string) ){
    message(1,"noaa18_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_day_c0_threshold_constant) ){
      message(1,"noaa18_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c1_threshold_mult",string) ){
    message(1,"noaa18_day_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_day_c1_threshold_mult) ){
      message(1,"noaa18_day_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c1_threshold_constant",string) ){
    message(1,"noaa18_day_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_day_c1_threshold_constant) ){
      message(1,"noaa18_day_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c0_threshold_mult",string) ){
    message(1,"noaa18_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c0_threshold_mult) ){
      message(1,"noaa18_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c0_threshold_constant",string) ){
    message(1,"noaa18_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c0_threshold_constant) ){
      message(1,"noaa18_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c1_threshold_mult",string) ){
    message(1,"noaa18_night_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c1_threshold_mult) ){
      message(1,"noaa18_night_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c1_threshold_constant",string) ){
    message(1,"noaa18_night_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c1_threshold_constant) ){
      message(1,"noaa18_night_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c0_threshold_mult",string) ){
    message(1,"noaa19_day_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_day_c0_threshold_mult) ){
      message(1,"noaa19_day_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c0_threshold_constant",string) ){
    message(1,"noaa19_day_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_day_c0_threshold_constant) ){
      message(1,"noaa19_day_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c1_threshold_mult",string) ){
    message(1,"noaa19_day_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_day_c1_threshold_mult) ){
      message(1,"noaa19_day_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c1_threshold_constant",string) ){
    message(1,"noaa19_day_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_day_c1_threshold_constant) ){
      message(1,"noaa19_day_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c0_threshold_mult",string) ){
    message(1,"noaa19_night_c0_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c0_threshold_mult) ){
      message(1,"noaa19_night_c0_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c0_threshold_constant",string) ){
    message(1,"noaa19_night_c0_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c0_threshold_constant) ){
      message(1,"noaa19_night_c0_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c1_threshold_mult",string) ){
    message(1,"noaa19_night_c1_threshold_mult keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c1_threshold_mult) ){
      message(1,"noaa19_night_c1_threshold_mult keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c1_threshold_constant",string) ){
    message(1,"noaa19_night_c1_threshold_constant keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c1_threshold_constant) ){
      message(1,"noaa19_night_c1_threshold_constant keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c0_std_default",string) ){
    message(1,"noaa17_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_day_c0_std_default) ){
      message(1,"noaa17_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c0_std_default",string) ){
    message(1,"noaa17_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c0_std_default) ){
      message(1,"noaa17_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_day_c0_std_default",string) ){
    message(1,"METOPA_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_day_c0_std_default) ){
      message(1,"METOPA_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPA_night_c0_std_default",string) ){
    message(1,"METOPA_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPA_night_c0_std_default) ){
      message(1,"METOPA_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_day_c0_std_default",string) ){
    message(1,"METOPB_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_day_c0_std_default) ){
      message(1,"METOPB_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPB_night_c0_std_default",string) ){
    message(1,"METOPB_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPB_night_c0_std_default) ){
      message(1,"METOPB_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_day_c0_std_default",string) ){
    message(1,"METOPC_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_day_c0_std_default) ){
      message(1,"METOPC_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"METOPC_night_c0_std_default",string) ){
    message(1,"METOPC_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.METOPC_night_c0_std_default) ){
      message(1,"METOPC_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_day_c0_std_default",string) ){
    message(1,"VIIRS_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_day_c0_std_default) ){
      message(1,"VIIRS_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"VIIRS_night_c0_std_default",string) ){
    message(1,"VIIRS_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.VIIRS_night_c0_std_default) ){
      message(1,"VIIRS_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_day_c0_std_default",string) ){
    message(1,"JPSS_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_day_c0_std_default) ){
      message(1,"JPSS_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"JPSS_night_c0_std_default",string) ){
    message(1,"JPSS_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.JPSS_night_c0_std_default) ){
      message(1,"JPSS_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3A SLSTR */

  if( 1 == find_key(fp,"slstra_day_std_default",string) ){
    message(1,"slstra_day_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_day_std_default) ){
      message(1,"slstra_day_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstra_night_std_default",string) ){
    message(1,"slstra_night_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstra_night_std_default) ){
      message(1,"slstra_night_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

/* S3B SLSTR */

  if( 1 == find_key(fp,"slstrb_day_std_default",string) ){
    message(1,"slstrb_day_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_day_std_default) ){
      message(1,"slstrb_day_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"slstrb_night_std_default",string) ){
    message(1,"slstrb_night_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.slstrb_night_std_default) ){
      message(1,"slstrb_night_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_day_c1_std_default",string) ){
    message(1,"noaa17_day_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_day_c1_std_default) ){
      message(1,"noaa17_day_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa17_night_c1_std_default",string) ){
    message(1,"noaa17_night_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa17_night_c1_std_default) ){
      message(1,"noaa17_night_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c0_std_default",string) ){
    message(1,"noaa18_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_day_c0_std_default) ){
      message(1,"noaa18_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c0_std_default",string) ){
    message(1,"noaa18_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c0_std_default) ){
      message(1,"noaa18_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_day_c1_std_default",string) ){
    message(1,"noaa18_day_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa18_day_c1_std_default) ){
      message(1,"noaa18_day_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa18_night_c1_std_default",string) ){
    message(1,"noaa18_night_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa18_night_c1_std_default) ){
      message(1,"noaa18_night_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c0_std_default",string) ){
    message(1,"noaa19_day_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa19_day_c0_std_default) ){
      message(1,"noaa19_day_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c0_std_default",string) ){
    message(1,"noaa19_night_c0_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c0_std_default) ){
      message(1,"noaa19_night_c0_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_day_c1_std_default",string) ){
    message(1,"noaa19_day_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.noaa19_day_c1_std_default) ){
      message(1,"noaa19_day_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"noaa19_night_c1_std_default",string) ){
    message(1,"noaa19_night_c1_std_default keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,
		    &par_info.noaa19_night_c1_std_default) ){
      message(1,"noaa19_night_c1_std_default keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"std_threshold",string) ){
    message(1,"std_threshold keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.std_threshold) ){
      message(1,"std_threshold keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"min_geo_obs_per_cell",string) ){
    message(1,"min_geo_obs_per_cell keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.min_geo_obs_per_cell) ){
      message(1,"min_geo_obs_per_cell keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"min_avhrr_obs_per_cell",string) ){
    message(1,"min_avhrr_obs_per_cell keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.min_avhrr_obs_per_cell) ){
      message(1,"min_avhrr_obs_per_cell keyword not found in parameters file");
      ok = 1;
    }
  }

    /* Momoe: Copy from the other use_** parameter */
    if( 1 == find_key(fp,"thinning",string) ){
      message(1,"thinning keyword not found in parameters file");
      ok = 1;
    } else {
      if( 1 != sscanf(string,"%d",&par_info.thinning) ){
        message(1,"thinning keyword not parsable in parameters file");
        ok = 1;
      }
      if( (0 != par_info.thinning) && (1 != par_info.thinning) ){
        message(1,"noaa16_use_sses_stdev keyword not 0/1 in parameters file");
        ok = 1;
      }
    }

  if( 1 == find_key(fp,"thinning_ratio",string) ){
    message(1,"thinning_ratio keyword not found in parameters file — using default (1.0)");
    ok = 1;
  } else {
      if( 1 != sscanf(string,"%lf",&par_info.thinning_ratio) ){
      message(1,"Invalid thinning_ratio format in parameters file — using default (1.0)");
      ok = 1;
    }
  }
    
    if( 1 == find_key(fp,"seed_base",string) ){
      message(1,"seed_base keyword not found in parameters file");
      ok = 1;
    } else {
      if( 1 != sscanf(string,"%d",&par_info.seed_base) ){
        message(1,"seed_base keyword not found in parameters file");
        ok = 1;
      }
    }

  /* Code below is for the new parameters */

  /* Get the output grid (reference) start position */
  if( 1 == find_key(fp,"ref_latmin",string) ){
    message(1,"ref_latmin keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.ref_latmin) ){
      message(1,"ref_latmin keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"ref_lonmin",string) ){
    message(1,"ref_lonmin keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.ref_lonmin) ){
      message(1,"ref_lonmin keyword not found in parameters file");
      ok = 1;
    }
  }

  /* Here we get the sizes and min/max lat/lons for the geostationary files */

  /* GOES E/W */

  if( 1 == find_key(fp,"goes_e_xsize",string) ){
    message(1,"goes_e_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_e_xsize) ){
      message(1,"goes_e_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_e_ysize",string) ){
    message(1,"goes_e_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_e_ysize) ){
      message(1,"goes_e_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_e_minlon",string) ){
    message(1,"goes_e_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_e_minlon) ){
      message(1,"goes_e_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_e_minlon ){
    par_info.goes_e_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_e_maxlon",string) ){
    message(1,"goes_e_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_e_maxlon) ){
      message(1,"goes_e_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_e_maxlon ){
    par_info.goes_e_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_e_minlat",string) ){
    message(1,"goes_e_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_e_minlat) ){
      message(1,"goes_e_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_e_maxlat",string) ){
    message(1,"goes_e_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_e_maxlat) ){
      message(1,"goes_e_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_w_xsize",string) ){
    message(1,"goes_w_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_w_xsize) ){
      message(1,"goes_w_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_w_ysize",string) ){
    message(1,"goes_w_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_w_ysize) ){
      message(1,"goes_w_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_w_minlon",string) ){
    message(1,"goes_w_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_w_minlon) ){
      message(1,"goes_w_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_w_minlon ){
    par_info.goes_w_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_w_maxlon",string) ){
    message(1,"goes_w_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_w_maxlon) ){
      message(1,"goes_w_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_w_maxlon ){
    par_info.goes_w_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_w_minlat",string) ){
    message(1,"goes_e_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_w_minlat) ){
      message(1,"goes_w_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_w_maxlat",string) ){
    message(1,"goes_w_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_w_maxlat) ){
      message(1,"goes_w_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  /* GOES P */

  if( 1 == find_key(fp,"goes_p_xsize",string) ){
    message(1,"goes_ew_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_p_xsize) ){
      message(1,"goes_p_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_p_ysize",string) ){
    message(1,"goes_p_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.goes_p_ysize) ){
      message(1,"goes_p_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_p_minlon",string) ){
    message(1,"goes_p_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_p_minlon) ){
      message(1,"goes_p_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_p_minlon ){
    par_info.goes_p_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_p_maxlon",string) ){
    message(1,"goes_p_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_p_maxlon) ){
      message(1,"goes_p_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.goes_p_maxlon ){
    par_info.goes_p_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"goes_p_minlat",string) ){
    message(1,"goes_p_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_p_minlat) ){
      message(1,"goes_p_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"goes_p_maxlat",string) ){
    message(1,"goes_p_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.goes_p_maxlat) ){
      message(1,"goes_p_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  /* MSG */

  if( 1 == find_key(fp,"msg_xsize",string) ){
    message(1,"msg_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.msg_xsize) ){
      message(1,"msg_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_ysize",string) ){
    message(1,"msg_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.msg_ysize) ){
      message(1,"msg_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_minlon",string) ){
    message(1,"msg_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_minlon) ){
      message(1,"msg_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.msg_minlon ){
    par_info.msg_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"msg_maxlon",string) ){
    message(1,"msg_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_maxlon) ){
      message(1,"msg_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.msg_maxlon ){
    par_info.msg_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"msg_minlat",string) ){
    message(1,"msg_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_minlat) ){
      message(1,"msg_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"msg_maxlat",string) ){
    message(1,"msg_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.msg_maxlat) ){
      message(1,"msg_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  /* Meteosat Indian Ocean (MIO) */

  if( 1 == find_key(fp,"mio_xsize",string) ){
    message(1,"mio_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mio_xsize) ){
      message(1,"mio_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_ysize",string) ){
    message(1,"mio_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mio_ysize) ){
      message(1,"mio_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_minlon",string) ){
    message(1,"mio_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_minlon) ){
      message(1,"mio_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.mio_minlon ){
    par_info.mio_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"mio_maxlon",string) ){
    message(1,"mio_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_maxlon) ){
      message(1,"mio_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.mio_maxlon ){
    par_info.mio_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"mio_minlat",string) ){
    message(1,"mio_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_minlat) ){
      message(1,"mio_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mio_maxlat",string) ){
    message(1,"mio_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mio_maxlat) ){
      message(1,"mio_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  /* MTSAT */

  if( 1 == find_key(fp,"mtsat_xsize",string) ){
    message(1,"mtsat_xsize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mtsat_xsize) ){
      message(1,"mtsat_xsize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_ysize",string) ){
    message(1,"mtsat_ysize keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.mtsat_ysize) ){
      message(1,"mtsat_ysize keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_minlon",string) ){
    message(1,"mtsat_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_minlon) ){
      message(1,"mtsat_minlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.mtsat_minlon ){
    par_info.mtsat_minlon -= 360.; 
  }

  if( 1 == find_key(fp,"mtsat_maxlon",string) ){
    message(1,"mtsat_maxlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_maxlon) ){
      message(1,"mtsat_maxlon keyword not found in parameters file");
      ok = 1;
    }
  }
  /* Convert to -180,180 if 0,360 */
  /* output analysis is on -180,180 */
  if( 180 < par_info.mtsat_maxlon ){
    par_info.mtsat_maxlon -= 360.; 
  }

  if( 1 == find_key(fp,"mtsat_minlat",string) ){
    message(1,"mtsat_minlon keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_minlat) ){
      message(1,"mtsat_minlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"mtsat_maxlat",string) ){
    message(1,"mtsat_maxlat keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,format_spec,&par_info.mtsat_maxlat) ){
      message(1,"mtsat_maxlat keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"start_day_night",string) ){
    message(1,"start_day_night keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.start_day_night) ){
      message(1,"start_day_night keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 1 == find_key(fp,"end_day_night",string) ){
    message(1,"end_day_night keyword not found in parameters file");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&par_info.end_day_night) ){
      message(1,"end_day_night keyword not found in parameters file");
      ok = 1;
    }
  }

  if( 0 != fclose(fp) ){
    message(1,"ERROR: Error closing file");
    /* EXIT */
    exit(-1);
  }

  if( 1 == ok ){
    message(1,"ERROR: Missing entries in parameters file");
    /* EXIT */
    exit(-1);
  }
    
  /* Set init_run flag */
  init_run = 1;

}

/*
 * NAME: 
 *
 *    parse_float_values
 *
 * FUNCTION:
 *
 *    C Code to parse up to 3 numbers from parameter file from [,,,]
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses up to 3-element arrays from parameter file of the form
 *
 *    parameter_name = [1.]
 *    parameter_name = [1.,2.]
 *    parameter_name = [1.,2.,3.]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring outvar1 outvar2 outvar2
 *
 *       get number of commas in instring
 *       
 *       IF one comma THEN
 *          get outvar1
 *       ELSE IF two commas THEN
 *          get outvar1 outvar2
 *       ELSE IF three commas THEN
 *          get outvar1 outvar2 outvar3
 *       ENDIF
 *
 *       RETURN number of output variables (1-3) or bad (-1)
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    number_of_commas
 *    parse_float_values_1
 *    parse_float_values_2
 *    parse_float_values_3
 *
 * CALLING SEQUENCE:  
 *
 *    nvalues = parse_float_values(string,&var1,&var2,&va3)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2  - second number
 *    pVar3  - third number
 *
 * RETURNS:
 *
 *    returns the number of entries read, -1 fail
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_float_values(const char *pString, VFLOAT *pVar, VFLOAT *pVar2,
			       VFLOAT *pVar3)
{
  VINT ok = 0;
  VINT ncommas = 0;

  /* Get number of commas up to 2 */
  ncommas = number_of_commas(pString);
  
  /* Depending on number of commas, parse out correct number of values */
  switch(ncommas){
  case 0:
    /* No commas, just the one number e.g. [1.] */
    ok = parse_float_values_1(pString,pVar);
    if( 1 != ok ){
      /* If bad parse return bad */
      ok = -1;
    } else {
      /* Set unused values to NaN */
      *pVar2 = NaN;
      *pVar3 = NaN;
    }
    break;
  case 1:
    /* One commas, two numbers e.g. [1.,2.] */
    ok = parse_float_values_2(pString,pVar,pVar2);
    if( 2 != ok ){
      /* If bad parse return bad */
      ok = -1;
    }else {
      /* Set unused values to NaN */
      *pVar3 = NaN;
    }
    break;
  case 2:
    /* Two commas, three numbers e.g. [1.,2.,3.] */
    ok = parse_float_values_3(pString,pVar,pVar2,pVar3);
    if( 3 != ok ){
      /* If bad parse return bad */
      ok = -1;
    }
    break;
  default:
    /* If not 0, 1 or 2 commas, return bad */
    message(1,"ERROR: Wrong number of commas in [,,,] parsing (float)");
    ok = -1;
  }

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_int_values
 *
 * FUNCTION:
 *
 *    C Code to parse up to 3 numbers from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses up to 3-element arrays (integer) from parameter file of the form
 *
 *    parameter_name = [1]
 *    parameter_name = [1,2]
 *    parameter_name = [1,2,3]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring outvar1 outvar2 outvar2
 *
 *       get number of commas in instring
 *       
 *       IF one comma THEN
 *          get outvar1
 *       ELSE IF two commas THEN
 *          get outvar1 outvar2
 *       ELSE IF three commas THEN
 *          get outvar1 outvar2 outvar3
 *       ENDIF
 *
 *       RETURN number of output variables (1-3) or bad (-1)
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    number_of_commas
 *    parse_int_values_1
 *    parse_int_values_2
 *    parse_int_values_3
 *
 * CALLING SEQUENCE:  
 *
 *    nvalues = parse_int_values_1(string,&var)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2  - second number
 *    pVar3  - third number
 *
 * RETURNS:
 *
 *    returns the number of entries read, -1 fail
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_int_values(const char *pString, VINT *pVar, VINT *pVar2, VINT *pVar3)
{
  VINT ok = 0;
  VINT ncommas = 0;

  /* Get number of commas up to 2 */
  ncommas = number_of_commas(pString);
  
  switch(ncommas){
  case 0:
    /* Zero commas, one numbers e.g. [1] */
    ok = parse_int_values_1(pString,pVar);
    if( 1 != ok ){
      /* If bad parse return bad */
      ok = -1;
    } else {
      /* Set unused values to NaN */
      *pVar2 = NaN;
      *pVar3 = NaN;
    }
    break;
  case 1:
    /* One commas, two numbers e.g. [1,2] */
    ok = parse_int_values_2(pString,pVar,pVar2);
    if( 2 != ok ){
      /* If bad parse return bad */
      ok = -1;
    } else {
      /* Set unused values to NaN */
      *pVar3 = NaN;
    }
    break;
  case 2:
    /* Two commas, three numbers e.g. [1,2,3] */
    ok = parse_int_values_3(pString,pVar,pVar2,pVar3);
    if( 3 != ok ){
      /* If bad parse return bad */
      ok = -1;
    }
    break;
  default:
    /* If not 0, 1 or 2 commas, return bad */
    message(1,"ERROR: Wrong number of commas in [,,,] parsing (int)");
    ok = -1;
  }

  return(ok);
}

/*
 * NAME: 
 *
 *    number_of_commas
 *
 * FUNCTION:
 *
 *    C Code to work out how many commans are in [,,,] - up to 2
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Get number of commans in [] arrays
 *
 *    parameter_name = [1.]       - 0 comma
 *    parameter_name = [1.,2.]    - 1 comma
 *    parameter_name = [1.,2.,3.] - 2 commas
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       strip off surrounding []
 *
 *       DO while , exists 
 *          strip string to after , 
 *          increment ncommas 
 *       END DO
 *
 *       return ncommas
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
 *    ncommas = number_of_commas(string)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    returns the number of commas
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT number_of_commas(const char *pString)
{
  VINT ncommas = 0;
  VINT len = 0;
  char searchstr[MAX_STRING_LENGTH];
  char new_searchstr[MAX_STRING_LENGTH];
  char *pStr =NULL;

  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }

  /* Get length of string to search - make sure we miss closing ] */
  len = pStr-pString-1;

  /* Make new string which only contains chars between [ ] */
  strncpy(searchstr,pString+1,len);
  searchstr[len] = '\0';

  /* Loop round and find number of commas */
  while( NULL != (pStr = strstr(searchstr,",")) ){
    ncommas++;
    /* Avoid memory overlap */
    strncpy(new_searchstr,pStr+1,MAX_STRING_LENGTH);
    strncpy(searchstr,new_searchstr,MAX_STRING_LENGTH);
  }

  return(ncommas);
}

/*
 * NAME: 
 *
 *    parse_float_values_1
 *
 * FUNCTION:
 *
 *    C Code to parse float 1-element array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 1-element arrays from parameter file of the form
 *
 *    parameter_name = [1.]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring var
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get var from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_float_values_1(string,&var)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strncpy
 *    strstr
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_float_values_1(const char *pString, VFLOAT *pVar)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* find ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string within [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get float variable taking into account float size */
  if( 8 == sizeof(VFLOAT) ){
    ok = sscanf(line,"%lf",pVar);
  } else {
    ok = sscanf(line,"%f",pVar);
  }

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_float_values_2
 *
 * FUNCTION:
 *
 *    C Code to parse float 2-dimensional array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 2-element arrays from parameter file of the form
 *
 *    parameter_name = [1.,2.]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring var1 var2
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get vat1, var2 from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_float_values_2(string,&var1,&var2)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2 - second number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_float_values_2(const char *pString, VFLOAT *pVar, VFLOAT *pVar2)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* remove [ ] */
  /* find ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string in [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get float variables taking into account float size */
  if( 8 == sizeof(VFLOAT) ){
    ok = sscanf(line,"%lf,%lf",pVar,pVar2);
  } else {
    ok = sscanf(line,"%f,%f",pVar,pVar2);
  }

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_float_values_3
 *
 * FUNCTION:
 *
 *    C Code to parse float 3-element array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 1-element arrays from parameter file of the form
 *
 *    parameter_name = [1.,2.,3.]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring var1 var2 var3
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get vat1, var2, var3 from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_float_values_1(string,&var1,&var2,&var3)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2 - second number
 *    pVar3 - third number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_float_values_3(const char *pString, VFLOAT *pVar, VFLOAT *pVar2, 
				 VFLOAT *pVar3)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* remove [ ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string in [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get float variables taking into account float size */
  if( 8 == sizeof(VFLOAT) ){
    ok = sscanf(line,"%lf,%lf,%lf",pVar,pVar2,pVar3);
  } else {
    ok = sscanf(line,"%f,%f,%f",pVar,pVar2,pVar3);
  }

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_int_values_1
 *
 * FUNCTION:
 *
 *    C Code to parse int 1-element array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 1-element arrays from parameter file of the form
 *
 *    parameter_name = [1]
 *
 *    FUNCTION instring var1
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get vat1 from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_int_values_1(string,&var)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_int_values_1(const char *pString, VINT *pVar)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* remove [ ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string in [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get int variable */
  ok = sscanf(line,"%d",pVar);

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_int_values_2
 *
 * FUNCTION:
 *
 *    C Code to parse int 2-dimensional array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 2-element arrays from parameter file of the form
 *
 *    parameter_name = [1,2]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring var1 var2
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get vat1, var2 from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_int_values_2(string,&var1,&var2)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2 - second number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_int_values_2(const char *pString, VINT *pVar, VINT *pVar2)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* remove [ ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string in [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get int variables */
  ok = sscanf(line,"%d,%d",pVar,pVar2);

  return(ok);
}

/*
 * NAME: 
 *
 *    parse_int_values_3
 *
 * FUNCTION:
 *
 *    C Code to parse int 3-element array from parameter file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Parses out 1-element arrays from parameter file of the form
 *
 *    parameter_name = [1,2,3]
 *
 * PSEUDO CODE
 *
 *    FUNCTION instring var1 var2 var3
 *
 *       find ] 
 *       IF ] not there THEN
 *          RETURN error
 *       ENDIF
 *
 *       get string length
 *
 *       get string insize []
 *
 *       get vat1, var2, var3 from string
 *
 *       RETURN pass/fail code
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
 *    nvalues = parse_int_values_1(string,&var1,&var2,&var3)
 *
 * INPUTS:  
 *    
 *    string - input string to parse information from 
 *
 * OUTPUTS:  
 *
 *    pVar  - first number
 *    pVar2 - second number
 *    pVar3 - third number
 *
 * RETURNS:
 *
 *    returns the number of entries read
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    strncpy
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT parse_int_values_3(const char *pString, VINT *pVar, VINT *pVar2, 
			       VINT *pVar3)
{
  VINT ok = 0;
  VINT len = 0;
  char *pStr = NULL;
  char line[MAX_STRING_LENGTH];

  /* remove [ ] */
  if( NULL == (pStr = strstr(pString,"]")) ){
    message(1,"ERROR: Missing ] in  parameters.param file");
    /* EXIT return bad */
    return(-1);
  }
  len = (VINT) (pStr - pString - 1);  /* Takes into account [ */

  /* Get string in [] */
  strncpy(line,(pString+1),len);
  line[len] = '\0';

  /* Get int variables */
  ok = sscanf(line,"%d,%d,%d",pVar,pVar2,pVar3);

  return(ok);
}

