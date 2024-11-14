#pragma once

class Canvas
{
public:
    Canvas(int width, int height) : width(width), height(height)
    {
        __allocMap();
    }
    ~Canvas()
    {
        __freeMap();
    }
    void resize(int width, int height)
    {
        this->width = width;
        this->height = height;
        __freeMap();
        __allocMap();
    }

    char *getTextureMap() const { return textureMap; }

private:
    void __allocMap()
    {
        textureMap = new char[width * height * 3];
    }
    void __freeMap()
    {
        delete[] textureMap;
    }
    int width, height;
    char *textureMap;
};