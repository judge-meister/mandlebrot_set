//////////////////////////////////////////////////////////////////////////////////////////
// MandelbrotWindow.h

// main class to run the program

#ifndef MANDELBROT_WINDOW_H
#define MANDELBROT_WINDOW_H

#include "Shader.h"
#include "Window.h"
#include "Texture.h"
#include "CmdOptions.h"
#include "MandelbrotAdapter.h"
#include "ImageFile.h"
#include "ImageData.h"

// -----------------------------------
class MandelbrotWindow {
  public:
    MandelbrotWindow();
    ~MandelbrotWindow();

    void initialise(CmdOptions* cmdOptions);
    void createShaders();
    void updateDisplay();
    void zoomIn(const double pX, const double pY);
    void zoomOut(double pX, double pY);
    void setCurrentShaderToInit();

    Texture* getTexture(){return m_texture;}
    Window* getWindow(){return m_window;}
    MandelbrotAdapter* getMandAdapter(){return m_mandAdapter;}
    Shader* getCurrentShader(){return m_currentShader;}

  private:

    Shader* m_mainShader;
    Shader* m_initShader;
    Shader* m_currentShader;

    Window* m_window;
    Texture* m_texture; 
    CmdOptions* m_cmdOptions;
    MandelbrotAdapter* m_mandAdapter;
};

#endif // MANDELBROT_WINDOW_H
