/*
 * NAME: 
 *
 *    process_raw_amsr_c
 *
 * FUNCTION:
 *
 *    C version of process_raw_amsr.m - reads in AMSR data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in AMSR data.  Note that this works
 *    in a different way from the MATLAB code and fixes some of the problems 
 *    such as excluding too many cold pixels and biasing the SST, as well 
 *    as properly adding in all orbits correctly rather than only keeping 
 *    data from the last referenced orbit in any given pixel.
 * 
 *    Code uses linked lists to store SSTs from different orbits/day/night 
 *    at any given location and then uses the lists to calculate the mean/
 *    standard deviation and also to clean (remove significantly cold pixels)
 *    from the data.  The final pixel value is a weighted average of selected
 *    pixels, the weights being based on the final estimated variance of
 *    each pixel.
 *
 *    Also, input for different AVHRRs is a name.  For the NOAA satellites
 *    it is i.e. NOAA16 is 'noaa16' (note lower case(.  For METOP, the name 
 *    is e.g. 'METOPA' in uppercase.  This is consistent with the MATLAB codes
 *    naming conventions.  Internally in this code the satellites are referred 
 *    to via number references via #define constants in types_cnsts.h
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_AMSR
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
 *   Original version by Jonathan P.D. Mittaz on 05/12/2011
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* header for this routine */
#include "types_cnsts.h"
#include "logfile.h"
#include "init_file_info.h"
#include "init_par_info.h"
#include "matlab_functions.h"
#include "get_dates.h"
#include "read_amsr.h"
#include "process_raw_amsr_c.h"
#include "diurnal_warming.h"

/* Internally defined structures */
/* Structure to hold each cells list of SSTs for further analysis */
/* Storage increment - initial low for equator pixels then step size for
 * extra memory in linked lists */
#define INITIAL_STORE 2
#define STORE_STEP 5

/* Routines that are internal to this routine */
/* Note that make routines visible for unit testing, but not when running
 * operationally */
static void get_day_night_cld_mask( const struct char_struct *pCldmask, 
				    const struct float_struct *pSST,
				    const VINT day_or_night, char cmask, 
				    const VFLOAT sst_min,
				    struct char_struct *pNewMask,
				    VINT *pNgood );
static void add_to_storage(const VFLOAT sst, const VFLOAT variance, 
			   const VINT pos, struct SST_Storage *pStorage);
static void init_storage(struct SST_Storage *pStorage);
static void setup_storage(const VINT nx, const VINT ny, struct SST_Storage *pStorage);
static void free_storage(struct SST_Storage *pStorage);
static void cleanup_plus_mean_stdev_sst_data( const struct SST_Storage *pStored_SST, 
					      struct out_sst_str *pOutdata, 
					      const VFLOAT stdev_threshold, 
					      const VFLOAT stdev_default, 
					      VINT *pNBadStd, char use_sses_stdev );
static VFLOAT clean_stdev( struct float_struct1d *pArray,
			   struct float_struct1d *pArray_Var,
			   const VINT nstored,
			   const VFLOAT stdev_threshold, 
			   const VFLOAT stdev_default, 
			   struct float_struct1d *pSorted,
			   struct float_struct1d *pSorted_Var,
			   VINT *pNDump, 
			   VFLOAT *pMean, char use_sses_stdev );

/*
 * NAME: 
 *
 *    process_raw_amsr_c
 *
 * FUNCTION:
 *
 *    C Code to read in AMSR data for one day
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in AMSR-E/2 data for a complete day.  As each file is read in
 *    it is quality controlled using thresholds determined from SST variability
 *    values passed into routine which are scaled appropriately for day/night.
 *    A bias is also subtracted from the reference dataset - different for
 *    day and night.
 *    Data is then put into each cell and stored as a linked list.  Once all 
 *    the data is read in, there is some further quality control (an attempt 
 *    to remove spurious cold/cloudy pixels) and then the statistics of the 
 *    input SSTs are calculated (mean, standard deviation).
 *
 *    Note that the removal of cold pixels has the potential of biasing some
 *    pixels warm.  The thresholds for the test used here should minimize
 *    this effect and are controlled by a par_info parameter 
 *    min_amsr_obs_per_cell.
 *
 * PSEUDO CODE
 *
 *    FUNCTION amsr_name, year, day, ref_sst, sst_variability, sst_check, 
 *             biasDay, biasNight
 *
 *       initialize arrays
 *       initialize par_info/file_info
 *
 *       convert noaa_name to unique ID
 *
 *       get date string
 *       SELECT based on noaa_name
 *          get thresholds and file directories
 *       END SELECT
 *
 *       correct ref_sst for bias
 *
 *       setup link lists for day/night
 *
 *       find AMSR SST files
 *
 *       LOOP round number of files found
 *
 *          IF data read in OK THEN
 *
 *             LOOP round day/night or nightonly 
 *                IF daytime THEN
 *                   set daytime cloud mask filter
 *                   get daytime climatology and thresholds
 *                ELSE
 *                   set nighttime cloud mask filter
 *                   get nighttime climatology and thresholds
 *                ENDIF
 *                get cloud mask
 *                IF good data exists THEN
 *                   LOOP round cloud free data 
 *                      IF data passes test against climtology THEN
 *                         store data in link lists
 *                      ELSE
 *                         increment bad pixel counters
 *                      ENDIF
 *                   END LOOP
 *                ENDIF
 *             END LOOP
 *
 *          ENDIF
 *
 *       END LOOP
 *
 *       LOOP round day night to store data in output arrays 
 * 
 *          setup output structure
 *          clean data and get mean/standard deviations and store in 
 *          output structure
 *
 *       END LOOP
 *
 *       clear memory
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINES CONTAINED: 
 *
 * CALLING SEQUENCE:  
 *
 *    ok = process_raw_amsr_c( amsr_name, year, day, ref_sst, 
 *                            sst_variability, sst_check, biasDay, biasNight
 *			      Outdata, DumpArray, Total_badclim,
 *                            Total_badstd )
 *
 * INPUTS:  
 *    
 *   amsr_name       - AMSR name - [AMSR-E] or AMSR-2 
 *   year            - year
 *   day             - day number of year
 *   ref_sst         - reference sst for bias correction
 *   sst_variability - sst variability data
 *   sst_check       - check against limits of SST (0=no, 1=yes)
 *   biasDay         - bias data for Day from MATLAB - obtained from MEX file
 *   biasNight       - bias data for Night from MATLAB - obtained from MEX file
 *
 * OUTPUTS:  
 * 
 *   outdata         - structure containing day/night output (SST,Var,nGrid)
 *   dumparray       - structure containing day/night dump arrays
 *   total_badclim   - total number of pixels rejected from climatology check (day/night)
 *   total_badstd    - total number of pixels rejected from stdev check (day/night)
 *
 * RETURNS:
 *
 *    Returns a status (1 = ok)
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/05/2012
 *
 */
