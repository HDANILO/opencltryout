#include <CL/cl.hpp>

static void check_error(cl_int error, char* name)
{
	if (error != CL_SUCCESS)
	{
		printf("Erro %d while doing the following operation %s\n",error,name);
		system("PAUSE");
		exit(1);
	}
}

class opencl_devices
{
public:
	cl_device_id id;
	char* name;
	char* vendor;
	char* extensions;
	cl_ulong global_mem_size;
	cl_uint address_bits;
	cl_bool available;
	cl_bool compiler_available;

	opencl_devices()
	{
		id = NULL;
	}

	opencl_devices(cl_device_id id)
	{
		this->id = id;
		this->getDevicesInfo();
	}
	
	void printInfo()
	{
		printf("Device ID: %d\n",id);
		printf("Device Name: %s\n",name);
		printf("Device Vendor: %s\n",vendor);
		printf("Device Extensions: %s\n",extensions);
		printf("Device Global Memory Size: %d\n",global_mem_size);
		printf("Device Address Space: %d\n",address_bits);
		if (available)
			printf("Device Available: True\n");
		else
			printf("Device Available: False\n");
		if (compiler_available)
			printf("Device Compiler Available: True\n");
		else
			printf("Device Compiler Available: False\n");
	}

	
	
private:
	void getDevicesInfo()
	{
		cl_int err_check;
		name = new char[128];
		vendor = new char[128];
		extensions = new char[4096];
		global_mem_size = 0;
		address_bits = 0;
		available = 0;
		compiler_available = 0;
		err_check = clGetDeviceInfo(id,CL_DEVICE_NAME,128,name, NULL);
		check_error(err_check,"clGetDeviceInfo Name");
		err_check = clGetDeviceInfo(id,CL_DEVICE_VENDOR,128,vendor, NULL);
		check_error(err_check,"clGetDeviceInfo Vendor");
		err_check = clGetDeviceInfo(id,CL_DEVICE_EXTENSIONS,4096,extensions, NULL);
		check_error(err_check,"clGetDeviceInfo Exntesions");
		err_check = clGetDeviceInfo(id,CL_DEVICE_GLOBAL_MEM_SIZE,sizeof(global_mem_size),&global_mem_size, NULL);
		check_error(err_check,"clGetDeviceInfo Global Size");
		err_check = clGetDeviceInfo(id,CL_DEVICE_ADDRESS_BITS,sizeof(address_bits),&address_bits, NULL);
		check_error(err_check,"clGetDeviceInfo Address Size");
		err_check = clGetDeviceInfo(id,CL_DEVICE_AVAILABLE,sizeof(available),&available, NULL);
		check_error(err_check,"clGetDeviceInfo Available");
		err_check = clGetDeviceInfo(id,CL_DEVICE_COMPILER_AVAILABLE,sizeof(compiler_available),&compiler_available, NULL);
		check_error(err_check,"clGetDeviceInfo Compiler Available");
	}
};

class opencl_platform
{
	public:

	cl_platform_id id;
	opencl_devices* devices;
	cl_uint num_devices;
	char* name;
	char* vendor;
	char* version;
	char* extensions;

	opencl_platform(cl_platform_id id)
	{
		this->id = id;
		this->getPlatformInfo();
		this->getDevices();
	}

	void printInfo()
	{
		printf("Platform ID: %d\n",id);
		printf("Platform name: %s\n",name);
		printf("Platform vendor: %s\n",vendor);
		printf("Platform version: %s\n",version);
		printf("Platform Extensions: %s\n",extensions);
		printf("Platform number of devices: %d\n\n",num_devices);
		for(int i = 0; i < num_devices; i++)
		{
			printf("Device number %d\n",i+1);
			devices[i].printInfo();
			printf("\n");
		}
	}


	static cl_uint getPlatformNumber()
	{
		static cl_uint num_platforms = 0;
		if (num_platforms == 0)
		{
			cl_int err_check;
			err_check = clGetPlatformIDs(NULL,NULL,&num_platforms);
			check_error(err_check,"clGetPlatformIDs get number of platforms");
			return num_platforms;
		}
		else
			return num_platforms;
	}

	static cl_platform_id* getAllPlatformId(cl_uint num_platforms)
	{
		cl_int err_check;
		cl_platform_id* all_platform_id = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
		err_check = clGetPlatformIDs(num_platforms,all_platform_id,NULL);
		check_error(err_check,"clGetPlatformIDs get platforms id");
		return all_platform_id;
	}

	private:
	void getPlatformInfo()
	{
		cl_int err_check;
		name = new char[128];
		vendor = new char[128];
		extensions = new char[256];
		version = new char[128];
		err_check = clGetPlatformInfo(id,CL_PLATFORM_NAME,128,name, NULL);
		check_error(err_check,"clGetPlatformInfo Name");
		err_check = clGetPlatformInfo(id,CL_PLATFORM_VENDOR,128,vendor, NULL);
		check_error(err_check,"clGetPlatformInfo Vendor");
		err_check = clGetPlatformInfo(id,CL_PLATFORM_EXTENSIONS,256,extensions, NULL);
		check_error(err_check,"clGetPlatformInfo Extensions");
		err_check = clGetPlatformInfo(id,CL_PLATFORM_VERSION,128,version, NULL);
		check_error(err_check,"clGetPlatformInfo Version");
	}

	void getDevices()
	{
		cl_int err_check;
		err_check = clGetDeviceIDs(id,CL_DEVICE_TYPE_ALL,0,NULL,&num_devices);
		check_error(err_check,"clGetDeviceIDs get number of devices");
		cl_device_id* devices_id = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devices);
		err_check = clGetDeviceIDs(id,CL_DEVICE_TYPE_ALL,num_devices,devices_id,NULL);
		check_error(err_check,"clGetDeviceIDs get devices id");
		devices = new opencl_devices[num_devices];
		for(int i = 0; i < num_devices; i++)
		{
			devices[i] = *(new opencl_devices(devices_id[i]));
		}
	}

};

int main(int argc, char* argv[])
{
	cl_uint num_platforms, num_devices;
	cl_platform_id* platforms_id = NULL;
	num_platforms = opencl_platform::getPlatformNumber();
	platforms_id = opencl_platform::getAllPlatformId(num_platforms);
	if (num_platforms == 0)
		printf("found no platform for opencl\n\n");
	else if (num_platforms > 1)
		printf("found %d platforms for opencl\n\n",&num_platforms);
	else
		printf("found 1 platform for opencl\n\n");

	opencl_platform* platforms = (opencl_platform*)malloc(sizeof(opencl_platform)*num_platforms);

	for(int i = 0; i < num_platforms; i++)
	{
		platforms[i] = *new opencl_platform(platforms_id[i]);
		platforms[i].printInfo();
	}

	system("PAUSE");

	free(platforms);
	free(platforms_id);
	return 0;
}

