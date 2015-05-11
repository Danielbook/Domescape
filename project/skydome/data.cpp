#include <fstream>

#include "data.hpp"
#include "obj.hpp"

//-----------------------------------------------------------------------------

int data::get_obj(std::string filename)
{
    std::string pathname = path + "/" + filename;

    // Load the named OBJ only if it is not already loaded.

    if (obj_map.find(filename) == obj_map.end())
        obj_map[filename] = obj_add_file(pathname.c_str());

    return obj_map[filename];
}

std::string data::get_txt(std::string filename)
{
    std::string pathname = path + "/" + filename;

    // Load the named text file only if it is not already loaded.

    if (txt_map.find(filename) == txt_map.end())
    {
        std::ifstream file(pathname.c_str());
        std::string   line;

        while (std::getline(file, line))
            txt_map[filename] += line + "\n";
    }

    return txt_map[filename];
}

void *data::get_img(std::string filename, int& w, int& h, int& b)
{
    std::string pathname = path + "/" + filename;
    img I;

    // Load the named image only if it is not already loaded.

    if (img_map.find(filename) == img_map.end())
    {
        I.p = obj_read_image(pathname.c_str(), &I.w, &I.h, &I.b);
        img_map[filename] = I;
    }

    w    = img_map[filename].w;
    h    = img_map[filename].h;
    b    = img_map[filename].b;
    return img_map[filename].p;
}

//-----------------------------------------------------------------------------

data::~data()
{
    std::map<std::string, int>::iterator i;
    std::map<std::string, img>::iterator j;

    // Delete all OBJ structures.

    while (obj_num_file() > 0)
        obj_del_file(0);

    // Release all image buffers.

    for (j = img_map.begin(); j != img_map.end(); ++j)
        free(j->second.p);
}

//-----------------------------------------------------------------------------

