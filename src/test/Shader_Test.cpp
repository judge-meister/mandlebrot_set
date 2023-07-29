/////////////////////////////////////////////////////////////////////////////////////////
// SHADER CLASS TEST CODE

// use following to build
//
// g++ -g Shader.cpp Shader_Test.cpp -o Shader_Test -I../include glad.o -lglfw -lGL
//

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

static void createWindow(GLFWwindow** window, int width, int height)
{
  glfwInit();
  *window = glfwCreateWindow(width, height, "Mandelbrot Set", NULL, NULL);
  glfwMakeContextCurrent(*window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(1);
  }   
}

int main()
{
  GLFWwindow *window;
  createWindow(&window, 100, 100);

  Shader shader; // = new Shader();
  std::cout << "create vert shader\n";
  shader.createVertexShaderFromFile("../shader/texshader.vert");
  std::cout << "create frag shader\n";
  shader.createFragmentShaderFromFile("../shader/texshader.frag");
  std::cout << "create shader\n";
  shader.createShaderProgram();

  unsigned int shaderProg = shader.getShaderProgram();
  shader.useProgram();
  shader.uniformMouse(24.0,24.0);
  
  std::cout << "End Test\n";
  return 0;
}
