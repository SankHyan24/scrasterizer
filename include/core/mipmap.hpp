#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>

#include <obj_loader/objtype.hpp>
struct MipMapOneLvl
{
    int width, height;
    float *data;
    float &operator()(int x, int y)
    {
        return data[y * width + x];
    }
    bool *mask;
};
class MipMap
{
public:
    int width, height;
    std::vector<MipMapOneLvl> mipMap;
    MipMap(int width, int height, float initValue = -std::numeric_limits<float>::infinity()) : width(width), height(height)
    {
        int maxLevel = log2(Max2(width, height));
        mipMap.resize(maxLevel);
        for (int i = 0; i < maxLevel; i++)
        {
            mipMap[i].width = Max2(1, width >> (i + 1));
            mipMap[i].height = Max2(1, height >> (i + 1));
            mipMap[i].data = new float[mipMap[i].width * mipMap[i].height];
            mipMap[i].mask = new bool[mipMap[i].width * mipMap[i].height];
            std::fill(mipMap[i].data, mipMap[i].data + mipMap[i].width * mipMap[i].height, initValue);
            std::fill(mipMap[i].mask, mipMap[i].mask + mipMap[i].width * mipMap[i].height, false);
        }
    }
    ~MipMap()
    {
        for (int i = 0; i < mipMap.size(); i++)
            delete[] mipMap[i].data;
    }

    void getMipMapBB(int level, int pixelX, int pixelY, Uint2 &bbMin, Uint2 &bbMax)
    {
        bbMin = Uint2(Max2(0, pixelX >> (level + 1)), Max2(0, pixelY >> (level + 1)));
        bbMax = Uint2(Min2(mipMap[level].width - 1, pixelX >> level), Min2(mipMap[level].height - 1, pixelY >> level));
    }

    void findMinBBPixel(Uint2 bbMin, Uint2 bbMax, int &lvl, int &pixelX, int &pixelY)
    {
        for (int i = mipMap.size() - 1; i >= 0; i--)
        {
            Uint2 bbMinTmp, bbMaxTmp;
            for (int j = bbMin.y; j <= bbMax.y; j++)
                for (int k = bbMin.x; k <= bbMax.x; k++)
                {
                    getMipMapBB(i, k, j, bbMinTmp, bbMaxTmp);
                    if (bbMaxTmp.x >= bbMin.x && bbMaxTmp.y >= bbMin.y && bbMinTmp.x <= bbMax.x && bbMinTmp.y <= bbMax.y)
                    {
                        lvl = i;
                        pixelX = k;
                        pixelY = j;
                        std::cout << "bbMin: " << bbMin.x << " " << bbMin.y << " bbMax: " << bbMax.x << " " << bbMax.y << std::endl;
                        std::cout << "findMinBBPixel: " << lvl << " " << pixelX << " " << pixelY << std::endl;

                        return;
                    }
                }
        }
        assert(false && "findMinBBPixel: not found");
    }
};