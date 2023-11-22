//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter

// adapter class for interfacing with the mpfr mandelbrot library

#ifndef MANDELBROTADAPTER_H
#define MANDELBROTADAPTER_H

#include <string>
#include "ImageData.h"
#include "CmdOptions.h"

class MandelbrotAdapter
{
  public:
    MandelbrotAdapter(CmdOptions *options);
    virtual ~MandelbrotAdapter(){}

    void zoomIn(const double mouseX, const double mouseY);
    void zoomIn();
    void zoomOut();
    void reset(const std::string &real, const std::string &imag);
    void reset();
    void useFixed() { m_fixedCentre = true; }
    void useMouse() { m_fixedCentre = false; }
    
    void getTextureData(ImageData *imageData);
    void cleanUp();
    unsigned int framecount() { return m_framecount; }
    
  private:
    
    bool m_fixedCentre;
    unsigned int m_width, m_height;
    unsigned int m_maxiter;
    unsigned int m_framecount;
    unsigned int m_factor;
};

#endif /* MANDELBROTADAPTER_H */
