/*
 * NAME: 
 *
 *    process_raw_avhrr_acspo_c
 *
 * FUNCTION:
 *
 *    C version of process_raw_avhrr_acspo.m - reads in AVHRR data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in AVHRR ACSPO data.  Note that this works
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
 *    process_raw_avhrr_acspo
 *
 * ROUTINES USED FROM process_raw_avhrr_acspo.c :
 *
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
#include "read_acspo.h"
#include "process_raw_avhrr_acspo_c.h"
#include "diurnal_warming.h"
#include "read_geo_data.h"
#include "read_geo_ghrsst.h"

/* Internally defined structures */
/* Structure to hold each cells list of SSTs for further analysis */
/* Storage increment - initial low for equator pixels then step size for
 * extra memory in linked lists */
#define INITIAL_STORE 2
#define STORE_STEP 5

#define MAX_GOES_NUMBER 10

/*
 * NAME: 
 *
 *    process_raw_avhrr_acspo_c
 *
 * FUNCTION:
 *
 *    C Code to read in AVHRR ACSPO data for one day
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in AVHRR data for a complete day.  As each ASCPO file is read in
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
 *    this effect and are controled by a par_info parameter 
 *    min_avhrr_obs_per_cell.
 *
 *    Note also that this code deals with all Polar satellites in one 
 *    routine
 *
 * PSEUDO CODE
 *
 *    FUNCTION noaa_name, year, day, ref_sst, sst_variability, sst_check, 
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
 *       find ACSPO files
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
 *    ok = process_raw_avhrr_acspo_c( noaa_name, year, day, ref_sst, 
 *                            sst_variability, sst_check, biasDay, biasNight
 *			      Outdata, DumpArray, Total_badclim,
 *                            Total_badstd )
 *
 * INPUTS:  
 *    
 *   noaa_name       - AVHRR name - noaa16, noaa17, noaa18, noaa19,  
 *                     METOPA/METOPB or VIIRS 
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
VINT process_raw_geo_ghrsst_c( const char *pType, const VINT year, 
				const VINT day, 
				const struct sst_struct *pRef_SST, 
				const struct sst_struct *pSST_Variability, 
				const VINT sst_check, 
				const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				const struct par_files_str par_files )
/* VINT process_raw_geo_c( 	const char *pType, const VINT year, 
				const VINT day,  
				struct sst_struct *pRef_SST, 
				struct sst_struct *pSST_Variability, 
				const VINT sst_check, 
				const struct float_struct *pBiasDay,
				const struct float_struct *pBiasNight,
				struct out_sst_str *pOutdata,
				const struct par_files_str par_files ) */
{
  /* Local variables */
  VINT correct_bias = 0;
  VINT use_sses_stdev = 0;

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
  VINT noaa_no = 0;

  VFLOAT std_default = 0.;
  VFLOAT std_default_day = 0.;
  VFLOAT std_default_night = 0.;
  VFLOAT sst_min = 0.;
  VFLOAT sst_max = 0.;
  VFLOAT ref_deltalon = 0.;
  VFLOAT ref_deltalat = 0.;
  VFLOAT sses_stdev = 0.;

  char cmask = 0;
  VINT start_day_night = 0;
  VINT end_day_night = 0;

  struct float_struct Threshold_Day;
  struct float_struct Threshold_Night;
  struct float_struct *pClimThresh = NULL;
  struct float_struct compare_sst_Night;
  struct float_struct compare_sst_Day;
  struct float_struct *pClimArray = NULL;
  struct char_struct Mask; /* again, should be able to simply test on valid SST */
  struct char_struct okdata;

  /* Data from Geo GHRSST */

  struct float_struct SST;
  struct float_struct Lat;
  struct float_struct Lon;
  struct float_struct SST_Variance;
  struct char_struct CldMask;  /* not actually used because Geo GHRSST reader already masks on pClr */

  char **ppFilelist = NULL;
  char directory_nav[MAX_STRING_LENGTH];
  char directory_sst[MAX_STRING_LENGTH];
  char daystr[MAX_STRING_LENGTH];
  char datestring[MAX_STRING_LENGTH];
  char messageStr[MAX_STRING_LENGTH];
  char searchString[MAX_STRING_LENGTH];
  char nameStr[MAX_STRING_LENGTH];
  char acspo_format[MAX_STRING_LENGTH];
  char DWfile_stem[MAX_STRING_LENGTH];
  char DWfile[MAX_STRING_LENGTH];

  /* Storage structure */
  /* Note - dimension of 2 for day/night */
  struct SST_Storage Stored_SST[2];

  char use_GHRSST = 1;
  char use_NETCDF = 1;
  char use_HDF = 1;

  char found_goes = 0;
  char ngoes_number = 0;
  char goes_number[MAX_GOES_NUMBER];

  char *myPtr;

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
  init_byte_array_matlab(&CldMask); /* Not filled, but is an argument (see also read_raw_geo.c) */
  init_storage(&Stored_SST[0]);
  init_storage(&Stored_SST[1]);

  /* Inits files/pars */
  init_file_info(par_files.file_info);
  init_par_info(par_files.par_info);


  /* Get day/night flags */
  /* Note 0 = day and 1 = night */
  start_day_night = par_info.start_day_night;
  end_day_night = par_info.end_day_night;  
 
  /* Allocate memory and assign for Thresholds (Day/Night) */
  copy_float_matlab(&pSST_Variability->sst,&Threshold_Day);
  copy_float_matlab(&pSST_Variability->sst,&Threshold_Night);


  /* Get default standard deviation & other params */
  /* Depends on input type */
  if( (0 == strstr("e",pType)) ){
    std_default = par_info.default_goes_std_e;
    use_sses_stdev = par_info.goese_use_sses_stdev;
    correct_bias = par_info.goese_sses_bias;
    sses_stdev = par_info.goese_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_goes_std_e;
    std_default_night = par_info.default_goes_std_e;
  } else if( (0 == strstr("w",pType)) ){
    std_default = par_info.default_goes_std_w;
    use_sses_stdev = par_info.goesw_use_sses_stdev;
    correct_bias = par_info.goesw_sses_bias;
    sses_stdev = par_info.goesw_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_goes_std_w;
    std_default_night = par_info.default_goes_std_w;
 } else if( (0 == strstr("p",pType)) ){
    std_default = par_info.default_goes_std_p;
    use_sses_stdev = par_info.goesp_use_sses_stdev;
    correct_bias = par_info.goesp_sses_bias;
    sses_stdev = par_info.goesp_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_goes_std_p;
    std_default_night = par_info.default_goes_std_p;
  } else if( (0 == strstr("msg",pType)) ){
    std_default = par_info.default_msg_std;
    use_sses_stdev = par_info.msg_use_sses_stdev;
    correct_bias = par_info.msg_sses_bias;
    sses_stdev = par_info.msg_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_msg_std;
    std_default_night = par_info.default_msg_std;
  } else if( (0 == strstr("mio",pType)) ){
    std_default = par_info.default_mio_std;
    use_sses_stdev = par_info.mio_use_sses_stdev;
    correct_bias = par_info.mio_sses_bias;
    sses_stdev = par_info.mio_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_mio_std;
    std_default_night = par_info.default_mio_std;
  } else if( (0 == strstr("mtsat",pType)) ){
    std_default = par_info.default_mtsat_std;
    use_sses_stdev = par_info.mtsat_use_sses_stdev;
    correct_bias = par_info.mtsat_sses_bias;
    sses_stdev = par_info.mtsat_sses_stdev;
    mult_float_array_matlab(&Threshold_Day, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Day, par_info.goes_threshold_constant);
    /* Night threshold for quality control - identical for geo data */
    mult_float_array_matlab(&Threshold_Night, par_info.goes_threshold_mult);
    add_float_array_matlab(&Threshold_Night, par_info.goes_threshold_constant);
    std_default_day = par_info.default_mtsat_std;
    std_default_night = par_info.default_mtsat_std;
  } else {
    message(1,"ERROR: Cannot match GEO type to e/w/p/msg/mio/mtsat");
    /* EXIT Type incorrect */
    return(1);
  }

  /* We know it's GHRSST */
  strcpy(acspo_format,par_info.geo_format);


  /* Check to see if HDF or NETCDF or GHRSST format needed - set to GHRSST!! */
  use_GHRSST = 1;
  use_NETCDF = 0;
  use_HDF = 0;

  /* Get datestring - get_datestring2 specific for AVHRR */
  /* For GHRSST format */
  strncpy(datestring,get_ghrsst_datestring(year,day),MAX_STRING_LENGTH);

  /* Set minimum SST from previous days analysis which is an input - also set the max */
  sst_min = par_info.sst_analysis_min - 2.;
  sst_max = par_info.sst_analysis_max + 2.;

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
    message(1,"ERROR: Climatology/Tresholds have difference sizes (process_raw_geo_ghrsst_c)");
    /* EXIT */
    exit(-1);
  }

  /* Get search string */
  if( (0 == strcmp(pType,"e")) || (0 == strcmp(pType,"w")) || (0 == strcmp(pType,"p")) ){
    /*
     *    GOES DATA 
     */
      /* GHRSST GDS 2.0 format */      
      /* Depending on E/W get which GOESs to look for */
      /* For East */
      if( 0 == strcmp(pType,"e") ){
	ngoes_number = 5;
	goes_number[0] = 19;
	goes_number[1] = 16;
	goes_number[2] = 13;
	goes_number[3] = 12;
	goes_number[4] = 8;
      } else if( 0 == strcmp(pType,"w") ){
	ngoes_number = 6;
	goes_number[0] = 18;
	goes_number[1] = 17;
	goes_number[2] = 15;
	goes_number[3] = 11;
	goes_number[4] = 10;
	goes_number[5] = 9;
      } else if( 0 == strcmp(pType,"p") ){
	ngoes_number = 1;
	goes_number[0] = 9;
      } else {
	message(1,"ERROR: Goes type not e or w or p");
	/* EXIT type not recognized */
	exit(-1);
      }
      /* Get search string */
      found_goes = 0;
      for(i=0;i<ngoes_number;i++){
	if( 0 == strcmp(pType,"p") ){
		sprintf(searchString,"%s/%s*-*L2P_GHRSST-SSTskin-GOES%d-*.nc",
			file_info.dir_mtsat,datestring,goes_number[i]);
	} else {
		sprintf(searchString,"%s/%s*-*L2P_GHRSST-SST*G*%02d-*.nc",
			file_info.dir_goes,datestring,goes_number[i]);
	}
	if( 0 != find_files(searchString,&nFiles,&ppFilelist) ){
	  found_goes = 1;
	  break;
	}
      }
      /* If haven't found any... */
      if( 0 == found_goes ){
	message(1,"*** WARNING");
	message(6,"*** No files found for GOES ",pType," for ",
		datestring," in ",file_info.dir_goes);
/*	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;	*/
	/* EXIT no files found */
	return(1);
      }
      sprintf(nameStr,"GOES");
  } else if( 0 == strcmp(pType,"mtsat") ){
    /*
     *    MTSAT DATA 
     */
      /* GHRSST */
/*      Check for Himawari first, then try for MTSAT	*/
      sprintf(searchString,"%s/%s*-*_GHRSST-*-AHI_H*.nc",
	      file_info.dir_mtsat,datestring);
      if( 1 != find_files(searchString,&nFiles,&ppFilelist) ){
         sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-MTSAT*.nc",
	         file_info.dir_mtsat,datestring);
         if( 1 != find_files(searchString,&nFiles,&ppFilelist) ){
	   message(1,"*** WARNING");
	   message(4,"*** No files found for MTSAT for ",
		   datestring," in ",searchString);
/*	   pBigGridGeoSST->nData = 0;
	   pPixel_Extent->xlow = 0;
	   pPixel_Extent->xhigh = 0;
	   pPixel_Extent->ylow = 0;
	   pPixel_Extent->yhigh = 0;	*/
	   /* EXIT no files found */
	   return(1);
         }  
      }
      sprintf(nameStr,"MTSAT");
  } else if( 0 == strcmp(pType,"msg") ){
    /*
     *    MSG DATA 
     */
      /* GHRSST */
      sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-MSG*.nc",
	      file_info.dir_msg,datestring);
      if( 1 != find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(4,"*** No files found for MSG for ",
		datestring," in ",file_info.dir_msg);
/*	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;	*/
	/* EXIT no files found */
	return(1);
      }
      sprintf(nameStr,"MSG");
  } else if( 0 == strcmp(pType,"mio") ){
    /*
     *    MSG DATA in Indian Ocean
     */
      /* GHRSST */
      sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-MSG*.nc",
	      file_info.dir_mio,datestring);
      if( 1 != find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(4,"*** No files found for MSG for ",
		datestring," in ",file_info.dir_mio);
/*	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;	*/
	/* EXIT no files found */
	return(1);
      }
      sprintf(nameStr,"MSG");
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

  /* Allocate dump arrays for day and night *//*
  if( (0 == start_day_night) || (0 == end_day_night) ){
    allocate_int_array_matlab(pRef_SST->sst.nx,pRef_SST->sst.ny,
			      pDumpArray);
  }
  if( (1 == start_day_night) || (1 == end_day_night) ){
    allocate_int_array_matlab(pRef_SST->sst.nx,pRef_SST->sst.ny,
			      (pDumpArray+1));
  }	*/

  /* Read in diurnal warming NetCDF file if necessary */
  if( 1 == par_info.correct_diurnal_warming ){
    strcpy(DWfile_stem,file_info.dir_Diurnal);
    strcat(DWfile_stem,"/");
    strcat(DWfile_stem,file_info.name_Diurnal);
    make_diurnal_filename(DWfile_stem,year,day,DWfile);
    if( 1 == read_diurnal_file(DWfile) ){
      message(1,"ERROR: Cannot read in DW NetCDF file");
      message(1,"ERROR: Turning off DW correction (ACSPO processing)");
      par_info.correct_diurnal_warming = 0;
    }
  }

  /* Now loop round files */
  for(i=0;i<nFiles;i++){
    /* Read in acspo data */
    /* Note, if requested, diurnal warming is corrected for within read_acspo */
    if( 0 == read_geo_ghrsst( *(ppFilelist+i), acspo_format, &SST, &Lat, &Lon, 
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
        if((myPtr = strstr(*(ppFilelist+i), "ACSPO")) == NULL){
        /*  If can't find "ACSPO" in filename, it's a "regular" Geo SST L2P, so...  */
	/* ...make our own cloud mask first - Geo GHRSST data are already screened vs. pClr & Day/Night*/
   	  allocate_byte_array_matlab( CldMask.nx, CldMask.ny, &Mask );

	  NGood = 0;

	  for(j=0;j<CldMask.size;j++){
	    if( *(SST.array+j) > sst_min && *(SST.array+j) < sst_max && daynight == *(CldMask.array+j)){
	    *(Mask.array+j) = 1;
	    NGood += 1;
	    }
	  }
        } else {
        /*  ...otherwise it's an ACSPO Geo SST L2P (e.g Himawari-8).  Note change of ghrsst_min_quality  */
 	get_day_night_cld_mask( &CldMask, &SST, daynight, cmask, sst_min,
				&Mask, &NGood );
        }

	/* Loop round cloud mask and check both against min SST and cloud/day/night
	* mask */

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
		  /* Rejected due to climatology check - count number of times *//*
		  *((pDumpArray+daynight)->array+pos) += 1;	*/
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
  
  /* A final check to set all pixels with a count of zero to NaN */

    for(i=0;i<nx*ny;i++){
      if(*((pOutdata+daynight)->Gridcnt.array+i) < 1){
	*((pOutdata+daynight)->SST.array+i) = NaN;
	*((pOutdata+daynight)->Stdev.array+i) = NaN;
      }
    }

    /* Output number of bad entries etc. */
    nreject = nBadStd + badclim[daynight];

    /* Output message */
    if( 0 > sprintf(messageStr,
		     "*** For GEO GHRSST %s data (date %s) keeping %d & rejecting %d : (clim check) %d : (std check) %d",
		    nameStr,
		    datestring,nkeep[daynight],nreject,
		    badclim[daynight],nBadStd) ){
      message(1,"ERROR: Writing message string for GEO GHRSST");
      /* EXIT */
      exit(-1);
    }
    message(1,messageStr);  

    /* Copy badclim/badstd to output *//*
    *(pTotal_badclim+daynight) = badclim[daynight];
    *(pTotal_badstd+daynight) = nBadStd;	*/
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


