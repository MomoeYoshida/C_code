/*
 * NAME: 
 *
 *    read_geo_data
 *
 * FUNCTION:
 *
 *    C version of read_day*.m - reads in GOES/MTSAT/MSG data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in Geostationary data (choose which one)
 *    data.  Note original MATLAB version had separate functions for each 
 *    geo, but this does them all
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_geo_data
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
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
#include "read_raw_geo.h"
#include "read_geo_data.h"

/* Global variable */
#define MAX_GOES_NUMBER 10

/*
 * NAME: 
 *
 *    read_geo_data
 *
 * FUNCTION:
 *
 *    C Code to read in Geostionary data for one day
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in GOES/MTSAT/MSG data for a 24 hr period, selecting either 
 *    daytime or nighttime images as selected by the day_night_type input flag.
 *    If selected some quality control is done by comparing with the SST
 *    variability data.
 *
 * PSEUDO CODE
 *
 *    FUNCTION  type, year, day, frequency, ref_sst, 
 *              sst_variability, sst_check, biggridsst, 
 *              pixel_extent, day_night_type, biggeo_index,
 *              par_files 
 *
 *       initialize arrays
 *       initialize par_files
 *
 *       get datestring
 *       check goes type
 *
 *       get day/night stem on the basisi of day_night_type
 *
 *       IF GOES type required THEN 
 *          search for GOES filelist
 *       ELSE IF MSG type required THEN
 *          search for MSG filelist
 *       ELSE IF MTSAT type required THEN
 *          search for MTSAT filelist
 *       ENDIF
 *
 *       zero index
 *       LOOP round number of files found (index)
 *
 *          IF read in GOES/MSG/MTSAT data is OK THEN
 *
 *            IF first time read in data THEN
 *                get Pixel extents
 *                IF sst check turned on THEN
 *                    rebin reference SST to SST gridsize
 *                    rebin variability SST to SST gridsize
 *                    get threshold array from array = 
 *                       (variability*threshold_mult + threshold_constant)
 *                ENDIF
 *            ENDIF    
 *
 *            IF image size > 0 THEN
 *
 *                IF sst check turned on THEN
 *                    get absolute difference between SST and reference
 *                    find pixels where SST difference > threshold and set to NaN
 *                    determine number of good pixels
 *                    rebin SST to output grid (at index location)
 *                    increment index
 *                ELSE
 *                    rebin SST to output grid (at index location)
 *                    increment index
 *                ENDIF
 *
 *            ENDIF
 *
 *          ENDIF
 *
 *       END LOOP
 *
 *       set number of filled output SST arrays in output structure
 *
 *       free filelist
 *
 *       IF sst check turned on THEN
 *           free variability test arrays
 *       ENDIF
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 *    Input GOES/MSG/MTSAT files
 *
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    init_sst_matlab
 *    init_float_array_matlab
 *    init_elem_matlab
 *    init_file_info
 *    init_par_info
 *    find_files
 *    read_raw_geo
 *    rebin_data_matlab
 *    allocate_float_array_matlab
 *    copy_float_matlab
 *    mult_float_array_matlab
 *    add_float_array_matlab
 *    array_float_oper_matlab
 *    array_float_abs_matlab
 *    find_float_array_matlab
 *    set_find_float_matlab
 *    free_elem_matlab
 *    free_sst_matlab
 *    free_filelist
 *    free_float_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_geo_data( type, year, day, frequency, ref_sst, 
 *                        sst_variability, sst_check, biggridsst, 
 *                        pixel_extent, day_night_type, par_files )
 *
 * INPUTS:  
 *    
 *   type            - type of satellite to read in (GOES_TYPE etc.)
 *   year            - year
 *   day             - day number of year
 *   frequency       - hourly frequency of input data (currently 1, 3, 24)
 *   ref_sst         - reference sst for bias correction
 *   sst_variability - sst variability data
 *   sst_check       - check against limits of SST (0=no, 1=yes)
 *   day_night_type  - type of read - 0=daytime 1=nighttime
 *   par_files       - structure containing names of file and parameter files
 *
 * OUTPUTS:  
 * 
 *   biggridsst      - rebinned data to required resolution
 *   pixel_extent    - extent in global map - struct pixel_extent_str
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    strcmp
 *    strcpy
 *    toupper
 *    sprintf
 *    sscanf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
VINT read_geo_data( const char *pType, const VINT year, const VINT day, 
		    const VINT frequency, 
		    struct sst_struct *pRef_SST, 
		    struct sst_struct *pSST_Variability, 
		    const VINT sst_check, struct big_geo_grid *pBigGridGeoSST, 
		    struct pixel_extent_str *pPixel_Extent,
		    const unsigned char day_night_type, 
		    const struct par_files_str par_files, char use_sses_stdev )
{
  /* Local variabiles */
  VINT i = 0;
  VINT j = 0;
  VINT k = 0;
  VINT nFiles = 0;
  char **ppFilelist = NULL;
  char searchString[MAX_STRING_LENGTH];
  char datestring[MAX_STRING_LENGTH];
  char *pStr = NULL;
  char Hour_Str[3];
  char messageStr[MAX_STRING_LENGTH];
  char day_night_string[MAX_STRING_LENGTH];
  
  VINT pos = 0;
  VINT nbad = 0;
  VINT ngood = 0;
  VINT nx = 0;
  VINT ny = 0;
  VINT Hour = 0;
  VFLOAT threshold_mult = 0.;
  VFLOAT threshold_cnst = 0.;

  VFLOAT ref_deltalat = 0.;
  VFLOAT ref_deltalon = 0.;

  VINT xpos = 0;
  VINT ypos = 0;
  
  struct sst_struct SST;
  struct sst_struct dummySST;
  struct float_struct RebinSSTRef;
  struct float_struct RebinVarRef;
  struct float_struct Diff;
  struct float_struct threshold;
  struct find_elems Bad;
  struct find_elems Good;

  struct float_struct *Lat;
  struct float_struct *Lon;

  char type = 0;
  char use_GHRSST = 0;
  char found_goes = 0;
  char ngoes_number = 0;
  char goes_number[MAX_GOES_NUMBER];

  /* Init arrays */
  init_sst_matlab(&SST);
  init_sst_matlab(&dummySST);
  init_float_array_matlab(&RebinSSTRef);
  init_float_array_matlab(&RebinVarRef);
  init_float_array_matlab(&Diff);
  init_float_array_matlab(&threshold);
  init_elem_matlab(&Bad);
  init_elem_matlab(&Good);

  /* Run init_par_info/init_file_info */
  init_file_info(par_files.file_info);
  init_par_info(par_files.par_info);

  /* Check if GHRSST is to be used or not */
  if( NULL != strstr(par_info.geo_format,"GHRSST") ){
    use_GHRSST = 1;
  } else {
    use_GHRSST = 0;
  }

  /* Get datestring - GOES/MSG and MTSAT have same format */
  /* If flat binary */
  if( 0 == use_GHRSST ){
    strncpy(datestring,get_datestring(year,day),MAX_STRING_LENGTH);
  } else {
    strncpy(datestring,get_ghrsst_datestring(year,day),MAX_STRING_LENGTH);
  }

  if( (0 != strcmp(pType,"e")) && (0 != strcmp(pType,"w")) && 
      (0 != strcmp(pType,"p")) && (0 != strcmp(pType,"mtsat")) &&
      (0 != strcmp(pType,"msg")) ){
    message(1,
	    "ERROR: Input type must be e,w,p,mtsat,msg in read_geo_data");
    /* EXIT */
    exit(-1);
  }

  /* day_night_type tells code to retrieve either day or night data */  
  /* If flat binary (not GHRSST) then defines a file stem */
  if( 0 == use_GHRSST ){
    if( 0 == day_night_type ){
      strcpy(day_night_string,"_day");
    } else if( 1 == day_night_type ){
      strcpy(day_night_string,"_nite");
    } else {
      message(1,"ERROR: day_night_type param to read_geo_data incorrect");
      /* EXIT */
      exit(-1);
    }
  } else {
    if( 0 != day_night_type && 1 != day_night_type ){
      message(1,"ERROR: day_night_type param to read_geo_data incorrect");
      /* EXIT */
      exit(-1);
    }
  }

  /* Get lists of files of given type */
  /* Get search string */
  if( (0 == strcmp(pType,"e")) || (0 == strcmp(pType,"w")) ){
    /*
     *    GOES DATA 
     */
    /* Thresholds */
    threshold_mult = par_info.goes_threshold_mult;
    threshold_cnst = par_info.goes_threshold_constant;
    /* For flat binary make filesearch string and find files */
    if( 0 == use_GHRSST ){
      /* Make uppercase */
      type = toupper(*pType);
      sprintf(searchString,"%s/sst%1do%c*%s*%s",file_info.dir_goes,frequency,
	      *pType,datestring,day_night_string);
      if( 0 == find_files(searchString,&nFiles,&ppFilelist) ){
	/* Try uppercase version of 'W' 'E' */
	sprintf(searchString,"%s/sst%1do%c*%s*%s",file_info.dir_goes,frequency,
		type,datestring,day_night_string);
	if( 0 == find_files(searchString,&nFiles,&ppFilelist) ){ 
	  message(1,"*** WARNING");
	  message(6,"*** No files found for GOES ",pType," for ",
		  datestring," in ",file_info.dir_goes);
	  pBigGridGeoSST->nData = 0;
	  pPixel_Extent->xlow = 0;
	  pPixel_Extent->xhigh = 0;
	  pPixel_Extent->ylow = 0;
	  pPixel_Extent->yhigh = 0;
	  /* EXIT no files found */
	  return(1);
	}
      }
    } else {
      /* GHRSST GDS 2.0 format */      
      /* Depending on E/W get which GOESs to look for */
      /* For East */
      if( 0 == strcmp(pType,"e") ){
	ngoes_number = 3;
	goes_number[0] = 13;
	goes_number[1] = 12;
	goes_number[2] = 8;
      } else if( 0 == strcmp(pType,"w") ){
	ngoes_number = 4;
	goes_number[0] = 15;
	goes_number[1] = 11;
	goes_number[2] = 10;
	goes_number[3] = 9;
      } else {
	message(1,"ERROR: Goes type not e or w");
	/* EXIT type not recognized */
	exit(-1);
      }
      /* Get search string */
      found_goes = 0;
      for(i=0;i<ngoes_number;i++){
	sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-GOES%2.2d-*.nc",
		file_info.dir_goes,datestring,goes_number[i]);
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
	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;
	/* EXIT no files found */
	return(1);
      }
    }
  } else if( 0 == strcmp(pType,"mtsat") ){
    /*
     *    MTSAT DATA 
     */
    /* Thresholds */
    threshold_mult = par_info.mtsat_threshold_mult;
    threshold_cnst = par_info.mtsat_threshold_constant;
    /* For flat binary make filesearch string and find files */
    if( 0 == use_GHRSST ){
      sprintf(searchString,"%s/sst%1do*%s*%s",file_info.dir_mtsat,frequency,
	      datestring,day_night_string);
      if( 0 == find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(4,"*** No files found for MTSAT (Day) for ",datestring,
		" in ",file_info.dir_mtsat);
	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;
	/* EXIT no files found */
	return(1);
      }
    } else {
      /* GHRSST */
      sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-MTSAT*.nc",
	      file_info.dir_goes,datestring);
      if( 0 != find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(6,"*** No files found for MTSAT for",
		datestring," in ",file_info.dir_goes);
	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;
	/* EXIT no files found */
	return(1);
      }
    }
  } else if( 0 == strcmp(pType,"msg") ){
    /*
     *    MSG DATA 
     */
    threshold_mult = par_info.msg_threshold_mult;
    threshold_cnst = par_info.msg_threshold_constant;
    /* For flat binary make filesearch string and find files */
    if( 0 == use_GHRSST ){
      sprintf(searchString,"%s/sst%1do*%s*%s",file_info.dir_msg,frequency,
	      datestring,day_night_string);
      if( 0 == find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(4,"*** No files found for MSG for ",datestring,
		" in ",file_info.dir_msg);
	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;
	/* EXIT no files found */
	return(1);
      }
    } else {
      /* GHRSST */
      sprintf(searchString,"%s/%s*-*_GHRSST-SSTskin-MSG*.nc",
	      file_info.dir_goes,datestring);
      if( 0 != find_files(searchString,&nFiles,&ppFilelist) ){
	message(1,"*** WARNING");
	message(6,"*** No files found for MTSAT for",
		datestring," in ",file_info.dir_goes);
	pBigGridGeoSST->nData = 0;
	pPixel_Extent->xlow = 0;
	pPixel_Extent->xhigh = 0;
	pPixel_Extent->ylow = 0;
	pPixel_Extent->yhigh = 0;
	/* EXIT no files found */
	return(1);
      }
    }
  }
  
  /* Read in data */
  j = 0; /* Index for number of good files */
  for(i=0;i<nFiles;i++){
    /* read_raw_geo knows about GOES E/W/P and MSG and MTSAT */
    /* Make sure we only add in data if it's OK */
    if( 0 == read_raw_geo( *(ppFilelist+i), &SST, pType, par_files, 
			   day_night_type ) ){
    
      /* Get hours from filename */
      pStr = strstr(*(ppFilelist+i),datestring);
      pos = (VINT) (pStr - *(ppFilelist+i)) + strlen(datestring) + 1;
      
      /* GOES type is a single character */
      Hour_Str[0] = *(*(ppFilelist+i)+pos);
      Hour_Str[1] = *(*(ppFilelist+i)+pos+1);
      Hour_Str[2] = '\0';

      if( 1 != sscanf(Hour_Str,"%d",&Hour) ){
	message(1,"Non-compliant filename. LST array will not be correct");
	Hour = 0;
      }
      
      /* If this is the first successful read (j==0) */
/*      if( 0 == j ){*/ /* ** Logic doesn't work if next L2P image is different size ** */
	/* After having read in first dataset use min/max lat/lon rebin 
	 * reference.  If checks to be run on the SST, then rebin reference to 
	 * SST resolution */
	
	/* Get pixel extent in global image */
	pPixel_Extent->xlow = (int)((SST.minlat-par_info.ref_latmin)*
				    par_info.cells_per_degree)+1;
	pPixel_Extent->xhigh = (int)((SST.maxlat-par_info.ref_latmin)*
				     par_info.cells_per_degree);
	pPixel_Extent->ylow = (int)((SST.minlon-par_info.ref_lonmin)*
				    par_info.cells_per_degree)+1;
	pPixel_Extent->yhigh = (int)((SST.maxlon-par_info.ref_lonmin)*
				     par_info.cells_per_degree);
	
	nx = (pPixel_Extent->xhigh - pPixel_Extent->xlow)+1;
	ny = (pPixel_Extent->yhigh - pPixel_Extent->ylow)+1;
	
	if( 1 == sst_check ){
	  /* Generate rebinned reference SST and variability for this image */
	  /* This effectively maps the reference SST and variabiliry to the SST image */
	  if( 0 == use_GHRSST ){

	  /* rebin_data_matlab() works for equal-angle satellite data, i.e. 'BINARY' Geo data */

	    rebin_data_matlab(&SST,pRef_SST,&RebinSSTRef); 
	    rebin_data_matlab(&SST,pSST_Variability,&RebinVarRef);
	  } else {

	  /* allocate_float_array_matlab() frees up array storage if pointer is not NULL then reallocates */

            allocate_float_array_matlab(SST.sst.nx, SST.sst.ny, &RebinSSTRef);
            allocate_float_array_matlab(SST.sst.nx, SST.sst.ny, &RebinVarRef);

            Lat = &(SST.lat);
            Lon = &(SST.lon);

            ref_deltalat = (VFLOAT)(1./par_info.cells_per_degree);
            ref_deltalon = (VFLOAT)(1./par_info.cells_per_degree);

	  /* This maps equal-angle gridded reference data to the satellite projection (i.e. GHRSST L2P data) */
          /* So loop through the satellite pixels and obtain the nearest neighbour reference SST */

	    for(k=0;k<Lat->size;k++){
	    /* Check requested longitude/latitude are within bounds */
	      if( -180. <= *(Lon->array+k) && 360. >= *(Lon->array+k) &&
	  	   -90. <= *(Lat->array+k) &&  90. >= *(Lat->array+k) ){

	      /* Now based on lat/lon, determine which cell this is in */

		xpos = (*(Lat->array+k)-pRef_SST->minlat)/ref_deltalat;
		xpos = (xpos % pRef_SST->sst.nx);
		ypos = (*(Lon->array+k)-pRef_SST->minlon)/ref_deltalon;
		ypos = (ypos % pRef_SST->sst.ny);
		
		pos = xpos+ypos*pRef_SST->sst.nx;

	      /* If calculated reference pixel is within array bounds, copy the data */

		if( ( 0 <= pos ) && ( pRef_SST->sst.size > pos ) ){

		  *(RebinSSTRef.array+k) = *(pRef_SST->sst.array+pos);
		  *(RebinVarRef.array+k) = *(pSST_Variability->sst.array+pos);

		} else {
		    message(1,
			  "ERROR: pos out of range (read_geo_data.c)");
		  /* FAIL this is a bad error */
		    exit(-1);
		}
	      }
	    }
	  }

	  /* Now get threshold for SST checks */
	  /* copy rebinned variability array to new array for processing */
	  copy_float_matlab(&RebinVarRef,&threshold);
	  /* Multiply by threshold_mult */
	  mult_float_array_matlab(&threshold,threshold_mult);
	  /* add threshold_cnst */
	  add_float_array_matlab(&threshold,threshold_cnst);
/*	}   ** Need to remove one of the if{} blocks to make logic correct ** */     
      }
      
      /* Setup dummy SST array to hold biggridsst lat/lon ranges */
      dummySST.sst.nx = pRef_SST->sst.nx;
      dummySST.sst.ny = pRef_SST->sst.ny;
      dummySST.minlat = pRef_SST->minlat;
      dummySST.maxlat = pRef_SST->maxlat;
      dummySST.deltalat = pRef_SST->deltalat;
      dummySST.minlon = pRef_SST->minlon;
      dummySST.maxlon = pRef_SST->maxlon;
      dummySST.deltalon = pRef_SST->deltalon;
      
      /* Now process this image */
      /* If data is there */
      if( 0 < SST.sst.size ){
	
	/* Use thresholding on SST data */
	if( 1 == sst_check ){
	  /* Get ABS(difference) */
	  array_float_oper_matlab( &SST.sst, "-", &RebinSSTRef, &Diff );
	  array_float_abs_matlab( &Diff );
	  
	  /* Find and set to NaN bad data */
	  if( 0 == find_float_array_matlab( &Diff, "gt", &threshold, &Bad ) ){
	    set_find_float_matlab( &SST.sst, &Bad, NaN );
	    nbad = Bad.nelems;
	    free_elem_matlab( &Bad );
	  } else {
	    nbad = 0;
	  }
	  
	  /* Get number of good elements */
	  if( 0 == find_float_array_matlab( &Diff, "le", &threshold, &Good ) ){
	    ngood = Good.nelems;
	    free_elem_matlab( &Good );
	  } else {
	    ngood = 0;
	  }
	  
	  /* Free difference array */
	  free_float_matlab( &Diff );

	  sprintf(messageStr,"*** Using %d & rejecting %d pixels from %s",
		  ngood,nbad,*(ppFilelist+i));
	  message(1,messageStr);
	  /* Rebin input SST image to BigGridSST size with weighting */
	  /* Uses dummySST to provide the size/lat/lon ranges for big grid */
	  /* Note output is to index of j.  This is because if one file was  */
	  /* all bad we don't need to process it */
	  if( 0 < ngood ){
	    if( 1 == use_sses_stdev ){
	      rebin_data_matlab_weight(&dummySST,&SST,&pBigGridGeoSST->SST[j]);
	    } else {
	      rebin_data_matlab(&dummySST,&SST,&pBigGridGeoSST->SST[j]);
	    }
	    j++;
	  }
	} else {
	  /* No checks so just rebin data (with weighting) */
	  if( 1 == use_sses_stdev ){
	    rebin_data_matlab_weight(&dummySST,&SST,&pBigGridGeoSST->SST[j]);
	  } else {
	    rebin_data_matlab(&dummySST,&SST,&pBigGridGeoSST->SST[j]);
	  }
	  j++;
	}
      
      }
      /* Deallocate this image */
      free_sst_matlab(&SST);
    }  
  }

  /* Set number of output files */
  pBigGridGeoSST->nData = j;      
      
  /* Free filelist */
  free_filelist(&nFiles,&ppFilelist);
  
  /* Free variability array */
  if( 1 == sst_check ){
    free_float_matlab(&RebinSSTRef);
    free_float_matlab(&RebinVarRef);
    free_float_matlab(&threshold);
  }

  return(0);
}


