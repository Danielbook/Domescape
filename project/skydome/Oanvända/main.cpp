#include <stdexcept>
#include <iostream>
#include <sstream>
#include <cmath>

#include <SDL.h>
#include <png.h>
#include <stdio.h>

#include "opengl.hpp"
#include "demo.hpp"

#define JIFFY (1000 / 60)

//-----------------------------------------------------------------------------

void snap(std::string filename, int w, int h)
{
    FILE       *filep  = NULL;
    png_structp writep = NULL;
    png_infop   infop  = NULL;
    png_bytep  *bytep  = NULL;

    // Initialize all PNG export data structures.

    if (!(filep = fopen(filename.c_str(), "wb")))
        throw std::runtime_error("Failure opening PNG file for writing");

    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG write structure");

    if (!(infop = png_create_info_struct(writep)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        unsigned char *p = NULL;

        // Initialize the PNG header.

        png_init_io (writep, filep);
        png_set_IHDR(writep, infop, w, h, 8, PNG_COLOR_TYPE_RGB,
                                             PNG_INTERLACE_NONE,
                                             PNG_COMPRESSION_TYPE_DEFAULT,
                                             PNG_FILTER_TYPE_DEFAULT);

        // Allocate the pixel buffer and copy pixels there.

        if ((p = new unsigned char[w * h * 3]))
        {
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

            // Allocate and initialize the row pointers.

            if ((bytep = (png_bytep *)png_malloc(writep, h*sizeof(png_bytep))))
            {
                for (int i = 0; i < h; ++i)
                    bytep[h - i - 1] = (png_bytep) (p + i * w * 3);

                // Write the PNG image file.

                png_set_rows  (writep, infop, bytep);
                png_write_info(writep, infop);
                png_write_png (writep, infop, 0, NULL);

                free(bytep);
            }
            else throw std::runtime_error("Failure allocating PNG row array");

            delete [] p;
        }
    }

    // Release all resources.

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);
}

//-----------------------------------------------------------------------------

void stat()
{
    static float t0 = 0;
    static float t1 = 0;
    static float tt = 0;
    static int   n  = 0;

    // Accumulate passing time.

    t1 = SDL_GetTicks() / 1000.0f;
    tt = tt + t1 - t0;
    t0 = t1;

    // Count frames and report frames per second.

    n++;

    if (tt > 0.25f)
    {
        std::ostringstream str;

        str << int(n / tt);

        SDL_WM_SetCaption(str.str().c_str(),
                          str.str().c_str());
        tt = 0;
        n  = 0;
    }
}

static bool loop(int argc, char *argv[])
{
    static int tock;

    static conf *C;
    static data *D;
    static app  *A;

    int w, h, b, m = SDL_OPENGL;
    SDL_Event e;

    // While there are available events, dispatch event handlers.

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_QUIT:
            if (A) delete A;
            if (D) delete D;
            if (C) delete C;

            return false;

        case SDL_USEREVENT:
            if (A) delete A;
            if (D) delete D;
            if (C) delete C;

            C = new conf(argc, argv);
            D = new data("data");

            // Look up the video mode parameters.

            w = C->get_i("window_w");
            h = C->get_i("window_h");
            b = C->get_i("window_b");

            if (C->get_b("window_fullscreen")) m |= SDL_FULLSCREEN;
            if (C->get_b("window_noframe"))    m |= SDL_NOFRAME;

            if (w == 0) w = 800;
            if (h == 0) h = 600;

            // Initialize the video.

            if (SDL_SetVideoMode(w, h, b, m) == 0)
                throw std::runtime_error(SDL_GetError());

            init_ogl();

            // Initialize the demo.

            A = new demo(C, D);

            break;

        case SDL_MOUSEMOTION:
            A->point(e.motion.x, e.motion.y);  break;

        case SDL_MOUSEBUTTONDOWN:
            A->click(e.button.button,  true);  break;

        case SDL_MOUSEBUTTONUP:
            A->click(e.button.button,  false); break;

        case SDL_KEYDOWN:
            A->keybd(e.key.keysym.sym, true);  break;

        case SDL_KEYUP:
            A->keybd(e.key.keysym.sym, false); break;
        }

    // If a jiffy has expired, call the timer method.

    while (SDL_GetTicks() - tock >= JIFFY)
    {
        tock += JIFFY;
        A->timer(JIFFY / 1000.0f);
    }

    // Paint the scene and display statistics. 

    A->paint();

    stat();
    
    return true;
}

int main(int argc, char *argv[])
{
    try
    {
        if (SDL_Init(SDL_INIT_VIDEO) == 0)
        {
            SDL_Event e = { SDL_USEREVENT };

            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            
            SDL_PushEvent(&e);

            while (loop(argc, argv))
                SDL_GL_SwapBuffers();

            SDL_Quit();
        }
        else throw std::runtime_error(SDL_GetError());
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

//-----------------------------------------------------------------------------
