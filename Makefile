CC = gcc # this will still invoke Clang on macOS
#MEX = /data/starfs1/compilers/matlabR2013b/bin/mex # original
#MEX = /data/starfs1/compilers/matlabR2019a/bin/mex
#MEX = mex # Andy's edit

# Path to the mex directory.
#MEX = /Applications/MATLAB_R2025b.app/bin/mex # for MacBook Pro (MomoeYoshida Oct2025)
MEX = /gpfs01/sw/matlab/R2024a/bin/mex # for JCU HPC

#C_FLAGS = -fPIC -O
#C_FLAGS = -fPIC -Wall -g
C_FLAGS = -fPIC -Wall -g -Dfinite=isfinite 
#CFLAGS = -fPIC -Wall -g -Dfinite=isfinite -arch arm64 # Momoe added the arm64 architecture flags

# Link to the same libraries that MATLAB itself uses on macOS (MacBook Pro–UMD).
#MATLABROOT = /Applications/MATLAB_R2025b.app
MATLABROOT = /sw/matlab/R2024a

INCLUDE = -I.

# Include headers from MATLAB.
INCLUDE_NETCDF = -I$(MATLABROOT)/extern/include
INCLUDE_HDF = -I$(MATLABROOT)/extern/include

# Link against MATLAB’s built-in NetCDF/HDF5 libs.
#LDFLAGS = -L$(MATLABROOT)/bin/maca64 -lnetcdf -lhdf5 -lz -lm
#LIBS = -L$(MATLABROOT)/bin/maca64 -lnetcdf -lhdf5 -lz -lm
LDFLAGS = -L$(MATLABROOT)/bin/glnxa64 -lnetcdf -lhdf5 -lz -lm # for JCU HPC
LIBS = -L$(MATLABROOT)/bin/glnxa64 -lnetcdf -lhdf5 -lz -lm


# Path to the library directory.
#LDFLAGS = -L/opt/homebrew/lib -L/opt/homebrew/opt/jpeg/lib -lnetcdf -ljpeg -lz -lm # for MacBook Pro (MomoeYoshida Oct2025)
M_FLAGS = -g

#INCLUDE_HDF = -I/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/include
#LIBS_HDF = -L/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/lib -lmfhdf -ldf -ljpeg -lz
#INCLUDE_HDF = -I/opt/homebrew/include -I/opt/homebrew/opt/jpeg/include # Andy's edit + Momoe's edit
#LIBS_HDF = -L/opt/homebrew/lib -L/opt/homebrew/opt/jpeg/lib -ljpeg -lz # Andy's edit + Momoe's edit (drop HDF4 libraries, eliminate the need for HDF4)
#INCLUDE_HDF = -I/Users/momotalo/hdf4_build/hdf-4.2.15/hdf/src -I/Users/momotalo/hdf4_build/hdf-4.2.15/mfhdf/libsrc # Momoe's edit
#LIBS_HDF = -L/Users/momotalo/hdf4_build/hdf-4.2.15/hdf/src -L/Users/momotalo/hdf4_build/hdf-4.2.15/mfhdf/libsrc -lmfhdf -ldf -ljpeg -lz # Momoe's edit

#INCLUDE_NETCDF = -I/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/include
#LIBS_NETCDF = -L/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/lib -lnetcdf
#INCLUDE_NETCDF = -I/opt/homebrew/include
#LIBS_NETCDF = -L/opt/homebrew/lib -lnetcdf


#LIBS = $(LIBS_HDF) $(LIBS_NETCDF)

OBJECTS_C = get_dates.o init_file_info.o init_par_info.o logfile.o \
	matlab_functions.o process_raw_avhrr_acspo_c.o process_raw_geo_c.o \
	read_acspo_ghrsst.o read_geo_data.o read_raw_geo.o read_ghrsst.o \
	read_netcdf.o diurnal_warming.o process_raw_geo_ghrsst_c.o read_geo_ghrsst.o
#OBJECTS_C = get_dates.o init_file_info.o init_par_info.o logfile.o \
#	matlab_functions.o process_raw_avhrr_acspo_c.o process_raw_geo_c.o \
#	read_geo_data.o read_raw_geo.o read_ghrsst.o \
#	read_netcdf.o diurnal_warming.o process_raw_geo_ghrsst_c.o \
#	process_raw_amsr_ghrsst_c.o process_raw_amsr_c.o read_amsr_ghrsst.o stubs.o # Momoe removed files that contain HDF4 calls and added stubs.o (dummy versions of old functions using HDF4)

all: process_raw_avhrr_acspo process_raw_geo
#all: process_raw_avhrr_acspo process_raw_geo process_raw_amsr

#process_raw_avhrr_acspo: $(OBJECTS_C)
#	$(MEX) $(CFLAGS) $(LDFLAGS) process_raw_avhrr_acspo_mex.c mex_functions.c $(OBJECTS_C) # Momoe edited
#process_raw_geo: $(OBJECTS_C)
#	$(MEX) $(CFLAGS) $(LDFLAGS) process_raw_geo_mex.c mex_functions.c $(OBJECTS_C) # Momoe edited
#process_raw_amsr: $(OBJECTS_C)
#	$(MEX) $(CFLAGS) $(LDFLAGS) process_raw_amsr_mex.c mex_functions.c $(OBJECTS_C) # Momoe edited

