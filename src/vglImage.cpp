#include <vglImage.h>

int vglInit()
{
	return vglInit(1280,960);
}

/** Initialize GLUT and create output window with size (w, h).
  */
int vglInit(int w, int h)
{
    char* argv = new char[255];  
    int argc = 0;
    int window_id = 0;

    int glut_time = glutGet(GLUT_ELAPSED_TIME);
    printf("Glut elapsed time = %dms\n", glut_time);
    static int started = 0;
    if(!started)
    {
      glutInit(&argc, &argv);
      glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
      glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
      glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
      glutInitWindowSize(w, h);
      if (w < 10 && h < 10)
      {
        glutInitWindowPosition(-50, -50);
      }
      else
      {
        glutInitWindowPosition(-50, -50);
      }
      window_id = glutCreateWindow("Main window");
	  printf("windows handle: %d\n",(HDC)window_id);
	  //wglCreateContext((HDC)window_id);
      glEnable(GL_TEXTURE_2D); //deve ficar depois de glutCreateWindow

      GLenum err = glewInit();
	  if (err != GLEW_OK)
		printf("Erro initiating glew: %s\n",glewGetErrorString(err));
      if (GLEW_VERSION_1_3)
      {
        if(!glewIsSupported("GL_EXT_framebuffer_object"))
          fprintf(stderr, "%s: %s: glGenFramebuffersEXT not supported. The program may not work.\n", __FILE__, __FUNCTION__);

      }
      else
      {
        fprintf(stderr, "%s: %s: OpenGL 1.3 not supported. The program may not work.\n", __FILE__, __FUNCTION__);
      }
      started = 1;

      //glutDisplayFunc(display);
      //glutIdleFunc(display);
      //glutReshapeFunc(reshape);
      //glutKeyboardFunc(keyboard);
    }
    return window_id;
}

int vglSetContext(VglImage* img, int context){
  if (!vglIsContextUnique(context) && context != 0){
    fprintf(stderr, "vglSetContext: Error: context = %d is not unique\n", context);
    return 0;
  }
  #if DEBUG_VGLCONTEXT
  printf("vglSetContext: context = %d\n", context);
  #endif
  img->inContext = context;
  return img->inContext;
}

int vglAddContext(VglImage* img, int context){
  if (!vglIsContextUnique(context)){
    fprintf(stderr, "vglAddContext: Error: context = %d is not unique or invalid\n", context);
    return 0;
  }
  img->inContext = img->inContext | context;
  return img->inContext;
}

VglImage* vglCreateImage(VglImage* img_in)
{
  return vglCreateImage(cvSize(img_in->width, img_in->height), img_in->depth, img_in->nChannels, img_in->dim3, img_in->has_mipmap);
}

VglImage* vglCreateImage(IplImage* img_in, int dim3 /*=1*/, int has_mipmap /*=0*/)
{
  return vglCreateImage(cvGetSize(img_in), img_in->depth, img_in->nChannels, dim3, has_mipmap);

}

VglImage* vglCreateImage(CvSize size, int depth, int nChannels, int dim3, int has_mipmap)
{
  VglImage* vglImage = new VglImage;
  IplImage* ipl = cvCreateImage(size, depth, nChannels);
  if (!ipl){
    fprintf(stderr, "vglCreateImage: Error creating vglImage->ipl field\n");
    free(vglImage);
    return 0;
  }
  vglImage->ipl = ipl;
  vglImage->iplRGBA = cvCreateImage(cvGetSize(ipl),IPL_DEPTH_8U,4);
  vglImage->width     = ipl->width;
  vglImage->height    = ipl->height;
  vglImage->dim3      = dim3;
  vglImage->depth     = ipl->depth;
  vglImage->nChannels = ipl->nChannels;
  vglImage->has_mipmap = has_mipmap;
  vglImage->fbo = -1;
  vglImage->tex = -1;
  vglImage->cudaPtr = NULL;
  vglImage->cudaPbo = -1;
  vglImage->oclPtr = NULL;

  vglSetContext(vglImage, VGL_BLANK_CONTEXT);
  vglUpload(vglImage);

  return vglImage;
}



