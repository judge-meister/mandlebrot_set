//////////////////////////////////////////////////////////////////////////////////////////
// ImageFile.cpp

#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include "ImageData.h"

ImageData::ImageData(const int width, const int height, const int depth)
// : m_bottomingOut(false)
{
    // create memory for bytearray
    m_width = width;
    m_height = height;
    m_depth = depth;
    size_t size = (size_t)(m_width * m_height * m_depth);
    m_bytearray = (unsigned char*)calloc(size, sizeof(unsigned char)); 
}

ImageData::~ImageData()
{
    // free the byte array of
    if(NULL != m_bytearray) {
        free(m_bytearray);
    }
}

// allow caller to get a pointer to the bytearray
bool ImageData::getByteArray(unsigned char **ba)
{
    *ba = m_bytearray;
    return true;
}

//
bool ImageData::isNotBottomedOut(const int framecount)
{
  /* look at a small (16x16) square in the middle of the image data */
  int w = m_width*m_depth;
  int h = m_height;
  int r=-1, g=-1, b=-1;
  int rc=0, gc=0, bc=0;
  bool ok = true;

  //std::cout << std::boolalpha << "ok: " << ok << " (init)" << std::endl;
  if(framecount < 10)
  {
    return true;
  }

  for(int y=(h/2)-8; y < (h/2)+8 && ok; y++)
  {
    for(int x=(w/2)-8*m_depth; x < (w/2)+8*m_depth && ok; x=x+3)
    {
      if((r==-1)&&(g==-1)&&(b==-1))
      { 
        r = m_bytearray[x+(y*w)];
        g = m_bytearray[x+(y*w)+1];
        b = m_bytearray[x+(y*w)+2]; 
      }
      else 
      {
          if(r == m_bytearray[x+(y*w)]) { rc++; }
          if(g == m_bytearray[x+(y*w)+1]) { gc++; }
          if(b == m_bytearray[x+(y*w)+2]) { bc++; }
      }
    }
  }
  if((rc > 250)&&(gc > 250)&&(bc > 250))
  {
    ok = false;
  }
  std::cout << std::boolalpha << "ok: " << ok << "  rc " << rc << " gc " << gc << " bc " << bc << std::endl;

  return ok;
}
