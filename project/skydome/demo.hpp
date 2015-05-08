#ifndef DEMO_HPP
#define DEMO_HPP

#include "opengl.hpp"
#include "camera.hpp"
#include "image.hpp"
#include "light.hpp"
#include "scene.hpp"
#include "earth.hpp"
#include "bugs.hpp"
#include "sky.hpp"
#include "app.hpp"

//-----------------------------------------------------------------------------

class demo : public app
{
    float now;

    int mouse_x;
    int mouse_y;
    int mouse_b;
    int motion[3];

    float blink_delay;
    bool  blink_state;

    // Scene entities.

    camera demo_camera;
    scene  demo_scene;
    earth  demo_earth;
    sky    demo_sky;

    swarm  demo_bugs1;
    swarm  demo_bugs2;

    // Scene light sources.

    light_dir demo_sun;
    light_dir demo_moon;
    light_pos demo_bulb1;
    light_pos demo_bulb2;

    // Shadow map framebuffer objects.

    fbo shadow_sun;
    fbo shadow_moon;
    fbo shadow_bulb1;
    fbo shadow_bulb2;

    image lightmask;

    void shadow_map(fbo&, light&, GLenum);

public:

    demo(conf *, data *);

    void timer(float);
    void point(int, int);
    void click(int, bool);
    void keybd(int, bool);
    void paint();
};

//-----------------------------------------------------------------------------

#endif
