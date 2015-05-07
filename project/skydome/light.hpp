#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "opengl.hpp"
#include "entity.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class light : public entity
{
protected:

    GLenum  name;
    GLfloat color[4];
    bool    state;

public:

    light(int, GLenum, float, float, float);

    void on(bool s) { state = s;    }
    bool on()       { return state; }

    virtual void mult_P() = 0;
};

//-----------------------------------------------------------------------------

class light_pos : public light
{
public:

    light_pos(data *, GLenum, float, float, float,
                              float, float, float);
    void mult_P();
    void draw();
};

class light_dir : public light
{
public:

    light_dir(data *, GLenum, float, float, float,
                              float, float, float);
    void mult_P();
    void draw();
};

//-----------------------------------------------------------------------------

#endif
