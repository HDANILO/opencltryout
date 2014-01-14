#include <CL/cl.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>

void check_error(cl_int error, char* name);
struct VisionCL
{
	cl_platform_id* platformId;
	cl_device_id* deviceId;
	cl_context context;
	cl_command_queue commandQueue;
};

void init_OpenCL(VisionCL* cl);
void flush_OpenCL(VisionCL* cl);
void debugBuildProgram(cl_int err, cl_program program, cl_device_id device);
void OpenCLInvert(IplImage* src, IplImage* dst, VisionCL cl);
void OpenCLThreshold(IplImage* src, IplImage* dst, float thresh, VisionCL cl);
void OpenCLConvolution(IplImage* src, IplImage* dst, float* convolution_window, int window_size_x, int window_size_y, VisionCL cl);