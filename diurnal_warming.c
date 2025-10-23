/*
 * NAME: 
 *
 *    diurnal_warming
 *
 * FUNCTION:
 *
 *    C module to add in diurnal warming correction to individual data files
 *
 * TYPE/LANGUAGE: 
 *
 *    C module
 *
 * DESCRIPTION:  
 *
 *    Reads in a diurnal model NetCDF file and applies correction to data
 *    as well as adding in a standard deviation if present to the est_variance
 *    value used for weighting averaging.
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 *    read_diurnal_file
 *    correct_diurnal_warming
 *
 * ROUTINES USED INTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */

/* Standard C library headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netcdf.h>
#include <math.h>

/* header for this routine */
#include "types_cnsts.h"
#include "logfile.h"
#include "init_par_info.h"
#include "matlab_functions.h"
#include "diurnal_warming.h"
#include "get_dates.h"

/* Local global strcuture to hold DW data */
/* Note use float, int etc. due to NetCDF types */
struct DW_Struct {
  int read_in_file;
  float minLat;
  float maxLat;
  float latStep;
  size_t nLat;
  float minLon;
  float maxLon;
  float lonStep;
  size_t nLon;
  size_t nTime;
  int minTime;
  float timeStep;
  int *pTime;
  float *pDW;
  int stdev_there;
  float *pStdev;
};

static struct DW_Struct DW_Store;
/* Store for read in file */
char stored_DWfile[MAX_STRING_LENGTH];

/* Structure for indices */
struct index_struct {
  VINT nindices[2];
  VINT indices_x[2][4];
  VINT indices_y[2][4];
  VINT time[2];
  VFLOAT time_fraction;
  VINT pos[2][4];
  VINT index_pos[2][4];
  VFLOAT values[2][4];
  VFLOAT lonWeight;
  VFLOAT latWeight;
};

/* Constant define */
#define SQRT_2 1.414213538169861

/* Internal routines */
static VINT find_indices( VFLOAT Lat, VFLOAT Lon, VINT time, 
			  struct index_struct *pIndices );
static int interpolate( float inlon, float inlat, int time, float *pValue, 
			float *pValue_Stdev );
static float interp_2pnt( float inlon, float inlat, float *pData, 
			  struct index_struct *pLoc, int ival );
static float interp_3pnt( float *pData, struct index_struct *pLoc, int ival );
static float interp_4pnt( float *pData, struct index_struct *pLoc, int ival );

/*
 * NAME: 
 *
 *    make_diurnal_filename
 *
 * FUNCTION:
 *
 *    C Code to make the name of the DW model file based on date
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Code to make the correct filename based on the date
 *
 * PSEUDO CODE
 *
 *    FUNCTION filename file_stem year month day
 *
 *       Make filename
 *
 *    END
 *
 * FILES NEEDED: 
 *    
 * SUBROUTINE/FUNCTION CALLS: 
 *
 * CALLING SEQUENCE:  
 *
 *    dwfilename = make_diurnal_file( file_stem, year, day )
 *
 * INPUTS:  -=
 *    
 *   file_stem - Diurnal warming filename stem including directory
 *   year      - year
 *   day       - day number
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 *    Returns a complete DW filename
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 05/27/2014
 *
 */
VINT make_diurnal_filename( char *filestem, VINT Year, VINT Day, char *pOutfile )
{
  /* Local variables */

  strcpy(pOutfile,filestem);
  strcat(pOutfile,get_ghrsst_datestring(Year,Day));
  strcat(pOutfile,".nc");
  return(0);
}

