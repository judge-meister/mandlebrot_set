/////////////////////////////////////////////////////////////////////////////////////////
// Shader.cpp

#include <iostream>
#include <fstream>
#include <string>

#include <glad/glad.h>

#include "Shader.h"

// CONSTRUCTORS --------------------------------------------------------------------------
Shader::Shader(const std::string vertexFilename, const std::string fragmentFilename)
  : m_shaderProgram(0), m_vertexShader(0), m_fragmentShader(0)
{
  createVertexShaderFromFile(vertexFilename);
  //std::cout << "create frag shader\n";
  createFragmentShaderFromFile(fragmentFilename);
  //std::cout << "create shader\n";
  createShaderProgram();
}
// --------------------------------------------------------------------------------------
Shader::Shader()
  : m_shaderProgram(0), m_vertexShader(0), m_fragmentShader(0)
{}

// PUBLIC METHODS ------------------------------------------------------------------------
void Shader::useProgram()
{
  glUseProgram(m_shaderProgram);
}

// --------------------------------------------------------------------------------------
void Shader::uniformMouse(const double xpos, const double ypos)
{
  glUniform2f(m_uniformMouse, xpos, ypos);
}

// --------------------------------------------------------------------------------------
void Shader::uniformScale(const double scale)
{
  glUniform1f(m_uniformScale, scale);
}

// --------------------------------------------------------------------------------------
void Shader::uniformResolution(const int width, const int height)
{
  glUniform2f(m_uniformResolution, width, height);
}

// --------------------------------------------------------------------------------------
void Shader::createVertexShaderFromFile(const std::string filename)
{
  //std::cout << "call createShaderFromFile(vert)\n";
  m_vertexShader = createShaderFromFile(filename, GL_VERTEX_SHADER, "VERTEX");
}

// --------------------------------------------------------------------------------------
void Shader::createFragmentShaderFromFile(const std::string filename)
{
  //std::cout << "call createShaderFromFile(frag)\n";
  m_fragmentShader = createShaderFromFile(filename, GL_FRAGMENT_SHADER, "FRAGMENT");
}

// --------------------------------------------------------------------------------------
void Shader::createShaderProgram()
{
  int success;
  const unsigned int logsize = 1024;
  char infoLog[logsize];
  m_shaderProgram = glCreateProgram();

  glAttachShader(m_shaderProgram, m_vertexShader);
  glAttachShader(m_shaderProgram, m_fragmentShader);
  glLinkProgram(m_shaderProgram);

  glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
  if(!success) 
  {
    glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
  }
  glDeleteShader(m_vertexShader);
  glDeleteShader(m_fragmentShader); 
  
  // setup uniform locations
  m_uniformMouse = glGetUniformLocation(m_shaderProgram, "mouse");
  m_uniformScale = glGetUniformLocation(m_shaderProgram, "scale");
  m_uniformResolution = glGetUniformLocation(m_shaderProgram, "resolution");
}

// PRIVATE METHODS -----------------------------------------------------------------------
unsigned int Shader::createShaderFromFile(const std::string filename, 
                                          unsigned int shaderType, 
                                          const char* shaderTypeName)
{
  std::fstream newfile;
  std::string shaderSource;
  std::string line;
  int  success;
  const unsigned int logsize = 1024;
  char infoLog[logsize];

  newfile.open(filename, std::ios::in); // open a file to perform read operation using file object
  if (newfile.is_open()) {              // checking whether the file is open
    while(getline(newfile, line)) {     // read data from file object and put it into string.
         shaderSource += "\n" + line;
    }
    newfile.close();                    // close the file object.
  }

  //std::cout << shaderSource << "\n";

  //std::cout << "glCreateShader\n";
  unsigned int shader = glCreateShader(shaderType);
  char const *shaderSourcePtr = shaderSource.c_str();
  //std::cout << "glShaderSource\n";
  glShaderSource(shader, 1, &shaderSourcePtr, NULL);
  //std::cout << "glCompileShader\n";
  glCompileShader(shader);
  
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if(!success)
  {
    glGetShaderInfoLog(shader, logsize, NULL, infoLog);
    std::cout << "ERROR::SHADER::" << shaderTypeName << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(1);
  }

  return shader;
}

