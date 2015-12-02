HDF5_C_INCLUDE=-I /usr/local/hdf5/intel/15/mvapich2/2.1a/1.8.13/include
HDF5_C_LIBS=-L /usr/local/hdf5/intel/15/mvapich2/2.1a/1.8.13/lib -lpthread -lssl -lcrypto -lz -lm

CC = mpicc -Wall -g
CFLAGS = $(CFLAGSSERIAL)
INCLDIRS = $(HDF5_C_INCLUDE) -I /usr/include -I ../
LFLAGS = -lm $(HDF5_C_LIBS) -lhdf5 -lhdf5_hl -lgsl -lgslcblas -lm # -lsvml                                                  
SOURCES = read_hdf5.c write_hdf5.c NFW_CDF.c hod.c compute_mocks.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = read_hdf5.h
EXEC = computeHOD

.c.o:
	$(CC) $(CFLAGS) $(INCLDIRS) -c $<

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) -o $(EXEC) $(OBJECTS) $(LFLAGS)

$(OBJECTS): $(HEADERS) Makefile

clean:
	rm -f $(EXEC) *~ $(OBJECTS)