VINT process_raw_amsr_c( const char *amsr_name, const VINT year, 
				const VINT day, 
				const struct sst_struct *pRef_SST, 
				const struct sst_struct *pSST_Variability, 
				const VINT sst_check, 
				const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				struct int_struct *pDumpArray,
				VINT *pTotal_badclim,
				VINT *pTotal_badstd,
				const struct par_files_str par_files )
{
  /* Local variables */
  char correct_bias = 0;
  char use_sses_stdev = 0;

  VINT i = 0;
  VINT j = 0;
  VINT nFiles = 0;
  VINT NGood = 0;
  VINT nkeep[2] = {0,0};
  VINT badclim[2] = {0,0};
  VINT daynight = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT nx = 0;
  VINT ny = 0;
  VINT nBadStd = 0;
  VINT nreject = 0;
  VINT amsr_no = 0;

  VFLOAT std_default = 0.;
  VFLOAT std_default_day = 0.;
  VFLOAT std_default_night = 0.;
  VFLOAT sst_min = 0.;
  VFLOAT ref_deltalon = 0.;
  VFLOAT ref_deltalat = 0.;
  VFLOAT sses_stdev = 0.;

  char cmask = 0;
  unsigned char start_day_night = 0;
  unsigned char end_day_night = 0;

  struct float_struct Threshold_Day;
  struct float_struct Threshold_Night;
  struct float_struct *pClimThresh = NULL;
  struct float_struct compare_sst_Night;
  struct float_struct compare_sst_Day;
  struct float_struct *pClimArray = NULL;
  struct char_struct Mask;
  struct char_struct okdata;

  /* Data from AMSR */
  struct float_struct SST;
  struct float_struct Lat;
  struct float_struct Lon;
  struct float_struct SST_Variance;
  struct char_struct CldMask;

  char **ppFilelist = NULL;
  char directory_nav[MAX_STRING_LENGTH];
  char directory_sst[MAX_STRING_LENGTH];
  char daystr[MAX_STRING_LENGTH];
  char datestring[MAX_STRING_LENGTH];
  char messageStr[MAX_STRING_LENGTH];
  char searchStr[MAX_STRING_LENGTH];
  char nameStr[MAX_STRING_LENGTH];
  char acspo_format[MAX_STRING_LENGTH];
  char DWfile_stem[MAX_STRING_LENGTH];
  char DWfile[MAX_STRING_LENGTH];

  /* Storage structure */
  /* Note - dimension of 2 for day/night */
  struct SST_Storage Stored_SST[2];

  char use_GHRSST = 0;	/*  Will only be reading GHRSST L2P at this time  */
  char use_NETCDF = 0;
  char use_HDF = 0;

  /* Init arrays */
  init_float_array_matlab(&Threshold_Day);
  init_float_array_matlab(&Threshold_Night);
  init_float_array_matlab(&compare_sst_Night);
  init_float_array_matlab(&compare_sst_Day);
  init_byte_array_matlab(&Mask);
  init_byte_array_matlab(&okdata);
  init_float_array_matlab(&SST);
  init_float_array_matlab(&Lat);
  init_float_array_matlab(&Lon);
  init_float_array_matlab(&SST_Variance);
  init_byte_array_matlab(&CldMask);
  init_storage(&Stored_SST[0]);
  init_storage(&Stored_SST[1]);

  /* Inits files/pars */
  init_file_info(par_files.file_info);
  init_par_info(par_files.par_info);

  /* Convert input name string to unique ID */
  strcpy(nameStr,noaa_name);
  for(i=0;i<strlen(nameStr);i++){    
    nameStr[i] = toupper(nameStr[i]);
  }
  if( NULL != strstr(nameStr,"AMSRE") ){
    /* AMSR-E */
    amsr_no = AMSR_E;
  } else if( NULL != strstr(nameStr,"AMSR2") ){
    /* AMSR-2 */
    amsr_no = AMSR_2;
  } else {
    message(2,"ERROR: Unknown AMSR type ",nameStr);
    exit(-1);
  }

  /* Get day/night flags */
  /* Note 0 = day and 1 = night */
  start_day_night = par_info.start_day_night;
  end_day_night = par_info.end_day_night;  
 
  /* Allocate memory and assign for Thresholds (Day/Night) */
  copy_float_matlab(&pSST_Variability->sst,&Threshold_Day);
  copy_float_matlab(&pSST_Variability->sst,&Threshold_Night);

  /* Assign appropriate directory for input data as appropriate for selected 
   * satellite and setup SST thresholds for each satellite */
  switch( amsr_no ){
  case AMSR_E:
    /* Check to see if there is an entry */
    if( 0 == strlen(file_info.dir_amsr_e_nav) ){
      message(1,"ERROR: dir_amsr_e_nav/sst is not defined");
      /* EXIT */
      exit(-1);
    }
    /* Get directory names */
    strncpy(directory_nav,file_info.dir_amsr_e_nav,MAX_STRING_LENGTH);
    strncpy(directory_sst,file_info.dir_amsr_e_sst,MAX_STRING_LENGTH);
    /* Day threshold for quality control */
    mult_float_array_matlab(&Threshold_Day,
			    par_info.amsre_day_threshold_mult);
    add_float_array_matlab(&Threshold_Day,
			   par_info.amsre_day_threshold_constant);
    /* Night threshold for quality control */
    mult_float_array_matlab(&Threshold_Night,
			    par_info.amsre_night_threshold_mult);
    add_float_array_matlab(&Threshold_Night,
			   par_info.amsre_night_threshold_constant);
    std_default_day = par_info.amsre_day_std_default;
    std_default_night = par_info.amsre_night_std_default;
    /* GHRSST */
    correct_bias = par_info.amsre_sses_bias;
    sses_stdev = par_info.amsre_sses_stdev;
    use_sses_stdev = par_info.amsre_use_sses_stdev;
    strcpy(acspo_format,par_info.amsre_acspo_format);
    break;
  case AMSR_2:
    /* Check to see if there is an entry */
    if( 0 == strlen(file_info.dir_amsr_2_nav) ){
      message(1,"ERROR: dir_amsr_2_nav/sst is not defined");
      /* EXIT */
      exit(-1);
    }
    /* Get directory names */
    strncpy(directory_nav,file_info.dir_amsr_2_nav,MAX_STRING_LENGTH);
    strncpy(directory_sst,file_info.dir_amsr_2_sst,MAX_STRING_LENGTH);
    /* Day threshold for quality control */
    mult_float_array_matlab(&Threshold_Day,
			    par_info.amsr2_day_threshold_mult);
    add_float_array_matlab(&Threshold_Day,
			   par_info.amsr2_day_threshold_constant);
    /* Night threshold for quality control */
    mult_float_array_matlab(&Threshold_Night,
			    par_info.amsr2_night_threshold_mult);
    add_float_array_matlab(&Threshold_Night,
			   par_info.amsr2_night_threshold_constant);
    std_default_day = par_info.amsr2_day_std_default;
    std_default_night = par_info.amsr2_night_std_default;
    /* GHRSST */
    correct_bias = par_info.amsr2_sses_bias;
    sses_stdev = par_info.amsr2_sses_stdev;
    use_sses_stdev = par_info.amsr2_use_sses_stdev;
    strcpy(acspo_format,par_info.amsr2_acspo_format);
    break;
  default:
    sprintf(messageStr,
	    "*** AMSR type is invalid. Choose from AMSR -E or -2.'");
    message(1,messageStr);
    /* EXIT Error in type */
    return(1);
  }

  /* Check to see if HDF or NETCDF or GHRSST format needed */
  if( NULL != strstr(amsr_format,"HDF") ){
    use_GHRSST = 0;
    use_NETCDF = 0;
    use_HDF = 1;
  } else {
    if( NULL != strstr(amsr_format,"NETCDF") ){
      use_GHRSST = 0;
      use_NETCDF = 1;
      use_HDF = 0;
    } else {
      use_GHRSST = 1;
      use_NETCDF = 0;
      use_HDF = 0;
    }
  }

  /* Get datestring */
  if( 1 == use_GHRSST ){
    /* For GHRSST format */
    strncpy(datestring,get_ghrsst_datestring(year,day),MAX_STRING_LENGTH);
  } else {
    sprintf(messageStr,
	    "*** Can only read AMSR-2 data in GHRSST L2P format.'");
    message(1,messageStr);
    /* EXIT Error in type */
    return(1);
  }

  /* Set minimum SST from previous days analysis which is an input */
  sst_min = par_info.sst_analysis_min - 2.;

  /* Nighttime corrected dataset */
  /* Note that this routine allocates compare_SST */
  array_float_oper_matlab(&pRef_SST->sst,"-",pBiasNight,&compare_sst_Night);

  /* Daytime */
  array_float_oper_matlab(&pRef_SST->sst,"-",pBiasDay,&compare_sst_Day);

  /* Check image dimensions for climatology/thresholds */
  if( (compare_sst_Night.nx != Threshold_Night.nx) ||
      (compare_sst_Day.nx != Threshold_Day.nx) ||
      (compare_sst_Night.ny != Threshold_Night.ny) ||
      (compare_sst_Day.ny != Threshold_Day.ny) ||
      (compare_sst_Night.size != Threshold_Night.size) ||
      (compare_sst_Day.size != Threshold_Day.size) ||
      (pRef_SST->sst.nx != Threshold_Night.nx) ||
      (pRef_SST->sst.ny != Threshold_Night.ny) ||
      (pRef_SST->sst.size != Threshold_Night.size) ){
    message(1,"ERROR: Climatology/Tresholds have difference sizes (process_raw_amsr_c)");
    /* EXIT */
    exit(-1);
  }

  /* Get list of AMSR files */
  /* Get correct string depending on in NOAA- or METOP or VIIRS */
  if( 1 == use_GHRSST ){
    switch( amsr_no ){
    case AMSR_E:  
      sprintf(searchStr,"%s/%s*-L2P_GHRSST-*AMSRE*.nc",directory_sst,datestring);
      break;
    case AMSR_2:  
      sprintf(searchStr,"%s/%s*-L2P_GHRSST-*AMSR2*.nc",directory_sst,datestring);
      break;
    }
  }

  /* Get filelist */
  if( 0 == find_files(searchStr,&nFiles,&ppFilelist) ){
    message(3,"*** AMSR directory is ",directory_sst,".");
    /* Output instrument type specific error message */
    if( AMSR_2 <= amsr_no ){
      sprintf(messageStr,"*** No files found for AMSR-%d for %4.4d day %3.3d.",
	      amsr_no,year,day);
    } else {
      sprintf(messageStr,"*** No files found for AMSR-E for %4.4d day %3.3d.",
	      year,day);
    }
    message(1,messageStr);
    message(1,
    "*** Check appropriate directory has been defined in init_file_info.");
    /* EXIT no files found */
    return(1);
  }  

  /* Get delta latitude/longitude used to determine position in reference
   * dataset */
  ref_deltalon = pRef_SST->deltalon;
  ref_deltalat = pRef_SST->deltalat;

  /* Now setup storage array - 0 for day, 1 for night */
  /* This depends on value of start_day_night/end_day_night */
  /* 0 is for day, 1 is for night */
  if( (0 == start_day_night) || (0 == end_day_night) ){
    setup_storage(pRef_SST->sst.nx,pRef_SST->sst.ny,&Stored_SST[0]);
  }
  if( (1 == start_day_night) || (1 == end_day_night) ){
    setup_storage(pRef_SST->sst.nx,pRef_SST->sst.ny,&Stored_SST[1]);
  }

  /* Allocate dump arrays for day and night */
  if( (0 == start_day_night) || (0 == end_day_night) ){
    allocate_int_array_matlab(pRef_SST->sst.nx,pRef_SST->sst.ny,
			      pDumpArray);
  }
  if( (1 == start_day_night) || (1 == end_day_night) ){
    allocate_int_array_matlab(pRef_SST->sst.nx,pRef_SST->sst.ny,
			      (pDumpArray+1));
  }

  /* Read in diurnal warming NetCDF file if necessary */
  if( 1 == par_info.correct_diurnal_warming ){
    strcpy(DWfile_stem,file_info.dir_Diurnal);
    strcat(DWfile_stem,"/");
    strcat(DWfile_stem,file_info.name_Diurnal);
    make_diurnal_filename(DWfile_stem,year,day,DWfile);
    if( 1 == read_diurnal_file(DWfile) ){
      message(1,"ERROR: Cannot read in DW NetCDF file");
      message(1,"ERROR: Turning off DW correction (AMSR processing)");
      par_info.correct_diurnal_warming = 0;
    }
  }
/*** TO HERE ***/
  /* Now loop round files */
  for(i=0;i<nFiles;i++){
    /* Read in acspo data */
    /* Note, if requested, diurnal warming is corrected for within read_acspo */
    if( 0 == read_acspo( *(ppFilelist+i), acspo_format, &SST, &Lat, &Lon, 
			 &CldMask, &SST_Variance, par_info, correct_bias, 
			 sses_stdev ) ){
      /* Remove 273.15 from SST */
      subtract_float_array_matlab(&SST,ABS_ZERO);
      /* Loop round for day and night for seperate processing */
      /* day or night selected by start/end_day_night which are inputs */
       for(daynight=start_day_night;daynight<=end_day_night;daynight++){
	/* Get cloud mask - note at the moment it is 0 for both day and night
         * but code kept just in case this ever changes */
	if( 0 == daynight ){
	  /* Daytime */
	  cmask = 0;
	  /* Assign pointer to relevant climatology and threshold */
	  pClimArray = &compare_sst_Day;
	  pClimThresh = &Threshold_Day;
	} else {
	  /* Nighttime */
	  cmask = 0;
	  pClimArray = &compare_sst_Night;
	  pClimThresh = &Threshold_Night;
	}

	/* Now loop around image adding data as appropriate into 
	 * the storage structure */
	/* Get good pixels based on cloud/land mask and SST */
	get_day_night_cld_mask( &CldMask, &SST, daynight, cmask, sst_min,
				&Mask, &NGood );

	/* If there is good data, then fill structure */
	if( 0 < NGood ){
	  /* Loop round data looking for good data and putting data into 
	   * link list structure */
	  for(j=0;j<SST.size;j++){
	    /* Check longitude/latitude are within bounds */
	    if( -180. <= *(Lon.array+j) && 360. >= *(Lon.array+j) &&
		-90. <= *(Lat.array+j) && 90. >= *(Lat.array+j) ){
	      /* a 1 in Mask is good data */
	      if( 1 == *(Mask.array+j) ){
		/* Good data */
		/* Now based on lat/lon, determine which cell this is in */
		xpos = (*(Lat.array+j)-pRef_SST->minlat)/ref_deltalat;
		xpos = (xpos % pRef_SST->sst.nx);
		ypos = (*(Lon.array+j)-pRef_SST->minlon)/ref_deltalon;
		ypos = (ypos % pRef_SST->sst.ny);
		
		/* Check against climatology */
		pos = xpos+ypos*pClimArray->nx;
		if( 0 > pos ){
		  message(1,
			  "ERROR: pos < 0 to add to storage (process_raw_avhrr_acspo_c.c)");
		  /* FAIL the is a bad error */
		  exit(-1);
		}
		if( fabs(*(SST.array+j) - *(pClimArray->array+pos)) < 
		    *(pClimThresh->array+pos) ){
		  /* Add to storage structure */
		  nkeep[daynight] += 1;
		  add_to_storage(*(SST.array+j),*(SST_Variance.array+j),
				 pos,&Stored_SST[daynight]);
		} else {
		  /* Rejected due to climatology check - count number of times */
		  *((pDumpArray+daynight)->array+pos) += 1;
		  badclim[daynight] += 1;
		}
	      }
	    } else {
	      /* Bad position - take away from number of good points */
	      NGood--;
	    }
	  }
	}
	/* Free cloud/day/night mask */
	free_byte_matlab(&Mask);
      }
      /* Free ACSPO data from memory */
      free_float_matlab(&SST);
      free_float_matlab(&Lat);
      free_float_matlab(&Lon);
      free_float_matlab(&SST_Variance);
      free_byte_matlab(&CldMask);
    }
  }

  /* Free filelist */
  free_filelist(&nFiles,&ppFilelist);  
  
  /* Now have read in all ACSPO data */
  /* Now we have to do filtering and calculation of standard deviation/means
   * in each cell of analysis */

  /* Setup output arrays - loop round day/night as appropriate */
  for(daynight=start_day_night;daynight<=end_day_night;daynight++){
    nx = Stored_SST[daynight].nx;
    ny = Stored_SST[daynight].ny;
    (pOutdata+daynight)->nx = nx;
    (pOutdata+daynight)->ny = ny;
    (pOutdata+daynight)->Pixel_Extent.xlow = 1;
    (pOutdata+daynight)->Pixel_Extent.xhigh = nx;
    (pOutdata+daynight)->Pixel_Extent.ylow = 1;
    (pOutdata+daynight)->Pixel_Extent.yhigh = ny;

    /* Allocate memory */
    allocate_float_array_matlab(nx,ny,&(pOutdata+daynight)->SST);
    allocate_float_array_matlab(nx,ny,&(pOutdata+daynight)->Stdev);
    allocate_int_array_matlab(nx,ny,&(pOutdata+daynight)->Gridcnt);
  
    /* Clean data but using more sensible parameters than was done in the 
     * original MATLAB code and calculate mean and standard deviation */
    /* Get appropriate default values */
    if( 0 == daynight ){
      std_default = std_default_day;
    } else {
      std_default = std_default_night;
    }

    /* Actually do clean and mean/stdev calculations */
    cleanup_plus_mean_stdev_sst_data( &Stored_SST[daynight], 
				      (pOutdata+daynight), 
				      par_info.std_threshold, 
				      std_default, &nBadStd, use_sses_stdev );
  
    /* Output number of bad entries etc. */
    nreject = nBadStd + badclim[daynight];

    /* Output message */
    if( 0 > sprintf(messageStr,
		     "*** For ACSPO %s data (date %s) keeping %d & rejecting %d : (clim check) %d : (std check) %d",
		    nameStr,
		    datestring,nkeep[daynight],nreject,
		    badclim[daynight],nBadStd) ){
      message(1,"ERROR: Writing message string for ACSPO");
      /* EXIT */
      exit(-1);
    }
    message(1,messageStr);  

    /* Copy badclim/badstd to output */
    *(pTotal_badclim+daynight) = badclim[daynight];
    *(pTotal_badstd+daynight) = nBadStd;
  }

  /* Free memory */
  free_float_matlab(&compare_sst_Day);
  free_float_matlab(&compare_sst_Night);
  free_float_matlab(&Threshold_Day);
  free_float_matlab(&Threshold_Night);
  if( (0 == start_day_night) || (0 == end_day_night) ){
    free_storage(&Stored_SST[0]);
  }
  if( (1 == start_day_night) || (1 == end_day_night) ){
    free_storage(&Stored_SST[1]);
  }

  /* Return OK */
  return(0);
}

