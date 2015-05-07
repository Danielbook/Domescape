#include <iostream>
#include <cmath>
#include <SDL.h>

#include "opengl.hpp"
#include "util.hpp"
#include "demo.hpp"

//-----------------------------------------------------------------------------

demo::demo(conf *c, data *d) :
    app(c, d),

    demo_camera(c),
    demo_scene(c, d),
    demo_earth(d),
    demo_sky(d),

    demo_bugs1(c, d, -3.5f, 10.4f, 1.8f),
    demo_bugs2(c, d,  3.5f, 10.4f, 1.8f),

    demo_sun  (d, GL_LIGHT0, -90.0f,  0.0f, 0.0f, 1.0f, 1.0f, 1.0f),
    demo_moon (d, GL_LIGHT1,  90.0f,  0.0f, 0.0f, 0.1f, 0.1f, 0.2f),
    demo_bulb1(d, GL_LIGHT2,  -3.5f, 10.4f, 1.8f, 0.8f, 0.5f, 0.2f),
    demo_bulb2(d, GL_LIGHT3,   3.5f, 10.4f, 1.8f, 0.8f, 0.6f, 0.2f),

    shadow_sun  (GL_RGBA8, GL_DEPTH_COMPONENT24,
                 c->get_i("shadow_w"),
                 c->get_i("shadow_h")),
    shadow_moon (GL_RGBA8, GL_DEPTH_COMPONENT24,
                 c->get_i("shadow_w"),
                 c->get_i("shadow_h")),
    shadow_bulb1(GL_RGBA8, GL_DEPTH_COMPONENT24,
                 c->get_i("shadow_w") / 2,
                 c->get_i("shadow_h") / 2),
    shadow_bulb2(GL_RGBA8, GL_DEPTH_COMPONENT24,
                 c->get_i("shadow_w") / 2,
                 c->get_i("shadow_h") / 2),
    
    lightmask(d, "lightmask.png")
{
    now = 0;

    blink_delay = 0;
    blink_state = true;

    // Aim the lights toward the ground.

    demo_bulb1.turn(-75.0f, 0.0f, 0.0f);
    demo_bulb2.turn(-75.0f, 0.0f, 0.0f);

    // Rotate the scene to 40 degrees north latitude.

    demo_sky .turn(0.0f, 0.0f, 40.f);
    demo_sun .turn(0.0f, 0.0f, 40.f);
    demo_moon.turn(0.0f, 0.0f, 40.f);

    // Zero the camera motion.

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;
}

//-----------------------------------------------------------------------------

void demo::timer(float dt)
{
    float k = app_conf->get_f("camera_move_rate") * dt;

    // Apply camera motion.

    demo_camera.move(float(motion[0]) * k,
                     float(motion[1]) * k,
                     float(motion[2]) * k);

    // Advance time.

    if (app_conf->get_b("time_pass"))
    {
        float dx = -app_conf->get_f("time_rate") * dt;

        now += dx;

        float b = float(MAX(-cos(RAD(now)), 0.0));

        demo_sky.turn  (dx, 0, 0);
        demo_sun.turn  (dx, 0, 0);
        demo_moon.turn (dx, 0, 0);
        demo_earth.turn(dx, 0, 0);

        demo_bugs1.size(b);
        demo_bugs2.size(b);
    }

    // Blink one of the bulbs.

    if ((blink_delay -= dt) < 0)
    {
        if ((blink_state = !blink_state))
            blink_delay = app_conf->get_f("max_blink_on")  * rand() / RAND_MAX;
        else
            blink_delay = app_conf->get_f("max_blink_off") * rand() / RAND_MAX;

        demo_bulb1.on(blink_state);
    }

    // Make the bugs swarm.

    demo_bugs1.step(dt);
    demo_bugs2.step(dt);

    app::timer(dt);
}

void demo::point(int x, int y)
{
    float k = app_conf->get_f("camera_turn_rate");

    float dx = float(mouse_y - y) * k;
    float dy = float(mouse_x - x) * k;

    if (mouse_b == 1)
        demo_camera.turn(dx, dy, 0);

    if (mouse_b == 3)
    {
        now += dx;

        float b = float(MAX(-cos(RAD(now)), 0.0));

        demo_sky.turn  (dx, 0, 0);
        demo_sun.turn  (dx, 0, 0);
        demo_moon.turn (dx, 0, 0);
        demo_earth.turn(dx, 0, 0);

        demo_bugs1.size(b);
        demo_bugs2.size(b);
    }

    mouse_x = x;
    mouse_y = y;

    app::point(x, y);
}

void demo::click(int b, bool d)
{
    float  z = app_conf->get_f("camera_zoom");
    float dz = app_conf->get_f("camera_zoom_rate");

    if (b == 4 && d) app_conf->set_f("camera_zoom", z + dz);
    if (b == 5 && d) app_conf->set_f("camera_zoom", z - dz);

    mouse_b = d ? b : 0;

    app::click(b, d);
}

