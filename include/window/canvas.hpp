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

    void clearTextureMap()
    {
        memset(textureMap, 0, width * height * 3);
    }

    void setRandomTextureMap()
    {
        for (int i = 0; i < width * height * 3; i++)
            textureMap[i] = rand() % 256;
    }

    char *getTextureMap() const { return textureMap; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

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