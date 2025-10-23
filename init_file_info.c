/*
 * NAME: 
 *
 *    init_file_info
 *
 * FUNCTION:
 *
 *    C version of init_file_info routines
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains routines and parameters to define where and which files
 *    the code reads/writes and logs to.  Reads in the MATLAB init_file_info.m
 *    file used by the overlying MATLAB code.  Note this means that the 
 *    MATLAB init_file_info.m code cannot have implicit calculations in it.
 *    Data stored in the file_info structure accessable by other routines via
 *    init_file_info.h.
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    init_file_info - read info file
 *    free_file_info - free memory used in reading init_file_info
 *    find_key       - get string after key = in file (also used in 
 *                     init_par_info.c)
 *
 * ROUTINES USED INTERNALLY:
 *
 *    get_key_string - from an input line get keyword and assocaited variable
 *                     as a string
 *    file_strip     - gets strings from e.g. [top_level_dir "/Data/test"]
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

#include "init_file_info.h"
#include "logfile.h"

/* Routines for internal use only */
static VINT get_key_string( const char *pInput_line, const char *pKey, char *pFilename );
static void file_strip( const char *inp_string, char *dirstring, char *outstring );

/* variables for internal global use */
static VINT init_run = 0;  /* Flag to make sure we only do this once */

/* variable for external use */
struct file_info_str file_info;

/*
 * NAME: 
 *
 *    init_file_info
 *
 * FUNCTION:
 *
 *    C Code to setup directories/filenames - version of init_file_info.m
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Set up directories/filenames for processing.  Reads in a file containing
 *    top level values (init_file_pars.m) and also a directory listing which 
 *    should normally not be changed unless there is a change in the directory 
 *    structure to be set up.  If this has already been run, then just return.
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE input_file
 *
 *       IF run before THEN
 *          RETURN
 *       ENDIF
 *
 *       open input_file
 *
 *       get "logfile_required" 
 *       get "name_logfile" 
 *       get "dir_metopa_sst"
 *       get "dir_metopa_nav"
 *       get "dir_metopb_sst"
 *       get "dir_metopb_nav"
 *       get "dir_metopc_sst"
 *       get "dir_metopc_nav"
 *       get "dir_viirs_sst"
 *       get "dir_viirs_nav"
 *       get "dir_jpss_sst"
 *       get "dir_jpss_nav"
 *       get "dir_noaa_18_sst"
 *       get "dir_noaa_18_nav"
 *       get "dir_noaa_19_sst"
 *       get "dir_noaa_19_nav"
 *       get "dir_goes"
 *       get "dir_msg"
 *       get "dir_mtsat"
 *       get "dir_ms_read_grib"
 *       get "dir_ms_overlap"
 *       get "dir_ms_smoother"
 *       get "dir_ms_statecorr"
 *       get "dir_ms_executable"
 *       get "dir_rtg_lowres"
 *       get "dir_rtg_hires"
 *       get "name_rtg_lowres"
 *       get "name_rtg_hires"
 *       get "dir_input_ssts"
 *       get "dir_analysis"
 *       get "dir_coastwatch"
 *       get "dir_GHRSST"
 *       get "dir_Diurnal"
 *       get "name_Diurnal"
 *       get "name_sst_analysis"
 *       get "name_error_analysis"
 *       get "name_sst_variability"
 *       get "name_ice_mask"
 *       get "name_correlation_map"
 *       get "name_biases"
 *       get "name_land_mask"
 *       get "name_oi_oceans_coupling"
 *       get "name_oi_state_values"
 *       get "name_oi_scales"
 *       get "name_coastwatch_file"
 *       get "var_n_datasets"
 *
 *       allocate name_dataset structure
 *
 *       LOOP var_n_datasets 
 *          read in array of data set names 
 *       END LOOP
 *
 *       get "dir_ice_lowres"
 *       get "dir_ice_hires"
 *       get "name_ice_lowres"
 *       get "name_ice_hires"
 *       get "dir_ostia"
 *       get "name_ostia"
 *
 *       close input file
 *
 *    END
 *
 * FILES NEEDED: 
 *
 *    input file_info file
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    message
 *    find_key
 *    file_strip
 *
 * CALLING SEQUENCE:  
 *
 *    init_file_info( file_info_file )
 *
 * INPUTS:  
 *    
 *    file_info_file - the init_file_info.m file to be used
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    fopen
 *    fclose
 *    sscanf
 *    strncpy
 *    strncat
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 29/11/2011
 *
 */
