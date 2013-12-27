#include "OpenCL.h"
//#include "LinkedList.h"

void check_error(cl_int error, char* name)
{
	if (error != CL_SUCCESS)
	{
		printf("Erro %d while doing the following operation %s\n",error,name);
		system("pause");
		exit(1);
	}
}

void init_OpenCL(VisionCL* cl)
{
	cl_int err;
	cl_uint num_platforms, num_devices;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	check_error(err, (char*) "clGetPlatformIDs get number of platforms");
	cl->platformId = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	err = clGetPlatformIDs(num_platforms,cl->platformId,NULL);
	check_error(err, (char*) "clGetPlatformIDs get platforms id");	

	if (num_platforms == 0)
		printf("found no platform for opencl\n\n");
	else if (num_platforms > 1)
	  printf("found %p platforms for opencl\n\n", (unsigned int*) &num_platforms);
	else
		printf("found 1 platform for opencl\n\n");

	err = clGetDeviceIDs(*cl->platformId,CL_DEVICE_TYPE_GPU,0,NULL,&num_devices);
	check_error(err, (char*) "clGetDeviceIDs get number of devices");
	cl->deviceId = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devices);
	err = clGetDeviceIDs(*cl->platformId,CL_DEVICE_TYPE_GPU,num_devices,cl->deviceId,NULL);
	check_error(err, (char*) "clGetDeviceIDs get devices id");

	cl->context = clCreateContext(NULL,1,cl->deviceId,NULL, NULL, &err );
	check_error(err, (char*) "clCreateContext GPU");

	cl->commandQueue = clCreateCommandQueue( cl->context, *cl->deviceId, 0, &err );
	check_error( err, (char*) "clCreateCommandQueue" );

}

void flush_OpenCL(VisionCL* cl)
{
	cl_int err;
	err = clFlush( cl->commandQueue );
	check_error(err, (char*) "clFlush command_queue");
	err = clFinish( cl->commandQueue );
	check_error(err, (char*) "clFinish command_queue");
	err = clReleaseCommandQueue( cl->commandQueue );
	check_error(err, (char*) "clReleaseCommandQueue command_queue");
	err = clReleaseContext( cl->context );
	check_error(err, (char*) "clReleaseContext context");

}

void debugBuildProgram(cl_int err, cl_program program, cl_device_id device)
{
	if (err != CL_SUCCESS)
	{
		printf("Error building program\n");

		char buffer[4096];
		size_t length;
		clGetProgramBuildInfo(
			program, // valid program object
			device, // valid device_id that executable was built
			CL_PROGRAM_BUILD_LOG, // indicate to retrieve build log
			sizeof(buffer), // size of the buffer to write log to
			buffer, // the actual buffer to write log to
			&length // the actual size in bytes of data copied to buffer
		);
		printf("%s\n",buffer);
		system("pause");
		exit(1);
	}
}

void OpenCLInvert(IplImage* src, IplImage* dst, VisionCL cl)
{
	cl_int err;
	cl_mem mobj_A = NULL;
	cl_image_format format;
	
	format.image_channel_order = CL_A;
	format.image_channel_data_type = CL_UNORM_INT8;

	size_t Origin[3] = { 0,0,0};
	size_t Size3d[3] = { src->width,src->height,1 };

	mobj_A = clCreateImage2D( cl.context, CL_MEM_READ_ONLY, &format,src->widthStep,src->height,NULL, NULL, &err );
	check_error( err, (char*) "clCreateImage2D in" );
	err = clEnqueueWriteImage( cl.commandQueue, mobj_A, CL_TRUE, Origin, Size3d,0,0, src->imageData, 0, NULL, NULL );
    check_error( err, (char*) "clEnqueueWriteImage" );

	cl_mem mobj_B = NULL;
	cl_image_format formatB;

	formatB.image_channel_data_type = CL_UNORM_INT8;
	formatB.image_channel_order = CL_A;
	mobj_B = clCreateImage2D( cl.context, CL_MEM_WRITE_ONLY, &formatB,src->widthStep,src->height,NULL, NULL, &err );
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
#ifdef __DEBUG__
    printf("Kernel to be compiled:\n%s\n", source_str);
#endif

	cl_program program = NULL;
	program = clCreateProgramWithSource( cl.context, 1, (const char **) &source_str, 0, &err );
	check_error( err, (char*) "clCreateProgramWithSource" );

	err = clBuildProgram( program, 1, cl.deviceId, NULL, NULL, NULL );
	debugBuildProgram(err,program,*cl.deviceId);
	

	cl_kernel kernel = NULL;
	kernel = clCreateKernel( program, "invert", &err ); 
	check_error( err, (char*) "clCreateKernel" );

	err = clSetKernelArg( kernel, 0, sizeof( cl_mem ), (void *) &mobj_A );
	check_error( err, (char*) "clSetKernelArg A" );
	err = clSetKernelArg( kernel, 1, sizeof( cl_mem ), (void *) &mobj_B );
	check_error( err, (char*) "clSetKernelArg B" );

	size_t worksize[] = { src->widthStep, src->height, 1 };
	clEnqueueNDRangeKernel( cl.commandQueue, kernel, 2, NULL, worksize, 0, 0, 0, 0 );
	check_error( err, (char*) "clEnqueueNDRangeKernel" );

	uchar* auxdst = (uchar*)malloc(src->widthStep*src->height*src->nChannels);
	err = clEnqueueReadImage( cl.commandQueue, mobj_B, CL_TRUE, Origin, Size3d,0,0, auxdst, 0, NULL, NULL );
	check_error( err, (char*) "clEnqueueReadImage" );
	
	dst->imageData = (char*) auxdst;

	err = clReleaseKernel( kernel );
	check_error(err, (char*) "clReleaseKernel kernel");
	err = clReleaseProgram( program );
	check_error(err, (char*) "clReleaseProgram program");
	err = clReleaseMemObject( mobj_A );
	check_error(err, (char*) "clReleaseMemObject mobj_A");
	err = clReleaseMemObject( mobj_B );
	check_error(err, (char*) "clReleaseMemObject mobj_B");

}

