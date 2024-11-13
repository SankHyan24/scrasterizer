#pragma once
// TODO: Implement Camera class
class Camera
{
public:
    Camera() {}
    ~Camera() {}

private:
    float x, y, z;
    float pitch, yaw, roll;
    float fov;
    float aspect;
    float near, far;
};