/*
 * NAME: 
 *
 *    free_diurnal_file
 *
 * FUNCTION:
 *
 *    C Code to free memory from diurnal model
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    C Code to free memory from diurnal model
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
 *    free_diurnal_file( )
 *
 * INPUTS:  
 *    
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
void free_diurnal_file(  )
{
  if( 1 == DW_Store.read_in_file ){
    free(DW_Store.pTime);
    free(DW_Store.pDW);
    free(DW_Store.pStdev);
  }
}

/*
 * NAME: 
 *
 *    read_diurnal_file
 *
 * FUNCTION:
 *
 *    C Code to read in a NetCDF containing the diurnal model
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Code to read in a NetCDF containing the diurnal model.
 *    Note reads into a locally global structure for access by
 *    other routines.
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
 *    ok = read_diurnal_file( filename )
 *
 * INPUTS:  
 *    
 *   filename - Diurnal warming filename
 *
 * OUTPUTS:  
 * 
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
VINT read_diurnal_file( const char *pFilename )
{
  /* Local variables */
  int nc_id = 0;
  int status = 0;
  int dimid_lon = 0;
  int dimid_lat = 0;
  int dimid_time = 0;
  int varid_time = 0;
  int varid_dw = 0;
  int varid_stdev = 0;
  size_t start1[1] = {0};
  size_t count1[1] = {0};
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};

  VINT i = 0;
  VINT j = 0;
  VINT k = 0;

  if( 0 == strcmp(pFilename,stored_DWfile) ){
    /* Already read in this file - don't bother again */
    return(0);
  }

  /* Open file */
  status = nc_open( pFilename, NC_NOWRITE, &nc_id );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot open NetCDF Diurnal Warming file ",pFilename);
    /* EXIT Can't open file */
    return(1);
  }

  /* Get min lat/lon and lat/lon step from global attributes */
  status = nc_get_att_float(nc_id, NC_GLOBAL, "minLat", &DW_Store.minLat);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read minLat attribute in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get minLat */
    return(1);
  }

  status = nc_get_att_float(nc_id, NC_GLOBAL, "minLon", &DW_Store.minLon);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read minLon attribute in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get minLon */
    return(1);
  }

  status = nc_get_att_float(nc_id, NC_GLOBAL, "latStep", &DW_Store.latStep);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read latStep attribute in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get latStep */
    return(1);
  }

  status = nc_get_att_float(nc_id, NC_GLOBAL, "lonStep", &DW_Store.lonStep);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read lonStep attribute in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get lonStep */
    return(1);
  }

  /* Get dimensions lon,lat and time */
  status = nc_inq_dimid(nc_id, "lon", &dimid_lon);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get lon dimid in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get lon dimid */
    return(1);
  }

  status = nc_inq_dimid(nc_id, "lat", &dimid_lat);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get lat dimid in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get lat dimid */
    return(1);
  }

  status = nc_inq_dimid(nc_id, "time", &dimid_time);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get time dimid in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  /* Get dimension sizes */
  status = nc_inq_dimlen(nc_id, dimid_lon, &DW_Store.nLon);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get nLon in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  status = nc_inq_dimlen(nc_id, dimid_lat, &DW_Store.nLat);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get nLat in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  status = nc_inq_dimlen(nc_id, dimid_time, &DW_Store.nTime);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get nTime in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }
  if( 1 >= DW_Store.nTime ){
    message(2,"ERROR: Need more than 1 time entry in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  /* Make max longitude/latitude */
  DW_Store.maxLon = DW_Store.minLon + (DW_Store.nLon-1)*DW_Store.lonStep;
  DW_Store.maxLat = DW_Store.minLat + (DW_Store.nLat-1)*DW_Store.latStep;

  /* Get variable IDs and check to see if stdev is present */
  status = nc_inq_varid(nc_id, "time", &varid_time);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varid time in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }
  
  status = nc_inq_varid(nc_id, "DW", &varid_dw);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot get varid DW in file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }
  
  status = nc_inq_varid(nc_id, "standard_deviation", &varid_stdev);
  if( NC_NOERR != status ){
    /* Does not exist */
    DW_Store.stdev_there = 0;
  } else {
    DW_Store.stdev_there = 1;
  }

  /* Allocate arrays */
  if( NULL == (DW_Store.pTime = (int *) calloc(DW_Store.nTime,sizeof(int))) ){
    message(1,"ERROR: Cannot allocate DW Time array");
    /* EXIT Can't allocate data */
    exit(-1);
  }

  if( NULL == (DW_Store.pDW = (float *) 
	       calloc(DW_Store.nLon*DW_Store.nLat*DW_Store.nTime,
					       sizeof(float))) ){
    message(1,"ERROR: Cannot allocate DW array");
    /* EXIT Can't allocate data */
    exit(-1);
  }

  /* If standard deviation there, make array */
  if( 1 == DW_Store.stdev_there ){
    if( NULL == (DW_Store.pStdev = (float *) 
		 calloc(DW_Store.nLon*DW_Store.nLat*DW_Store.nTime,
			sizeof(float))) ){
      message(1,"ERROR: Cannot allocate DW stdev array");
      /* EXIT Can't allocate data */
      exit(-1);
    }
  }

  /* Read in data */
  /* Get Time */
  start1[0] = 0;
  count1[0] = DW_Store.nTime;

  status = nc_get_vara_int(nc_id,varid_time,&start1[0],&count1[0],
			   DW_Store.pTime);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read time from file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  /* Get first entry (assumed minimnum time) and step (assumes that step is 
   * uniform through file) */
  DW_Store.minTime = *(DW_Store.pTime);
  DW_Store.timeStep = (VFLOAT)(*(DW_Store.pTime+1) - *(DW_Store.pTime));

  /* Get DW/stdev arrays */
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;
  count[0] = DW_Store.nTime;
  count[1] = DW_Store.nLat;
  count[2] = DW_Store.nLon;

  /* Get DW data */
  status = nc_get_vara_float(nc_id,varid_dw,&start[0],&count[0],
			     DW_Store.pDW);
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot read DW from file ",pFilename);
    nc_close( nc_id );
    /* EXIT Can't get time dimid */
    return(1);
  }

  /* If there get standard deviation */
  if( 1 == DW_Store.stdev_there ){
    status = nc_get_vara_float(nc_id,varid_stdev,&start[0],&count[0],
			       DW_Store.pStdev);
    if( NC_NOERR != status ){
      message(2,"ERROR: Cannot read stdev from file ",pFilename);
      nc_close( nc_id );
      /* EXIT Can't get time dimid */
      return(1);
    }
  }
  
  /* Close NetCDF file */
  status = nc_close( nc_id );
  if( NC_NOERR != status ){
    message(2,"ERROR: Cannot close NetCDF Diurnal Warming file ",pFilename);
    /* EXIT Can't open file */
    return(1);
  }

  /* Set read in flag */
  DW_Store.read_in_file = 1;
  
  /* Store filename to make sure we don't read it in multiple times */
  strcpy(stored_DWfile,pFilename);

  /* Return read status */
  return(0);
}

