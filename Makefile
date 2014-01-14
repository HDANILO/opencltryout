


OPENCL_INCLUDEDIR1 = /usr/local/cuda/include
OPENCL_INCLUDEDIR2 = ~/src/AMD-APP-SDK-v2.9-RC-lnx64/include/

OPENCV_INCLUDEDIR1 =  ~/bin/opencv/include
OPENCV_INCLUDEDIR2 =  /usr/local/opencv/include
OPENCV_LIBDIR1     =  ~/bin/opencv/lib
OPENCV_LIBDIR2     =  /usr/local/opencv/lib


all:
	g++ src/*.cpp -o bin/OpenCLTryOut \
	-l OpenCL -I $(OPENCL_INCLUDEDIR1) -I $(OPENCL_INCLUDEDIR2) \
	-l opencv_highgui -l opencv_core  \
	-I $(OPENCV_INCLUDEDIR1) -I $(OPENCV_INCLUDEDIR2) \
	-L $(OPENCV_LIBDIR1) -L $(OPENCV_LIBDIR2) 