/*
 * NAME: 
 *
 *    cleanup_plus_mean_stdev_sst
 *
 * FUNCTION:
 *
 *    C Code to filter SST values (clean) and then calculate mean and stdev
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Runs a clean algorithm to remove possible cloudy pixels, and then 
 *    calculates the mean and standard deviation.  If a calculation of standard
 *    deviation is not possible (e.g. nSST < MIN_N_SST (5)) then the default 
 *    value is stored
 *
 *    Note that this is an updated version of the algorithm used in the 
 *    original MATLAB code which attempted to calculate the standard devation
 *    and filter cold pixels when there were < MIN_N_SST (5) values which can 
 *    lead to an artificial warming of the mean
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE Stored_SST, SST, SSTVar, stdev_threshold, stdev_default, 
 *               nBadStd 
 * 
 *       allocate sorted arrays
 *
 *       LOOP round all linked lists 
 *          IF data exists THEN
 *             IF number of remaining pixels >= min_no_obs THEN
 *                get standard deviation and mean
 *                IF standard devation > threshold THEN
 *                   clean pixel list by removing cold pixels
 *                ENDIF
 *             ELSE
 *                get mean
 *                set standard deviation to default
 *             ENDIF
 *             Store mean/standard deviation
 *          ENDIF
 *       END LOOP 
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINES/FUNCTIONS CALLS: 
 *
 *    init_float_1d_array_matlab
 *    allocate_float_1d_array_matlab
 *    std_matlab
 *    clean_stdev
 *    free_float_1d_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    cleanup_plus_mean_stdev_sst_data( Stored_SST, SST, SSTVar, 
 *                                      stdev_threshold, stdev_default, 
 *                                      nBadStd, use_sses_stdev )
 *
 * INPUTS:  
 *    
 *   Stored_SST      - Stored lists of SSTs at each cell 
 *   stdev_threshold - threshold to apply clean and also default value
 *   stdev_default   - default value to use when standard deviation cannot be
 *                     calculated
 *   use_sses_stdev  - use any SSTS/Standard deviation for weighted average
 *
 * OUTPUTS:  
 * 
 *   SST             - mean value of SST on grid
 *   SSTVar          - standard deviation of SST on grid
 *   nBadStd         - number of times the clean algorithm was invoked
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *
 */