void vglUpload(VglImage* image, int swapRGB){
  IplImage* ipl = image->ipl;
  const int LEVEL = 0;
  GLenum glFormat;
  GLenum glTarget;
  GLenum glType;
  GLenum internalFormat;
  int depth      = image->depth;
  int nChannels  = image->nChannels;
  int dim3       = image->dim3;
  int has_mipmap = image->has_mipmap;


  //printf("vglUpload: image context = %d\n", image->inContext);
  if (!vglIsInContext(image, VGL_RAM_CONTEXT)  && 
      !vglIsInContext(image, VGL_BLANK_CONTEXT)    ){
    fprintf(stderr, "vglUpload: Error: image context = %d not in VGL_RAM_CONTEXT or VGL_BLANK_CONTEXT\n", image->inContext);
    return;
  }

  if (nChannels == 3){
    if (swapRGB){
      glFormat = GL_RGB;
    }
    else{
      glFormat = GL_BGR;
    }
  }
  else{
    glFormat = GL_LUMINANCE;
  }

  //vglInit(1, 1);
  if (image->tex == -1){
    glGenTextures(1, &image->tex);
  }

  if (dim3 <= 0){
    fprintf(stderr, "vglUpload: Image dim3 (depth) must be greater than zero. Assuming dim3 = 1\n");
    dim3 = 1;
  }

  if (dim3 == 1){
    glTarget = GL_TEXTURE_2D;
  }
  else{
    glTarget = GL_TEXTURE_3D;
  }

  //printf("w x h x d = %d x %d x %d\n", ipl->width, ipl->height, dim3);


  glBindTexture(glTarget, image->tex);
  glTexParameteri(glTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(glTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);

  // MIPMAPPING!!!!
  if (has_mipmap){
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  }
           
  switch (depth){
          case IPL_DEPTH_8U:  glType = GL_UNSIGNED_BYTE;  break; 
          case IPL_DEPTH_16U: glType = GL_UNSIGNED_SHORT; exit(1); break; 
          case IPL_DEPTH_32F: glType = GL_FLOAT; break; 
          case IPL_DEPTH_8S:  glType = GL_BYTE; exit(1);  break; 
          case IPL_DEPTH_16S: glType = GL_SHORT; exit(1); break; 
          case IPL_DEPTH_32S: glType = GL_INT; exit(1); break; 
          case IPL_DEPTH_1U:
          default: 
            fprintf(stderr, "vglUpload: Error: uploading unsupported image depth\n");
            exit(1);
  }

  if (nChannels == 3){
    if (glType == GL_FLOAT){
        internalFormat = GL_RGBA32F_ARB;
    }
    else{
        internalFormat = GL_RGBA;
    }
  }
  else{
    internalFormat = GL_LUMINANCE;
  }

  if (glTarget == GL_TEXTURE_3D){
    glTexImage3D(glTarget, LEVEL, internalFormat, 
                 ipl->width, ipl->height, depth, 0,
                 glFormat, glType, 0);
  }
  else{
    //printf("uploading with glTexImage2D (w, h) = (%d, %d)\n", ipl->width, ipl->height);
    glTexImage2D(glTarget, LEVEL, internalFormat, 
                 ipl->width, ipl->height, 0,
                 glFormat, glType, ipl->imageData);
  }

  if (image->fbo == -1){
    glGenFramebuffersEXT(1, &image->fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, image->fbo);
    if (glTarget == GL_TEXTURE_3D){
      glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT, 
				glTarget, image->tex, 0, 0);
    }
    else{
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT, 
                              glTarget, image->tex, 0);
    }
  }

  if (vglIsInContext(image, VGL_RAM_CONTEXT)){
    vglAddContext(image, VGL_GL_CONTEXT);
  }

}
void vglUpload(VglImage* image)
{
	vglUpload(image,1);
}

VglImage* vglLoadImage(char* filename, int iscolor, int has_mipmap)
{
  VglImage* vglImage = new VglImage;
  IplImage* ipl = cvLoadImage(filename, iscolor);
  if (!ipl){
    fprintf(stderr, "vglCreateImage: Error loading image from file %s\n", filename);
    free(vglImage);
    return 0;
  }
  vglImage->ipl = ipl;
  vglImage->iplRGBA = cvCreateImage(cvGetSize(ipl),IPL_DEPTH_8U,4);
  cvCvtColor(vglImage->ipl,vglImage->iplRGBA,CV_BGR2RGBA);
  vglImage->width     = ipl->width;
  vglImage->height    = ipl->height;
  vglImage->dim3      = 1;
  vglImage->depth     = ipl->depth;
  vglImage->nChannels = ipl->nChannels;
  vglImage->has_mipmap = has_mipmap;
  vglImage->fbo = -1;
  vglImage->tex = -1;
  vglImage->cudaPtr = NULL;
  vglImage->cudaPbo = -1;
  vglImage->oclPtr = NULL;

  vglSetContext(vglImage, VGL_RAM_CONTEXT);
  vglUpload(vglImage);
  if (vglImage->ipl){
    return vglImage;
  }
  else{
    free(vglImage);
    return 0;
  }

}
