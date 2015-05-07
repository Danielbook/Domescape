#include <SDL.h>

#include "main.hpp"
#include "app.hpp"

//-----------------------------------------------------------------------------

void app::timer(float dt)
{
}

void app::point(int dx, int dy)
{
}

void app::click(int b, bool d)
{
}

void app::keybd(int k, bool d)
{
    SDL_Event user = { SDL_USEREVENT };
    SDL_Event quit = { SDL_QUIT      };

    if (d)
    {
        switch (k)
        {
        case SDLK_F9:
            snap(app_conf->get_s("screenshot_file"),
                 app_conf->get_i("window_w"),
                 app_conf->get_i("window_h"));
            break;

        case SDLK_F12:
            SDL_PushEvent(&user);
            break;

        case SDLK_ESCAPE:
            SDL_PushEvent(&quit);
            break;
        }
    }
}

void app::paint()
{
}

app::~app()
{
}

//-----------------------------------------------------------------------------