/*
 * NAME: 
 *
 *    remove_diurnal_warming
 *
 * FUNCTION:
 *
 *    C Removes diurnal warming from a dataset
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Removes diurnal warming from a dataset by interpolating from diurnal 
 *    model grid in time and space.  Also outputs an estimate of the error
 *    on the model if present
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
 *    remove_diurnal_warming( Time, Lat, Lon, SST, Variance )
 *
 * INPUTS:  
 *    
 *   Time     - GHRSST Time of each line  
 *   Lat      - Latitude
 *   Lon      - Longitude
 *
 * INPUTS/OUTPUTS:  
 * 
 *   SST      - SST
 *   Variance - Variance
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
void remove_diurnal_warming( struct int_struct *pTime,
			     struct float_struct *pLon,
			     struct float_struct *pLat,
			     struct float_struct *pSST,
			     struct float_struct *pVariance )
{
  int i=0;
  float value = 0.;
  float stdev = 0.;

  if( 1 != DW_Store.read_in_file ){
    message(1,"ERROR: No Diurnal Warming file read in");
    return;
  }

  for(i=0;i<pSST->size;i++){
    if( finite(*(pSST->array+i)) && -1e29 < *(pSST->array+i) ){
      if( 0 == interpolate(*(pLon->array+i),*(pLat->array+i),*(pTime->array+i),
			 &value,&stdev) ){
	*(pSST->array+i) -= value;
	*(pVariance->array+i) += stdev*stdev;
      }
    }
  }

}

/*
 * NAME: 
 *
 *    find_indices
 *
 * FUNCTION:
 *
 *    C Find the nearest indices from an input lat/lon
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Given a lat/lon pair, find the nearest surrounding grid points from 
 *    the DW model.  Can be 1-4 acceptable points
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
 *    ok = find_indices( lat, lon, time, indices )
 *
 * INPUTS:  
 *    
 *   lat     - Latitude
 *   lon     - Longitude
 *   time    - GHRSST time
 *
 * OUTPUTS:  
 * 
 *   indices - lust of valid surrounding points
 *
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
static VINT find_indices( VFLOAT Lat, VFLOAT Lon, VINT time, 
			  struct index_struct *pIndices )
{
  /* Local variables */
  int i = 0;
  int j = 0;
  int ipos = 0;
  int jpos = 0;
  int iipos = 0;
  int jjpos = 0;
  int pos1[4] = {0,0,0,0};
  int pos2[4] = {0,0,0,0};

  /* Check that file been read in */
  if( 1 != DW_Store.read_in_file ){
    message(1,"ERROR: NetCDF DW file not read in");
    /* EXIT Can't open file */
    return(1);
  }

  /* Check we are in bound */
  /* Note - longitude is allowed to wrap (359.5 -> 0.). Latitude is not */
  /* Strangely, some NaN values for Lat &/or Lon have also been getting in  */
  if( finite(Lat) && finite(Lon) ){
    if( (Lat < DW_Store.minLat) || (Lat > DW_Store.maxLat) ){
      return(1);
    }
    if( (Lon < -360.) || (Lon > 360.) ){
      return(1);
    }
  } else {
    return(1);
  }

  /* Get surrounding time location2 */
  pIndices->time[0] = (int) ((time - DW_Store.minTime)/DW_Store.timeStep);
  pIndices->time_fraction = (time - 
		     (DW_Store.minTime+pIndices->time[0]*DW_Store.timeStep))/
    DW_Store.timeStep;
  pIndices->time[1] = pIndices->time[0]+1;
  if( DW_Store.nTime <= pIndices->time[1] ){
    pIndices->time[1] = -1;
  }

  /* Set number of indices = 0 */
  pIndices->nindices[0] = 0;
  pIndices->nindices[1] = 0;
  /* Set index positions to zero */
  pIndices->index_pos[0][0] = 0;
  pIndices->index_pos[0][1] = 0;
  pIndices->index_pos[0][2] = 0;
  pIndices->index_pos[0][3] = 0;
  pIndices->index_pos[1][0] = 0;
  pIndices->index_pos[1][1] = 0;
  pIndices->index_pos[1][2] = 0;
  pIndices->index_pos[1][3] = 0;
  pIndices->indices_x[0][0] = -1;
  pIndices->indices_x[0][1] = -1;
  pIndices->indices_x[0][2] = -1;
  pIndices->indices_x[0][3] = -1;
  pIndices->indices_x[1][0] = -1;
  pIndices->indices_x[1][1] = -1;
  pIndices->indices_x[1][2] = -1;
  pIndices->indices_x[1][3] = -1;
  pIndices->indices_y[0][0] = -1;
  pIndices->indices_y[0][1] = -1;
  pIndices->indices_y[0][2] = -1;
  pIndices->indices_y[0][3] = -1;
  pIndices->indices_y[1][0] = -1;
  pIndices->indices_y[1][1] = -1;
  pIndices->indices_y[1][2] = -1;
  pIndices->indices_y[1][3] = -1;

  /* Find locations of indices and check we have valid data */
  /* Take into account possible -180,180 and 0, 360. confusion */
  ipos = (int) ((Lon - DW_Store.minLon)/DW_Store.lonStep);
  jpos = (int) ((Lat - DW_Store.minLat)/DW_Store.latStep);
  
  pIndices->lonWeight = (Lon - (DW_Store.minLon+ipos*DW_Store.lonStep))/
			 DW_Store.lonStep;
  pIndices->latWeight = (Lat - (DW_Store.minLat+jpos*DW_Store.latStep))/
			 DW_Store.latStep;
  /* First point */
  pos1[0] = ipos + jpos*DW_Store.nLon + 
    pIndices->time[0]*DW_Store.nLon*DW_Store.nLat;
  if( (-1e20 < *(DW_Store.pDW+pos1[0])) && finite(*(DW_Store.pDW+pos1[0])) ){
    pIndices->index_pos[0][0] = 1;
    pIndices->values[0][0] = *(DW_Store.pDW+pos1[0]);
    pIndices->indices_x[0][pIndices->nindices[0]] = ipos;
    pIndices->indices_y[0][pIndices->nindices[0]] = jpos;
    pIndices->nindices[0] += 1;
  }
  if( 0 <= pIndices->time[1] ){
    pos2[0] = ipos + jpos*DW_Store.nLon + 
      pIndices->time[1]*DW_Store.nLon*DW_Store.nLat;
    if( -1e20 < *(DW_Store.pDW+pos2[0]) && finite(*(DW_Store.pDW+pos2[0])) ){
      pIndices->index_pos[1][0] = 1;
      pIndices->values[1][0] = *(DW_Store.pDW+pos2[0]);
      pIndices->indices_x[1][pIndices->nindices[1]] = ipos;
      pIndices->indices_y[1][pIndices->nindices[1]] = jpos;
      pIndices->nindices[1] += 1;
    }
  }

  /* Step 1 in Lon */
  iipos = ipos+1;
  /* Check if we need to loop round */
  if( iipos >= DW_Store.nLon ){
    iipos = 0;
  }
  pos1[1] = iipos + jpos*DW_Store.nLon + 
    pIndices->time[0]*DW_Store.nLon*DW_Store.nLat;
  if( -1e20 < *(DW_Store.pDW+pos1[1]) && finite(*(DW_Store.pDW+pos1[1])) ){
    pIndices->index_pos[0][1] = 1;
    pIndices->values[0][1] = *(DW_Store.pDW+pos1[1]);
    pIndices->indices_x[0][pIndices->nindices[0]] = iipos;
    pIndices->indices_y[0][pIndices->nindices[0]] = jpos;
    pIndices->nindices[0] += 1;
  }
  if( 0 <= pIndices->time[1] ){
    pos2[1] = iipos + jpos*DW_Store.nLon + 
      pIndices->time[1]*DW_Store.nLon*DW_Store.nLat;
    if( -1e20 < *(DW_Store.pDW+pos2[1]) && finite(*(DW_Store.pDW+pos2[1])) ){
      pIndices->index_pos[1][1] = 1;
      pIndices->values[1][1] = *(DW_Store.pDW+pos2[1]);
      pIndices->indices_x[1][pIndices->nindices[1]] = iipos;
      pIndices->indices_y[1][pIndices->nindices[1]] = jpos;
      pIndices->nindices[1] += 1;
    }
  }

  /* Step 1 in Lon and Lat */
  jjpos = jpos+1;
  /* Check if we need to loop round */
  if( jjpos >= DW_Store.nLat ){
    jjpos = 0;
  }
  pos1[2] = iipos + jjpos*DW_Store.nLon + 
    pIndices->time[0]*DW_Store.nLon*DW_Store.nLat;
  if( -1e20 < *(DW_Store.pDW+pos1[2]) && finite(*(DW_Store.pDW+pos1[2]))  ){
    pIndices->index_pos[0][2] = 1;
    pIndices->values[0][2] = *(DW_Store.pDW+pos1[2]);
    pIndices->indices_x[0][pIndices->nindices[0]] = iipos;
    pIndices->indices_y[0][pIndices->nindices[0]] = jjpos;
    pIndices->nindices[0] += 1;
  }
  if( 0 <= pIndices->time[1] ){
    pos2[2] = iipos + jjpos*DW_Store.nLon + 
      pIndices->time[1]*DW_Store.nLon*DW_Store.nLat;
    if( -1e20 < *(DW_Store.pDW+pos2[2]) && finite(*(DW_Store.pDW+pos2[2])) ){
      pIndices->index_pos[1][2] = 1;
      pIndices->values[1][2] = *(DW_Store.pDW+pos2[2]);
      pIndices->indices_x[1][pIndices->nindices[1]] = iipos;
      pIndices->indices_y[1][pIndices->nindices[1]] = jjpos;
      pIndices->nindices[1] += 1;
    }
  }

  /* Step 1 in Lat */
  pos1[3] = ipos + jjpos*DW_Store.nLon + 
    pIndices->time[0]*DW_Store.nLon*DW_Store.nLat;
  if( -1e20 < *(DW_Store.pDW+pos1[3]) && finite(*(DW_Store.pDW+pos1[3]))  ){
    pIndices->index_pos[0][3] = 1;
    pIndices->values[0][3] = *(DW_Store.pDW+pos1[3]);
    pIndices->indices_x[0][pIndices->nindices[0]] = ipos;
    pIndices->indices_y[0][pIndices->nindices[0]] = jjpos;
    pIndices->nindices[0] += 1;
  }
  if( 0 <= pIndices->time[1] ){
    pos2[3] = ipos + jjpos*DW_Store.nLon + 
      pIndices->time[1]*DW_Store.nLon*DW_Store.nLat;
    if( -1e20 < *(DW_Store.pDW+pos2[3]) && finite(*(DW_Store.pDW+pos2[3])) ){
      pIndices->index_pos[1][3] = 1;
      pIndices->values[1][3] = *(DW_Store.pDW+pos2[3]);
      pIndices->indices_x[1][pIndices->nindices[1]] = ipos;
      pIndices->indices_y[1][pIndices->nindices[1]] = jjpos;
      pIndices->nindices[1] += 1;
    }
  }

  /* Return status */
  if( 0 == pIndices->nindices[0] || 0 == pIndices->nindices[1] ){
    return(1);
  } else {
    /* Setup pos values */
    j = 0;
    for(i=0;i<4;i++){
      if( 1 == pIndices->index_pos[0][i] ){
	pIndices->pos[0][j] = pos1[i];
	j++;
      }
    }
    if( j != pIndices->nindices[0] ){
      message(1,"ERROR: number of good indices inconsistent in find_indeces");
      return(1);
    }
    j = 0;
    for(i=0;i<4;i++){
      if( 1 == pIndices->index_pos[1][i] ){
	pIndices->pos[1][j] = pos2[i];
	j++;
      }
    }
    if( j != pIndices->nindices[1] ){
      message(1,"ERROR: number of good indices inconsistent in find_indeces");
      return(1);
    }
    return(0);
  }
}

