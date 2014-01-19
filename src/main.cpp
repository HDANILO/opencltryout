#include "OpenCL.h"
#include <time.h>

int main()
{
	
	VisionCL cl;
	int whnd = vglInit(1280,700);
	init_OpenCL(&cl,whnd);

	char* image_path = "../Images/lena.jpg";
	VglImage* img = vglLoadImage(image_path,1,0);
	if (img == NULL)
	{
		std::string str("cvLoadImage/File not found: ");
		str.append(image_path);
		check_error(-1,(char*) str.c_str());
	}
	VglImage* out = vglCreateImage(img);
	float convolution[3][3] = { {1.0f/13.0f,2.0f/13.0f,1.0f/13.0f},{2.0f/13.0f,1.0f/13.0f,2.0f/13.0f},{1.0f/13.0f,2.0f/13.0f,1.0f/13.0f} }; //Operador blur
	clock_t t1, t2;
	t1 = clock();
	OpenCLConvolution(img,out,*convolution,3,3,cl);
	t2 = clock();
	printf("Primeira chamada da OpenCLConvolution: %.2f s \n", (t2-t1)/(double)CLOCKS_PER_SEC);
	//Mede o tempo para 1000 convoluções 3x3 sem a criação da operação
	int p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		OpenCLConvolution(img,out,*convolution,3,3,cl,false);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 convolucoes 3x3: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);
	//Mede o tempo para 1000 convoluções 5x5 sem a criação da operação
	float convolution2[5][5] = { {1.0f/29.0f,2.0f/29.0f,3.0f/29.0f,2.0f/29.0f,1.0f/29.0f},
								 {3.0f/29.0f,2.0f/29.0f,1.0f/29.0f,2.0f/29.0f,3.0f/29.0f},
								 {1.0f/29.0f,2.0f/29.0f,3.0f/29.0f,2.0f/29.0f,1.0f/29.0f} };
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		OpenCLConvolution(img,out,*convolution,5,5,cl,false);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 convolucoes 5x5: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);
	//Mede o tempo para 1000 thresholds sem a criação da operação
	t1 = clock();
	OpenCLThreshold(img,out,0.5f,cl);
	t2 = clock();
	printf("Primeira chamada da OpenCLThreshold: %.2f s \n", (t2-t1)/(double)CLOCKS_PER_SEC);
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		OpenCLThreshold(out,out,0.5f,cl,false);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 threshold: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);
	//Mede o tempo para 1000 thresholds sem a criação da operação
	t1 = clock();
	OpenCLInvert(img,out,cl);
	t2 = clock();
	printf("Primeira chamada da OpenCLInvert: %.2f s \n", (t2-t1)/(double)CLOCKS_PER_SEC);
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		OpenCLInvert(out,out,cl,false);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 invert: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);
	VglImage* gray = vglCreateImage(img);
	gray->ipl = cvCreateImage(cvGetSize(gray->ipl),IPL_DEPTH_8U,1);
	//Mede o tempo para 1000 conversão RGB->Grayscale na CPU
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		cvCvtColor(img->ipl,gray->ipl, CV_BGR2GRAY);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 conversões BGR->Gray: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);

	//Mede o tempo para 1000 conversão RGB->RGBA na CPU
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		cvCvtColor(img->ipl,out->iplRGBA, CV_BGR2RGBA);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 conversões BGR->RGBA: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);

	//Mede o tempo para 1000 copia CPU->GPU
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		vglCLUpload(img,cl,true);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 copia CPU->GPU, inclui conversão BGR->RGBA: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);

	//Mede o tempo para 1000 copia GPU->CPU
	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		vglDownloadCLtoRam(img,cl);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 copia GPU->CPU inclui conversão RGBA->BGR: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);

	//Mede o tempo para 1000 copia GPU->GPU
	t1 = clock();
	OpenCLCopy(img,out,cl);
	t2 = clock();
	printf("Primeira chamada da OpenCLCopy: %.2f s \n", (t2-t1)/(double)CLOCKS_PER_SEC);

	p = 0;
	t1 = clock();
	while (p < 1000)
	{
		p++;
		OpenCLCopy(img,out,cl,false);
	}
	t2 = clock();
	printf("Tempo gasto para fazer 1000 copia GPU->GPU: %.2f s\n", (t2-t1)/(double)CLOCKS_PER_SEC);

	flush_OpenCL(&cl);
	system("pause");
	return 0;
}