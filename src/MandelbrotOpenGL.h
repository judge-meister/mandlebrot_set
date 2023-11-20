//////////////////////////////////////////////////////////////////////////////////////////
// MandelbrotOpenGL.h

// class to create a vertex array

#ifndef MANDELBROT_OPEN_GL_H
#define MANDELBROT_OPEN_GL_H

class MandelbrotOpenGL {
    public:

    MandelbrotOpenGL();
    ~MandelbrotOpenGL();

    void createVertexArray();
    unsigned int getVertexArray(){return m_vertexArrayObject;}

    private:
    unsigned int m_vertexArrayObject;
};

#endif // MANDELBROT_OPEN_GL_H