static void cleanup_plus_mean_stdev_sst_data( const struct SST_Storage *pStored_SST, 
					      struct out_sst_str *pOutput, 
					      const VFLOAT stdev_threshold, 
					      const VFLOAT stdev_default, 
					      VINT *pNBadStd, char use_sses_stdev )
{
  VINT i = 0;
  VINT j = 0;
  VINT nstored = 0;
  VINT max_stored = 0;
  VINT ndump = 0;
  VFLOAT stdev = 0.;
  VFLOAT mean = 0.;
  VFLOAT w = 0.;
  VFLOAT weight_sum = 0.;
  struct float_struct1d *pArr = NULL;
  struct float_struct1d *pArr_Var = NULL;
  struct float_struct1d Sorted;
  struct float_struct1d Sorted_Var;

  /* Initialize arrays */
  init_float_1d_array_matlab(&Sorted);
  init_float_1d_array_matlab(&Sorted_Var);

  /* Check that the inputs have consistent sizes */
  if( (pStored_SST->nx != pOutput->SST.nx) ||
      (pStored_SST->ny != pOutput->SST.ny) ){
    message(1,"ERROR: Inconsistent array sizes in cleanup_plus_mean_stdev_sst_data");
    /* EXIT */
    exit(-1);
  }

  /* Get maximum number of stored entries to allocated sorted array */
  /* This is so we only have to allocate memory once and just re-use it */
  max_stored = 0;
  for(i=0;i<pStored_SST->nx*pStored_SST->ny;i++){
    max_stored = MAX(max_stored,*(pStored_SST->pNStored+i));
  }

  /* Make sure we have some data */
  if( 0 < max_stored ){

    allocate_float_1d_array_matlab(max_stored,&Sorted);
    allocate_float_1d_array_matlab(max_stored,&Sorted_Var);

    /* Initialize badstd variable which contains number of SST rejected by 
     * clean */
    *pNBadStd = 0;

    /* Loop round storage structure */
    for(i=0;i<pStored_SST->nx*pStored_SST->ny;i++){
      /* Check to make sure we have some data stored */
      nstored = *(pStored_SST->pNStored+i);
      if( 0 < nstored ){
	/* Test against number of SSTs - if < min_avhrr_obs_per_cell then use 
	 * default */
	pArr = (pStored_SST->pSST+i);
	pArr_Var = (pStored_SST->pSST_Variance+i);
	if( par_info.min_avhrr_obs_per_cell <= nstored ){
	  /* Determine standard deviation */
	  if( 1 == use_sses_stdev ){
	    stdev = std_matlab_weighted(pArr->array,pArr_Var->array,0,
					nstored,&mean);
	  } else {
	    stdev = std_matlab(pArr->array,0,nstored,&mean);
	  }
	  /* If standard devation > threshold - run clean algorithm */
	  if( stdev_threshold < stdev ){	
	    /* Iterively clean cold pixels */
	    stdev = clean_stdev(pArr,pArr_Var,nstored,stdev_threshold,
				stdev_default,&Sorted,&Sorted_Var,&ndump,
				&mean,use_sses_stdev);
	    /* Increment number of bad SSTs */
	    *pNBadStd += ndump;
	  }
	} else {
	  /* Can't run clean algorithm */
	  /* Set to default value */
	  stdev = stdev_default;
	  /* Calculate mean */
	  if( 1 == use_sses_stdev ){
	    mean = 0.;
	    weight_sum = 0.;
	    for(j=0;j<nstored;j++){
	      w = 1. / *(pArr_Var->array+j);
	      weight_sum += w;
	      mean += w * *(pArr->array+j);
	    }
	    if( weight_sum <= 0. ){
	      mean = NaN;
	      stdev = NaN;
	      nstored = 0;
	    } else {
	      mean /= weight_sum;
	    }
	  } else {
	    mean = 0.;
	    for(j=0;j<nstored;j++){
	      mean += *(pArr->array+j);
	    }
	    mean /= nstored;
	  }
	}
	/* Store values */
	*(pOutput->SST.array+i) = mean;
	*(pOutput->Stdev.array+i) = stdev;      
	*(pOutput->Gridcnt.array+i) = nstored;      
      } else {
	/* No data available */
	/* Set to NaN */
	*(pOutput->SST.array+i) = NaN;
	*(pOutput->Stdev.array+i) = NaN;
	*(pOutput->Gridcnt.array+i) = 0;      
      }
    }

    /* Free Sorted memory */
    free_float_1d_matlab(&Sorted);
    free_float_1d_matlab(&Sorted_Var);

  }
}

