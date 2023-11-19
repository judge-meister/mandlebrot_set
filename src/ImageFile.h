
#ifndef IMAGEFILE_H
#define IMAGEFILE_H

#include <string>

class ImageFile {
  public:
    ImageFile(const int width, const int height);
    ~ImageFile();

    void writeImage(const int framecount,
                    unsigned char* data);

  private:
    int save_png(std::string filename,
                 int bitdepth, int colortype,
                 unsigned char* data, int pitch, int transform);

    int m_width;
    int m_height;
};

#endif // IMAGEFILE_H