void demo::keybd(int k, bool d)
{
    int dd = d ? +1 : -1;

    if (k == app_conf->get_i("move_L")) motion[0] -= dd;
    if (k == app_conf->get_i("move_R")) motion[0] += dd;
    if (k == app_conf->get_i("move_F")) motion[2] -= dd;
    if (k == app_conf->get_i("move_B")) motion[2] += dd;

    if (d)
    {
        if (k == app_conf->get_i("toggle_time"))
            app_conf->set_b("time_pass", !app_conf->get_b("time_pass"));
        if (k == app_conf->get_i("toggle_wire"))
            app_conf->set_b("wireframe", !app_conf->get_b("wireframe"));
    }

    app::keybd(k, d);
}

//-----------------------------------------------------------------------------

void demo::shadow_map(fbo& shadow, light& lightsource, GLenum target)
{
    GLint o[1], v[4];

    // Render a shadow map.

    get_framebuffer(o, v);
    glPushAttrib(GL_ENABLE_BIT);
    {
        // Set up the depth render target.

        glColorMask(0, 0, 0, 0);
        shadow.bind_frame();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_POLYGON_OFFSET_FILL);

        glPolygonOffset(4.0f, 4.0f);

        // Set the lightsource's projection.

        glMatrixMode(GL_PROJECTION);
        {
            glLoadIdentity();
            lightsource.mult_P();
        }
        glMatrixMode(GL_MODELVIEW);

        // Draw the scene from the lightsource's point of view.

        glPushMatrix();
        {
            lightsource.mult_V();

            demo_scene.draw(false);
            demo_bugs1.draw(false);
            demo_bugs2.draw(false);
        }
        glPopMatrix();

        glColorMask(1, 1, 1, 1);
    }
    glPopAttrib();
    set_framebuffer(o, v);

    // Compose a matrix for later use in mapping this shadow map.

    glActiveTextureARB(target);
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();

        lightsource.mult_S();
        lightsource.mult_P();
        lightsource.mult_V();
        demo_camera.mult_M();
    }
    glMatrixMode(GL_MODELVIEW);
    glActiveTextureARB(GL_TEXTURE0);
}

void demo::paint()
{
    // Set the ambient light for the given time of day.

    float k = float(MAX(+cos(RAD(now)), 0.0));

    float A[4] = { 0.5f * k,
                   0.5f * k,
                   0.7f * k, 0.0f };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);

    // Prepare the shadow maps.

    glEnable(GL_SCISSOR_TEST);

    if (demo_sun.on())   shadow_map(shadow_sun,   demo_sun,   GL_TEXTURE1);
    if (demo_moon.on())  shadow_map(shadow_moon,  demo_moon,  GL_TEXTURE2);
    if (demo_bulb1.on()) shadow_map(shadow_bulb1, demo_bulb1, GL_TEXTURE3);
    if (demo_bulb2.on()) shadow_map(shadow_bulb2, demo_bulb2, GL_TEXTURE4);

    // Apply the projection.

    glMatrixMode(GL_PROJECTION);
    {
        int w = app_conf->get_i("window_w");
        int h = app_conf->get_i("window_h");

        float f = app_conf->get_f("camera_far");
        float n = app_conf->get_f("camera_near");
        float z = app_conf->get_f("camera_zoom");

        float a = float(w) / float(h);

        glViewport(0, 0, w, h);
        glScissor (0, 0, w, h);

        glLoadIdentity();
        glFrustum(-a * z, +a * z, -z, +z, n, f);
    }
    glMatrixMode(GL_MODELVIEW);

    // Render the scene.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);

        if (app_conf->get_b("wireframe"))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glPushMatrix();
        {
            // Apply the camera transform. Draw the sky and landscape.

            demo_sun.draw();
            demo_moon.draw();
            demo_camera.mult_Ri();
            demo_sky.draw();

            demo_sun.draw();
            demo_moon.draw();
            demo_earth.draw(true);
            demo_camera.mult_Ti();

            // Draw the foreground objects.

            shadow_sun  .bind_depth(GL_TEXTURE1);
            shadow_moon .bind_depth(GL_TEXTURE2);
            shadow_bulb1.bind_depth(GL_TEXTURE3);
            shadow_bulb2.bind_depth(GL_TEXTURE4);

            lightmask.bind(GL_TEXTURE5);

            demo_sun  .draw();
            demo_moon .draw();
            demo_bulb1.draw();
            demo_bulb2.draw();
            demo_scene.draw(true);
            demo_bugs1.draw(true);
            demo_bugs2.draw(true);
        }
        glPopMatrix();
    }
    glPopAttrib();

    app::paint();
}

//-----------------------------------------------------------------------------