/*
 * NAME: 
 *
 *    clean_stdev
 *    std_matlab
 *
 * FUNCTION:
 *
 *    C Code to filter SST values (clean) based on the standard deviation
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Runs a clean algorithm to remove possible cloudy pixels.  It is loosly 
 *    based on the cleanup.mat MATLAB code.
 *
 *    Note that this is an updated version of the algorithm used in the 
 *    original MATLAB code which attempted to calculate the standard devation
 *    and filter cold pixels when there were < 5 values which can lead to 
 *    an artificial warming of the mean
 *
 * PSEUDO CODE
 *
 *    FUNCTION pArray nstored stdev_threshold stdev_default pSorted ndump mean
 *
 *       sort input data
 *
 *       remove coldest pixel
 *
 *       IF number of remaining pixels < min_no_obs THEN
 *          calculate mean
 *          RETURN default standard deviation
 *       ELSE
 *          LOOP while number of remaining pixels > min_no_obs
 *             recalculate standard devation and mean
 *             IF standard devation < standard deviation threshold THEN
 *                RETURN standard deviation
 *             ENDIF
 *             remove coldest pixel
 *             increment number of removed pixels
 *          END LOOP
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    sort_matlab   
 * 
 * CALLING SEQUENCE:  
 *
 *    stdev = clean_stdev( pArray, pArray_Var, nstored, stdev_threshold, 
 *                         stdev_default, pSorted, pSorted_Var, ndump. mean, use_sses_stdev ) 
 *
 * INPUTS:  
 *
 *   pArray          - List of SSTs to be filtered
 *   pArray_Var      - Lust of SST Variances to be filterd
 *   nstored         - number of valid SSTs to be filtered
 *   stdev_threshold - threshold to apply clean and also default value
 *   stdev_default   - default value to use when standard deviation cannot be
 *                     calculated
 *   pSorted         - an array to hold sorted SSTs - size is maximum possible
 *   pSorted_Var     - an array to hold sorted variances - size is maximum 
 *                     possible
 *   use_sses_stdev  - use SSES standard deviation for weighted mean
 *
 * OUTPUTS:  
 *
 *   ndump           - number of entries that have been removed
 *   mean            - mean value
 *
 * RETURNS:
 *
 *   standard deviation of filtered list
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *   Modified to deal with input stdeviations on 06/12/2013
 *
 */
