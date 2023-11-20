//////////////////////////////////////////////////////////////////////////////////////////
// ImageFile.cpp

#include <stdlib.h>
#include "ImageData.h"

ImageData::ImageData(const int width, const int height, const int depth)
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
