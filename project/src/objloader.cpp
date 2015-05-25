#include "objloader.hpp"
#include "sgct.h"
#include "GLFW/glfw3.h"


/*! Parse differently formated triplets like: n0, n0/n1/n2, n0//n2, n0/n1.          */
/*! All indices are converted to C-style (from 0). Missing entries are assigned -1. */
Vertex ObjReader::getInt3(const char*& token)
{
    Vertex v(-1);
    v.v = fix_v(atoi(token));
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/') return(v);
    token++;

    // it is i//n
    if (token[0] == '/') {
        token++;
        v.vn = fix_vn(atoi(token));
        token += strcspn(token, " \t\r");
        return(v);
    }

    // it is i/t/n or i/t
    v.vt = fix_vt(atoi(token));
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/') return(v);
    token++;

    // it is i/t/n
    v.vn = fix_vn(atoi(token));
    token += strcspn(token, " \t\r");
    return(v);
}

/*! \brief load a OBJ material file
 *  \param mtlFilename is the full path to the material file
 */
void ObjReader::loadMTL(const std::string &mtlFilename)
{
    std::ifstream ifs;
    ifs.open(mtlFilename.c_str());
    if (!ifs.is_open()) {
        std::cerr << "can't open " << mtlFilename << std::endl;
        return;
    }
    std::shared_ptr<Material> mat;
    while (ifs.peek() != EOF) {
        char line[MAX_LINE_LENGTH];
        ifs.getline(line, sizeof(line), '\n');
        const char* token = line + strspn(line, " \t"); // ignore spaces and tabs
        if (token[0] == 0) continue; // ignore empty lines
        if (token[0] == '#') continue; // ignore comments

        if (!strncmp(token, "newmtl", 6)) {
            parseSep(token += 6);
            std::string name(token); printf("Name of the material %s\n", name.c_str());
            materials[name] = mat = std::shared_ptr<Material>(new Material);
            continue;
        }

        if (!mat) throw std::runtime_error("invalid material file: newmtl expected first");

        if (!strncmp(token, "d", 1)) { parseSep(token += 1); mat->d = getFloat(token); continue; }
        if (!strncmp(token, "Ns", 1)) { parseSep(token += 2); mat->Ns = getFloat(token); continue; }
        if (!strncmp(token, "Ni", 1)) { parseSep(token += 2); mat->Ni = getFloat(token); continue; }
        if (!strncmp(token, "Ka", 2)) { parseSep(token += 2); mat->Ka = getVec3f(token); continue; }
        if (!strncmp(token, "Kd", 2)) { parseSep(token += 2); mat->Kd = getVec3f(token); continue; }
        if (!strncmp(token, "Ks", 2)) { parseSep(token += 2); mat->Ks = getVec3f(token); continue; }
    }
    ifs.close();
}

/*! \brief load the geometry defined in an OBJ/Wavefront file
 *  \param filename is the path to the OJB file
 */
