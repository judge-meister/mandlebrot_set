/////////////////////////////////////////////////////////////////////////////////////////
// Shader.h
#ifndef SHADER_H
#define SHADER_H

#include <string>

class Shader {
  public:
    Shader();
    Shader(const std::string vertexFilename, const std::string fragmentFilename);
    ~Shader(){}

    void createVertexShaderFromFile(const std::string filename);
    void createFragmentShaderFromFile(const std::string filename);
    void createShaderProgram();

    unsigned int getShaderProgram() { return m_shaderProgram; }
    void useProgram();
    
    void uniformMouse(const double xpos, const double ypos);
    void uniformScale(const double scale);
    void uniformResolution(const int width, const int height);
    
  private:
    unsigned int createShaderFromFile(const std::string filename, 
                                      unsigned int shaderType, 
                                      const char* shaderTypeName);
    unsigned int m_shaderProgram;
    unsigned int m_vertexShader;
    unsigned int m_fragmentShader;
    int m_uniformMouse;
    int m_uniformScale;
    int m_uniformResolution;
};

#endif /* SHADER_H */
