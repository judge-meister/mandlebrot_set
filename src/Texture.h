//////////////////////////////////////////////////////////////////////////////////////////
// Texture.h
#ifndef TEXTURE_H
#define TEXTURE_H

class Texture {
  public:
    Texture(const int width, const int height);
    ~Texture();
    
    void createTexture(unsigned char **pixels);
    unsigned int texture() { return m_texture; }

  private:
    
    unsigned int m_texture;
    unsigned int m_width;
    unsigned int m_height;
    
};
  
#endif /* TEXTURE_H */
