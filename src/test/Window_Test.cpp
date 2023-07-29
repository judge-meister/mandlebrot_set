//////////////////////////////////////////////////////////////////////////////////////////
// TEST CODE

#include <iostream>
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include "Window.h"

static void framebuffer_size_cb(GLFWwindow* window, int width, int height)
{}
static void mouse_button_cb(GLFWwindow* window, int button, int action, int mods)
{}

int main()
{
  Window* window = Window::GetInstance();
  window->setTitle("Title");
  //Window* window = Window::GetInstance("Title", 100, 100);
  window->setFramebufferSizeCB(framebuffer_size_cb);
  window->setMouseButtonCB(mouse_button_cb);
  
  window->eventLoop();
  
  std::cout << "Test Complete\n";
  return 0;
}
