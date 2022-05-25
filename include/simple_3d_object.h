#pragma once

#include <sl/Camera.hpp>
#include <GL/glew.h>
#include "shader.h"

class Simple3DObject
{
public:
    Simple3DObject() {}
    Simple3DObject(sl::Translation position, bool isStatic);
    ~Simple3DObject();

    void addPoint(sl::float3 pt, sl::float3 clr);
    // 模拟相机视角的椎头框
    void addFace(sl::float3 p1, sl::float3 p2, sl::float3 p3, sl::float3 clr);
    void pushToGPU();
    void clear();

    void setDrawingType(GLenum type);

    void draw();

    void translate(const sl::Translation &t);
    void setPosition(const sl::Translation &p);

    void setRT(const sl::Transform &mRT);

    void rotate(const sl::Orientation &rot);
    void rotate(const sl::Rotation &m);
    void setRotation(const sl::Orientation &rot);
    void setRotation(const sl::Rotation &m);

    const sl::Translation &getPosition() const;

    sl::Transform getModelMatrix() const;

private:
    std::vector<float> vertices_;
    std::vector<float> colors_;
    std::vector<unsigned int> indices_;

    bool isStatic_;

    GLenum drawingType_;

    GLuint vaoID_;
    /*
            Vertex buffer IDs:
            - [0]: Vertices coordinates;
            - [1]: Vertices colors;
            - [2]: Indices;
     */
    GLuint vboID_[3];

    sl::Translation position_;
    sl::Orientation rotation_;
};