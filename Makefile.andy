CC = gcc
#MEX = /data/starfs1/compilers/matlabR2013b/bin/mex
MEX = /data/starfs1/compilers/matlabR2019a/bin/mex

#C_FLAGS = -fPIC -O
C_FLAGS = -fPIC -Wall -g
M_FLAGS = -g

INCLUDE = -I.

INCLUDE_HDF = -I/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/include
LIBS_HDF = -L/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/lib -lmfhdf -ldf -ljpeg -lz 

INCLUDE_NETCDF = -I/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/include
LIBS_NETCDF = -L/data/data008/pub/jmittaz/POES_GOES/c_code_andy/lib/lib -lnetcdf

LIBS = $(LIBS_HDF) $(LIBS_NETCDF) 

OBJECTS_C = get_dates.o init_file_info.o init_par_info.o logfile.o \
	matlab_functions.o process_raw_avhrr_acspo_c.o process_raw_geo_c.o \
	read_acspo.o read_geo_data.o read_raw_geo.o read_ghrsst.o \
	read_netcdf.o diurnal_warming.o process_raw_geo_ghrsst_c.o read_geo_ghrsst.o \
	process_raw_amsr_ghrsst_c.o read_amsr_ghrsst.o

all: process_raw_avhrr_acspo process_raw_geo process_raw_amsr

process_raw_avhrr_acspo: $(OBJECTS_C)
	$(MEX) $(CFLAGS) process_raw_avhrr_acspo_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)
process_raw_geo: $(OBJECTS_C)
	$(MEX) $(CFLAGS) process_raw_geo_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)
process_raw_amsr: $(OBJECTS_C)
	$(MEX) $(CFLAGS) process_raw_amsr_mex.c mex_functions.c $(OBJECTS_C) $(LIBS)

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
process_raw_geo_c.o: process_raw_geo_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h read_geo_data.h process_raw_geo_c.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_geo_c.c
read_acspo.o: read_acspo.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_acspo.c
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
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_geo_ghrsst.c
process_raw_amsr_ghrsst_c.o: process_raw_amsr_ghrsst_c.c types_cnsts.h logfile.h init_file_info.h matlab_functions.h process_raw_amsr_c.h process_raw_amsr_ghrsst_c.h
	$(CC) $(INCLUDE) $(C_FLAGS) $(CFLAGS) -c process_raw_amsr_ghrsst_c.c
read_amsr_ghrsst.o: read_amsr_ghrsst.c types_cnsts.h logfile.h init_par_info.h matlab_functions.h read_acspo.h read_ghrsst.h init_par_info.h
	$(CC) $(INCLUDE) $(INCLUDE_HDF) $(C_FLAGS) $(CFLAGS) -c read_amsr_ghrsst.c

