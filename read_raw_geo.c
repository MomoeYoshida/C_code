/*
 * NAME: 
 *
 *    read_raw_geo
 *
 * FUNCTION:
 *
 *    C combined version read_raw_goes/msg/mtsat.m - reads in GEO data
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Contains the routine to read in hourly GEO data
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_raw_goes
 *
 * ROUTINES USED INTERNALLY:
 *
 *    read_raw_geo_binary
 *    map_ghrsst_sst_to_input
 *    get_geo_size_position
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

/* header for this routine */
#include "types_cnsts.h"
#include "logfile.h"
#include "init_par_info.h"
#include "matlab_functions.h"
#include "read_ghrsst.h"
#include "read_raw_geo.h"
#include "diurnal_warming.h"

/* Internal routines */
static VINT read_raw_geo_binary( const char *pFilename, 
				 struct int_struct *pTime,
				 struct sst_struct *pSST, const char *pType, 
				 struct par_files_str par_files );
static VINT map_ghrsst_sst_to_input( const char *pFilename, 
				     struct sst_struct *pSST, 
				     struct int_struct *pTime, 
				     struct float_struct *pInSST,
				     struct int_struct *pinTime, 
				     struct float_struct *pSSES_Variance,
				     struct float_struct *pInLat,
				     struct float_struct *pInLon,
				     struct float_struct *pLat,
				     struct float_struct *pLon,
				     const char *pType,
				     struct par_files_str par_files );
static VINT get_geo_size_position(const char *pFilename, 
				  const char *pType, VINT *pDims, 
				  VFLOAT *pMinlon, VFLOAT *pMaxlon, 
				  VFLOAT *pDeltaLon, VFLOAT *pMinlat, 
				  VFLOAT *pMaxlat, VFLOAT *pDeltaLat,
				  VFLOAT *pDeltaTime, 
				  struct par_files_str par_files );

/*
 * NAME: 
 *
 *    read_raw_geo
 *
 * FUNCTION:
 *
 *    C Code to read in GEO data
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in GEO data either flat binary or GHRSST format
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename SST type par_files
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_raw_geo( filename, sst, type, format )
 *
 * INPUTS:  
 *    
 *   filename - GOES hourly filename
 *   type     - GEO type ('e','w','p','msg','mtsat')
 *   format   - "BINARY" or "GHRSST"
 *
 * OUTPUTS:  
 * 
 *   sst      - float_struct array with the SSTs
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *   Modified to read in GHRSST data and to calculate image time for DW 
 *        on 26/11/2013
 *
 */
