# Simple makefile to build a square.c in the labs.
#
# Path to the OpenCL libraries.  In this case AMD SDK
# is being used, as system uses AMD graphics card.
OPENCL        := /opt/AMDAPP

# C flags with strictest warnings.
CFLAGS        += -O3 -Wall -Wextra -I$(OPENCL)/include -D_GNU_SOURCE

# Linker flags.
LDFLAGS += -L$(OPENCL)/lib/x86_64  -l OpenCL -lrt

all: square_ported matmult_ported
#matmul totient

# Build a binary from C source.
KGF_API_OpenCL.o: KGF_API_OpenCL.cpp KGF_API_OpenCL.h
	$(CXX) $(CFLAGS) -c KGF_API_OpenCL.cpp

KGF_OpenCL_easy.o: KGF_OpenCL_easy.cpp KGF_API_OpenCL.h KGF_OpenCL_easy.h KGF_API_OpenCL.o
	$(CXX) $(CFLAGS) KGF_API_OpenCL.o -c KGF_OpenCL_easy.cpp

square_ported: square_ported.cpp KGF_OpenCL_easy.o KGF_API_OpenCL.o KGF_API_OpenCL.h KGF_OpenCL_easy.h
	$(CXX) $(CFLAGS) $(LDFLAGS) KGF_OpenCL_easy.o KGF_API_OpenCL.o square_ported.cpp -o square_ported

matmult_ported: matmult_ported.cpp KGF_OpenCL_easy.o KGF_API_OpenCL.o KGF_API_OpenCL.h KGF_OpenCL_easy.h
	$(CXX) $(CFLAGS) $(LDFLAGS) KGF_OpenCL_easy.o KGF_API_OpenCL.o matmult_ported.cpp -o matmult_ported


# Remove the binary.
clean:
	$(RM) square_ported matmult_ported KGF_API_OpenCL.o KGF_OpenCL_easy.o

