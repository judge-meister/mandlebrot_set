//////////////////////////////////////////////////////////////////////////////////////////
// ImageFile.h

// class to handle writing png images

#ifndef IMAGEFILE_H
#define IMAGEFILE_H

// class to write a png image file given some image data

#include <string>
#include "ImageData.h"

class ImageFile {
  public:
    ImageFile();
    ~ImageFile();

    void writeImage(const int framecount,
                    ImageData *imageData);

  private:
    int save_png(std::string filename,
                 int bitdepth, int colortype,
                 unsigned char* data, int pitch, int transform);

    int m_width;
    int m_height;
};

#endif // IMAGEFILE_H
