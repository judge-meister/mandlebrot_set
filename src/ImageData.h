//////////////////////////////////////////////////////////////////////////////////////////
// ImageData.h

// class to manage bytearray data used for storing raw image data generated 
// by the mpfr mandelbrot library

#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

// class to contain a byte array used for storing raw image data generated by the mpfr code
// and converted to either a texture for opengl or to a png image

class ImageData {
  public:
    ImageData(const int width, const int height, const int depth); // give size of bytearray to create
    ~ImageData();

    bool getByteArray(unsigned char **ba); // output param to write stored bytearrray into
    bool isNotBottomedOut(const int framecount);

    int getWidth() {return m_width;}
    int getHeight() {return m_height;}
    int getDepth() {return m_depth;}

  private:
    unsigned char *m_bytearray;

    int m_width;
    int m_height;
    int m_depth;
    //bool m_bottomingOut;
};

#endif // IMAGE_DATA_H