VINT read_raw_geo( const char *pFilename, struct sst_struct *pSST, 
		   const char *pType, struct par_files_str par_files, 
		   unsigned char day_night )
{
  /* Local variables */
  VINT status = 0;
  VINT i = 0;
  VINT default_sses_bias = 0;
  VFLOAT default_std = 0.;

  struct int_struct Time;
  struct int_struct inTime;
  struct float_struct inSST;
  struct float_struct inLat;
  struct float_struct inLon;
  struct char_struct CldMask;

  /* Initialize structures */
  init_int_array_matlab(&Time);
  init_int_array_matlab(&inTime);
  init_float_array_matlab(&inSST);
  init_float_array_matlab(&inLat);
  init_float_array_matlab(&inLon);
  init_byte_array_matlab(&CldMask);

  /* Get par info */
  init_par_info(par_files.par_info);

  /* Get default SSES stdev value */
  if( (0 == strstr("e",pType)) ){
    default_sses_bias = par_info.goese_sses_bias;
    default_std = par_info.goese_sses_stdev;
  } else if( (0 == strstr("w",pType)) ){
    default_sses_bias = par_info.goesw_sses_bias;
    default_std = par_info.goesw_sses_stdev;
  } else if( (0 == strstr("p",pType)) ){
    default_sses_bias = par_info.goese_sses_bias;
    default_std = par_info.goese_sses_stdev;
  } else if( (0 == strstr("msg",pType)) ){
    default_sses_bias = par_info.msg_sses_bias;
    default_std = par_info.msg_sses_stdev;
  } else if( (0 == strstr("mtsat",pType)) ){
    default_sses_bias = par_info.msg_sses_bias;
    default_std = par_info.mtsat_sses_stdev;
  } else {
    message(1,"ERROR: Cannot match GEO type to e/w/p/msg/mtsat");
    /* EXIT Type incorrect */
    return(1);
  }
  /* Get square of default std */
  default_std = default_std*default_std;

  /* Read depending on which format */
  if( 0 == strncmp(par_info.geo_format,"BINARY",6) ){
    status = read_raw_geo_binary( pFilename, &Time, pSST, pType, par_files );
    if( 1 == status ){
      free_int_matlab(&Time);
      free_sst_matlab(pSST);
      return(status);
    }
    /* Set SSES Variance to constant value (0.6K) */
    allocate_float_array_matlab(pSST->sst.nx,pSST->sst.ny,
				&pSST->est_variance);  
    for(i=0;i<pSST->sst.size;i++){
      *(pSST->est_variance.array+i) = default_std;
    }
  } else if( 0 == strncmp(par_info.geo_format,"GHRSST",6) ){
    /* If geo_sses_bias = 1 then removes SSTS bias from SST */
    status = read_ghrsst( pFilename, &Time, &pSST->sst, &pSST->lat, &pSST->lon, 
			  &pSST->est_variance,"GEO", &CldMask, day_night, 
			  default_sses_bias, default_std, 
			  par_info.geo_ghrsst_min_quality, 0, 0. );  /* Couple of extra parameters (min_algorithm, min_windspeed) - only for SLSTR @present */
    if( 1 == status ){
      free_float_matlab(&pSST->lat);
      free_float_matlab(&pSST->lon);
      free_float_matlab(&pSST->est_variance);
      free_byte_matlab(&CldMask);
      return(status);
    }
    /* Turn into Centigrade (GHRSST in K) */
    subtract_float_array_matlab(&pSST->sst,ABS_ZERO);

    /* Note for GEO data cloud mask aready applied in GHRSST reader - this is not the case for the AVHRR
     * ACSPO GHRSST data (see processing_avhrr_acpso_c.c)  */
    /* flip the arrays to matlab ordering */
    transpose_flip_float_matlab( &pSST->sst );
    transpose_flip_float_matlab( &pSST->est_variance );
    transpose_flip_int_matlab( &Time );
    transpose_flip_float_matlab( &pSST->lon );
    transpose_flip_float_matlab( &pSST->lat );

    /* Setup min/max lat/lon */
/*    pSST->minlon = *(pSST->lon.array);
    pSST->maxlon = *(pSST->lon.array+pSST->lon.size);
    pSST->minlat = *(pSST->lat.array);
    pSST->maxlat = *(pSST->lat.array+pSST->lat.size);  */


    pSST->minlon=180.0;
    pSST->maxlon=-180.0;
    pSST->minlat=90.0;
    pSST->maxlat=-90.0;

    for(i=0;i<pSST->sst.size;i++){
      if((*(pSST->lat.array+i)>=-90.0) && (*(pSST->lat.array+i)<=90.0)){
        if(pSST->minlon>*(pSST->lon.array+i))
          pSST->minlon=*(pSST->lon.array+i);
        if(pSST->maxlon<*(pSST->lon.array+i))
          pSST->maxlon=*(pSST->lon.array+i);
        if(pSST->minlat>*(pSST->lat.array+i))
          pSST->minlat=*(pSST->lat.array+i);
        if(pSST->maxlat<*(pSST->lat.array+i))
          pSST->maxlat=*(pSST->lat.array+i);
      }
    }
    pSST->deltalon = (pSST->maxlon - pSST->minlon)/(pSST->lon.ny-1.);
    pSST->deltalat = (pSST->maxlat - pSST->minlat)/(pSST->lat.nx-1.);

    /* Free memory */
    free_byte_matlab(&CldMask);
  } else {
    message(2,"ERROR: Do not recognize GEO format : ",par_info.geo_format);
    /* EXIT unrecognized format */
    return(1);
  }

  /* Remove DW if requested */
  if( 1 == par_info.correct_diurnal_warming ){
    remove_diurnal_warming( &Time, &pSST->lon, &pSST->lat, &pSST->sst, 
			    &pSST->est_variance );
  }
  free_int_matlab(&Time);

  /* Return read status */
  return(status);
}

