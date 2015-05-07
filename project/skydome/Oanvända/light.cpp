#include <iostream>
#include <cmath>

#include "opengl.hpp"
#include "light.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

light::light(int f, GLenum n, float R, float G, float B)
    : entity(0, 0, 0, f), name(n), state(true)
{
    color[0] = R;
    color[1] = G;
    color[2] = B;
    color[3] = 1;
}

//-----------------------------------------------------------------------------

light_pos::light_pos(data *d, GLenum n, float x, float y, float z,
                                        float R, float G, float B) :
    light(-1, n, R, G, B)
{
    p[0] = x;
    p[1] = y;
    p[2] = z;
}

void light_pos::mult_P()
{
    float k = 1.0f;

    glFrustum(-k, +k, -k, +k, k, 16);
}

void light_pos::draw()
{
    GLfloat p[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat c[4];

    glPushMatrix();
    {
        float k = state ? 1.0f : 0.0f;

        c[0] = color[0] * k;
        c[1] = color[1] * k;
        c[2] = color[2] * k;
        c[3] = 1;

        mult_T();

        glEnable(name);

        glLightfv(name, GL_DIFFUSE,  c);
        glLightfv(name, GL_POSITION, p);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

light_dir::light_dir(data *d, GLenum n, float x, float y, float z,
                                        float R, float G, float B) :
    light(-1, n, R, G, B)
{
    r[0] = x;
    r[1] = y;
    r[2] = z;
}

void light_dir::mult_P()
{
    double d = 20.0;

    glOrtho(-d, +d, -d, +d, -128, 128);
}

void light_dir::draw()
{
    GLfloat p[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
    GLfloat c[4];

    glPushMatrix();
    {
        // Automatically nullify the lightsource if it dips below the horizon.

        state = (-sin(RAD(r[0])) > 0.0);

        float k = float(state ? pow(MAX(-sin(RAD(r[0])), 0.0), 0.5) : 0.0);

        c[0] = color[0] * k;
        c[1] = color[1] * k;
        c[2] = color[2] * k;
        c[3] = 1;

        // Position and enable the lightsource.

        mult_R();

        glEnable(name);

        glLightfv(name, GL_DIFFUSE,  c);
        glLightfv(name, GL_POSITION, p);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
