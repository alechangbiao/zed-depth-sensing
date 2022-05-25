#pragma once

#include <sl/Camera.hpp>

#include <GL/glew.h>

#include <cuda.h>
#include <cuda_gl_interop.h>

#include "shader.h"

extern GLchar *POINTCLOUD_VERTEX_SHADER;
extern GLchar *POINTCLOUD_FRAGMENT_SHADER;

class PointCloud
{
public:
    PointCloud();
    ~PointCloud();

    // Initialize Opengl and Cuda buffers
    // Warning: must be called in the Opengl thread
    void initialize(sl::Resolution res);
    // Push a new point cloud
    // Warning: can be called from any thread but the mutex "mutexData" must be locked
    void pushNewPC(sl::Mat &matXYZRGBA);
    // Update the Opengl buffer
    // Warning: must be called in the Opengl thread
    void update();
    // Draw the point cloud
    // Warning: must be called in the Opengl thread
    void draw(const sl::Transform &vp);
    // Close (disable update)
    void close();

    std::mutex mutexData;

private:
    sl::Mat matGPU_;
    bool hasNewPCL_ = false;
    Shader shader_;
    GLuint shMVPMatrixLoc_;
    size_t numBytes_;
    float *xyzrgbaMappedBuf_;
    GLuint bufferGLID_;
    cudaGraphicsResource *bufferCudaID_;
};