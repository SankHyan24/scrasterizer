#pragma once
#include <memory>
#include <string>

#include <obj_loader/objtype.hpp>
#include <obj_loader/obj.hpp>

// paser obj file
// can generate obj class
class OBJLoader
{
public:
    OBJLoader(const char *filename);
    ~OBJLoader();
    std::unique_ptr<OBJ> getOBJ() { return std::move(obj); }

private:
    void __loadFile();
    void __parse();
    Vertex __parseVertex(const std::string &line);
    Normal __parseNormal(const std::string &line);
    TexutreUV __parseUV(const std::string &line);
    Face __parseFace(const std::string &line);
    std::string file_name;
    std::string file_content;
    std::unique_ptr<OBJ> obj;

    // utils
    void __autoAddNormal();
};