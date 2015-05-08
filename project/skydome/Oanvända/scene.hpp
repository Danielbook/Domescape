#ifndef SCENE_HPP
#define SCENE_HPP

#include "entity.hpp"
#include "opengl.hpp"
#include "conf.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class scene : public entity
{
    shader prog;

public:

    scene(conf *c, data *d);

    void draw(bool);
};

//-----------------------------------------------------------------------------

#endif
