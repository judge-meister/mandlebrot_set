//////////////////////////////////////////////////////////////////////////////////////////
// MandelbrotOpenGL.cpp


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MandelbrotOpenGL.h"

MandelbrotOpenGL::MandelbrotOpenGL()
{
}

MandelbrotOpenGL::~MandelbrotOpenGL()
{
}

// ----------------------------------------------------------------------------
// create the vertexbuffer, elementbuffer and vertexarray
void MandelbrotOpenGL::createVertexArray()
{         
    // create 2 triangles to cover all of the viewport
    float vertices[] = {
        // position           // color           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  // tr
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // br
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  // bl
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f   // tl
    };  
    unsigned int indices[] = {
      0, 1, 3,
      1, 2, 3
    };

    // create VertexBuffer, VertexArray and ElementArray
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenBuffers(1, &VBO);  
    glGenBuffers(1, &EBO);  
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBindVertexArray(0);
    m_vertexArrayObject = VAO;
} 