/*
 * NAME: 
 *
 *    interpolate
 *
 * FUNCTION:
 *
 *    C Interpolates spatially
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Interpolates spatially.  Have different algorithms depending on 
 *    how many indices there are
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
 *    value = interpolate( lat, lon, time )
 *
 * INPUTS:  
 *    
 *   lat     - Latitude
 *   lon     - Longitude
 *   time    - time
 *
 * OUTPUTS:  
 *
 *   value   - output value
 * 
 * RETURNS:
 *
 *    Returns a status
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
static int interpolate( float inlon_in, float inlat_in, 
			int time, float *pValue, 
			float *pValue_Stdev )
{
  /* Local variables */
  int i = 0;
  struct index_struct loc;
  float values[2];
  float stdev[2];
  float inlon = 0.;
  float inlat = 0.;

  /* Make sure the input longitude matches the DW model */
  if( DW_Store.minLon >= 0. ){
    if( inlon_in < 0 ){
      inlon = inlon_in + 360.;
    } else {
      inlon = inlon_in;
    }
  } else {
    if( inlon_in > 180. ){
      inlon = inlon_in - 360.;
    } else {
      inlon = inlon_in;
    }
  }
  inlat = inlat_in;

  /* If data available */
  if( 0 == find_indices(inlat,inlon,time,&loc) ){
    /* Loop round time[0] or time[1] */
    for(i=0;i<2;i++){
      values[i] = 0.;
      stdev[i] = 0.;
      /* Interpolation depends on number of indices */
      switch(loc.nindices[i]){
      case 1:
	values[i] = *(DW_Store.pDW+loc.pos[i][0]);	
	if( 1 == DW_Store.stdev_there ){
	  stdev[i] = *(DW_Store.pStdev+loc.pos[i][0]);
	}
	break;
      case 2:
	values[i] = interp_2pnt(inlon,inlat,DW_Store.pDW,&loc,i);
	if( 1 == DW_Store.stdev_there ){
	  stdev[i] = interp_2pnt(inlon,inlat,DW_Store.pStdev,&loc,i);
	}
	break;
      case 3:
	values[i] = interp_3pnt(DW_Store.pDW,&loc,i);
	if( 1 == DW_Store.stdev_there ){
	  stdev[i] = interp_3pnt(DW_Store.pStdev,&loc,i);
	}
	break;
      case 4:
	values[i] = interp_4pnt(DW_Store.pDW,&loc,i);
	if( 1 == DW_Store.stdev_there ){
	  stdev[i] = interp_4pnt(DW_Store.pStdev,&loc,i);
	}
	break;
      default:
	message(1,"Wrong number of indices");
	/* EXIT wrong number indices */
	return(1);
      }
    }
  } else {
    /* No interpolation possible */
    return(1);
  }

  /* Do time interpolation */
  *pValue = (1-loc.time_fraction)*values[0] + loc.time_fraction*values[1];
  if( 1 == DW_Store.stdev_there ){
    *pValue_Stdev = (1-loc.time_fraction)*stdev[0] + 
      loc.time_fraction*stdev[1];
  } else {
    *pValue_Stdev = 0.;
  }

  /* Success */
  return(0);
}

