#include "OpenCL.h"

int main()
{
	VisionCL cl;
	init_OpenCL(&cl);

	char* image_path = "../Images/lena.jpg";
	IplImage* img = cvLoadImage(image_path,0);
	if (img == NULL)
	{
		std::string str("cvLoadImage/File not found: ");
		str.append(image_path);
		check_error(-1,(char*) str.c_str());
	}

	IplImage* out = cvCreateImage(cvSize(img->widthStep, img->height), IPL_DEPTH_8U, 1);;
	float convolution[3][3] = { {1.0f/13.0f,2.0f/13.0f,1.0f/13.0f},{2.0f/13.0f,1.0f/13.0f,2.0f/13.0f},{1.0f/13.0f,2.0f/13.0f,1.0f/13.0f} }; //Operador blur

	//aplica várias vezes pro resultado ser evidente
	OpenCLConvolution(img,out,*convolution,3,3,cl);
	OpenCLConvolution(out,out,*convolution,3,3,cl);
	OpenCLConvolution(out,out,*convolution,3,3,cl);
	OpenCLConvolution(out,out,*convolution,3,3,cl);
	OpenCLConvolution(out,out,*convolution,3,3,cl);
	cvSaveImage("lenaout.png",out);

	flush_OpenCL(&cl);
	system("pause");
	return 0;
}