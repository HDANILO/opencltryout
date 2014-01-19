#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <vglImage.h>

#pragma OPENCL EXTENSION CL_KHR_gl_sharing : enable

void check_error(cl_int error, char* name);
struct VisionCL
{
	cl_platform_id* platformId;
	cl_device_id* deviceId;
	cl_context context;
	cl_command_queue commandQueue;
};



void init_OpenCL(VisionCL* cl, int window);
void flush_OpenCL(VisionCL* cl);
void debugBuildProgram(cl_int err, cl_program program, cl_device_id device);
void vglCLUpload(VglImage* src,VisionCL cl);
void vglCLUpload(VglImage* src,VisionCL cl, bool force_copy);
void OpenCLCopy(VglImage* src, VglImage* dst, VisionCL cl, bool copyToRam);
void OpenCLCopy(VglImage* src, VglImage* dst, VisionCL cl);
void OpenCLInvert(VglImage* src, VglImage* dst, VisionCL cl, bool copyToRam);
void OpenCLInvert(VglImage* src, VglImage* dst, VisionCL cl);
void OpenCLThreshold(VglImage* src, VglImage* dst, float thresh, VisionCL cl, bool copyToRam);
void OpenCLThreshold(VglImage* src, VglImage* dst, float thresh, VisionCL cl);
void OpenCLConvolution(VglImage* src, VglImage* dst, float* convolution_window, int window_size_x, int window_size_y, VisionCL cl, bool copyToRam);
void OpenCLConvolution(VglImage* src, VglImage* dst, float* convolution_window, int window_size_x, int window_size_y, VisionCL cl);
void vglDownloadCLtoRam(VglImage* img, VisionCL cl);
