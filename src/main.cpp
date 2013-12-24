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
	OpenCLThreshold(img,out,0.5f,cl);
	OpenCLInvert(out,out,cl);

	cvShowImage("saida",out);
	cvWaitKey(5000);

	flush_OpenCL(&cl);
	system("pause");
	return 0;
}