/*
 * NAME: 
 *
 *    map_ghrsst_sst_to_input
 *
 * FUNCTION:
 *
 *    C Code to map GHRSST data onto the uniform binary grid defined by the binary 
 *    files
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    C Code to map GHRSST data onto the uniform binary grid defined by the binary 
 *    files.  Note output is weighted average based on value of SSES Stdeviation.
 *    If no SSES was available/used the value is a constant which is equiv.
 *    to a normal average.
 *
 *    NOTE
 *
 *    THis code is now not used as the lat/lon are stored in the SST structure directly and are
 *    used directly in the remapping (rebin) code
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename SST type par_files
 *
 *       Get sizes and max/min lat/lon
 *       Allocate output SST array
 *
 *       LOOP GHRSST SST array
 *          Bin to output array
 *       END LOOP
 *
 *       Renormalize output SST if needed
 *
 *       Transpose to MATLAB orientation
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    allocate_float_array_matlab
 *    transpose_flip_float_matlab
 *    message
 *
 * CALLING SEQUENCE:  
 *
 *    ok = map_ghrsst_sst_to_input( filename, Time, SST, inSST, inTime,
 *                                  SSES_Stdev, inLat, inLon, Lat, Lon, 
 *                                  type, par_files )
 *
 * INPUTS:  
 *    
 *   filename   - input filename
 *   inSST      - SST from GHRSST data
 *   inTime     - Time from GHRSST data
 *   SSES_Stdev - SSES Std. deviation for weighted average
 *   inLat      - Latitude from GHRSST data
 *   inLon      - Longitude from GHRSST data
 *   Lat        - Latitude (rebinned)
 *   Lon        - Longitude (rebinned)
 *   type       - data type
 *   par_files  - parameter files
 *
 * OUTPUTS:  
 * 
 *   SST        - float_struct array with the SSTs remapped 
 *   Time       - remapped Time array
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT map_ghrsst_sst_to_input( const char *pFilename, 
				     struct sst_struct *pSST, 
				     struct int_struct *pTime, 
				     struct float_struct *pInSST,
				     struct int_struct *pInTime, 
				     struct float_struct *pSSES_Variance,
				     struct float_struct *pInLat,
				     struct float_struct *pInLon,
				     struct float_struct *pLat,
				     struct float_struct *pLon,
				     const char *pType,
				     struct par_files_str par_files )
{
  /* Local variables */
  VINT i = 0;
  VINT j = 0;
  VINT xpos = 0;
  VINT ypos = 0;
  VINT pos = 0;
  VINT dims[2] = {0,0};
  VINT *pNCounter = NULL;
  VINT renorm = 0;
  
  VFLOAT *pCounter = NULL;
  VFLOAT *pNewVariance = NULL;
  VFLOAT minlon = 0.;
  VFLOAT maxlon = 0.;
  VFLOAT deltaLon = 0.;
  VFLOAT minlat = 0.;
  VFLOAT maxlat = 0.;
  VFLOAT deltaLat = 0.;
  VFLOAT deltaTime = 0.;

  /* Get sizes etc */
  if( 1 == get_geo_size_position(pFilename, pType, &dims[0], &minlon, 
				 &maxlon, &deltaLon, &minlat, &maxlat, 
				 &deltaLat, &deltaTime, par_files ) ){
    message(1,"ERROR: Cannot get size/position");
    /* EXIT cannot get size/position */
    return(1);
  }

  /* Make output structure */
  /* Set min/max etc. lat/lon */
  pSST->minlon = minlon;
  pSST->maxlon = maxlon;
  pSST->deltalon = deltaLon;
  pSST->minlat = minlat;
  pSST->maxlat = maxlat;
  pSST->deltalat = deltaLat;  

  /* Allocate output structure */
  allocate_float_array_matlab(dims[0],dims[1],&pSST->sst);  
  allocate_int_array_matlab(dims[0],dims[1],pTime);  
  allocate_float_array_matlab(dims[0],dims[1],pLat);  
  allocate_float_array_matlab(dims[0],dims[1],pLon);  

  /* Make Lat/Lon arrays */
  for(i=0;i<dims[1];i++){
    for(j=0;j<dims[0];j++){
      pos = j+i*dims[0];
      *(pLat->array+pos) = minlat + i*deltaLat;
      *(pLon->array+pos) = minlon + j*deltaLon;
      if( 360. < *(pLon->array+pos) ){
	*(pLon->array+pos) -= 360.;
      }
      if( 0. > *(pLon->array+pos) ){
	*(pLon->array+pos) += 360.;
      }
    }
  }

  /* Allocate counter for pSST->sst */
  if( NULL == (pCounter = (VFLOAT *) calloc(dims[0]*dims[1],sizeof(VFLOAT))) ){
    message(1,"ERROR: Cannot allocate pCounter in mapping GEO data");
    /* EXIT error allocating */
    return(1);
  }

  /* Allocate array for new variance after rebinning */
  if( NULL == (pNewVariance = (VFLOAT *) 
	       calloc(dims[0]*dims[1],sizeof(VFLOAT))) ){
    message(1,"ERROR: Cannot allocate pNewVariance in mapping GEO data");
    /* EXIT error allocating */
    return(1);
  }
  if( NULL == (pNCounter = (VINT *) calloc(dims[0]*dims[1],sizeof(VINT))) ){
    message(1,"ERROR: Cannot allocate pNCounter in mapping GEO data");
    /* EXIT error allocating */
    return(1);
  }

  /* Make sure sst output is zero */
  memset(&pSST->sst.array,0,dims[0]*dims[1]*sizeof(VFLOAT));

  /* Make sure Counter is zero */
  memset(pCounter,0,dims[0]*dims[1]*sizeof(VFLOAT));
  memset(pNCounter,0,dims[0]*dims[1]*sizeof(VINT));

  /* Loop round input data and map to output data grid */
  renorm = 0;
  for(i=0;i<pInSST->size;i++){
    /* Deal with the 360->0 boundary in Longitude */
    if( *(pLon->array+i) >= minlon ){
      xpos = (VINT) ((*(pInLon->array+i)-minlon)/deltaLon);
    } else {
      xpos = (VINT) ((*(pInLon->array+i)+360.-minlon)/deltaLon);
    }
    ypos = (VINT) ((*(pInLat->array+i)-minlat)/deltaLat);
    /* If in range of output */
    if( (xpos >= 0) && (xpos < pSST->sst.nx) && 
	(ypos >= 0) && (ypos < pSST->sst.ny) ){
      /* Only bin good data */
      if( NaN != *(pInSST->array+i) ){
	/* Weighted average */
	pos = xpos+ypos*dims[0];
	*(pSST->sst.array+pos) += *(pInSST->array+i) / 
	  *(pSSES_Variance->array+i);
	*(pTime->array+pos) += *(pInTime->array+i);
	*(pCounter+pos) += 1. / *(pSSES_Variance->array+i);
	*(pNewVariance+pos) += *(pSSES_Variance->array+i);
	*(pNCounter+pos) += 1;
	if( 1 < *(pNCounter+pos) ){
	  renorm = 1;
	}
      }
    }
  }

  /* If need to renormalize the pSST array then do so */
  if( 1 == renorm ){
    for(i=0;i<pSST->sst.size;i++){
      if( 0. < *(pCounter+i) ){
	*(pSST->sst.array+i) /= *(pCounter+i);
	*(pTime->array+i) /= *(pCounter+i);
	*(pSSES_Variance->array+i) = *(pNewVariance+i) / *(pNCounter+i);
      }      
    }
  }

  /* Free counter array */
  free(pCounter);
  free(pNewVariance);
  free(pNCounter);

  /*
  fp = fopen("test.dat","w");
  fprintf(fp,"%d %d\n",pSST->sst.nx,pSST->sst.ny);
  
  fclose(fp);
  */
  /* Tranpose and flip data - looks like MATLAB code works in (lat,lon) 
   * space .... */
  transpose_flip_float_matlab( &pSST->sst );
  transpose_flip_float_matlab( &pSST->est_variance );
  transpose_flip_int_matlab( pTime );
  transpose_flip_float_matlab( pLon );
  transpose_flip_float_matlab( pLat );

  /* Everything OK */
  return(0);

}

