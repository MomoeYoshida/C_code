/*
 * NAME: 
 *
 *    process_raw_geo_c
 *
 * FUNCTION:
 *
 *    C version low levels of process_raw_*.m - reads in GOES/MTSAT/MSG data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in process Geostationary data (choose which 
 *    one) data.  Note original MATLAB version had separate functions for each 
 *    geo, but this does them all
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    process_raw_geo_c
 *
 * ROUTINES USED INTERNALLY:
 *
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */

#include <math.h>
#define finite(x) isfinite(x)

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* header for this routine */
#include "types_cnsts.h"
#include "logfile.h"
#include "init_file_info.h"
#include "init_par_info.h"
#include "matlab_functions.h"
#include "read_geo_data.h"
#include "process_raw_geo_c.h"
#include "diurnal_warming.h"

/*
 * NAME: 
 *
 *    process_raw_geo_c
 *
 * FUNCTION:
 *
 *    C Code to process Geostionary data
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Process GOES/MTSAT/MSG data for a complete day
 *    
 *    Loops round day/night loop to read in day/night data seperately.
 *    To deal with cases where we want nighttime only, uses input loop
 *    ranges for day/night loop (start_daynight/end_daynight) which will 
 *    be 0,1 for whole day and 1,1 for nighttime only.  Code then reads in
 *    relevant data (read_geo_data) either day or night
 *
 * PSEUDO CODE
 *
 *    FUNCTION type, year, day, ref_sst, sst_variability, sst_check, biasDay,
 *             biasNight, outData 
 *
 *       initialize arrays
 *       setup parameters
 *
 *       get instrument specific standard deviation threshold
 *
 *       LOOP round day/night 
 *
 *          get reference sst corrected for bias
 *          read in day/night geo data
 *          setup output data
 *
 *          Get standard deviation and mean for geo data
 *          LOOP round image Y size
 *             LOOP round image X size
 *
 *                LOOP round ssts in pixel
 *                   mean and standard deviation calculation
 *                END
 *                
 *                IF data present THEN
 *                  Set mean value for this pixel 
 *                  Set number of valid SSTs for this pixel 
 *                  IF valid number of pixels gt min_obs_per_cell
 *                     Set standard devation for this pixel 
 *                  ELSE
 *                     Set standard devation to the default
 *                  ENDIF
 *               ELSE
 *                  Set pixel to bad
 *               ENDIF
 *           
 *            END LOOP
 *         END LOOP
 *
 *         free geo data
 *
 *      END LOOP
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    init_big_geo_grid_matlab
 *    init_sst_matlab
 *    init_par_info
 *    array_float_oper_matlab
 *    read_geo_data
 *    free_sst_matlab
 *    allocate_float_array_matlab
 *    allocate_float_array_matlab
 *    allocate_int_array_matlab
 *    free_bigsst_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = process_day_geo_data_c( type, year, day, ref_sst, 
 *                                 sst_variability, sst_check, biasDay,
 *                                 biasNight, outData )
 *
 * INPUTS:  
 *    
 *   type            - type of satellite to read in (GOES_TYPE etc.)
 *   year            - year
 *   day             - day number of year
 *   ref_sst         - reference sst for bias correction
 *   sst_variability - sst variability data
 *   sst_check       - check against limits of SST (0=no, 1=yes)
 *   biasDay         - bias for daytime data
 *   biasNight       - bias for nighttime data
 *
 * OUTPUTS:  
 * 
 *   outData         - output SSTs for a days worth of GEO data
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *    strstr
 *    sprintf
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
VINT process_raw_geo_c( const char *pType, const VINT year, const VINT day,  
			struct sst_struct *pRef_SST, 
			struct sst_struct *pSST_Variability, 
			const VINT sst_check, const struct float_struct *pBiasDay,
			const struct float_struct *pBiasNight,
			struct out_sst_str *pOutdata,
			const struct par_files_str par_files )
{
  /* Local variabiles */
  char messageStr[MAX_STRING_LENGTH];
  unsigned char day_night_type = 0;
  char use_sses_stdev = 0;
  
  VINT frequency = 0;
  VINT i = 0;
  VINT j = 0;
  VINT nx = 0;
  VINT ny = 0;
  VINT pos = 0;
  VINT ndata = 0;
  VINT good = 0;

  VFLOAT SSTval = 0.;
  VFLOAT default_std = 0.;
  VFLOAT delta = 0.;

  /* Force these nos to be double precision even if VFLOAT is single */
  double meanval = 0.;
  double M2 = 0.;
  double w = 0.;
  double value = 0.;
  double weight = 0.;
  double weight2 = 0.;
  double stdev = 0.;

  struct pixel_extent_str Pixel_Extent;

  struct big_geo_grid BigGridSST;
  struct sst_struct biased_sst;

  char DWfile_stem[MAX_STRING_LENGTH];
  char DWfile[MAX_STRING_LENGTH];

  /* Initialize array structures */
  init_big_geo_grid_matlab(&BigGridSST);
  init_sst_matlab(&biased_sst);

  /* Setup parameters */
  init_par_info(par_files.par_info);

  /* Get default standard deviation */
  /* Depends on input type */
  if( (0 == strstr("e",pType)) ){
    default_std = par_info.default_goes_std_e;
    use_sses_stdev = par_info.goese_use_sses_stdev;
  } else if( (0 == strstr("w",pType)) ){
    default_std = par_info.default_goes_std_w;
    use_sses_stdev = par_info.goesw_use_sses_stdev;
  } else if( (0 == strstr("p",pType)) ){
    default_std = par_info.default_goes_std_p;
    use_sses_stdev = par_info.goese_use_sses_stdev;
  } else if( (0 == strstr("msg",pType)) ){
    default_std = par_info.default_msg_std;
    use_sses_stdev = par_info.msg_use_sses_stdev;
  } else if( (0 == strstr("mtsat",pType)) ){
    default_std = par_info.default_mtsat_std;
    use_sses_stdev = par_info.mtsat_use_sses_stdev;
  } else {
    message(1,"ERROR: Cannot match GEO type to e/w/p/msg/mtsat");
    /* EXIT Type incorrect */
    return(1);
  }

  /* Get every hour */
  frequency = 1;

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

  /* Loop round day/night */
  /* For day and night start_day_night=0,end_day_night=1 */
  /* For nighttime only start_day_night=1,end_day_night=1 */ 
  /* These variables are set via the par_info parameters */
  for(day_night_type=par_info.start_day_night;
      day_night_type<=par_info.end_day_night;
      day_night_type++){
    
    /* Remove bias from reference for relevant dat/night type and get data */
    if( 0 == day_night_type ){
      array_float_oper_matlab(&pRef_SST->sst,"-",pBiasDay,&biased_sst.sst);
    } else {
      array_float_oper_matlab(&pRef_SST->sst,"-",pBiasNight,&biased_sst.sst);
    }
    /* Set lat/lon ranges for bias corrected data */
    biased_sst.minlat = pRef_SST->minlat;
    biased_sst.maxlat = pRef_SST->maxlat;
    biased_sst.deltalat = pRef_SST->deltalat;
    biased_sst.minlon = pRef_SST->minlon;
    biased_sst.maxlon = pRef_SST->maxlon;
    biased_sst.deltalon = pRef_SST->deltalon;

    /* Read in geo (24 hour) data */
    read_geo_data( pType, year, day, frequency, &biased_sst, 
		   pSST_Variability, sst_check, &BigGridSST, 
		   &Pixel_Extent, day_night_type, par_files, use_sses_stdev );

    /* Free bias corrected SST reference */
    free_sst_matlab( &biased_sst );

    /* If no data read in */
    if( 0 == BigGridSST.nData ){
      sprintf(messageStr,"*** No files found for Goes type %s for %d day %d",
	      pType,year,day);
      message(1,messageStr);
      /* EXIT No data found */
      return(1);
    }

    /* Now have all the geo data read into BigGridSST structure */
    /* calculate mean/standard devation */

    /* Loop round data and generate mean, standard deviation and count 
     * Uses the Wilton (Kunth) algorithm if not weighted mean 
     * Note that the standard deviation code is forced to double precision 
     * to try and ensure no rounding error problems */
    nx = BigGridSST.SST[0].nx;
    ny = BigGridSST.SST[0].ny;
    /* Setup output array */
    (pOutdata+day_night_type)->nx = nx;
    (pOutdata+day_night_type)->ny = ny;
    (pOutdata+day_night_type)->Pixel_Extent.xlow = Pixel_Extent.xlow;
    (pOutdata+day_night_type)->Pixel_Extent.xhigh = Pixel_Extent.xhigh;
    (pOutdata+day_night_type)->Pixel_Extent.ylow = Pixel_Extent.ylow;
    (pOutdata+day_night_type)->Pixel_Extent.yhigh = Pixel_Extent.yhigh;
    allocate_float_array_matlab(nx,ny,&(pOutdata+day_night_type)->SST);
    allocate_float_array_matlab(nx,ny,&(pOutdata+day_night_type)->Stdev);
    allocate_int_array_matlab(nx,ny,&(pOutdata+day_night_type)->Gridcnt);
    
    /* Now get mean, standard deviation 
     * this uses an algorithm which reduces the possibility of rounding errors
     * but only requires one pass */
    for(i=0;i<ny;i++){
      
      for(j=0;j<nx;j++){
	/* Array index */
	pos = (j+i*nx);
	
	if( 1 == use_sses_stdev ){
	  ndata = 0;
	  meanval = 0.;
	  weight = 0.;
	  /* Loop round stored data */
	  for(good=0;good<BigGridSST.nData;good++){
	    SSTval = *(BigGridSST.SST[good].array+pos);	    
	    /* Note SSTs are stored as degree Centigrade */
	    if( finite(SSTval) && (-10 < SSTval) && (100 > SSTval) ){
	      ndata++;
	      w = *(BigGridSST.Variance[good].array+pos);
	      meanval += w * SSTval;
	      weight += w;
	    }
	  }
	  meanval /= weight;
	  stdev = 0.;
	  weight2 = 0.;
	  for(good=0;good<BigGridSST.nData;good++){
	    SSTval = *(BigGridSST.SST[good].array+pos);	    
	    /* Note SSTs are stored as degree Centigrade */
	    if( finite(SSTval) && (-10 < SSTval) && (100 > SSTval) ){
	      w = *(BigGridSST.Variance[good].array+pos);
	      value = (SSTval - meanval);
	      stdev += w * value*value;
	      weight2 += w*w;
	    }
	  }
	  if( ndata >= par_info.min_geo_obs_per_cell ){
	    stdev = sqrt((weight/(weight*weight-weight2))*stdev);
	  }
	} else {
	  /* Calculate standard deviation and mean based on day/night data */
	  /* Uses Wilton (Kunth) algorithm */
	  ndata = 0;
	  meanval = 0.;
	  M2 = 0.;
	  /* Loop round stored data */
	  for(good=0;good<BigGridSST.nData;good++){
	    SSTval = *(BigGridSST.SST[good].array+pos);	    
	    /* Note SSTs are stored as degree Centigrade */
	    if( finite(SSTval) && (-10 < SSTval) && (100 > SSTval) ){
	      ndata++;
	      delta = SSTval - meanval;
	      meanval += delta/ndata;
	      M2 += delta*(SSTval-meanval);
	    }
	  }
	  if( ndata >= par_info.min_geo_obs_per_cell ){
	    stdev = (sqrt(M2/(ndata-1)));
	  }
	}
	/* If have data then store */
	if( 0 < ndata ){
	  *((pOutdata+day_night_type)->SST.array+pos) = (VFLOAT) meanval;
	  *((pOutdata+day_night_type)->Gridcnt.array+pos) = ndata;
	  /* If have a sensible standard deviation Store values, if not use
	   * default value */
	  if( par_info.min_geo_obs_per_cell <= ndata ){
	    *((pOutdata+day_night_type)->Stdev.array+pos) = (VFLOAT) stdev;
	  } else {
	    *((pOutdata+day_night_type)->Stdev.array+pos) = default_std;
	  }
	} else {
	  /* No data - set values to NaN */
	  *((pOutdata+day_night_type)->SST.array+pos) = NaN;
	  *((pOutdata+day_night_type)->Stdev.array+pos) = NaN;
	  *((pOutdata+day_night_type)->Gridcnt.array+pos) = 0;
	}
      }
    }    
    /* Free big grid data */
    free_bigsst_matlab(&BigGridSST);
  }

  return(0);
}
