#include "opengl.hpp"
#include "scene.hpp"

//-----------------------------------------------------------------------------

scene::scene(conf *c, data *d) : entity(0, 0, 0, d->get_obj("billboard.obj")),

                                   prog(d->get_txt("scene.vert"),
                                        d->get_txt("scene.frag"))
{
}

void scene::draw(bool onscreen)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        // Set transparency handling.

        glEnable(GL_BLEND);
        glEnable(GL_ALPHA_TEST);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glAlphaFunc(GL_GREATER, 0.01f);

        // Set up the shader.

        if (onscreen)
        {
            prog.bind();

            prog.uniform("diffuse", 0);
            prog.uniform("shadow0", 1);
            prog.uniform("shadow1", 2);
            prog.uniform("shadow2", 3);
            prog.uniform("shadow3", 4);
            prog.uniform("lightmask", 5);
        }

        // Draw.

        entity::draw();

        glUseProgramObjectARB(0);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------