/*
 * NAME: 
 *
 *    get_geo_size_position
 *
 * FUNCTION:
 *
 *    C Code to get the size of output array and positions from par_info/type
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    C Code to get the size of output array and positions from par_info/type
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename type dims minlon maxlon deltalon minlat maxlat deltalat
 *             par_files         
 *
 *       IF type eq GOES THEN
 *          get GOES type
 *          get GOES image size from par_info structure
 *       ELSE IF type eq MSG THEN
 *          get MSG image size from par_info structure
 *       ELSE IF type eq MTSAT THEN
 *          get MTSAT image size from par_info structure
 *       ENDIF
 *
 *       Get min/max lat/lon based on input Type
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    ok = get_geo_size_position(Filename, Type, Dims, Minlon, Maxlon, 
 *                               DeltaLon, Minlat, Maxlat, DeltaLat, par_files )
 *
 * INPUTS:  
 *    
 *    Filename  - input filename
 *    Type      - type of data 
 *    par_files - parameter files
 *
 * OUTPUTS:  
 * 
 *    Dims      - output dimensions
 *    MinLon    - minimum longitude
 *    MaxLon    - maximum longitude
 *    DeltaLon  - delta longitude
 *    MinLat    - minimum latitude
 *    MaxLat    - maximum latitude
 *    DeltaLat  - delta latitude
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 11/04/2013
 *
 */