/*
 * NAME: 
 *
 *    interp_2pnt
 *
 * FUNCTION:
 *
 *    C Interpolates spatially when we have 2 points only
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Interpolates spatially when we have 2 points only
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
 *    value = interp_2pnt( lon, lat, data, loc, ival )
 *
 * INPUTS:  
 *    
 *   lon     - longitude
 *   lat     - latitude
 *   data    - Data to interpolate
 *   loc     - index locations
 *   ival    - time location
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the interpolated value
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
static float interp_2pnt( float inlon, float inlat, float *pData, 
			   struct index_struct *pLoc, int ival )
{
  float lon[2];
  float lat[2];

  float xdiff = 0.;
  float ydiff = 0.;
  float fraction = 0.;
  float value = 0.;

  if( 2 != pLoc->nindices[ival] ){
    message(1,"ERROR: only 2 indices required for interpolation");
    return(0.);
  }

  /* Get locations */
  lon[0] = DW_Store.minLon+pLoc->indices_x[ival][0]*DW_Store.lonStep;
  lat[0] = DW_Store.minLat+pLoc->indices_y[ival][0]*DW_Store.latStep;
  lon[1] = DW_Store.minLon+pLoc->indices_x[ival][1]*DW_Store.lonStep;
  lat[1] = DW_Store.minLat+pLoc->indices_y[ival][1]*DW_Store.latStep;
  if( lon[1] != lon[0] ){
    xdiff = (inlon-lon[0])/(lon[1]-lon[0]);
  } else {
    xdiff = 0.;
  }
  if( lat[1] != lat[0] ){
    ydiff = (inlat-lat[0])/(lat[1]-lat[0]);
  } else {
    ydiff = 0.;
  }

  if( 0. == xdiff ){
    value = (1-ydiff) * *(pData+pLoc->pos[ival][0]) + 
      ydiff * *(pData+pLoc->pos[ival][1]);
  } else if( 0 == ydiff ){
    value = (1-xdiff) * *(pData+pLoc->pos[ival][0]) + 
      xdiff * *(pData+pLoc->pos[ival][1]);
  } else {
    fraction = sqrt((xdiff*xdiff + ydiff*ydiff))/SQRT_2;
    value = (1-fraction) * *(pData+pLoc->pos[ival][0]) + 
      fraction * *(pData+pLoc->pos[ival][1]);
  }

  return(value);
}

