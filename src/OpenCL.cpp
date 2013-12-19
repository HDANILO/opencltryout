#include <CL/cl.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include "LinkedList.h"

static void check_error(cl_int error, char* name)
{
	if (error != CL_SUCCESS)
	{
		printf("Erro %d while doing the following operation %s\n",error,name);
		system("pause");
		exit(1);
	}
}


int main(int argc, char* argv[])
{

	cl_int err;
	cl_uint num_platforms, num_devices;

	char* image_path = "../Images/lena.jpg";
	IplImage* img = cvLoadImage(image_path,0);
	if (img == NULL)
	{
		std::string str("cvLoadImage/File not found: ");
		str.append(image_path);
		check_error(-1,(char*) str.c_str());
	}

	err = clGetPlatformIDs(0, NULL, &num_platforms);
	check_error(err, (char*) "clGetPlatformIDs get number of platforms");
	cl_platform_id* platforms_id = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	err = clGetPlatformIDs(num_platforms,platforms_id,NULL);
	check_error(err, (char*) "clGetPlatformIDs get platforms id");	

	if (num_platforms == 0)
		printf("found no platform for opencl\n\n");
	else if (num_platforms > 1)
	  printf("found %p platforms for opencl\n\n", (unsigned int*) &num_platforms);
	else
		printf("found 1 platform for opencl\n\n");

	err = clGetDeviceIDs(platforms_id[0],CL_DEVICE_TYPE_GPU,0,NULL,&num_devices);
	check_error(err, (char*) "clGetDeviceIDs get number of devices");
	cl_device_id* devices_id = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devices);
	err = clGetDeviceIDs(platforms_id[0],CL_DEVICE_TYPE_GPU,num_devices,devices_id,NULL);
	check_error(err, (char*) "clGetDeviceIDs get devices id");

	//cl_context_properties context_props[] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platforms_id[0]};
	//cl_context context = clCreateContextFromType(context_props,CL_DEVICE_TYPE_ALL,NULL, NULL, &err );
	cl_context context = clCreateContext(NULL,1,&devices_id[0],NULL, NULL, &err );
	check_error(err, (char*) "clCreateContext GPU");

	cl_command_queue command_queue = NULL;
	command_queue = clCreateCommandQueue( context, devices_id[0], 0, &err );
	check_error( err, (char*) "clCreateCommandQueue" );

	cl_mem mobj_A = NULL;
	cl_image_format format;
	
	format.image_channel_order = CL_A;
	format.image_channel_data_type = CL_UNORM_INT8;

	size_t Origin[3] = { 0,0,0};
	size_t Size3d[3] = { img->width,img->height,1 };

	printf("width: %d height; %d\n",img->widthStep,img->height);
	mobj_A = clCreateImage2D( context, CL_MEM_READ_ONLY, &format,img->widthStep,img->height,NULL, NULL, &err );
	check_error( err, (char*) "clCreateImage2D in" );
	err = clEnqueueWriteImage( command_queue, mobj_A, CL_TRUE, Origin, Size3d,0,0, img->imageData, 0, NULL, NULL );
    check_error( err, (char*) "clEnqueueWriteImage" );

	cl_mem mobj_B = NULL;
	cl_image_format formatB;

	formatB.image_channel_data_type = CL_UNORM_INT8;
	formatB.image_channel_order = CL_A;
	mobj_B = clCreateImage2D( context, CL_MEM_WRITE_ONLY, &formatB,img->widthStep,img->height,NULL, NULL, &err );
	check_error( err, (char*) "clCreateImage2D out" );

	char* file_path = "CL/vglInvert.cl";
	std::ifstream file(file_path);
	if(file.fail())
	{
		std::string str("File not found: ");
		str.append(file_path);
		check_error(-1,(char*)str.c_str());
	}
	std::string prog( std::istreambuf_iterator<char>( file ), ( std::istreambuf_iterator<char>() ) );
	const char *source_str = prog.c_str();
    printf("Kernel to be compiled:\n%s\n", source_str);

	cl_program program = NULL;
	program = clCreateProgramWithSource( context, 1, (const char **) &source_str, 0, &err );
	check_error( err, (char*) "clCreateProgramWithSource" );

	err = clBuildProgram( program, 1, &devices_id[0], NULL, NULL, NULL );
	check_error( err, (char*) "clBuildProgram" );

	cl_kernel kernel = NULL;
	kernel = clCreateKernel( program, "invert", &err ); 
	check_error( err, (char*) "clCreateKernel" );

	err = clSetKernelArg( kernel, 0, sizeof( cl_mem ), (void *) &mobj_A );
	check_error( err, (char*) "clSetKernelArg" );
	err = clSetKernelArg( kernel, 1, sizeof( cl_mem ), (void *) &mobj_B );
	check_error( err, (char*) "clSetKernelArg" );

	size_t worksize[] = { img->widthStep, img->height, 1 };
	clEnqueueNDRangeKernel( command_queue, kernel, 2, NULL, worksize, 0, 0, 0, 0 );
	check_error( err, (char*) "clEnqueueNDRangeKernel" );

	uchar* output = (uchar*)malloc(img->widthStep*img->height*img->nChannels);
	err = clEnqueueReadImage( command_queue, mobj_B, CL_TRUE, Origin, Size3d,0,0, output, 0, NULL, NULL );
	check_error( err, (char*) "clEnqueueReadImage" );
	
	cv::Mat img_output = cv::Mat(img->height,img->widthStep,CV_8U,output);
	cv::namedWindow("teste");
	cv::imshow("teste",img_output);
	cv::waitKey(5000);

	err = clFlush( command_queue );
	check_error(err, (char*) "clFlush command_queue");
	err = clFinish( command_queue );
	check_error(err, (char*) "clFinish command_queue");
	err = clReleaseKernel( kernel );
	check_error(err, (char*) "clReleaseKernel kernel");
	err = clReleaseProgram( program );
	check_error(err, (char*) "clReleaseProgram program");
	err = clReleaseMemObject( mobj_A );
	check_error(err, (char*) "clReleaseMemObject mobj_A");
	err = clReleaseMemObject( mobj_B );
	check_error(err, (char*) "clReleaseMemObject mobj_A");
	err = clReleaseCommandQueue( command_queue );
	check_error(err, (char*) "clReleaseCommandQueue command_queue");
	err = clReleaseContext( context );
	check_error(err, (char*) "clReleaseContext context");

	free(output);
	free(platforms_id);
	system("pause");
	return 0;
}

