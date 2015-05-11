#ifndef CONF_HPP
#define CONF_HPP

#include <string>
#include <map>

#define DEFAULT_CONF "default.conf"

//-----------------------------------------------------------------------------

class conf
{
    std::map<std::string, int>         i_map;
    std::map<std::string, bool>        b_map;
    std::map<std::string, float>       f_map;
    std::map<std::string, std::string> s_map;

    void load(const char *);

public:

    conf(int argc, char *argv[]);

    int         get_i(std::string k) { return i_map[k]; }
    bool        get_b(std::string k) { return b_map[k]; }
    float       get_f(std::string k) { return f_map[k]; }
    std::string get_s(std::string k) { return s_map[k]; }

    void  set_i(std::string k, int         i) { i_map[k] = i; }
    void  set_b(std::string k, bool        b) { b_map[k] = b; }
    void  set_f(std::string k, float       f) { f_map[k] = f; }
    void  set_s(std::string k, std::string s) { s_map[k] = s; }
};

//-----------------------------------------------------------------------------

#endif