static VINT get_geo_size_position(const char *pFilename,
				  const char *pType, VINT *pDims, 
				  VFLOAT *pMinlon, VFLOAT *pMaxlon, 
				  VFLOAT *pDeltaLon, VFLOAT *pMinlat, 
				  VFLOAT *pMaxlat, VFLOAT *pDeltaLat,
				  VFLOAT *pDeltaTime,
				  struct par_files_str par_files )
{
  /* Local variables */
  VINT pos = 0;
  char *pStr = NULL;
  unsigned char charval = 0;
  char Goes_Type[3] = {0,0,0};

  /* Get par info */
  init_par_info(par_files.par_info);

  if( (0 == strncmp("e",pType,1)) || (0 == strncmp("w",pType,1)) || 
      (0 == strncmp("p",pType,1)) ){
    /* Get GOES type from filename */
    pStr = strstr(pFilename,"_2");
    pos = (VINT) (pStr - pFilename);
    
    /* GOES type is a single character */
    /* Make uppercase */
    charval = (unsigned char)*(pFilename+pos-1); 
    Goes_Type[0] = toupper(charval);
    Goes_Type[1] = '\0';

    /* Check GOES type - only accept e,w,p */
    if( 0 == strncmp("E",Goes_Type,1) ){
      *pDims = par_info.goes_e_xsize;
      *(pDims+1) = par_info.goes_e_ysize;
    } else if ( 0 == strncmp("W",Goes_Type,1) ){
      *pDims = par_info.goes_w_xsize;
      *(pDims+1) = par_info.goes_w_ysize;
    } else if( 0 == strncmp("P",Goes_Type,1) ){
      *pDims = par_info.goes_p_xsize;
      *(pDims+1) = par_info.goes_p_ysize;
    } else {
      message(2,"*** Unrecognized GOES filetype: ",Goes_Type);
      message(1,"*** Expect E, W, or P before date: e.g. sst1oW_2005_035_00");
      /* EXIT error in GOES type */
      return(1);
    }

    /* Set scan line time for GOES Imager */
    *pDeltaTime = 0.47;

  } else if( 0 == strncmp("msg",pType,3) ){
    /* For MSG */
    *pDims = par_info.msg_xsize;
    *(pDims+1) = par_info.msg_ysize;

    /* Set scan line time for MSG */
    *pDeltaTime = 0.6;
  } else if( 0 == strncmp("mtsat",pType,5) ){
    /* For MTSAT */
    *pDims = par_info.mtsat_xsize;
    *(pDims+1) = par_info.mtsat_ysize;

    /* Set scan line time for MTSAT */
    *pDeltaTime = 0.6;
  } else {
    message(1,"ERROR: Cannot recognize input type to get size/min/max lat/lon");
    /* EXIT cannot recognize input type */
    return(1);
  }

