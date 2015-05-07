//#include "opengl.hpp"
#include "sky.hpp"

//-----------------------------------------------------------------------------

sky::sky(data *d) :

    entity(0, 0, 0, d->get_obj("geodesic_dome.obj")),

    sun_map  (d, "sun.png"),
    moon_map (d, "moon.png"),
    star_map (d, "star.png"),
    sky_glow (d, "glow.png"),
    sky_color(d, "sky.png"),

    sky_prog(d->get_txt("sky.vert"),
             d->get_txt("sky.frag")),

    sun (-90.0f, 0.0f, 0.0f, 1.0f),
    moon(+90.0f, 0.0f, 0.0f, 1.0f)
{
    for (int i = 0; i < 512; ++i)
    {
        random_sprite s;
        stars.push_back(s);
    }
}

void sky::draw()
{
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    {
        std::vector<random_sprite>::iterator i;

        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glScalef(8.0f, 8.0f, 8.0f);

        // Draw the stars.

        glPushMatrix();
        {
            mult_R();

            glBlendFunc(GL_ONE, GL_ONE);

            star_map.bind(GL_TEXTURE0);

            for (i = stars.begin(); i != stars.end(); ++i)
                i->draw();
        }
        glPopMatrix();

        // Draw the sky dome.

        glPushMatrix();
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            sky_prog.bind();

            sky_color.bind(GL_TEXTURE0);
            sky_glow.bind(GL_TEXTURE1);

            sky_prog.uniform("color", 0);
            sky_prog.uniform("glow",  1);

            obj_draw_file(file);

            glUseProgramObjectARB(0);
        }
        glPopMatrix();

        // Draw the sun and moon.

        glPushMatrix();
        {
            mult_R();

            sun_map.bind(GL_TEXTURE0);
            sun.draw();

            moon_map.bind(GL_TEXTURE0);
            moon.draw();
        }
        glPopMatrix();
    }
    glPopAttrib();
    glPopMatrix();
}

//-----------------------------------------------------------------------------

void sprite::draw()
{
    float k = 0.1f * s;
    float d = 1.0f;

    glPushMatrix();
    {
        glRotatef(x, 1.0f, 0.0f, 0.0f);
        glRotatef(y, 0.0f, 1.0f, 0.0f);
        glRotatef(z, 0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 0); glVertex3f(-k, -k, d);
            glTexCoord2i(1, 0); glVertex3f(-k, +k, d);
            glTexCoord2i(1, 1); glVertex3f(+k, +k, d);
            glTexCoord2i(0, 1); glVertex3f(+k, -k, d);
        }
        glEnd();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
