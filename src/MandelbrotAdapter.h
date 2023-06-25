//////////////////////////////////////////////////////////////////////////////////////////
// Mandelbrot Adapter
#ifndef MANDELBROTADAPTER_H
#define MANDELBROTADAPTER_H

class MandelbrotAdapter
{
  public:
    //MandelbrotAdapter(const int width, const int height);
    MandelbrotAdapter(const int width, const int height, const char* real, const char* imag, const int factor);
    ~MandelbrotAdapter();

    void zoomIn(const double mouseX, const double mouseY);
    //void zoomIn(const int factor);
    void zoomOut();
    void reset(const char* real, const char* imag);
    void reset();
    void useFixed() { m_fixedCentre = true; }
    void useMouse() { m_fixedCentre = false; }
    
    void createTextureFromData();
    void cleanUp();
    unsigned int texture() { return m_texture; }
    
  private:
    void writeImage(unsigned int **rgb);
    
    bool m_fixedCentre;
    unsigned int m_width, m_height;
    unsigned int m_maxiter;
    unsigned int m_framecount;
    unsigned int m_factor;
    unsigned int m_texture;
};

#endif /* MANDELBROTADAPTER_H */
