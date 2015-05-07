#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>

#include "opengl.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class image
{
    GLenum target;
    GLenum format;
    GLuint texture;

public:

    image(data *, std::string);
   ~image();

    void bind(GLenum);
};

//-----------------------------------------------------------------------------

#endif
