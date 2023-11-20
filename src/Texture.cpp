//////////////////////////////////////////////////////////////////////////////////////////
// Texture.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Texture.h"

// CONSTRUCTORS --------------------------------------------------------------------------
Texture::Texture()
{}

// --------------------------------------------------------------------------------------
Texture::~Texture()
{
}

// PUBLIC METHODS ------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
void Texture::createTexture(ImageData *imageData)
{
  unsigned char *pixels = NULL;
  imageData->getByteArray(&pixels);
  m_width = imageData->getWidth();
  m_height = imageData->getHeight();

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  glGenerateMipmap(GL_TEXTURE_2D);
}


