//////////////////////////////////////////////////////////////////////////////////////////
// Texture.h
#ifndef TEXTURE_H
#define TEXTURE_H

#include "ImageData.h"

class Texture {
  public:
    Texture();
    ~Texture();
    
    void createTexture(ImageData *imageData);
    unsigned int texture() { return m_texture; }

  private:
    
    unsigned int m_texture;
    unsigned int m_width;
    unsigned int m_height;
    
};
  
#endif /* TEXTURE_H */
