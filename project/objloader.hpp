#ifndef __Domescape__objloader__
#define __Domescape__objloader__

/*!
    \file objloader.cpp
    \brief load an OBJ file and store its geometry/material in memory

    This code was adapted from the project Embree from Intel.
    Copyright 2009-2012 Intel Corporation

    Compile with: clang++/c++ -o objloader objloader.cpp -O3 -Wall -std=c++0x
 */


#include <vector>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <map>

#include "sgct.h"
#include "GLFW/glfw3.h"

#if defined(__APPLE__)
#include <pthread.h>
#include <tr1/memory>

namespace std
{
using std::tr1::shared_ptr;
}
#endif

#if defined(__linux__)
#include <pthread.h>
#include <memory>
#endif

#define MAX_LINE_LENGTH 10000


template<typename T>
class Vec2
{
public:
    T x, y;
    Vec2() : x(0), y(0) {}
    Vec2(T xx, T yy) : x(xx), y(yy) {}
};

template<typename T>
class Vec3
{
public:
    T x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
    {
        os << v.x << ", " << v.y << ", " << v.z;
        return os;
    }
};

typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;
typedef Vec2<float> Vec2f;

Vec3f getVec3(std::ifstream &ifs)
{
    float x, y, z;
    ifs >> x >> y >> z;
    return Vec3f(x, y, z);
}

/*! returns the path of a file */
std::string getFilePath(const std::string &filename)
{
    size_t pos = filename.find_last_of('/');
    if (pos == std::string::npos) return filename;
    return filename.substr(0, pos);
}

/*! \struct Material
 *  \brief a simple structure to store material's properties
 */
struct Material
{
    Vec3f Ka, Kd, Ks;   /*! ambient, diffuse and specular rgb coefficients */
    float d;            /*! transparency */
    float Ns, Ni;       /*! specular exponent and index of refraction */
};

/*! \class TriangleMesh
 *  \brief a basic class to store a triangle mesh data
 */
class TriangleMesh
{
public:
    Vec3f *positions;   /*! position/vertex array */
    Vec3f *normals;     /*! normal array (can be null) */
    Vec2f *texcoords;   /*! texture coordinates (can be null) */
    int numTriangles;   /*! number of triangles */
    int *triangles;     /*! triangle index list */
    TriangleMesh() : positions(nullptr), normals(nullptr), texcoords(nullptr), triangles(nullptr) {}
    ~TriangleMesh()
    {
        if (positions) delete [] positions;
        if (normals) delete [] normals;
        if (texcoords) delete [] texcoords;
        if (triangles) delete [] triangles;
    }
};

/*! \class Primitive
 *  \brief a basic class to store a primitive (defined by a mesh and a material)
 */
struct Primitive
{
    Primitive(const std::shared_ptr<TriangleMesh> &m, const std::shared_ptr<Material> &mat) :
        mesh(m), material(mat) {}
    const std::shared_ptr<TriangleMesh> mesh;   /*! the object's geometry */
    const std::shared_ptr<Material> material;   /*! the object's material */
};

/*! Three-index vertex, indexing start at 0, -1 means invalid vertex. */
struct Vertex
{
    int v, vt, vn;
    Vertex() {};
    Vertex(int v) : v(v), vt(v), vn(v) {};
    Vertex(int v, int vt, int vn) : v(v), vt(vt), vn(vn) {};
};

// need to declare this operator if we want to use Vertex in a map
static bool operator < ( const Vertex& a, const Vertex& b )
{
    if (a.v  != b.v)  return a.v  < b.v;
    if (a.vn != b.vn) return a.vn < b.vn;
    if (a.vt != b.vt) return a.vt < b.vt;
    return false;
}

/*! Parse separator. */
static const char* parseSep(const char*& token)
{
    size_t sep = strspn(token, " \t");
    if (!sep) throw std::runtime_error("separator expected");
    return token+=sep;
}

/*! Read float from a string. */
static float getFloat(const char*& token)
{
    token += strspn(token, " \t");
    float n = (float)atof(token);
    token += strcspn(token, " \t\r");
    return n;
}

/*! Read Vec2f from a string. */
static Vec2f getVec2f(const char*& token)
{
    float x = getFloat(token);
    float y = getFloat(token);
    return Vec2f(x,y);
}

/*! Read Vec3f from a string. */
static Vec3f getVec3f(const char*& token)
{
    float x = getFloat(token);
    float y = getFloat(token);
    float z = getFloat(token);
    return Vec3f(x, y, z);
}

/*! Parse optional separator. */
static const char* parseSepOpt(const char*& token)
{
    return token+=strspn(token, " \t");
}

/*! Determine if character is a separator. */
static bool isSep(const char c)
{
    return (c == ' ') || (c == '\t');
}


class ObjReader
{
public:
    ObjReader(const char *filename);
    Vertex getInt3(const char*& token);
    int fix_v(int index)
    {
        return(index > 0 ? index - 1 : (index == 0 ? 0 : (int)v .size() + index));
    }
    int fix_vt(int index)
    {
        return(index > 0 ? index - 1 : (index == 0 ? 0 : (int)vt.size() + index));
    }
    int fix_vn(int index)
    {
        return(index > 0 ? index - 1 : (index == 0 ? 0 : (int)vn.size() + index));
    }
    std::vector<Vec3f> v, vn;
    std::vector<Vec2f> vt;
    std::vector<std::vector<Vertex> > curGroup;
    std::map<std::string, std::shared_ptr<Material> > materials;
    std::shared_ptr<Material> curMaterial;
    void loadMTL(const std::string &mtlFilename);
    void flushFaceGroup();
    uint32_t getVertex(std::map<Vertex, uint32_t>&, std::vector<Vec3f>&, std::vector<Vec3f>&, std::vector<Vec2f>&, const Vertex&);
    std::vector<std::shared_ptr<Primitive> > model;
};


#endif /* defined(__Domescape__objloader__) */