#process_raw_avhrr_acspo: $(OBJECTS_C)
#	$(MEX) -v CFLAGS="\$$CFLAGS -Wall -g -Dfinite=isfinite -arch arm64" LDFLAGS="\$$LDFLAGS -L/opt/homebrew/lib -L/opt/homebrew/opt/jpeg/lib -lnetcdf -ljpeg -lz -lm" process_raw_avhrr_acspo_mex.c mex_functions.c $(OBJECTS_C)
#
#process_raw_geo: $(OBJECTS_C)
#	$(MEX) -v CFLAGS="\$$CFLAGS -Wall -g -Dfinite=isfinite -arch arm64" LDFLAGS="\$$LDFLAGS -L/opt/homebrew/lib -L/opt/homebrew/opt/jpeg/lib -lnetcdf -ljpeg -lz -lm" process_raw_geo_mex.c mex_functions.c $(OBJECTS_C)
#
#process_raw_amsr: $(OBJECTS_C)
#	$(MEX) -v CFLAGS="\$$CFLAGS -Wall -g -Dfinite=isfinite -arch arm64" LDFLAGS="\$$LDFLAGS -L/opt/homebrew/lib -L/opt/homebrew/opt/jpeg/lib -lnetcdf -ljpeg -lz -lm" process_raw_amsr_mex.c mex_functions.c $(OBJECTS_C)

process_raw_avhrr_acspo: $(OBJECTS_C)
	$(MEX) $(CFLAGS) process_raw_avhrr_acspo_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)
process_raw_geo: $(OBJECTS_C)
	$(MEX) $(CFLAGS) process_raw_geo_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)
#process_raw_amsr: $(OBJECTS_C)
#	$(MEX) $(CFLAGS) process_raw_amsr_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)


.PHONY: clean
clean:
	rm -f *.o *.mex*

get_dates.o: get_dates.c matlab_functions.h get_dates.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c get_dates.c
init_file_info.o: init_file_info.c init_file_info.h logfile.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c init_file_info.c
init_par_info.o: init_par_info.c init_par_info.h init_file_info.h logfile.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c init_par_info.c
logfile.o: logfile.c logfile.h init_file_info.h get_dates.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c logfile.c
matlab_functions.o: matlab_functions.c matlab_functions.h logfile.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c matlab_functions.c
process_raw_avhrr_acspo_c.o: process_raw_avhrr_acspo_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h get_dates.h read_acspo.h process_raw_avhrr_acspo_c.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_avhrr_acspo_c.c
#process_raw_avhrr_acspo_c.o: process_raw_avhrr_acspo_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h get_dates.h process_raw_avhrr_acspo_c.h
#	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_avhrr_acspo_c.c # Momoe removed files that contain HDF4 calls
process_raw_geo_c.o: process_raw_geo_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h read_geo_data.h process_raw_geo_c.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_geo_c.c
#read_acspo.o: read_acspo.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
#	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_acspo.c # Momoe commented out
read_acspo_ghrsst.o: read_acspo_ghrsst.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_acspo_ghrsst.c # Momoe commented out
read_geo_data.o: read_geo_data.c types_cnsts.h logfile.h init_par_info.h init_file_info.h matlab_functions.h get_dates.h read_raw_geo.h read_geo_data.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c read_geo_data.c
read_raw_geo.o: read_raw_geo.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_ghrsst.h read_raw_geo.h
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_raw_geo.c
read_ghrsst.o: read_ghrsst.c types_cnsts.h matlab_functions.h logfile.h read_ghrsst.h
	$(CC) $(INCLUDE) $(INCLUDE_NETCDF) $(C_FLAGS) $(CFLAGS) -c read_ghrsst.c
read_netcdf.o: read_netcdf.c types_cnsts.h matlab_functions.h logfile.h read_netcdf.h
	$(CC) $(INCLUDE) $(INCLUDE_NETCDF) $(C_FLAGS) $(CFLAGS) -c read_netcdf.c
diurnal_warming.o: diurnal_warming.c types_cnsts.h matlab_functions.h logfile.h init_par_info.h diurnal_warming.h
	$(CC) $(INCLUDE) $(INCLUDE_NETCDF) $(C_FLAGS) $(CFLAGS) -c diurnal_warming.c
process_raw_geo_ghrsst_c.o: process_raw_geo_ghrsst_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h read_geo_data.h process_raw_geo_c.h process_raw_geo_ghrsst_c.h process_raw_avhrr_acspo_c.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_geo_ghrsst_c.c
read_geo_ghrsst.o: read_geo_ghrsst.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_geo_ghrsst.c # Momoe commented out
#process_raw_amsr_c.o: process_raw_amsr_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h get_dates.h process_raw_amsr_c.h
#	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_amsr_c.c
#process_raw_amsr_ghrsst_c.o: process_raw_amsr_ghrsst_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h process_raw_amsr_c.h process_raw_amsr_ghrsst_c.h
#	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_amsr_ghrsst_c.c
#read_amsr_ghrsst.o: read_amsr_ghrsst.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
#	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_amsr_ghrsst.c # Momoe commented out
