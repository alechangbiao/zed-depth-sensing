#include "point_cloud.h"

GLchar *POINTCLOUD_VERTEX_SHADER =
    "#version 330 core\n"
    "layout(location = 0) in vec4 in_VertexRGBA;\n"
    "uniform mat4 u_mvpMatrix;\n"
    "out vec4 b_color;\n"
    "void main() {\n"
    // Decompose the 4th channel of the XYZRGBA buffer to retrieve the color of the point (1float to 4uint)
    "   uint vertexColor = floatBitsToUint(in_VertexRGBA.w); \n"
    "   vec3 clr_int = vec3((vertexColor & uint(0x000000FF)), (vertexColor & uint(0x0000FF00)) >> 8, (vertexColor & uint(0x00FF0000)) >> 16);\n"
    "   b_color = vec4(clr_int.r / 255.0f, clr_int.g / 255.0f, clr_int.b / 255.0f, 1.f);"
    "	gl_Position = u_mvpMatrix * vec4(in_VertexRGBA.xyz, 1);\n"
    "}";

GLchar *POINTCLOUD_FRAGMENT_SHADER =
    "#version 330 core\n"
    "in vec4 b_color;\n"
    "layout(location = 0) out vec4 out_Color;\n"
    "void main() {\n"
    "   out_Color = b_color;\n"
    "}";

PointCloud::PointCloud() : hasNewPCL_(false)
{
}

PointCloud::~PointCloud()
{
    close();
}

void checkError(cudaError_t err)
{
    if (err != cudaSuccess)
        std::cerr << "Error: (" << err << "): " << cudaGetErrorString(err) << std::endl;
}

void PointCloud::close()
{
    if (matGPU_.isInit())
    {
        matGPU_.free();
        checkError(cudaGraphicsUnmapResources(1, &bufferCudaID_, 0));
        glDeleteBuffers(1, &bufferGLID_);
    }
}

// TODO: 有时间的时候 了解或学习一下 cuda的使用流程
void PointCloud::initialize(sl::Resolution res)
{
    glGenBuffers(1, &bufferGLID_);
    glBindBuffer(GL_ARRAY_BUFFER, bufferGLID_);
    glBufferData(GL_ARRAY_BUFFER, res.area() * 4 * sizeof(float), 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    checkError(cudaGraphicsGLRegisterBuffer(&bufferCudaID_, bufferGLID_, cudaGraphicsRegisterFlagsNone));

    shader_ = Shader(POINTCLOUD_VERTEX_SHADER, POINTCLOUD_FRAGMENT_SHADER);
    shMVPMatrixLoc_ = glGetUniformLocation(shader_.getProgramId(), "u_mvpMatrix");

    matGPU_.alloc(res, sl::MAT_TYPE::F32_C4, sl::MEM::GPU);

    checkError(cudaGraphicsMapResources(1, &bufferCudaID_, 0));
    checkError(cudaGraphicsResourceGetMappedPointer((void **)&xyzrgbaMappedBuf_, &numBytes_, bufferCudaID_));
}

void PointCloud::pushNewPC(sl::Mat &matXYZRGBA)
{
    if (matGPU_.isInit())
    {
        matGPU_.setFrom(matXYZRGBA, sl::COPY_TYPE::GPU_GPU);
        hasNewPCL_ = true;
    }
}

void PointCloud::update()
{
    if (hasNewPCL_ && matGPU_.isInit())
    {
        checkError(cudaMemcpy(xyzrgbaMappedBuf_, matGPU_.getPtr<sl::float4>(sl::MEM::GPU), numBytes_, cudaMemcpyDeviceToDevice));
        hasNewPCL_ = false;
    }
}

void PointCloud::draw(const sl::Transform &vp)
{
    if (matGPU_.isInit())
    {
        glUseProgram(shader_.getProgramId());
        glUniformMatrix4fv(shMVPMatrixLoc_, 1, GL_TRUE, vp.m);

        glBindBuffer(GL_ARRAY_BUFFER, bufferGLID_);
        glVertexAttribPointer(Shader::ATTRIB_VERTICES_POS, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(Shader::ATTRIB_VERTICES_POS);

        glDrawArrays(GL_POINTS, 0, matGPU_.getResolution().area());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }
}