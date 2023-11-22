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

#define FIXED_FLOAT(x) std::fixed <<std::setprecision(2)<<(x)

// CONSTRUCTORS --------------------------------------------------------------------------
MandelbrotAdapter::MandelbrotAdapter(CmdOptions *options)
: m_fixedCentre(false),  m_maxiter(1000),  m_framecount(0)
{
  m_width = options->getWidth(); 
  m_height = options->getHeight(); 
  m_factor = options->getFactor();

  setup_c();
  reset(options->getReal(), options->getImag());
}

// PUBLIC METHODS -------------------------------------------------------------------------
void MandelbrotAdapter::reset(const std::string &real, const std::string &imag)
{
  if ((real != "") && (imag != "")) {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    initialize_c("-2.0", "1.0", "-1.5", "1.5", real.c_str(), imag.c_str());
    std::cout << "Using provided zoom point\n";
    m_fixedCentre = true;
  } else {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  }
  m_framecount = 1;
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::reset()
{
  std::cout << "Cursor Pos " << m_width/2 << ", " << m_height/2 << " factor ";
  std::cout << m_factor << " screen " << m_width << "\n";
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
void MandelbrotAdapter::getTextureData(ImageData *imageData)
{
  unsigned char *pixels = NULL;
  imageData->getByteArray(&pixels);

#ifdef USES_THREADS
  mandelbrot_mpfr_c(m_width, m_height, m_maxiter, true, &pixels);
#else
  mandelbrot_mpfr_c(m_width, m_height, m_maxiter, false, &pixels);
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
      std::cout << "Cursor Pos " << FIXED_FLOAT(mouseX) << ", " << FIXED_FLOAT(mouseY) << " factor " << m_factor << "  screen " << m_width << "\n";
      mpfr_zoom_in_via_mouse(mouseX, mouseY, m_width, m_height, m_factor);
    }
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomIn()
{
  m_framecount++;
  if (m_fixedCentre)
  {
    mpfr_zoom_in(m_width, m_height, m_factor);
  }
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomOut()
{
    m_framecount--;
    if (m_framecount < 0) { m_framecount = 0; }
    std::cout << "Cursor Pos " << FIXED_FLOAT(m_width/2) << ", " << FIXED_FLOAT(m_width/2) << " factor " << m_factor << " screen " << m_width << "\n";
    mpfr_zoom_out(m_factor);
}

// PRIVATE METHODS -----------------------------------------------------------------------