/*
 * NAME: 
 *
 *    interp_3pnt
 *
 * FUNCTION:
 *
 *    C Interpolates spatially when we have 3 points only
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Interpolates spatially when we have 3 points only
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
 *    value = interp_3pnt( data, loc, ival )
 *
 * INPUTS:  
 *    
 *   data    - Data to interpolate
 *   loc     - index locations
 *   ival    - time location
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the interpolated value
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
static float interp_3pnt( float *pData, struct index_struct *pLoc, int ival )
{
  /* Local variables */
  int i = 0;
  int index = 0;
  int map_vals[4] = {1,2,3,4};
  float vals[3] = {0.,0.,0.};
  float dlon = 0.;
  float dlat = 0.;

  /* Check that this is a 3pnt interpolation */
  if( 3 != pLoc->nindices[ival] ){
    message(1,"ERROR: only 3 indices required for interpolation");
    return(0.);
  }
  
  index = 0;
  for(i=0;i<4;i++){
    if( 1 == pLoc->index_pos[ival][i] ){
      index += map_vals[i];
    }
  }

  switch(index){
  case 6:
    vals[0] = pLoc->values[ival][0];
    vals[1] = pLoc->values[ival][1];
    vals[2] = pLoc->values[ival][2];
    dlon = pLoc->lonWeight;
    dlat = pLoc->latWeight;
    break;
  case 7:
    vals[0] = pLoc->values[ival][1];
    vals[1] = pLoc->values[ival][0];
    vals[2] = pLoc->values[ival][3];
    dlon = 1.0 - pLoc->lonWeight;
    dlat = pLoc->latWeight;
    break;
  case 8:
    vals[0] = pLoc->values[ival][2];
    vals[1] = pLoc->values[ival][3];
    vals[2] = pLoc->values[ival][1];
    dlon = 1.0 - pLoc->lonWeight;
    dlat = 1.0 - pLoc->latWeight;
    break;
  case 9:
    vals[0] = pLoc->values[ival][3];
    vals[1] = pLoc->values[ival][2];
    vals[2] = pLoc->values[ival][1];
    dlon = pLoc->lonWeight;
    dlat = 1.0 - pLoc->latWeight;
    break;
  default:
    message(1,"ERROR: 3pt indexing incorrect");
    return(0.);
  }

  return((1.0-dlon)*vals[0]+(dlon-dlat)*vals[1]+dlat*vals[2]);

}

