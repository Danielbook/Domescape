#include <iostream>
#include <cmath>

#include "opengl.hpp"
#include "earth.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

earth::earth(data *d) : entity(0, 0, 0, d->get_obj("landscape.obj"))
{
}

void earth::draw(bool onscreen)
{
    float s = 128.0f;

    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    {
        if (onscreen)
        {
            float k = float(MAX(cos(RAD(r[0])), 0));
            float c[4];

            c[0] = 0.3f * k;
            c[1] = 0.5f * k;
            c[2] = 1.0f * k;
            c[3] = 1.0f;

            glDisable(GL_DEPTH_TEST);
            glEnable(GL_FOG);

            glFogi(GL_FOG_MODE, GL_LINEAR);
            glFogf(GL_FOG_START, 0.0f);
            glFogf(GL_FOG_END, 256.0f);
            glFogfv(GL_FOG_COLOR, c);

            glTranslatef(0.0f, -1.0f, 0.0f);
        }

        glScalef(s, s, s);
        obj_draw_file(file);
    }
    glPopAttrib();
    glPopMatrix();
}

//-----------------------------------------------------------------------------
