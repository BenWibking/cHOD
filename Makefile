HDF5_C_INCLUDE=-I /usr/local/anaconda/anaconda2/include
HDF5_C_LIBS=-L /usr/local/anaconda/anaconda2/lib -lpthread -lssl -lcrypto -lz -lm

CC = icc
CFLAGS = -Wall -O3 -ipo -openmp -std=c99
INCLDIRS = $(HDF5_C_INCLUDE) -I /usr/include -I ../
LFLAGS = -lm $(HDF5_C_LIBS) -lhdf5 -lhdf5_hl -lgsl -lgslcblas # -lsvml
SOURCES = read_hdf5.c write_hdf5.c NFW_CDF.c hod.c compute_mocks.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = read_hdf5.h
EXEC = computeHOD

.c.o:
	$(CC) $(CFLAGS) $(INCLDIRS) -c $<

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS) $(LFLAGS)

$(OBJECTS): $(HEADERS) Makefile

clean:
	rm -f $(EXEC) *~ $(OBJECTS)


