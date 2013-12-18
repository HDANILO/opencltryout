


OPENCL_INCLUDEDIR1 = /usr/local/cuda/include
OPENCL_INCLUDEDIR2 = /home/ddantas/src/AMD-APP-SDK-v2.9-RC-lnx64/include/


all:
	g++ src/OpenCL.cpp -o bin/OpenCLTryOut \
	-l OpenCL -I $(OPENCL_INCLUDEDIR1) -I $(OPENCL_INCLUDEDIR2)




