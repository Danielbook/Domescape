#include "image.hpp"

//-----------------------------------------------------------------------------

image::image(data *d, std::string filename)
{
    void *p;
    int   w;
    int   h;
    int   b;

    target = GL_TEXTURE_2D;
    format = GL_RGB;

    // Load the image data.

    if ((p = d->get_img(filename, w, h, b)))
    {
        // Determine the correct texture target and format.

        if (w & (w - 1)) target = GL_TEXTURE_RECTANGLE_ARB;
        if (h & (h - 1)) target = GL_TEXTURE_RECTANGLE_ARB;

        switch (b)
        {
        case 1: format = GL_LUMINANCE;       break;
        case 2: format = GL_LUMINANCE_ALPHA; break;
        case 3: format = GL_RGB;             break;
        case 4: format = GL_RGBA;            break;
        }

        // Initialize the texture object.

        glGenTextures(1, &texture);
        glBindTexture(target, texture);

        if (target == GL_TEXTURE_2D)
        {
            gluBuild2DMipmaps(target, b, w, h, format, GL_UNSIGNED_BYTE, p);

            glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
                                    GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            glTexImage2D(target, 0, b, w, h, 0, format, GL_UNSIGNED_BYTE, p);

            glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glBindTexture(target, 0);
    }

    GL_CHECK();
}

image::~image()
{
    glDeleteTextures(1, &texture);
}

//-----------------------------------------------------------------------------

void image::bind(GLenum T)
{
    glActiveTextureARB(T);
    {
        glBindTexture(target, texture);
    }
    glActiveTextureARB(GL_TEXTURE0);

    GL_CHECK();
}

//-----------------------------------------------------------------------------