static VFLOAT clean_stdev( struct float_struct1d *pArray,
			   struct float_struct1d *pArray_Var,
			   const VINT nstored,
			   const VFLOAT stdev_threshold, 
			   const VFLOAT stdev_default, 
			   struct float_struct1d *pSorted,
			   struct float_struct1d *pSorted_Var,
			   VINT *pNDump, 
			   VFLOAT *pMean, char use_sses_stdev )
{
  VINT i = 0;
  VINT nRemain = 0;
  VFLOAT stdev = 0.;
  VFLOAT w = 0.;
  VFLOAT weight = 0.;

  /* Sort array */
  /* Reset nx value to nstored - this forces only the stored SSTs to be 
   * sorted */
  pArray->size = nstored;
  bubblesort_matlab( pArray, pArray_Var, pSorted, pSorted_Var );

  /* Since we are in this routine this means that we have to remove the 
   * coldest entry to begin, since the standard deviation already is larger
   * than the threshold */
  nRemain = nstored-1;

  /* set pNDump to one to begin */
  *pNDump = 1;

  /* First check that by removing one SST we have enough SSTs to recalculate
   * the standard deviation */
  if( par_info.min_avhrr_obs_per_cell > nRemain ){
    /* Not enough - calculate mean and set standard deviation to default */
    /* Note use Sorted array since we want to remove the coldest SST */
    if( 1 == use_sses_stdev ){
      *pMean = 0.;
      weight = 0.;
      for(i=0;i<nRemain;i++){
	w = 1. / *(pSorted_Var->array+i);
	weight += w;
	*pMean += w * *(pSorted->array+i);
      }
      *pMean /= weight;    
    } else {
      *pMean = 0.;
      for(i=0;i<nRemain;i++){
	*pMean += *(pSorted->array+i);
      }
      *pMean /= nRemain;    
    }
    /* EXIT Return default standard deviation */
    return(stdev_default);
  } else {
    /* Loop round until the standard devation is < threshold or we have too
     * few SSTs in list to calculate it */
    while( par_info.min_avhrr_obs_per_cell <= nRemain ){
      /* Get standard deviation and mean of remaining pixels */
      if( 1 == use_sses_stdev ){
	stdev = std_matlab_weighted(pSorted->array,pSorted_Var->array,0,nRemain,pMean);
      } else {
	stdev = std_matlab(pSorted->array,0,nRemain,pMean);
      }
      /* If < threshold then return */
      if( stdev < stdev_threshold ){
	/* EXIT return standard deviation */
	return(stdev);
      }
      /* Still not below threshold - remove 1 from nRemain and loop again */
      nRemain--;
      *pNDump += 1;
    }
  }

  /* If we are here, then we've run out of entries in while loop - so 
   * recalculate the mean and output the default standard deviation */
  if( 1 == use_sses_stdev ){
    *pMean = 0.;
    weight = 0.;
    for(i=0;i<nRemain;i++){
      w = 1. / *(pSorted_Var->array+i);
      weight += w;
      *pMean += w * *(pSorted->array+i);
    }
    *pMean /= nRemain;    
  } else {
    *pMean = 0.;
    for(i=0;i<nRemain;i++){
      *pMean += *(pSorted->array+i);
    }
    *pMean /= nRemain;    
  }
    
  /* Return default standard deviation */
  return(stdev_default);
}

