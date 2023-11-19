//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter

#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstring>

#include "MandelbrotAdapter.h"

extern "C" {
#include "mandelbrot.h"
}

// CONSTRUCTORS --------------------------------------------------------------------------
MandelbrotAdapter::MandelbrotAdapter(
                const int width, 
                const int height, 
                const char* real, 
                const char* imag, 
                const int factor)
    : 
    m_fixedCentre(false), 
    m_width(width), 
    m_height(height), 
    m_maxiter(1000), 
    m_framecount(0), 
    m_factor(factor)
{
  setup_c();
  reset(real, imag);
  if ((real != NULL) && (imag != NULL))
  {
    m_fixedCentre = true;
  }
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
// pixels char array is allocated here, caller is responsible for freeing it.
void MandelbrotAdapter::getTextureData(unsigned char **pixels)
{
  *pixels = (unsigned char*)calloc((size_t)(m_width * m_height * 3), sizeof(unsigned char));
  
#ifdef USES_THREADS
  mandelbrot_mpfr_c(m_width, m_height, m_maxiter, true, pixels);
#else
  mandelbrot_mpfr_c(m_width, m_height, m_maxiter, false, pixels);
#endif

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
      printf("Cursor Pos %0.2f, %0.2f factor %d  screen %d\n", mouseX, mouseY, m_factor, m_width);
      mpfr_zoom_in_via_mouse(mouseX, mouseY, m_width, m_height, m_factor);
    }
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomOut()
{
    m_framecount--;
    if (m_framecount < 0) { m_framecount = 0; }
    printf("Cursor Pos %d, %d factor %d  screen %d\n", m_width/2, m_width/2, m_factor, m_width);
    mpfr_zoom_out(m_factor);
}

// PRIVATE METHODS -----------------------------------------------------------------------

