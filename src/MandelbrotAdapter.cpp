//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter

#include <string>
#include <iostream>
#include <cstring>

#include <SOIL/SOIL.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MandelbrotAdapter.h"

extern "C" {
#include "mandelbrot.h"
}

// CONSTRUCTORS --------------------------------------------------------------------------
MandelbrotAdapter::MandelbrotAdapter(const int width, const int height, const char* real, const char* imag, const int factor)
    : m_fixedCentre(false), m_width(width), m_height(height), m_maxiter(1000), m_framecount(0), m_factor(factor)
{
  setup_c();
  reset(real, imag);
  if ((real != NULL) && (imag != NULL))
  {
    m_fixedCentre = true;
  }
}
// ---------------------------------------------------------------------------------------
/*MandelbrotAdapter::MandelbrotAdapter(const int width, const int height)
    : m_width(width), m_height(height), m_maxiter(1000), m_framecount(0)
{
  setup_c();
  reset(NULL, NULL);
  useMouse();
}*/
// ---------------------------------------------------------------------------------------
MandelbrotAdapter::~MandelbrotAdapter()
{
}

// PUBLIC METHODS -------------------------------------------------------------------------
void MandelbrotAdapter::reset(const char* real, const char* imag)
{
  if ((real != NULL) && (imag != NULL)) {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    initialize_c("-2.0", "1.0", "-1.5", "1.5", real, imag);
    std::cout << "Using provided zoom point\n";
  } else {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  }
  m_framecount = 1;
}
void MandelbrotAdapter::reset()
{
  printf("Cursor Pos %d, %d factor %d  screen %d\n", m_width/2, m_height/2, m_factor, m_width);
  //            Xs      Xe     Ys      Ye     Cx      Cy
  initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  m_framecount = 1;
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::cleanUp() 
{ 
  free_mpfr_mem_c(); 
}
// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::createTextureFromData()
{
  unsigned int *pixels = NULL;
  pixels = (unsigned int*)calloc((size_t)(m_width * m_height * 3), sizeof(unsigned int));

#ifdef USES_THREADS
  mandelbrot_mpfr_thread_c(m_width, m_height, m_maxiter, &pixels);
#else
  mandelbrot_mpfr_c(m_width, m_height, m_maxiter, &pixels);
#endif
  
  writeImage(&pixels);
  unsigned char* image = NULL;
  image = (unsigned char*)calloc((size_t)(m_width * m_height * 3), sizeof(unsigned char));
  for(unsigned int i=0; i< (m_width * m_height * 3); i++)
  {
    image[i] = (pixels)[i];
  }

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(image);
  free(pixels);
}
// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomIn(const double mouseX, const double mouseY)
{
    m_framecount++;
    if (m_fixedCentre)
    {
      mpfr_zoom_in(m_width, m_height, m_factor);
    }
    else
    {
      printf("Cursor Pos %f, %f factor %d  screen %d\n", mouseX, mouseY, m_factor, m_width);
      mpfr_zoom_in_via_mouse(mouseX, mouseY, m_width, m_height, m_factor);
    }
}
// ---------------------------------------------------------------------------------------
/*void MandelbrotAdapter::zoomIn(const int factor)
{
    m_framecount++;
    mpfr_zoom_in(m_width, m_height, factor);
}*/
// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomOut()
{
    m_framecount--;
    if (m_framecount < 0) { m_framecount = 0; }
    printf("Cursor Pos %f, %f factor %d  screen %d\n", mouseX, mouseY, m_factor, m_width);
    mpfr_zoom_out(m_factor);
}
// PRIVATE METHODS -----------------------------------------------------------------------
void MandelbrotAdapter::writeImage(unsigned int **rgb)
{
  unsigned char* image = NULL;
  char* filename = NULL;
  char* idx = NULL;
  
  filename = (char*)calloc((size_t)100, sizeof(char));
  idx = (char*)calloc((size_t)10, sizeof(char));
  image = (unsigned char*)calloc((size_t)(m_width * m_height * 3), sizeof(unsigned char));
  for(unsigned int i=0; i< (m_width * m_height * 3); i++)
  {
    image[i] = (*rgb)[i];
  }
  strcpy(filename, "images/image");
  sprintf(idx, "%04d", m_framecount);
  strcat(filename, idx);
  strcat(filename, ".bmp");
  (void)SOIL_save_image(filename, SOIL_SAVE_TYPE_BMP, m_width, m_height, 3, image);
  printf("Soil save image = %s\n", filename);
  
  free(filename);
  free(idx);
  free(image);
}
  