/*
 * NAME: 
 *
 *    interp_4pnt
 *
 * FUNCTION:
 *
 *    C Interpolates spatially when we have all 4 points
 *
 * TYPE/LANGUAGE: 
 *
 *    C routine
 *
 * DESCRIPTION:  
 *
 *    Interpolates spatially when we have 4 points
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
 *    value = interp_4pnt( data, loc, ival )
 *
 * INPUTS:  
 *    
 *   data    - Data to interpolate
 *   loc     - index locations
 *   ival    - time location
 *
 * OUTPUTS:  
 *
 * RETURNS:
 *
 *    Returns the interpolated value
 *
 * SYSTEM CALLS:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 02/12/2013
 *
 */
static float interp_4pnt( float *pData, struct index_struct *pLoc, int ival )
{
  /* Local variables */
  float vals[3] = {0.,0.,0.};
  float dlon = 0.;
  float dlat = 0.;
  
  /* Check that this is a 3pnt interpolation */
  if( 4 != pLoc->nindices[ival] ){
    message(1,"ERROR: only 4 indices required for interpolation");
    return(0.);
  }
  
  vals[0] = pLoc->values[ival][0];
  vals[1] = pLoc->values[ival][1];
  vals[2] = pLoc->values[ival][2];
  vals[3] = pLoc->values[ival][3];
  dlon = pLoc->lonWeight;
  dlat = pLoc->latWeight;

  /* Do bilinear interpolation */
  return((1.0-dlon)*(1.0-dlat)*vals[0]+(1.0-dlat)*dlon*vals[1]+
	 dlon*dlat*vals[2]+(1.0-dlon)*dlat*vals[3]);
  
}