  /* Set lat/lon ranges and delta lat/lon */
  /* Note that xsize is for longitude and ysize is for latitude 
   * even though in final output array these are switched */
  /* Note also that the deltalon/deltalat calculation has to deal with 
   * the potential crossing over the 180/-180 point */
  if( 0 == strncmp("e",pType,1) ){
    /* GOES-E */
    *pMinlon = par_info.goes_e_minlon;
    *pMaxlon = par_info.goes_e_maxlon;
    /* Deal with possible crossing 180/-180 line */
    if( par_info.goes_e_maxlon > par_info.goes_e_minlon ){
      *pDeltaLon = (par_info.goes_e_maxlon-par_info.goes_e_minlon)/
	(par_info.goes_e_xsize-1);
    } else {
      *pDeltaLon = (360.+par_info.goes_e_maxlon-par_info.goes_e_minlon)/
	(par_info.goes_e_xsize-1);
    }
    /* Note binary image is upside down - deal with that here */
    if( par_info.goes_e_minlat < par_info.goes_e_maxlat ){
      *pMinlat = par_info.goes_e_maxlat;
      *pMaxlat = par_info.goes_e_minlat;
    } else {
      *pMinlat = par_info.goes_e_minlat;
      *pMaxlat = par_info.goes_e_maxlat;
    }
  } else if( 0 == strncmp("w",pType,1) ){
    /* GOES-E */
    *pMinlon = par_info.goes_w_minlon;
    *pMaxlon = par_info.goes_w_maxlon;
    /* Deal with possible crossing 180/-180 line */
    if( par_info.goes_w_maxlon > par_info.goes_w_minlon ){
      *pDeltaLon = (par_info.goes_w_maxlon-par_info.goes_w_minlon)/
	(par_info.goes_w_xsize-1);
    } else {
      *pDeltaLon = (360.+par_info.goes_w_maxlon-par_info.goes_w_minlon)/
	(par_info.goes_w_xsize-1);
    }
    /* Note binary image is upside down - deal with that here */
    if( par_info.goes_w_minlat < par_info.goes_w_maxlat ){
      *pMinlat = par_info.goes_w_maxlat;
      *pMaxlat = par_info.goes_w_minlat;
    } else {
      *pMinlat = par_info.goes_w_minlat;
      *pMaxlat = par_info.goes_w_maxlat;
    }
  } else if( 0 == strncmp("p",pType,1) ){
    /* GOES-P */
    *pMinlon = par_info.goes_p_minlon;
    *pMaxlon = par_info.goes_p_maxlon;
    /* Deal with possible crossing 180/-180 line */
    if( par_info.goes_p_maxlon > par_info.goes_p_minlon ){
      *pDeltaLon = (par_info.goes_p_maxlon-par_info.goes_p_minlon)/
	(par_info.goes_p_xsize-1);
    } else {
      *pDeltaLon = (360. + par_info.goes_p_maxlon-par_info.goes_p_minlon)/
	(par_info.goes_p_xsize-1);
    }
    /* Note binary image is upside down - deal with that here */
    if( par_info.goes_p_minlat < par_info.goes_p_maxlat ){
      *pMinlat = par_info.goes_p_maxlat;
      *pMaxlat = par_info.goes_p_minlat;
    } else {
      *pMinlat = par_info.goes_p_minlat;
      *pMaxlat = par_info.goes_p_maxlat;
    }
  } else if( 0 == strncmp("msg",pType,3) ){
    /* MSG */
    *pMinlon = par_info.msg_minlon;
    *pMaxlon = par_info.msg_maxlon;
    /* Deal with possible crossing 180/-180 line */
    if( par_info.msg_maxlon > par_info.msg_minlon ){
      *pDeltaLon = (par_info.msg_maxlon-par_info.msg_minlon)/
	(par_info.msg_xsize-1);
    } else {
      *pDeltaLon = (360. + par_info.msg_maxlon-par_info.msg_minlon)/
	(par_info.msg_xsize-1);
    }
    /* Note binary image is upside down - deal with that here */
    if( par_info.msg_minlat < par_info.msg_maxlat ){
      *pMinlat = par_info.msg_maxlat;
      *pMaxlat = par_info.msg_minlat;
    } else {
      *pMinlat = par_info.msg_minlat;
      *pMaxlat = par_info.msg_maxlat;
    }
  } else if( 0 == strncmp("mtsat",pType,5) ){
    /* MTSAT */
    *pMinlon = par_info.mtsat_minlon;
    *pMaxlon = par_info.mtsat_maxlon;
    if( par_info.mtsat_maxlon > par_info.mtsat_minlon ){
      *pDeltaLon=(par_info.mtsat_maxlon - par_info.mtsat_minlon)/
	(par_info.mtsat_xsize-1);
    } else {
      *pDeltaLon=(360. + par_info.mtsat_maxlon - par_info.mtsat_minlon)/
	(par_info.mtsat_xsize-1);
    }
    /* Note binary image is upside down - deal with that here */
    if( par_info.mtsat_minlat < par_info.mtsat_maxlat ){
      *pMinlat = par_info.mtsat_maxlat;
      *pMaxlat = par_info.mtsat_minlat;
    } else {
      *pMinlat = par_info.mtsat_minlat;
      *pMaxlat = par_info.mtsat_maxlat;
    }
  }
  *pDeltaLat = (*pMaxlat - *pMinlat) / (*(pDims+1) - 1);

  /* All OK */
  return(0);
}

