#include <fstream>
#include <sstream>

#include "conf.hpp"

//-----------------------------------------------------------------------------

void conf::load(const char *filename)
{
    std::ifstream file(filename);
    std::string   line;

    // Iterate over all lines of the named file.

    while (std::getline(file, line))
    {
        // Consider each non-empty, non-comment line.

        if (line.length() > 0 && line[0] != '#')
        {
            std::istringstream item(line);
            std::string type;
            std::string key;

            int         i;
            bool        b;
            float       f;
            std::string s;
            
            // Parse type, keyword, and value.

            item >> type >> key;

            if ((type == "int")    && (item >> i)) set_i(key, i);
            if ((type == "bool")   && (item >> b)) set_b(key, b);
            if ((type == "float")  && (item >> f)) set_f(key, f);
            if ((type == "string") && (item >> s)) set_s(key, s);
        }
    }       
}

conf::conf(int argc, char *argv[])
{
    int argi;

    // Load the default and all named config files.

    load(DEFAULT_CONF);

    for (argi = 1; argi < argc; ++argi)
        load(argv[argi]);
}

//-----------------------------------------------------------------------------