ObjReader::ObjReader(const char *filename)
{
    std::ifstream ifs;
    // extract the path from the filename (used to read the material file)
    std::string path = getFilePath(filename);
    try {
        ifs.open(filename);
        if (ifs.fail()) throw std::runtime_error("can't open file " + std::string(filename));

        // create a default material
        std::shared_ptr<Material> defaultMaterial(new Material);
        curMaterial = defaultMaterial;

        char line[MAX_LINE_LENGTH]; // line buffer

        while (ifs.peek() != EOF) // read each line until EOF is found
        {
            ifs.getline(line, sizeof(line), '\n');
            const char* token = line + strspn(line, " \t"); // ignore space and tabs

            if (token[0] == 0) continue; // line is empty, ignore
            // read a vertex
            if (token[0] == 'v' && isSep(token[1])) { v.push_back(getVec3f(token += 2)); continue; }
            // read a normal
            if (!strncmp(token, "vn",  2) && isSep(token[2])) { vn.push_back(getVec3f(token += 3)); continue; }
            // read a texture coordinates
            if (!strncmp(token, "vt",  2) && isSep(token[2])) { vt.push_back(getVec2f(token += 3)); continue; }
            // read a face
            if (token[0] == 'f' && isSep(token[1])) {
                parseSep(token += 1);
                std::vector<Vertex> face;
                while (token[0]) {
                    face.push_back(getInt3(token));
                    parseSepOpt(token);
                }
                curGroup.push_back(face);
                continue;
            }

            /*! use material */
            if (!strncmp(token, "usemtl", 6) && isSep(token[6]))
            {
                flushFaceGroup();
                std::string name(parseSep(token += 6));
                if (materials.find(name) == materials.end()) curMaterial = defaultMaterial;
                else curMaterial = materials[name];
                continue;
            }

            /* load material library */
            if (!strncmp(token, "mtllib", 6) && isSep(token[6])) {
                loadMTL(path + "/" + std::string(parseSep(token += 6)));
                continue;
            }
        }
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    flushFaceGroup(); // flush the last loaded object
    ifs.close();
}


/*! \brief utility function to keep track of the vertex already used while creating a new mesh
 *  \param vertexMap is a map used to keep track of the vertices already inserted in the position list
 *  \param position is a position list for the newly created mesh
 *  \param normals is a normal list for the newly created mesh
 *  \param texcoords is a texture coordinate list for the newly created mesh
 *  \param i is the Vertex looked for or inserted in vertexMap
 *  \return the index of this Vertex in the position vector list.
 */
uint32_t ObjReader::getVertex(
    std::map<Vertex, uint32_t> &vertexMap,
    std::vector<Vec3f> &positions,
    std::vector<Vec3f> &normals,
    std::vector<Vec2f> &texcoords,
    const Vertex &i)
{
    const std::map<Vertex, uint32_t>::iterator& entry = vertexMap.find(i);
    if (entry != vertexMap.end()) return(entry->second);

    positions.push_back(v[i.v]);
    if (i.vn >= 0) normals.push_back(vn[i.vn]);
    if (i.vt >= 0) texcoords.push_back(vt[i.vt]);
    return (vertexMap[i] = int(positions.size()) - 1);
}

/*! \brief flush the current content of currGroup and create new mesh
 */
void ObjReader::flushFaceGroup()
{
    if (curGroup.empty()) return;

    // temporary data arrays
    std::vector<Vec3f> positions;
    std::vector<Vec3f> normals;
    std::vector<Vec2f> texcoords;
    std::vector<Vec3i> triangles;
    std::map<Vertex, uint32_t> vertexMap;

    // merge three indices into one
    for (size_t j = 0; j < curGroup.size(); j++)
    {
        /* iterate over all faces */
        const std::vector<Vertex>& face = curGroup[j];
        Vertex i0 = face[0], i1 = Vertex(-1), i2 = face[1];

        /* triangulate the face with a triangle fan */
        for (size_t k = 2; k < face.size(); k++) {
            i1 = i2; i2 = face[k];
            uint32_t v0 = getVertex(vertexMap, positions, normals, texcoords, i0);
            uint32_t v1 = getVertex(vertexMap, positions, normals, texcoords, i1);
            uint32_t v2 = getVertex(vertexMap, positions, normals, texcoords, i2);
            triangles.push_back(Vec3i(v0, v1, v2));
        }
    }
    curGroup.clear();

    // create new triangle mesh, allocate memory and copy data
    std::shared_ptr<TriangleMesh> mesh = std::shared_ptr<TriangleMesh>(new TriangleMesh);
    mesh->numTriangles = triangles.size();
    mesh->triangles = new int[mesh->numTriangles * 3];
    memcpy(mesh->triangles, &triangles[0], sizeof(Vec3i) * mesh->numTriangles);
    mesh->positions = new Vec3f[positions.size()];
    memcpy(mesh->positions, &positions[0], sizeof(Vec3f) * positions.size());
    if (normals.size()) {
        mesh->normals = new Vec3f[normals.size()];
        memcpy(mesh->normals, &normals[0], sizeof(Vec3f) * normals.size());
    }
    if (texcoords.size()) {
        mesh->texcoords = new Vec2f[texcoords.size()];
        memcpy(mesh->texcoords, &texcoords[0], sizeof(Vec2f) * texcoords.size());
    }
    model.push_back(std::shared_ptr<Primitive>(new Primitive(mesh, curMaterial)));
}
