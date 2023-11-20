//////////////////////////////////////////////////////////////////////////////////////////
// MandelbrotWindow.cpp

#include "MandelbrotWindow.h"

const int RGB = 3; // size of color pixel

MandelbrotWindow::MandelbrotWindow()
{
}

void MandelbrotWindow::initialise(CmdOptions *cmdOptions)
{
    m_window = Window::GetInstance("Mandelbrot Set", cmdOptions->getWidth(), cmdOptions->getHeight());
    //window->setFramebufferSizeCB(framebuffer_size_callback);
    //m_window->setMouseButtonCB(mouse_button_callback);

    createShaders();  
    m_currentShader = m_initShader;

    // initialise mandelbrot library
    m_mandAdapter = new MandelbrotAdapter(cmdOptions);

    m_cmdOptions = cmdOptions;
    
    m_texture = new Texture();
    //mandOpenGL = new MandelbrotOpenGL();
    //mandOpenGL->createVertexArray();
}

void MandelbrotWindow::setCurrentShaderToInit()
{
    m_mandAdapter->reset();
    m_currentShader = m_initShader;
}
// ----------------------------------------------------------------------------
void MandelbrotWindow::updateDisplay()
{
    ImageFile* imageFile = new ImageFile();
    ImageData* imageData = new ImageData(m_cmdOptions->getWidth(), m_cmdOptions->getHeight(), RGB);
    m_mandAdapter->getTextureData(imageData);
    m_texture->createTexture(imageData);
    m_currentShader = m_mainShader;
    imageFile->writeImage(m_mandAdapter->framecount(), imageData);
}
// ----------------------------------------------------------------------------
void MandelbrotWindow::zoomIn(const double pX, const double pY)
{
    m_mandAdapter->zoomIn(pX, pY);
    updateDisplay();
}

// ----------------------------------------------------------------------------
void MandelbrotWindow::zoomOut(double pX, double pY)
{
    m_mandAdapter->zoomOut();
    updateDisplay();
}

// ----------------------------------------------------------------------------
// create the shader programs
void MandelbrotWindow::createShaders()
{
    m_mainShader = new Shader();
    m_initShader = new Shader();
    m_mainShader->createVertexShaderFromFile("../shader/texshader.vert");
    m_mainShader->createFragmentShaderFromFile("../shader/texshader.frag");
    m_mainShader->createShaderProgram();
    
    m_initShader->createVertexShaderFromFile("../shader/mandelbrot.vert");
    m_initShader->createFragmentShaderFromFile("../shader/mandelbrot.frag");
    m_initShader->createShaderProgram();
}

