#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "entity.hpp"
#include "conf.hpp"

//-----------------------------------------------------------------------------

class camera : public entity
{
public:

    camera(conf *c) : entity(c->get_f("camera_x"),
                             c->get_f("camera_y"),
                             c->get_f("camera_z"), -1) { }
};

//-----------------------------------------------------------------------------

#endif
