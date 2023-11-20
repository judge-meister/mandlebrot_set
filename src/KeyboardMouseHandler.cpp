//////////////////////////////////////////////////////////////////////////////////////////
// KeyboardMouseHandler.cpp

#include "KeyboardMouseHandler.h"

MandelbrotWindow* KeyboardMouseHandler::m_mandWindow = NULL;

void KeyboardMouseHandler::setMandWindow(MandelbrotWindow* mandWindow)
{
  KeyboardMouseHandler::m_mandWindow = mandWindow;
}

// ----------------------------------------------------------------------------
// react to the mouse clicks in the display
//
void KeyboardMouseHandler::mouseButtonCB(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    KeyboardMouseHandler::m_mandWindow->zoomIn(pX, pY);
  }

  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    KeyboardMouseHandler::m_mandWindow->zoomOut(pX, pY);
  }
}
  
// ----------------------------------------------------------------------------
// respond to the keyboard
//
void KeyboardMouseHandler::processKeyboardInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // 
    glfwSetWindowShouldClose(window, true);
  }
  if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { // reset to start
    KeyboardMouseHandler::m_mandWindow->setCurrentShaderToInit();
  }
  if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // zoom out
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    KeyboardMouseHandler::m_mandWindow->zoomOut(pX, pY);
  }
  if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // zoom in
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    KeyboardMouseHandler::m_mandWindow->zoomIn(pX, pY);
  }
  if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { // move up
  }
  if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { // move down
  }
  if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { // move left
  }
  if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { // move right
  }
}
