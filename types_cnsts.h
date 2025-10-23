/*
 * NAME: 
 *
 *    types_cnsts
 *
 * FUNCTION:
 *
 *    Header for typdefs and constants for POES-GOES blended code
 *
 * TYPE/LANGUAGE: 
 *
 *    C header
 *
 * DESCRIPTION:  
 *
 *    Defines int and float types for flexibility and portability 
 *
 *    Defines float and list structures which contain the number of elements
 *    and the values to more closely mimic the high level functionality of 
 *    MATLAB
 *
 *    Defines constants
 *
 * ROUTINES TO BE USED EXTERNALLY:
 *
 * HISTORY:
 * 
 *   Original version by Jonathan P.D. Mittaz on 28/11/2011
 *
 */

#ifndef TYPES_CNSTS_HEADER

/* Need to include math.h to define NAN */
/* Will work on machines that support IEEE floating point numbers */
#include <math.h>

/* Define what a float is - this is so it can be trivally changed */
/* NB. the MATLAB code works in doubles but if memory use is an issue the */
/* ingestor code can use floats by changing this typedef and everything */
/* will still work */
typedef double VFLOAT;

/* Define an integer in machine/archetecture independent manner */
typedef int VINT;

/* Define short integer in machine/architecture independent manner */
typedef short VSHORT;

/* Output structures */
struct find_elems {
  VINT nelems;
  VINT *elems;
};

struct char_struct {
  VINT nx;
  VINT ny;
  VINT size;
  unsigned char *array;
};

struct float_struct {
  VINT nx;
  VINT ny;
  VINT size;
  VFLOAT *array;
};

struct float_struct1d {
  VINT size;
  VFLOAT *array;
};

struct char_struct1d {
  VINT size;
  unsigned char *array;
};

struct int_struct {
  VINT nx;
  VINT ny;
  VINT size;
  VINT *array;
};

struct byte_struct {
  VINT nx;
  VINT ny;
  VINT size;
  char *array;
};

struct pixel_extent_str {
  VINT xlow;
  VINT xhigh;
  VINT ylow;
  VINT yhigh;
};

struct float_struct3d {
  VINT nx;
  VINT ny;
  VINT nz;
  VINT size;
  VFLOAT *array;
};

struct sst_struct {
  struct float_struct sst;
  struct float_struct est_variance;
  struct float_struct lat;
  struct float_struct lon;
  VFLOAT minlat;
  VFLOAT maxlat;
  VFLOAT deltalat;
  VFLOAT minlon;
  VFLOAT maxlon;
  VFLOAT deltalon;
};

struct out_sst_str {
  VINT nx;
  VINT ny;
  struct pixel_extent_str Pixel_Extent;
  struct float_struct SST;
  struct float_struct Stdev;
  struct int_struct Gridcnt;
};

#define MAX_STRING_LENGTH 256
struct par_files_str {
  char file_info[MAX_STRING_LENGTH];
  char par_info[MAX_STRING_LENGTH];
};

/* #define NHOURS_DAY 24 ** N.B. This is only used to define # files/day, which can be much more than 24 with Geo L2P ** */
#define NHOURS_DAY 96
struct big_geo_grid {
  VINT nData;
  struct float_struct SST[NHOURS_DAY];
  struct float_struct Variance[NHOURS_DAY];
};

/* Variables */
#define NaN ((VFLOAT) sqrt(-1))
#define ZERO_FLT 0.
#define ONE_THOUSAND_FLT 1000.
#define ABS_ZERO 273.15
#define DEFLATE_SAVE_FILE 9
#define DAYTIME_MIN 10.
#define DAYTIME_MAX 19.

/* AVHRR Numbers */
#define NOAA_16 16
#define NOAA_17 17
#define NOAA_18 18
#define NOAA_19 19
#define METOPA -1
#define METOPB -2
#define METOPC -3
#define VIIRS  -10
#define JPSS   -11

/* AMSR Numbers */
#define AMSR_E 1
#define AMSR_2 2

/* SLSTR Numbers */
#define S3A 101
#define S3B 102
#define S3A2 201
#define S3B2 202

/* macros */
#define MIN(a,b) ((a < b) ? a : b)
#define MAX(a,b) ((a > b) ? a : b)
#define NINT(a) ((int)(a+0.5))

#define TYPES_CNSTS_HEADER 1
#endif
