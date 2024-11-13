#include <obj_loader/obj_loader.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

OBJLoader::OBJLoader(const char *filename) : obj(std::make_unique<OBJ>())
{
    file_name = filename;
    __loadFile();
    __parse();
}

OBJLoader::~OBJLoader()
{
}

void OBJLoader::__loadFile()
{
    std::cerr << "Loading file: " << file_name;
    std::ifstream file(file_name);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << file_name << std::endl;
        exit(1);
    }
    // read file content
    std::stringstream ss;
    ss << file.rdbuf();
    file_content = ss.str();
    file.close();
    std::cerr << " - Done" << std::endl;
}

void OBJLoader::__parse()
{
    std::cerr << "Parsing file: " << file_name;
    // set file name
    size_t pos = file_name.find_last_of('/');
    if (pos != std::string::npos)
        file_name = file_name.substr(pos + 1);
    obj->setFileName(file_name);

    // set file content
    std::stringstream ss(file_content);
    std::string line;
    while (std::getline(ss, line))
    {
        switch (line[0])
        {
        case 'v':
            if (line[1] == ' ')
            {
                Vertex v = __parseVertex(line);
                obj->addVertex(v);
            }
            else if (line[1] == 'n')
            {
                Normal n = __parseNormal(line);
                obj->addNormal(n);
            }
            else if (line[1] == 't')
            {
                TexutreUV uv = __parseUV(line);
                obj->addUV(uv);
            }
            break;
        case 'f':
            Face f = __parseFace(line);
            obj->addFace(f);
            break;
        default:
            break;
        }
    }
    std::cerr << " - Done" << std::endl;
}

Vertex OBJLoader::__parseVertex(const std::string &line)
{
    std::stringstream ss(line);
    char c;
    Float32 x, y, z;
    ss >> c >> x >> y >> z;
    return Vertex(x, y, z);
}

Normal OBJLoader::__parseNormal(const std::string &line)
{
    std::stringstream ss(line);
    char c;
    Float32 nx, ny, nz;
    ss >> c >> c >> nx >> ny >> nz;
    return Normal(nx, ny, nz);
}

TexutreUV OBJLoader::__parseUV(const std::string &line)
{
    std::stringstream ss(line);
    char c;
    Float32 u, v;
    ss >> c >> c >> u >> v;
    return TexutreUV(u, v);
}

Face OBJLoader::__parseFace(const std::string &line)
{
    std::stringstream ss(line);
    char c;
    VertexIndex v1(-1), v2(-1), v3(-1);
    TextureUVIndex uv1(-1), uv2(-1), uv3(-1);
    NormalIndex n1(-1), n2(-1), n3(-1);
    ss >> c >> c;
    // v1//n1 or v1/vt1/vn1 or v1/vt1 or v1
    ss >> v1;
    if (ss.peek() == '/')
    {
        ss >> c;
        if (ss.peek() != '/')
        {
            ss >> uv1;
        }
        if (ss.peek() == '/')
        {
            ss >> c;
            ss >> n1;
        }
    }
    // v2//n2 or v2/vt2/vn2 or v2/vt2 or v2
    ss >> v2;
    if (ss.peek() == '/')
    {
        ss >> c;
        if (ss.peek() != '/')
        {
            ss >> uv2;
        }
        if (ss.peek() == '/')
        {
            ss >> c;
            ss >> n2;
        }
    }
    // v3//n3 or v3/vt3/vn3 or v3/vt3 or v3
    ss >> v3;
    if (ss.peek() == '/')
    {
        ss >> c;
        if (ss.peek() != '/')
        {
            ss >> uv3;
        }
        if (ss.peek() == '/')
        {
            ss >> c;
            ss >> n3;
        }
    }
    return Face{v1, v2, v3, uv1, uv2, uv3, n1, n2, n3};
}