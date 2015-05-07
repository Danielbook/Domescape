#include "opengl.hpp"
#include "bugs.hpp"

//-----------------------------------------------------------------------------

swarm::swarm(conf *c, data *d, float x, float y, float z) :
    entity(0, 0, 0, -1), bug_map(d, "bug.png")
{
    p[0] = x;
    p[1] = y;
    p[2] = z;

    count = 0;

    for (int i = 0; i < max_count; ++i)
    {
        bug b;
        bugs.push_back(b);
    }
}

void swarm::size(float k)
{
    count = max_count * k;
}

void swarm::step(float dt)
{
    std::vector<bug>::iterator i;

    for (i = bugs.begin(); i != bugs.end(); ++i)
        i->step(dt);
}

void swarm::draw(bool onscreen)
{
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    {
        int i;

        glEnable(GL_BLEND);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);

        // Draw the bugs.

        glPushMatrix();
        {
            mult_T();

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glAlphaFunc(GL_GREATER, 0.01f);

            bug_map.bind(GL_TEXTURE0);

            for (i = 0; i < count; ++i)
                bugs[i].draw();
        }
        glPopMatrix();
    }
    glPopAttrib();
    glPopMatrix();
}

//-----------------------------------------------------------------------------

void bug::step(float dt)
{
    x += dt * 180.0f;
    y += dt * 270.0f;
    z += dt * 360.0f;
}

void bug::draw()
{
    float k = 0.1f;

    glPushMatrix();
    {
        glRotatef(x, 1.0f, 0.0f, 0.0f);
        glRotatef(y, 0.0f, 1.0f, 0.0f);
        glRotatef(z, 0.0f, 0.0f, 1.0f);

        glBegin(GL_QUADS);
        {
            glTexCoord2i(0, 0); glVertex3f(-k, -k, r);
            glTexCoord2i(1, 0); glVertex3f(-k, +k, r);
            glTexCoord2i(1, 1); glVertex3f(+k, +k, r);
            glTexCoord2i(0, 1); glVertex3f(+k, -k, r);
        }
        glEnd();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
