#ifndef APP_HPP
#define APP_HPP

#include "conf.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

class app
{
protected:

    conf *app_conf;
    data *app_data;

public:

    app(conf *c, data *d) : app_conf(c), app_data(d) { }

    virtual void timer(float);
    virtual void point(int, int);
    virtual void click(int, bool);
    virtual void keybd(int, bool);
    virtual void paint();

    virtual ~app();
};

//-----------------------------------------------------------------------------

#endif
