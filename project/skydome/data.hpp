#ifndef DATA_HPP
#define DATA_HPP

#include <string>
#include <map>

//-----------------------------------------------------------------------------

class data
{
    struct img
    {
        void *p;
        int   w;
        int   h;
        int   b;
    };

    std::string path;

    std::map<std::string, int>         obj_map;
    std::map<std::string, std::string> txt_map;
    std::map<std::string, img>         img_map;

public:

    data(std::string p) : path(p) { }
   ~data();

    int         get_obj(std::string);
    std::string get_txt(std::string);
    void       *get_img(std::string, int&, int&, int&);
};

//-----------------------------------------------------------------------------

#endif
