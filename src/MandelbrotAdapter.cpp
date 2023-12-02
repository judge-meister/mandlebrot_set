//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter

#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <cstring>

#include "MandelbrotAdapter.h"

//extern "C" {
//#include "mandelbrot.h"
//}

#define FIXED_FLOAT(x) std::fixed <<std::setprecision(2)<<(x)

// CONSTRUCTORS --------------------------------------------------------------------------
MandelbrotAdapter::MandelbrotAdapter(CmdOptions *options)
: m_fixedCentre(false),  m_maxiter(1000),  m_framecount(0)
{
  m_width = options->getWidth(); 
  m_height = options->getHeight(); 
  m_factor = options->getFactor();

  //setup_c();
  reset(options->getReal(), options->getImag());
  mpfr = new MandelbrotMpfr(0, m_maxiter, 0); //precision. maxiter, zoom_factor
  
}

// PUBLIC METHODS -------------------------------------------------------------------------
void MandelbrotAdapter::reset(const std::string &real, const std::string &imag)
{
  if ((real != "") && (imag != "")) {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    mpfr->initialize_c("-2.0", "1.0", "-1.5", "1.5", real.c_str(), imag.c_str());
    std::cout << "Using provided zoom point\n";
    m_fixedCentre = true;
  } else {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    mpfr->initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  }
  m_framecount = 1;
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::reset()
{
  std::cout << "Cursor Pos " << m_width/2 << ", " << m_height/2 << " factor ";
  std::cout << m_factor << " screen " << m_width << "\n";
  //            Xs      Xe     Ys      Ye     Cx      Cy
  mpfr->initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  m_framecount = 1;
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::cleanUp() 
{ 
  mpfr->free_mpfr_mem_c(); 
}

// ---------------------------------------------------------------------------------------
// pixels char array is allocated here, caller is responsible for freeing it.
void MandelbrotAdapter::getTextureData(ImageData *imageData)
{
  unsigned char *pixels = NULL;
  imageData->getByteArray(&pixels);

  mpfr->mandelbrot_mpfr_c(m_width, m_height, &pixels);
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomIn(const double mouseX, const double mouseY)
{
    m_framecount++;
    if (m_fixedCentre)
    {
      mpfr->zoom_in(m_width, m_height);
    }
    else
    {
      std::cout << "Cursor Pos " << FIXED_FLOAT(mouseX) << ", " << FIXED_FLOAT(mouseY) << " factor " << m_factor << "  screen " << m_width << "\n";
      mpfr->zoom_in_via_mouse(mouseX, mouseY, m_width, m_height);
    }
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomIn()
{
  m_framecount++;
  if (m_fixedCentre)
  {
    mpfr->zoom_in(m_width, m_height);
  }
}

// ---------------------------------------------------------------------------------------
void MandelbrotAdapter::zoomOut()
{
    m_framecount--;
    if (m_framecount < 0) { m_framecount = 0; }
    std::cout << "Cursor Pos " << FIXED_FLOAT(m_width/2) << ", " << FIXED_FLOAT(m_width/2) << " factor " << m_factor << " screen " << m_width << "\n";
    mpfr->zoom_out();
}

// PRIVATE METHODS -----------------------------------------------------------------------

