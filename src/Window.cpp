//////////////////////////////////////////////////////////////////////////////////////////
// Window.cpp
#include <iostream>
#include "Window.h"

// intialise the static singleton Window reference
Window* Window::instance = nullptr;


// CONSTRUCTORS --------------------------------------------------------------------------
Window *Window::GetInstance(const std::string title, const int width, const int height)
{
  if (instance == nullptr)
  {
    instance = new Window(title, width, height);
  }
  return instance;
}

Window *Window::GetInstance()
{
  if (instance == nullptr)
  {
    std::cout << "Warning: Window has not been initialised yet\n";
    instance = new Window();
  }
  return instance;
}

Window::Window()
  : m_screenWidth(160), m_screenHeight(160), m_window(nullptr)
{
  create("Test");
}

// --------------------------------------------------------------------------------------
Window::Window(const std::string title, const int width, const int height)
  : m_screenWidth(width), m_screenHeight(height), m_window(nullptr)
{
  create(title);
}

// PUBLIC METHODS ------------------------------------------------------------------------
void Window::setFramebufferSizeCB(GLFWframebuffersizefun framebufferSizeCB)
{
  glfwSetFramebufferSizeCallback(m_window, framebufferSizeCB);
}
// --------------------------------------------------------------------------------------
void Window::setMouseButtonCB(GLFWmousebuttonfun mouseButtonCB)
{
  glfwSetMouseButtonCallback(m_window, mouseButtonCB);
}
// --------------------------------------------------------------------------------------
void Window::terminate()
{
  glfwTerminate();
}
// --------------------------------------------------------------------------------------
void Window::processInput()
{
/*   if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // 
     glfwSetWindowShouldClose(m_window, true);
   }
   if(glfwGetKey(m_window, GLFW_KEY_C) == GLFW_PRESS) { // reset to start
     //reset(); // MandAdapter Class
     printf("Cursor Pos %d, %d factor %d  screen %d\n", m_screenWidth/2, m_screenHeight/2, factor, m_screenWidth);
     framecount = 1;
     shaderProgram = initshaderProgram; // need to swap class instances here
   }
   if(glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS) { 
     // zoom out
     double pX, pY;
     glfwGetCursorPos(m_window, &pX, &pY);
     printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, m_screenWidth);
     //mpfr_zoom_out(factor);
     framecount--;
     if (framecount < 0) { framecount = 0; }
     createTextureFromData();
     shaderProgram = mainshaderProgram; // need to swap class instances here
   }
   if(glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS) { 
     // zoom in
     double pX, pY;
     glfwGetCursorPos(m_window, &pX, &pY);
     printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, m_screenWidth);
     if ((real == NULL) && (imag == NULL))
     {
       //mpfr_zoom_in_via_mouse(pX, pY, m_screenWidth, m_screenHeight, factor);
     }
     else
     {
       //mpfr_zoom_in(m_screenWidth, m_screenHeight, factor);
     }
     framecount++;
     createTextureFromData();
     shaderProgram = mainshaderProgram; // need to swap class instances here
   }
   if(glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) { // move up
   }
   if(glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) { // move down
   }
   if(glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) { // move left
   }
   if(glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) { // move right
   }
*/}
// --------------------------------------------------------------------------------------
void Window::eventLoop()
{
/*  int width, height;
  
  while(!glfwWindowShouldClose(m_window))
  {
    // input from keyboard
    processInput();
    
    // rendering commands
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // bind texture
    glBindTexture(GL_TEXTURE_2D, texture); // Texture Class
    // draw stuff
    glUseProgram(shaderProgram); //Shader Class
    glfwGetWindowSize(m_window, &width, &height);
    glUniform2f(uniform_Resolution, width, height); //Shader Class

    glBindVertexArray(VAO); // VertexArray, ElementBuffer, VertexBuffer
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    // swap buffers
    glfwSwapBuffers(m_window);
    // poll events
    glfwPollEvents();
  }*/
}
// --------------------------------------------------------------------------------------
bool Window::shouldClose()
{
  return glfwWindowShouldClose(m_window);
}
// --------------------------------------------------------------------------------------
void Window::setTitle(const std::string title)
{
  glfwSetWindowTitle(m_window, title.c_str());
}
// PRIVATE METHODS -----------------------------------------------------------------------
// create a OpenGL window
//
void Window::create(std::string title)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
  //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  m_window = glfwCreateWindow(m_screenWidth, m_screenHeight, title.c_str(), NULL, NULL);
  if (m_window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(m_window);
  glfwSetWindowAttrib(m_window, GLFW_RESIZABLE, GLFW_FALSE);
  
  //glfwSetInputMode(*m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(1);
  }
}

