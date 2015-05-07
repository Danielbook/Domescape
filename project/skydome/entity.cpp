#include <cmath>

#include "util.hpp"
#include "entity.hpp"
#include "opengl.hpp"

//-----------------------------------------------------------------------------

void entity::move(float kx, float ky, float kz)
{
    float P = float(RAD(r[0]));
    float T = float(RAD(r[1]));

    p[0] += cos(P) * (kz * sin(T) + kx * cos(T));
    p[1] -= sin(P) * (kz                       ) + ky;
    p[2] += cos(P) * (kz * cos(T) - kx * sin(T));
}

void entity::turn(float kx, float ky, float kz)
{
    r[0] += kx;
    r[1] += ky;
    r[2] += kz;

    if (r[0] > 360.0f) r[0] -= 360.0f;
    if (r[0] <   0.0f) r[0] += 360.0f;
    if (r[1] > 360.0f) r[1] -= 360.0f;
    if (r[1] <   0.0f) r[1] += 360.0f;
}

//-----------------------------------------------------------------------------

void entity::mult_Ri()
{
    glRotatef(-r[0], 1.0f, 0.0f, 0.0f);
    glRotatef(-r[1], 0.0f, 1.0f, 0.0f);
    glRotatef(-r[2], 0.0f, 0.0f, 1.0f);
}

void entity::mult_Ti()
{
    glTranslatef(-p[0], -p[1], -p[2]);
}

void entity::mult_S(void)
{
    float M[16] = {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f,
    };

    glMultMatrixf(M);
}

void entity::mult_R()
{
    glRotatef(+r[2], 0.0f, 0.0f, 1.0f);
    glRotatef(+r[1], 0.0f, 1.0f, 0.0f);
    glRotatef(+r[0], 1.0f, 0.0f, 0.0f);
}

void entity::mult_T()
{
    glTranslatef(+p[0], +p[1], +p[2]);
}

void entity::mult_M()
{
    mult_T();
    mult_R();
}

void entity::mult_V()
{
    mult_Ri();
    mult_Ti();
}

//-----------------------------------------------------------------------------

void entity::draw(bool shaders)
{
    glPushMatrix();
    {
        mult_M();

        // Draw the object associated with this geom.

        if (file >= 0) obj_draw_file(file);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

entity::entity(float x, float y, float z, int f) : file(f)
{
    p[0] = x;
    p[1] = y;
    p[2] = z;

    r[0] = 0;
    r[1] = 0;
    r[2] = 0;
}

entity::~entity()
{
}
