#pragma once

class GLTexture
{
public:
    GLTexture();
    ~GLTexture();

    void loadTexture(const char *filename);
    void bind();
    void unbind();

    unsigned int getID() { return textureID; }

private:
    unsigned int textureID;
    int width, height, nrChannels;
    unsigned char *data;
};