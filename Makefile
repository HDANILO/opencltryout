


OPENCL_INCLUDEDIR1 = /usr/local/cuda/include
OPENCL_INCLUDEDIR2 = ~/src/AMD-APP-SDK-v2.9-RC-lnx64/include/

OPENCV_INCLUDEDIR =  ~/bin/opencv/include
OPENCV_LIBDIR     =  ~/bin/opencv/lib


all:
	g++ src/OpenCL.cpp -o bin/OpenCLTryOut \
	-l OpenCL -I $(OPENCL_INCLUDEDIR1) -I $(OPENCL_INCLUDEDIR2) \
	-l opencv_highgui -I $(OPENCV_INCLUDEDIR) -L $(OPENCV_LIBDIR)




