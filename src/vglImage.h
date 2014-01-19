#include <CL/cl.h>
#include <stdio.h>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <GL/glew.h>
#include <GL/freeglut.h>


#define GLuint unsigned int
#define VGL_BLANK_CONTEXT 0
#define VGL_RAM_CONTEXT 1
#define VGL_GL_CONTEXT 2
#define VGL_CUDA_CONTEXT 4
#define VGL_CL_CONTEXT 8

#define vglIsContextValid(x) ( (x>=1) && (x<=15) )
#define vglIsContextUnique(x) ( (x>=0) && (x<=8) && (x!=3) && (x!=5) && (x!=7) && (x!=6) )
#define vglIsInContext(img, x) ( (img)->inContext & (x) || ((img)->inContext==0 && x==0))

class VglImage{
 public:
  IplImage* ipl;
  IplImage* iplRGBA;
  int       width;
  int       height;
  int       depth;
  int       dim3;
  int       nChannels;
  int       has_mipmap;
  GLuint    fbo;
  GLuint    tex;
  void*     cudaPtr;
  GLuint    cudaPbo;
  cl_mem    oclPtr;
  int       inContext;
};
int vglInit();
int vglInit(int w, int h);
int vglSetContext(VglImage* img, int context);
int vglAddContext(VglImage* img, int context);
VglImage* vglCreateImage(VglImage* img_in);
VglImage* vglCreateImage(IplImage* img_in, int dim3 /*=1*/, int has_mipmap /*=0*/);
VglImage* vglCreateImage(CvSize size, int depth, int nChannels, int dim3, int has_mipmap);
void vglUpload(VglImage* image, int swapRGB);
void vglUpload(VglImage* image);
VglImage* vglLoadImage(char* filename, int iscolor, int has_mipmap);