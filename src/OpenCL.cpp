#include <CL/cl.hpp>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include "LinkedList.h"

static void check_error(cl_int error, char* name)
{
	if (error != CL_SUCCESS)
	{
		printf("Erro %d while doing the following operation %s\n",error,name);
		system("PAUSE");
		exit(1);
	}
}


int main(int argc, char* argv[])
{

	cl_int err;
	cl_uint num_platforms, num_devices;
	char input[12] = { 'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '\0' };
	err = clGetPlatformIDs(NULL,NULL,&num_platforms);
	check_error(err,"clGetPlatformIDs get number of platforms");
	cl_platform_id* platforms_id = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	err = clGetPlatformIDs(num_platforms,platforms_id,NULL);
	check_error(err,"clGetPlatformIDs get platforms id");	

	if (num_platforms == 0)
		printf("found no platform for opencl\n\n");
	else if (num_platforms > 1)
		printf("found %d platforms for opencl\n\n",&num_platforms);
	else
		printf("found 1 platform for opencl\n\n");

	err = clGetDeviceIDs(platforms_id[0],CL_DEVICE_TYPE_GPU,0,NULL,&num_devices);
	check_error(err,"clGetDeviceIDs get number of devices");
	cl_device_id* devices_id = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devices);
	err = clGetDeviceIDs(platforms_id[0],CL_DEVICE_TYPE_GPU,num_devices,devices_id,NULL);
	check_error(err,"clGetDeviceIDs get devices id");

	//cl_context_properties context_props[] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platforms_id[0]};
	//cl_context context = clCreateContextFromType(context_props,CL_DEVICE_TYPE_ALL,NULL, NULL, &err );
	cl_context context = clCreateContext(NULL,1,&devices_id[0],NULL, NULL, &err );
	check_error(err,"clCreateContextFromType GPU");

	cl_command_queue command_queue = NULL;
	command_queue = clCreateCommandQueue( context, devices_id[0], 0, &err );
	check_error( err, "clCreateCommandQueue" );

	cl_mem mobj_A = NULL;
	mobj_A = clCreateBuffer( context, CL_MEM_READ_WRITE, sizeof(input), NULL, &err );
	check_error( err, "clCreateBuffer" );
	err = clEnqueueWriteBuffer( command_queue, mobj_A, CL_TRUE, 0, sizeof(input), input, 0, NULL, NULL );
	check_error( err, "clEnqueueWriteBuffer" );

	std::ifstream file("helloworld.cl");
	std::string prog( std::istreambuf_iterator<char>( file ), ( std::istreambuf_iterator<char>() ) );
	const char *source_str = prog.c_str();

	cl_program program = NULL;
	program = clCreateProgramWithSource( context, 1, (const char **) &source_str, 0, &err );
	check_error( err, "clCreateProgramWithSource" );

	err = clBuildProgram( program, 1, &devices_id[0], NULL, NULL, NULL );
	check_error( err, "clBuildProgram" );

	cl_kernel kernel = NULL;
	kernel = clCreateKernel( program, "copy", &err ); 
	check_error( err, "clCreateKernel" );

	err = clSetKernelArg( kernel, 0, sizeof( cl_mem ), (void *) &mobj_A );
	check_error( err, "clSetKernelArg" );

	size_t worksize[] = { sizeof(input)/sizeof(input[0]), 1, 1 };
	clEnqueueNDRangeKernel( command_queue, kernel, 1, NULL, worksize, 0, 0, 0, 0 );
	check_error( err, "clEnqueueNDRangeKernel" );

	char output[12];
	err = clEnqueueReadBuffer( command_queue, mobj_A, CL_TRUE, 0, sizeof(output), output, 0, NULL, NULL );
	check_error( err, "clEnqueueReadBuffer" );
	printf("O input de saida eh %s\n",output);

	err = clFlush( command_queue );
	check_error(err, "clFlush command_queue");
	err = clFinish( command_queue );
	check_error(err, "clFinish command_queue");
	err = clReleaseKernel( kernel );
	check_error(err, "clReleaseKernel kernel");
	err = clReleaseProgram( program );
	check_error(err, "clReleaseProgram program");
	err = clReleaseMemObject( mobj_A );
	check_error(err, "clReleaseMemObject mobj_A");
	err = clReleaseCommandQueue( command_queue );
	check_error(err, "clReleaseCommandQueue command_queue");
	err = clReleaseContext( context );
	check_error(err, "clReleaseContext context");

	printf("aperte uma tecla para finalizar...");
	getchar();

	free(input);
	free(output);
	free(platforms_id);
	return 0;
}