/*
 * NAME: 
 *
 *    add_to_storage
 *
 * FUNCTION:
 *
 *    C Code to store all AVHRR data on a grid for further processing
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Stores all AVHRR data for multiple orbits so further filtering can be 
 *    done.  Note deals with memory of linked lists by allocating memory in 
 *    blocks 
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE sst pos storage
 *    
 *       increment number of entries stored at position pos
 *
 *       IF new number of stored entries > memory allocated THEN
 *          increment size of memory at position pos
 *          reallocate memory at position pos 
 *       ENDIF
 *       
 *       store number of entries and sst at location pos
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    reallocate_float_1d_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    add_to_storage( sst, variance, pos, storage )
 *
 * INPUTS:  
 *    
 *   sst             - input sst to be stored
 *   variance        - input sst variance for weighted averaging
 *   pos             - array position in storage for data where data is to be 
 *                     stored
 *
 * OUTPUTS:  
 * 
 *   storage         - storage structure with all SSTs
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *
 */
static void add_to_storage(const VFLOAT sst, const VFLOAT variance,
			   const VINT pos, struct SST_Storage *pStorage)
{
  VINT nstored = 0;
  struct float_struct1d *pArray = NULL;
  struct float_struct1d *pArray_Var = NULL;

  nstored = *(pStorage->pNStored+pos)+1;
  
  /* Check to see if we need to allocate more memory for this slot */
  pArray = (pStorage->pSST+pos);
  pArray_Var = (pStorage->pSST_Variance+pos);
  if( nstored > *(pStorage->pNMax+pos) ){
    /* Allocate more memory */
    *(pStorage->pNMax+pos) += STORE_STEP;    
    reallocate_float_1d_array_matlab(*(pStorage->pNMax+pos),pArray);
    reallocate_float_1d_array_matlab(*(pStorage->pNMax+pos),pArray_Var);
  }

  /* Add new SST entry in */
  *(pStorage->pNStored+pos) = nstored;  
  *(pArray->array+nstored-1) = sst;
  *(pArray_Var->array+nstored-1) = variance;

}

/*
 * NAME: 
 *
 *    init_storage
 *
 * FUNCTION:
 *
 *    C Code to initialize storage arrays for SST
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Initializes the storage arrays for ACSPO SSTs
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE storage
 *
 *       set sizes to zero
 *       set pointers to NULL
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINES CONTAINED: 
 *
 * CALLING SEQUENCE:  
 *
 *    init_storage( storage )
 *
 * INPUTS:  
 *
 *   storage         - storage structure for all SSTs
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *
 */
static void init_storage(struct SST_Storage *pStorage)
{
  /* Setup top level entries */
  pStorage->nx = 0;
  pStorage->ny = 0;
  pStorage->pNMax = NULL;
  pStorage->pNStored = NULL;
  pStorage->pSST = NULL;
  pStorage->pSST_Variance = NULL;
}

/*
 * NAME: 
 *
 *    setup_storage
 *
 * FUNCTION:
 *
 *    C Code to setup storage arrays for SST
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE nx ny storage
 *
 *       allocate NMax storage
 *       allocate NStored storage
 *      
 *       allocate nx,ny pointer to linked lists of SST
 *
 *       LOOP over nx*ny cells
 *          allocate individial linked list (STORE_STEP elements)
 *       END LOOP
 *
 *    END
 *
 * DESCRIPTION:  
 *
 *    Sets up the storage arrays for ACSPO SSTs
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_1d_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    setup_storage( nx, ny, storage )
 *
 * INPUTS:  
 *    
 *   nx              - X size (reference) for array
 *   ny              - Y size (reference) for array
 *
 * OUTPUTS:  
 * 
 *   storage         - storage structure for all SSTs
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   calloc
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *
 */
