#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "simple_3d_object.h"
#include "point_cloud.h"
// #include "shader.h"
#include "camera_gl.h"

#ifndef M_PI
#define M_PI 3.141592653f
#endif

#define MOUSE_R_SENSITIVITY 0.015f
#define MOUSE_UZ_SENSITIVITY 0.75f
#define MOUSE_DZ_SENSITIVITY 1.25f
#define MOUSE_T_SENSITIVITY 10.f
#define KEY_T_SENSITIVITY 0.1f

/**
 * @brief This class manages input events, window and Opengl rendering pipeline
 */
class GLViewer
{
public:
    GLViewer();
    ~GLViewer();
    bool isAvailable();

    GLenum init(int argc, char **argv, sl::CameraParameters param);
    void updatePointCloud(sl::Mat &matXYZRGBA);

    void exit();

private:
    // Rendering loop method called each frame by glutDisplayFunc
    void render();
    // Everything that needs to be updated before rendering must be done in this method
    void update();
    // Once everything is updated, every renderable objects must be drawn in this method
    void draw();
    // Clear and refresh inputs' data
    void clearInputs();

    // Glut functions callbacks
    static void drawCallback();
    static void mouseButtonCallback(int button, int state, int x, int y);
    static void mouseMotionCallback(int x, int y);
    // 改变 Window 大小时候的回调
    static void reshapeCallback(int width, int height);
    static void keyPressedCallback(unsigned char c, int x, int y);
    static void keyReleasedCallback(unsigned char c, int x, int y);
    static void idle();

    bool available;

    enum MOUSE_BUTTON
    {
        LEFT = 0,
        MIDDLE = 1,
        RIGHT = 2,
        WHEEL_UP = 3,
        WHEEL_DOWN = 4
    };

    enum KEY_STATE
    {
        UP = 'u',
        DOWN = 'd',
        FREE = 'f'
    };

    bool mouseButton_[3];
    int mouseWheelPosition_;
    int mouseCurrentPosition_[2];
    int mouseMotion_[2];
    int previousMouseMotion_[2];
    KEY_STATE keyStates_[256];
    sl::float3 bckgrnd_clr;

    Simple3DObject frustum;
    PointCloud pointCloud_;
    CameraGL camera_;
    Shader shader_;
    GLuint shMVPMatrixLoc_;
};