void OpenCLThreshold(IplImage* src, IplImage* dst, float thresh, VisionCL cl)
{
	cl_int err;
	cl_mem mobj_A = NULL;
	cl_image_format format;
	
	format.image_channel_order = CL_A;
	format.image_channel_data_type = CL_UNORM_INT8;

	size_t Origin[3] = { 0,0,0};
	size_t Size3d[3] = { src->width,src->height,1 };

	mobj_A = clCreateImage2D( cl.context, CL_MEM_READ_ONLY, &format,src->widthStep,src->height,NULL, NULL, &err );
	check_error( err, (char*) "clCreateImage2D in" );
	err = clEnqueueWriteImage( cl.commandQueue, mobj_A, CL_TRUE, Origin, Size3d,0,0, src->imageData, 0, NULL, NULL );
    check_error( err, (char*) "clEnqueueWriteImage" );

	cl_mem mobj_B = NULL;
	cl_image_format formatB;

	formatB.image_channel_data_type = CL_UNORM_INT8;
	formatB.image_channel_order = CL_A;
	mobj_B = clCreateImage2D( cl.context, CL_MEM_WRITE_ONLY, &formatB,src->widthStep,src->height,NULL, NULL, &err );
	check_error( err, (char*) "clCreateImage2D out" );

	cl_mem mobj_C = NULL;
	mobj_C = clCreateBuffer(cl.context,CL_MEM_READ_ONLY,sizeof(thresh),NULL,&err);
	check_error( err, (char*) "clCreateBuffer thresh" );
	err = clEnqueueWriteBuffer(cl.commandQueue,mobj_C,CL_TRUE,NULL,sizeof(thresh),&thresh,NULL,NULL,NULL);
	check_error( err, (char*) "clEnqueueWriteBuffer thresh" );

	char* file_path = "CL/vglThreshold.cl";
	std::ifstream file(file_path);
	if(file.fail())
	{
		std::string str("File not found: ");
		str.append(file_path);
		check_error(-1,(char*)str.c_str());
	}
	std::string prog( std::istreambuf_iterator<char>( file ), ( std::istreambuf_iterator<char>() ) );
	const char *source_str = prog.c_str();
#ifdef __DEBUG__
    printf("Kernel to be compiled:\n%s\n", source_str);
#endif

	cl_program program = NULL;
	program = clCreateProgramWithSource( cl.context, 1, (const char **) &source_str, 0, &err );
	check_error( err, (char*) "clCreateProgramWithSource" );

	err = clBuildProgram( program, 1, cl.deviceId, NULL, NULL, NULL );
	debugBuildProgram(err,program,*cl.deviceId);

	cl_kernel kernel = NULL;
	kernel = clCreateKernel( program, "threshold", &err ); 
	check_error( err, (char*) "clCreateKernel" );

	err = clSetKernelArg( kernel, 0, sizeof( cl_mem ), (void *) &mobj_A );
	check_error( err, (char*) "clSetKernelArg A" );
	err = clSetKernelArg( kernel, 1, sizeof( cl_mem ), (void *) &mobj_B );
	check_error( err, (char*) "clSetKernelArg B" );
	err = clSetKernelArg( kernel, 2, sizeof( cl_mem ), (float *) &mobj_C );
	check_error( err, (char*) "clSetKernelArg C" );

	size_t worksize[] = { src->widthStep, src->height, 1 };
	clEnqueueNDRangeKernel( cl.commandQueue, kernel, 2, NULL, worksize, 0, 0, 0, 0 );
	check_error( err, (char*) "clEnqueueNDRangeKernel" );

	uchar* auxdst = (uchar*)malloc(src->widthStep*src->height*src->nChannels);
	err = clEnqueueReadImage( cl.commandQueue, mobj_B, CL_TRUE, Origin, Size3d,0,0, auxdst, 0, NULL, NULL );
	check_error( err, (char*) "clEnqueueReadImage" );
	
	dst->imageData = (char*) auxdst;

	err = clReleaseKernel( kernel );
	check_error(err, (char*) "clReleaseKernel kernel");
	err = clReleaseProgram( program );
	check_error(err, (char*) "clReleaseProgram program");
	err = clReleaseMemObject( mobj_A );
	check_error(err, (char*) "clReleaseMemObject mobj_A");
	err = clReleaseMemObject( mobj_B );
	check_error(err, (char*) "clReleaseMemObject mobj_B");
	err = clReleaseMemObject( mobj_C );
	check_error(err, (char*) "clReleaseMemObject mobj_C");
}

