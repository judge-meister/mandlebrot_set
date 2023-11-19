//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter

#ifndef MANDELBROTADAPTER_H
#define MANDELBROTADAPTER_H

class MandelbrotAdapter
{
  public:
    MandelbrotAdapter(
                const int width, 
                const int height, 
                const char* real, 
                const char* imag, 
                const int factor);
    virtual ~MandelbrotAdapter(){}

    void zoomIn(const double mouseX, const double mouseY);
    void zoomOut();
    void reset(const char* real, const char* imag);
    void reset();
    void useFixed() { m_fixedCentre = true; }
    void useMouse() { m_fixedCentre = false; }
    
    void getTextureData(unsigned char **pixels);
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