/*
 * NAME: 
 *
 *    read_raw_geo_binary
 *
 * FUNCTION:
 *
 *    C Code to read in GOES data from BINARY format
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Reads in GEO hourly gridded data in Binary format
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename SST type par_files
 *
 *       Initialize internal arrays
 *       Initialize par_info files
 *
 *       get GHRSST Time from filename
 *
 *       open file
 *
 *       allocate memory for SST
 *       allocate memory for input byte array
 *
 *       read byte array from file
 *
 *       close file
 *
 *       convert byte value to SST
 *
 *       find cloudy/bad data
 *       set cloudy/bad data to NaN
 *
 *       free search list
 *       free byte array
 *
 *       transpose and flip array to MATLAB array type
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 *    init_byte_array_matlab
 *    init_par_info
 *    allocate_float_array_matlab
 *    allocate_byte_array_matlab
 *    find_byte_matlab
 *    set_find_float_matlab
 *    free_elem_matlab
 *    free_byte_matlab
 *    transpose_flip_float_matlab
 *
 * CALLING SEQUENCE:  
 *
 *    ok = read_raw_geo( filename, time, lon, lat, sst, type, par_files )
 *
 * INPUTS:  
 *    
 *   filename  - GOES hourly filename
 *   type      - GEO type ('e','w','p','msg','mtsat')
 *   par_files - parameter files
 *
 * OUTPUTS:  
 * 
 *   time      - GHRSST Time for each scan line
 *   lon       - Longitude
 *   lat       - Latitude
 *   sst       - float_struct array with the SSTs
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 *   strstr
 *   toupper
 *   fopen
 *   fread
 *   fclose 
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2011
 *
 */
VINT read_raw_geo_binary( const char *pFilename, struct int_struct *pTime,
			  struct sst_struct *pSST, const char *pType, 
			  struct par_files_str par_files )
{
  VINT i = 0;
  VINT j = 0;
  VINT dims[2] = {0.0};
  VFLOAT *pArr = NULL;  
  unsigned char *pCharArr = NULL;
  FILE *fp = NULL;
  struct char_struct Byte;
  struct find_elems Bad;

  VFLOAT minlon = 0.;
  VFLOAT maxlon = 0.;
  VFLOAT deltaLon = 0.;
  VFLOAT minlat = 0.;
  VFLOAT maxlat = 0.;
  VFLOAT deltaLat = 0.;

  VINT pos = 0;
  char *pStr = NULL;
  char *pStr2 = NULL;
  char *pStr3 = NULL;
  char Str[5];
  VINT Year = 0;
  VINT Day = 0;
  VINT Hour = 0;
  float Hours = 0.;
  VFLOAT deltaTime = 0.;
  VINT start_time = 0;
  VINT nUnderscore = 0;

  /* Initialize structures */
  init_byte_array_matlab(&Byte);

  init_par_info(par_files.par_info); 
  init_elem_matlab(&Bad);

  /* Get sizes and min/max lat/lon values from par_info */
  if( 1 == get_geo_size_position(pFilename, pType, &dims[0], &minlon, 
				 &maxlon, &deltaLon, &minlat, &maxlat, 
				 &deltaLat, &deltaTime, par_files ) ){
    message(1,"ERROR: Cannot get size/position");
    /* EXIT cannot get size/position */
    return(1);
  }

  /* Make Lat/Lon arrays */
  allocate_float_array_matlab(dims[0],dims[1],&pSST->lat);  
  allocate_float_array_matlab(dims[0],dims[1],&pSST->lon);  
  for(i=0;i<dims[1];i++){
    for(j=0;j<dims[0];j++){
      pos = j+i*dims[0];
      *(pSST->lat.array+pos) = minlat + i*deltaLat;
      *(pSST->lon.array+pos) = minlon + j*deltaLon;
      /* In case we go slightly over */
      if( 360. < *(pSST->lon.array+pos) ){
	*(pSST->lon.array+pos) -= 360.;
      }
      /* Convert to -180,+180 range */
      if( 180. < *(pSST->lon.array+pos) ){
	*(pSST->lon.array+pos) -= 360.;
      }
    }
  }

  /* Get GHRSST Time from filename */
  /* Need Year, Dayno, Hour - found after 2nd _ in filename */
  nUnderscore = 0;
  i = strlen(pFilename)-1;
  while((nUnderscore < 4) && (i > 0)){
    if( *(pFilename+i) == '_' ){
      nUnderscore++;
    }
    i--;
  }
  if( i <= 0 ){
    message(1,"Cannot find 3 underscores in filename for geo");
    return(1);
  }
  pos = i+2;