static void setup_storage(const VINT nx, const VINT ny, struct SST_Storage *pStorage)
{
  /* Local variables */
  VINT i = 0;

  /* Setup top level entries */
  pStorage->nx = nx;
  pStorage->ny = ny;

  /* Setup size arrays to store how much memory has been allocated, and 
   * how many entries have been stored */
  if( NULL == (pStorage->pNMax = (VINT *) calloc(nx*ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pStorage->pNMax (setup_storage)");
    /* EXIT */
    exit(-1);
  }
  if( NULL == (pStorage->pNStored = (VINT *) calloc(nx*ny,sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate memory for pStorage->pNStored (setup_storage)");
    /* EXIT */
    exit(-1);
  }

  /* Set initial values - start out with a maximum of INITIAL_STORE  entries*/
  for(i=0;i<nx*ny;i++){
    *(pStorage->pNStored+i) = 0;
    *(pStorage->pNMax+i) = INITIAL_STORE;
  }

  /* Allocate top level storage structure */
  if( NULL == (pStorage->pSST = (struct float_struct1d *) 
	       calloc(nx*ny,sizeof(struct float_struct1d))) ){
    message(1,"ERROR: Cannot allocate meory for pStorage->pSST (setup_storage)");
    /* EXIT */
    exit(-1);
  }

  if( NULL == (pStorage->pSST_Variance = (struct float_struct1d *) 
	       calloc(nx*ny,sizeof(struct float_struct1d))) ){
    message(1,"ERROR: Cannot allocate meory for pStorage->pSST (setup_storage)");
    /* EXIT */
    exit(-1);
  }

  /* Set all entries to have storage for up to INITIAL_STORE SSTs - this will 
   * be expanded if necessary when the SST data is added */
  for(i=0;i<nx*ny;i++){
    /* Make sure that the pointer is NULL for allocate checks */
    init_float_1d_array_matlab((pStorage->pSST+i));
    init_float_1d_array_matlab((pStorage->pSST_Variance+i));
    /* Now allocate */
    allocate_float_1d_array_matlab(INITIAL_STORE,(pStorage->pSST+i)); 
    allocate_float_1d_array_matlab(INITIAL_STORE,(pStorage->pSST_Variance+i)); 
  }

}

/*
 * NAME: 
 *
 *    free_storage
 *
 * FUNCTION:
 *
 *    C Code to free up storage arrays for SST
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Frees the storage arrays for ACSPO SSTs
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE storage
 * 
 *      free NMax       
 *      free NStored
 *      LOOP over cells
 *         free link list
 *      END LOOP
 *      free SST (top level to link lists)
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    free_float_1d_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    free_storage( storage )
 *
 * INPUTS:  
 *    
 * OUTPUTS:  
 * 
 *   storage         - storage structure for all SSTs
 *
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 *   free
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 09/01/2012
 *
 */
static void free_storage(struct SST_Storage *pStorage)
{
  /* Local variables */
  VINT i = 0;

  /* Free memory for size arrays */
  free(pStorage->pNMax);
  pStorage->pNMax = NULL;
  free(pStorage->pNStored);
  pStorage->pNStored = NULL;

  /* Loop round and free individual entries */
  for(i=0;i<pStorage->nx*pStorage->ny;i++){
    free_float_1d_matlab((pStorage->pSST+i));
    free_float_1d_matlab((pStorage->pSST_Variance+i));
  }

  /* Free top level */
  free(pStorage->pSST);
  free(pStorage->pSST_Variance);
  pStorage->pSST = NULL;
  pStorage->pSST_Variance = NULL;

  /* Set size variables to zero */
  pStorage->nx = 0;
  pStorage->ny = 0;

}

/*
 * NAME: 
 *
 *    get_day_night_cld_mask
 *
 * FUNCTION:
 *
 *    C Code to get where day/night cloud free ocean data are
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    get day or night data which is cloud free ocean - use the ACSPO mask
 *
 * PSEUDO CODE
 *
 *    SUBROUTINE Cldmask, sst, day_or_night, cmask, outarray, ngood 
 *
 *       select day/night flag set 
 *
 *       add cmask to flags
 *
 *       allocate output mask
 *
 *       LOOP over input cloud mask
 *          check cloud mask against flags and add to output mask if needed
 *       END LOOP
 *       
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_char_array_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    get_day_night_cld_mask( Cldmask, sst, day_or_night, cmask, outarray, 
 *                            ngood )
 *
 * INPUTS:  
 *    
 *   type            - Cldmask - from ACSPO
 *   sst             - input sst to test against par_info.
 *   day_or_night    - 0 = day, 1 = night
 *   cmask           - select which cloud mask value to use
 *   sst_min         - SST minimum to accept
 *
 * OUTPUTS:  
 * 
 *   outarray        - output array that contains new mask (char_struct)
 *   ngood           - number of good entries
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
static void get_day_night_cld_mask( const struct char_struct *pCldmask, 
				    const struct float_struct *pSST,
				    const VINT day_or_night, char cmask, 
				    const VFLOAT sst_min,
				    struct char_struct *pNewMask,
				    VINT *pNgood )
{
  /* Local variables */
  VINT i = 0;
  /* Cloud mask bits */
  /* bit 1 - invalid (1) / valid (0) mask
   * bit 2 - day (1) / night (0)
   * bit 3 - land (1) / sea (0)
   * bit 4 - coast (1) / not coast (0)
   * bit 5 - glint (1) / no glint (0)
   * bit 6 - snow (1) / no snow (0)
   * bit 7 - cloud mask
   * bit 8 - ditto
   */  
  /* Note that because we don't care about glint there are 2 possbilities that
   * are acceptable for day or night */
  /* no glint - daytime bits are (from bit 0-6) 0 1 0 0 0 0) */
  char day_mask_1 = 2;
  /* glint - daytime bits are (from bit 0-6) 0 1 0 0 1 0) */
  char day_mask_2 = 18;
  /* no glint - night time bits are (from bit 0-6) 0 0 0 0 0 0) */
  char night_mask_1 = 0;
  /* glint - night time bits are (from bit 0-6) 0 0 0 0 1 0) */
  char night_mask_2 = 16;

  char mask_1 = 0;
  char mask_2 = 0;
  char mask = 0;

  *pNgood = 0;
  if( 0 == day_or_night ){
    mask_1 = day_mask_1;
    mask_2 = day_mask_2;
  } else {
    mask_1 = night_mask_1;
    mask_2 = night_mask_2;
  }

  /* Shift first 2 bits of input cmask to bits 7 and 8 */
  /* This enables control of cloud mask part from calling routine */
  mask_1 = (mask_1 | ((cmask & 0x03) << 6));
  mask_2 = (mask_2 | ((cmask & 0x03) << 6));

  /* Allocate output masks */
  allocate_byte_array_matlab( pCldmask->nx, pCldmask->ny, pNewMask );
  
  /* Loop round cloud mask and check both against min SST and cloud/day/night
   * mask */
  for(i=0;i<pCldmask->size;i++){
    mask = *(pCldmask->array+i);
    if( *(pSST->array+i) > sst_min && ((mask == mask_1) || (mask == mask_2)) ){
      *(pNewMask->array+i) = 1;
      *pNgood += 1;
    }
  }
}