void init_file_info( const char *fileInfoFile )
{
  VINT ok = 0;

  VINT i = 0;

  char key_string[MAX_STRING_LENGTH];
  char outdir[MAX_STRING_LENGTH];
  char outstring[MAX_STRING_LENGTH];
  char string[MAX_STRING_LENGTH];
  char blended_sst_home[MAX_STRING_LENGTH];

  FILE *fp = NULL;

  /* If we have run this before, return */
  if( 1 == init_run ){
    /* EXIT already setup parameters */
    return;
  }
  
  /* Open top level file information */
  if( NULL == (fp = fopen(fileInfoFile,"r")) ){
    message(1,"ERROR: Cannot open files.params");
    /* EXIT */
    exit(-1);
  }
  
  /* Read in required parameters */
  ok = 0;
  if( 1 == find_key(fp,"logfile_required",string) ){
    message(1,"logfile_required keyword not found in files.params");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&file_info.logfile_required) ){
      message(1,"ERROR: Parsing logfile_required parameter");
      ok = 1;
    }
  }
  if( 1 == find_key(fp,"name_logfile",string) ){
    message(1,"name_logfile keyword not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_logfile,string,MAX_STRING_LENGTH);
  }

  /* To find out the name of the home directory - look at dir_metopa_sst and
     return the name of the variable containing the home directory.  This can
     then be used to get the value of the home directory in the next step */
  if( 1 == find_key(fp,"dir_metopa_sst",string) ){
    message(1,"dir_metopa_sst keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
  }

  /* Now have the top level (home) directory parameter name, retrieve actual
   * directory name */
  if( 1 == find_key(fp,outdir,blended_sst_home) ){
    message(1,"blended_sst_home keyword not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.blended_sst_home,blended_sst_home,MAX_STRING_LENGTH);
  }

  /* For those entries of the form [blended_sst_home DIRECTORY] parse out
   * DIRECTORY using file_string and add to the value of blended_sst_home 
   * found above.  
   * For those entries of the form "DIRECTORY" just get the directory */

  /* MetopA */
  if( 1 == find_key(fp,"dir_metopa_sst",string) ){
    file_info.dir_metopa_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopa_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopa_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_metopa_nav",string) ){
    file_info.dir_metopa_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopa_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopa_nav,outstring,MAX_STRING_LENGTH);
  }

  /* MetopB */
  if( 1 == find_key(fp,"dir_metopb_sst",string) ){
    file_info.dir_metopb_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopb_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopb_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_metopb_nav",string) ){
    file_info.dir_metopb_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopb_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopb_nav,outstring,MAX_STRING_LENGTH);
  }

  /* MetopC */
  if( 1 == find_key(fp,"dir_metopc_sst",string) ){
    file_info.dir_metopc_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopc_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopc_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_metopc_nav",string) ){
    file_info.dir_metopc_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_metopc_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_metopc_nav,outstring,MAX_STRING_LENGTH);
  }

  /* VIIRS */
  if( 1 == find_key(fp,"dir_viirs_sst",string) ){
    file_info.dir_viirs_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_viirs_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_viirs_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_viirs_nav",string) ){
    file_info.dir_viirs_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_viirs_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_viirs_nav,outstring,MAX_STRING_LENGTH);
  }

  /* JPSS */
  if( 1 == find_key(fp,"dir_jpss_sst",string) ){
    file_info.dir_jpss_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_jpss_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_jpss_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_jpss_nav",string) ){
    file_info.dir_jpss_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_jpss_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_jpss_nav,outstring,MAX_STRING_LENGTH);
  }

  /* Sentinel-3A SLSTR */
  if( 1 == find_key(fp,"dir_slstra_sst",string) ){
    file_info.dir_slstra_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_slstra_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_slstra_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_slstra_nav",string) ){
    file_info.dir_slstra_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_slstra_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_slstra_nav,outstring,MAX_STRING_LENGTH);
  }

  /* Sentinel-3B SLSTR */
  if( 1 == find_key(fp,"dir_slstrb_sst",string) ){
    file_info.dir_slstrb_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_slstrb_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_slstrb_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_slstrb_nav",string) ){
    file_info.dir_slstrb_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_slstrb_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_slstrb_nav,outstring,MAX_STRING_LENGTH);
  }

  /* In the case of data directories sometimes they will not be defined */
  /* If so set string to zero length */
  if( 1 == find_key(fp,"dir_noaa_16_sst",string) ){
    file_info.dir_noaa_16_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_16_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_16_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_16_nav",string) ){
    file_info.dir_noaa_16_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_16_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_16_nav,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_17_sst",string) ){
    file_info.dir_noaa_17_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_17_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_17_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_17_nav",string) ){
    file_info.dir_noaa_17_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_17_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_17_nav,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_18_sst",string) ){
    file_info.dir_noaa_18_sst[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_18_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_18_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_18_nav",string) ){
    file_info.dir_noaa_18_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_18_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_18_nav,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_19_sst",string) ){
    file_info.dir_noaa_19_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_19_sst,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_19_sst,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_noaa_19_nav",string) ){
    file_info.dir_noaa_19_nav[0] = '\0';
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_noaa_19_nav,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_noaa_19_nav,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_goes",string) ){
    message(1,"dir_goes keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_goes,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_goes,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_msg",string) ){
    message(1,"dir_msg keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_msg,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_msg,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_mio",string) ){
    message(1,"dir_mio keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_mio,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_mio,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_mtsat",string) ){
    message(1,"dir_mtsat keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_mtsat,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_mtsat,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_amsr",string) ){
    message(1,"dir_amsr keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_amsr,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_amsr,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_ms_read_grib",string) ){
    message(1,"dir_ms_read_grib keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_ms_read_grib,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ms_read_grib,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_ms_overlap",string) ){
    message(1,"dir_ms_overlap keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_ms_overlap,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ms_overlap,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_ms_smoother",string) ){
    message(1,"dir_ms_smoother keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_ms_smoother,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ms_smoother,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_ms_statecorr",string) ){
    message(1,"dir_ms_statecorr keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_ms_statecorr,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ms_statecorr,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_ms_executable",string) ){
    message(1,"dir_ms_executable keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_ms_executable,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ms_executable,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_rtg_lowres",string) ){
    message(1,"dir_rtg_lowres keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_rtg_lowres,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_rtg_lowres,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_rtg_hires",string) ){
    message(1,"dir_rtg_hires keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_rtg_hires,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_rtg_hires,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"name_rtg_lowres",string) ){
    message(1,"name_rtg_lowres keyword not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_rtg_lowres,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.name_rtg_lowres,string,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"name_rtg_hires",string) ){
    message(1,"name_rtg_hires keyword not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_rtg_hires,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.name_rtg_hires,string,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_input_ssts",string) ){
    message(1,"dir_input_ssts keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_input_ssts,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_input_ssts,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_analysis",string) ){
    message(1,"dir_analysis keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_analysis,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_analysis,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_coastwatch",string) ){
    message(1,"dir_coastwatch keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_coastwatch,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_coastwatch,outstring,MAX_STRING_LENGTH);
  }

  if( 1 == find_key(fp,"dir_GHRSST",string) ){
    message(1,"dir_GHRSST keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_GHRSST,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_GHRSST,outstring,MAX_STRING_LENGTH);
  }    

  if( 1 == find_key(fp,"dir_Diurnal",string) ){
    message(1,"dir_Diurnal keyword not found in files.params");
    ok = 1;
  } else {
    file_strip(string,outdir,outstring);
    strncpy(file_info.dir_Diurnal,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_Diurnal,outstring,MAX_STRING_LENGTH);
  }    
  
  /* Name of Diurnal warming model input */
  if( 1 == find_key(fp,"name_Diurnal",string) ){
    message(1,"name_Diurnal not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_Diurnal,string,MAX_STRING_LENGTH);
  }
  
  /* Names/Stems of outputs - these do not have blended home parameters 
   * so only name stem is obtained */
  if( 1 == find_key(fp,"name_sst_analysis",string) ){
    message(1,"name_sst_analysis not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_sst_analysis,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_error_analysis",string) ){
    message(1,"name_error_analysis not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_error_analysis,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_sst_variability",string) ){
    message(1,"name_sst_variability not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_sst_variability,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_ice_mask",string) ){
    message(1,"name_ice_mask not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_ice_mask,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_correlation_map",string) ){
    message(1,"name_correlation_map not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_correlation_map,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_biases",string) ){
    message(1,"name_biases not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_biases,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_land_mask",string) ){
    message(1,"name_land_mask not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_land_mask,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_oi_oceans_coupling",string) ){
    message(1,"name_oi_oceans_coupling not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_oi_oceans_coupling,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_oi_state_values",string) ){
    message(1,"name_oi_state_values not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_oi_state_values,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_oi_scales",string) ){
    message(1,"name_oi_scales not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_oi_scales,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_coastwatch_file",string) ){
    message(1,"name_coastwatch_file not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_coastwatch_file,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"var_n_datasets",string) ){
    message(1,"var_n_datasets not found in files.params");
    ok = 1;
  } else {
    if( 1 != sscanf(string,"%d",&file_info.var_n_datasets) ){
      message(1,"ERROR: Reading in var_n_datasets");
      ok = 1;
    } 
  }

  /* Here we have a list of names which can be variable in length */
  /* Allocate top level name_dataset structure */
  if( NULL == (file_info.name_dataset = 
	       (char **) calloc(file_info.var_n_datasets,sizeof(char *))) ){
    message(1,"ERROR: Allocating array name_dataset (char **)");
    /* EXIT */
    exit(-1);
  }

  /* Read in array of data set names */
  for(i=0;i<file_info.var_n_datasets;i++){

    sprintf(key_string,"name_dataset_%3.3d",i+1);
    if( 1 == find_key(fp,key_string,string) ){
      message(1,key_string," not found in files.params");
      ok = 1;
    } else {
      /* Allocate filelength */
      if( NULL == (*(file_info.name_dataset+i) = 
		   (char *) calloc(MAX_STRING_LENGTH,sizeof(char))) ){
	message(1,"ERROR: Allocating array name_dataset (char *)");
	/* EXIT */
	exit(-1);
      }
      strncpy(*(file_info.name_dataset+i),string,MAX_STRING_LENGTH);
    }

  }

  /* Get remaining filenames - these have a blended_sst_home */
  if( 1 == find_key(fp,"dir_ice_lowres",string) ){
    message(1,"dir_ice_lowres not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.dir_ice_lowres,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ice_lowres,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"dir_ice_hires",string) ){
    message(1,"dir_ice_hires not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.dir_ice_hires,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ice_hires,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_ice_lowres",string) ){
    message(1,"name_ice_lowres not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_ice_lowres,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"name_ice_hires",string) ){
    message(1,"name_ice_hires not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_ice_hires,string,MAX_STRING_LENGTH);
  }
  
  if( 1 == find_key(fp,"dir_ostia",string) ){
    message(1,"dir_ostia not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.dir_ostia,blended_sst_home,MAX_STRING_LENGTH);
    strncat(file_info.dir_ostia,string,MAX_STRING_LENGTH);
  }
  
  /* Finally, just the name of the OSTIA data - no blended_sst_home */
  if( 1 == find_key(fp,"name_ostia",string) ){
    message(1,"name_ostia not found in files.params");
    ok = 1;
  } else {
    strncpy(file_info.name_ostia,string,MAX_STRING_LENGTH);
  }

  if( 0 != fclose(fp) ){
    message(1,"ERROR: Error closing files.params");
    /* EXIT */
    exit(-1);
  }

  /* Check to see if there are any missing entries and if there are, exit */
  if( 1 == ok ){
    message(1,"ERROR: Missing entries in files.params");
    /* EXIT */
    exit(-1);
  }
    
  /* Set init_run flag */
  /* Used to make sure we only run the above code once */
  init_run = 1;

}

/*
 * NAME: 
 *
 *    file_strip
 *
 * FUNCTION:
 *
 *    C Code to get required directory [] strings - internal to this module
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    From a line such as
 *
 *    file_info.dir_metop_sst     = [blended_sst_home '/Data/sst_metop/'];
 *
 *    where find_key will return
 *   
 *    [blended_sst_home '/Data/sst_metop/']
 *
 *    this routine returns 
 *
 *       dirstring = blended_sst_home
 *       outstring = /Data/sst_metop/
 *
 *    Note that this code does assume a fixed format for the input with a 
 *    single space between the blended_sst_home parameter name and the
 *    directory 
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE inp_string dirstring outstring
 *
 *        find number of chars to first ' ' (between two strings)
 *
 *        copy first string (ignoring [) to dirstring
 *
 *        find last ' (from 'string'])        
 *
 *        copy second string (ignoring 1st and 2nd ' and ]) to outstring
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    file_strip( inp_string, dirstring, outstring )
 *
 * INPUTS:  
 *
 *    inp_string   - string with [string1 string2] in
 *
 * OUTPUTS:  
 *
 *    dirstring    - name of variable containing the home directory
 *    outstring    - name of subdirectory
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *    strncpy
 *    strlen
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static void file_strip( const char *inp_string, char *dirstring, char *outstring )
{
  VINT i = 0;
  VINT j = 0;
  VINT len = 0;
  VINT start_pos = 0;

  /* remove the two strings from the [] */
  /* Ignore first [ in inp_string so start from 1 */
  j = 0;
  for(i=1;i<strlen(inp_string);i++){
    if( ' ' == *(inp_string+i) ){
      len = j;
      break;
    }
    j++;
  }
  strncpy(dirstring,(inp_string+1),len);
  /* Make sure the string ends in a \0 */
  *(dirstring+len) = '\0';

  /* Shift start pos to beginning of next string missing the first ' */
  start_pos = len+3;

  /* Retrieve subdir name */
  j = 0;
  for(i=start_pos;i<strlen(inp_string);i++){
    if( '\'' == *(inp_string+i) ){
      len = j;
      break;
    }
    j++;
  }
  strncpy(outstring,(inp_string+start_pos),len);
  /* Make sure the string ends in a \0 */
  *(outstring+len) = '\0';

}

/*
 * NAME: 
 *
 *    free_file_info
 *
 * FUNCTION:
 *
 *    C Code to free allocated memory in file_pars
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees allocated memory in file_pars
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE
 *
 *       LOOP number of datasets
 *          free memory
 *       END LOOP
 *       free top level memory
 *       reset run_init flag
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    free_file_info
 *
 * INPUTS:  
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
void free_file_info( void )
{
  /* Local variables */
  VINT i = 0;

  /* Loop round filelist and free strings */
  for(i=0;i<file_info.var_n_datasets;i++){
    free(*(file_info.name_dataset+i));
  }
  /* Free top level of list */
  free(file_info.name_dataset);

  /* Reset init_file_info */
  init_run = 0;
}

/*
 * NAME: 
 *
 *    find_key
 *
 * FUNCTION:
 *
 *    C Code to get string associated with a given key from input file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets string from .param file associated with a given keyword e.g.
 *
 *    logfile = noaa_op_sst_  ! logfile name
 *
 *    which for key "logfile" will return  *string as "noaa_op_sst_"
 *
 * PSEUDO CODE
 *
 *    FUNCTION fp key outstring
 *
 *       rewind file
 *
 *       LOOP round file
 *          IF get_key_string success THEN
 *             RETURN success
 *          ENDIF
 *       END LOOP
 *
 *       RETURN fail
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    get_key_string
 *
 * CALLING SEQUENCE:  
 *
 *    ok =  find_key(fp,key,string)
 *
 * INPUTS:  
 *
 *          fp = FILE* pointer to file to be read
 *         key = keyword to find
 *    
 * OUTPUTS:  
 *
 *      string = return string for keyword
 *
 * RETURNS:
 *
 *    status of read - if failed (no keyword found) returns 1, else returns 0
 *
 * SYSTEM CALLS:
 *
 *    rewind
 *    fgets
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
VINT find_key(FILE *fp, const char *pKey, char *pString)
{
  /* Local variables */
  char line[MAX_STRING_LENGTH];

  /* make sure file is rewound */
  rewind(fp);

  /* Read through file to find keyword */
  while( NULL != fgets(line,MAX_STRING_LENGTH,fp) ){
    if( 0 == get_key_string(line,pKey,pString) ){
      /* EXIT Success so return zero */
      return(0);
    }
  }

  /* Return 1 if keyword not found */
  return(1);
}


/*
 * NAME: 
 *
 *    get_key_string
 *
 * FUNCTION:
 *
 *    C Code to get key and filename from line read in from file
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Gets filename from read in line in cases where there is a comment 
 *    line such as
 *
 *    logfile = noaa_op_sst_  ! logfile name
 *
 *    will for input *key "logfile" gives *string as "noaa_op_sst_"
 *
 * PSEUDO CODE
 *
 *    FUNCTION input_line key outstring
 *
 *       remove leading whitespace
 *
 *       get length of key string
 *
 *       IF comment line THEN
 *          RETURN 1
 *       ENDIF
 *
 *       IF period in key string THEN
 *          get key string after period 
 *       ENDIF
 *
 *       IF key string does not match input key THEN
 *          RETURN 1
 *       ENDIF
 *
 *       Find = in string
 *
 *       get string after = removing leading whitespace
 *
 *       Find length of string after = to comment/end of line point
 *
 *       remove surrounding '' if necessary
 *
 *       RETURN 0
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    ok =  get_key_string(input_line,key,string)
 *
 * INPUTS:  
 *
 *    input_line = line read in from file
 *           key = key to be found from file
 *    
 * OUTPUTS:  
 *
 *        string = string associated with key
 *
 * RETURNS:
 *
 *    status of read - if failed (such as a blank line) return 1
 *
 * SYSTEM CALLS:
 *
 *    strncpy
 *    strcpy
 *    strcmp
 *    strlen
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 01/12/2011
 *
 */
static VINT get_key_string( const char *pInput_line, const char *pInkey, char *pFilename )
{
  /* Local variables */
  VINT i = 0;
  VINT len = 0;
  char key[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  char temp2[MAX_STRING_LENGTH];
  char *pPeriod = NULL;

  /* Make sure there are no leading whitespaces */
  len = 0;
  for(i=0;i<strlen(pInput_line);i++){
    if( ' ' == *(pInput_line+i) ){
      len = i+1;
    } else {
      break;
    }
  }

  /* Remove whitespace if needed */
  if( 0 < len ){
    strncpy(temp,(pInput_line+len),MAX_STRING_LENGTH);
  } else {
    strncpy(temp,pInput_line,MAX_STRING_LENGTH);
  }

  /* Find length of key string - look for a space or a newline or equals */
  len = 0;
  for(i=0;i<strlen(temp);i++){
    if( ' ' == temp[i] || '\n' == temp[i] || '=' == temp[i] ){
      len = i;
      break;
    }
  }
  if( 0 == len ){
    /* EXIT blank line - return error */
    return(1);
  } else {
    strncpy(key,temp,len);
    key[len] = '\0';
  }

  /* If comment line then return */
  /* Three different possible comment characters */
  if( '%' == key[0] ){
    /* EXIT comment line */
    return(1);
  }
  if( '#' == key[0] ){
    /* EXIT comment line */
    return(1);
  }
  if( '!' == key[0] ){
    /* EXIT comment line */
    return(1);
  }

  /* If there is a period then make the key string after this. This deals with 
   * cases like par_info.key where what we want is "key" without the "par_info"
   * part.  This is needed so we can read in the MATLAB init_par_info.m and
   * init_file_info.m files and get the keywords */
  if( NULL != (pPeriod = strstr(key,".")) ){
    /* We have a period, so strip leading characters */
    strcpy(temp2,(pPeriod+1));
    strcpy(key,temp2);
  }

  /* Check if key string matches */
  if( 0 != strcmp(pInkey,key) ){
    /* EXIT No match for this line */
    return(1);
  }

  /* Find where = is and remove characters */
  len = 0;
  for(i=0;i<strlen(temp);i++){
    if( '=' == temp[i] ){
      len = i+1;
      break;
    }
  }

  if( 0 == len ){
    /* EXIT If no = in string - return bad */
    return(1);
  } else {
    strncpy(temp2,(temp+len),MAX_STRING_LENGTH);
  }

  /* Make sure there are no leading whitespaces for string after = */
  len = 0;
  for(i=0;i<strlen(temp2);i++){
    if( ' ' == temp2[i] ){
      len = i+1;
    } else {
      break;
    }
  }

  /* Remove whitespace if needed */
  if( 0 < len ){
    strncpy(temp,&temp2[len],MAX_STRING_LENGTH);
  } else {
    strncpy(temp,temp2,MAX_STRING_LENGTH);
  }

  /* Get string before any comments/end of line */
  /* either end of string, a comment, a newline or a semi-colon */
  len = 0;
  for(i=0;i<strlen(temp);i++){
    if( '\0' == temp[i] || '!' == temp[i] || '#' == temp[i] || 
	'%' == temp[i] || '\n' == temp[i] || ';' == temp[i]  ){
      len=i;
      break;
    }
  }
  /* If no end of line etc. then return bad */
  if( 0 == len ){
    /* EXIT error in input string */
    return(1);
  }

  /* Check to see if we need to remove '' marks */
  if( '\'' == temp[0] && '\'' == temp[len-1] ){
    /* Copy to output removing '' */
    strncpy(pFilename,&temp[1],len-2);
    *(pFilename+len-2) = '\0';
  } else {
    /* Copy to output */
    strncpy(pFilename,temp,len);
    *(pFilename+len) = '\0';
  }

  /* Have found key string and return good */
  return(0);
}