  /* Get year string */
  Str[0] = *(pFilename+pos);
  Str[1] = *(pFilename+pos+1);
  Str[2] = *(pFilename+pos+2);
  Str[3] = *(pFilename+pos+3);
  Str[4] = '\0';

  if( 1 != sscanf(Str,"%d",&Year) ){
    message(1,"Year cannot be parsed from GEO filename");
    return(1);
  }

  /* Get dayno string */
  Str[0] = *(pFilename+pos+5);
  Str[1] = *(pFilename+pos+6);
  Str[2] = *(pFilename+pos+7);
  Str[3] = '\0';

  if( 1 != sscanf(Str,"%d",&Day) ){
    message(1,"Dayno cannot be parsed from GEO filename");
    return(1);
  }

  /* Get year string */
  Str[0] = *(pFilename+pos+9);
  Str[1] = *(pFilename+pos+10);
  Str[2] = '\0';

  if( 1 != sscanf(Str,"%d",&Hour) ){
    message(1,"Hour cannot be parsed from GEO filename");
    return(1);
  }

  Hours = 1.*Hour;

  /* Get GHRSST Time */
  start_time = get_GHRSST_Time(Year,Day,Hours);

  /* Open file */
  if( NULL == (fp = fopen(pFilename,"r")) ){
    /* Failed to open file */
    message(3,"*** ",pFilename," not found");
    pSST->sst.array = NULL;
    pSST->sst.nx = 0;
    pSST->sst.ny = 0;
    pSST->sst.size = 0;
      /* EXIT error in file open */
    return(1);
  }

  /* Read in data */
  /* First allocate byte array */
  /* Allocate input structure */
  allocate_byte_array_matlab(dims[0],dims[1],&Byte);  

  /* Read in array */
  /* Check we have a complete read */
  if( Byte.size != fread(Byte.array,sizeof(unsigned char),Byte.size,fp) ){
    /* If file truncated, return bad */
    message(1,"ERROR: Error reading in SST data");
    pSST->sst.array = NULL;
    pSST->sst.nx = 0;
    pSST->sst.ny = 0;
    pSST->sst.size = 0;
      /* EXIT error in file read */
    return(1);
  }

  /* Close file */
  fclose(fp);

  /* Allocate output structure */
  allocate_float_array_matlab(dims[0],dims[1],&pSST->sst);  

  /* Calculate SST */
  /* Scaled SST (K) =270K + 0.15*SST(scaled)
   * See http://coastwatch.noaa.gov/goes_sst_algorithms.html */

  pArr = pSST->sst.array;
  pCharArr = Byte.array;
  for(i=0;i<pSST->sst.size;i++){
    /* Convert to centigrade */
    *(pArr++) = 270. + 0.15 * *(pCharArr++) - ABS_ZERO;
  }

  /* Remove bad data */
  /* Find entries where byte value < 7 */
  find_byte_matlab( &Byte, "lt", 7, &Bad );

  /* Set these values to NaN */
  set_find_float_matlab( &pSST->sst, &Bad, NaN );

  /* Free Bad list */
  free_elem_matlab( &Bad );

  /* Free byte array */
  free_byte_matlab( &Byte );

  /* Allocate time array */
  allocate_int_array_matlab(dims[0],dims[1],pTime);  
  /* Loop round to make list of scan line times */
  for(i=0;i<pTime->ny;i++){
    for(j=0;j<pTime->nx;j++){
      /* To line by line timing use the following - although the direction
       * may be wrong (start time for top of image or bottom of image?)
       *
      *(pTime->array+j+i*pTime->nx) = (VINT)(start_time + i*deltaTime);
       *
       * Line by Line timing is only valid for satellite projection.  For standard GEO 
       * flat binary files (which are L3) time is just the filename time -
       * just have a constant time
      */
      *(pTime->array+j+i*pTime->nx) = (VINT)(start_time);
    }
  }

  /* Tranpose and flip data - looks like MATLAB code works in (lat,lon) 
   * space .... */
  transpose_flip_float_matlab( &pSST->sst );
  transpose_flip_int_matlab( pTime );
  transpose_flip_float_matlab( &pSST->lon );
  transpose_flip_float_matlab( &pSST->lat );

  /* Set min/max etc. lat/lon */
  /* Note effect of flip etc. */
  pSST->minlon = minlon;
  pSST->maxlon = maxlon;
  pSST->deltalon = deltaLon;
  pSST->minlat = maxlat;
  pSST->maxlat = minlat;
  pSST->deltalat = -deltaLat;

  /* Everything OK */
  return(0);
}
