#include "gl_viewer.h"

Simple3DObject createFrustum(sl::CameraParameters param)
{

    // Create 3D axis
    Simple3DObject it(sl::Translation(0, 0, 0), true);

    float Z_ = -150;
    sl::float3 cam_0(0, 0, 0);
    sl::float3 cam_1, cam_2, cam_3, cam_4;

    float fx_ = 1.f / param.fx;
    float fy_ = 1.f / param.fy;

    cam_1.z = Z_;
    cam_1.x = (0 - param.cx) * Z_ * fx_;
    cam_1.y = (0 - param.cy) * Z_ * fy_;

    cam_2.z = Z_;
    cam_2.x = (param.image_size.width - param.cx) * Z_ * fx_;
    cam_2.y = (0 - param.cy) * Z_ * fy_;

    cam_3.z = Z_;
    cam_3.x = (param.image_size.width - param.cx) * Z_ * fx_;
    cam_3.y = (param.image_size.height - param.cy) * Z_ * fy_;

    cam_4.z = Z_;
    cam_4.x = (0 - param.cx) * Z_ * fx_;
    cam_4.y = (param.image_size.height - param.cy) * Z_ * fy_;

    sl::float3 clr(0.2f, 0.5f, 0.8f);

    it.addFace(cam_0, cam_1, cam_2, clr);
    it.addFace(cam_0, cam_2, cam_3, clr);
    it.addFace(cam_0, cam_3, cam_4, clr);
    it.addFace(cam_0, cam_4, cam_1, clr);

    it.setDrawingType(GL_TRIANGLES);
    return it;
}

GLViewer *currentInstance_ = nullptr;

GLViewer::GLViewer() : available(false)
{
    currentInstance_ = this;
    mouseButton_[0] = mouseButton_[1] = mouseButton_[2] = false;
    clearInputs();
    previousMouseMotion_[0] = previousMouseMotion_[1] = 0;
}

GLViewer::~GLViewer() {}

void GLViewer::exit()
{
    if (currentInstance_)
    {
        // pointCloud_.close();
        available = false;
    }
}

bool GLViewer::isAvailable()
{
    if (available)
        glutMainLoopEvent();
    return available;
}

void CloseFunc(void)
{
    if (currentInstance_)
        currentInstance_->exit();
}

GLenum GLViewer::init(int argc, char **argv, sl::CameraParameters param)
{

    glutInit(&argc, argv);
    int wnd_w = glutGet(GLUT_SCREEN_WIDTH);
    int wnd_h = glutGet(GLUT_SCREEN_HEIGHT) * 0.9;
    glutInitWindowSize(wnd_w * 0.9, wnd_h * 0.9);
    glutInitWindowPosition(wnd_w * 0.05, wnd_h * 0.05);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("ZED Depth Sensing");

    GLenum err = glewInit();
    if (GLEW_OK != err)
        return err;

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    glEnable(GL_DEPTH_TEST);

    pointCloud_.initialize(param.image_size);

    // Compile and create the shader
    shader_ = Shader(VERTEX_SHADER, FRAGMENT_SHADER);
    shMVPMatrixLoc_ = glGetUniformLocation(shader_.getProgramId(), "u_mvpMatrix");

    // Create the camera
    camera_ = CameraGL(sl::Translation(0, 0, 0), sl::Translation(0, 0, -100));
    camera_.setOffsetFromPosition(sl::Translation(0, 0, 5000));

    frustum = createFrustum(param);
    frustum.pushToGPU();

    bckgrnd_clr = sl::float3(223, 230, 233);
    bckgrnd_clr /= 255.f;

    // Map glut function on this class methods
    glutDisplayFunc(GLViewer::drawCallback);
    glutMouseFunc(GLViewer::mouseButtonCallback);
    glutMotionFunc(GLViewer::mouseMotionCallback);
    glutReshapeFunc(GLViewer::reshapeCallback);
    glutKeyboardFunc(GLViewer::keyPressedCallback);
    glutKeyboardUpFunc(GLViewer::keyReleasedCallback);
    glutCloseFunc(CloseFunc);

    available = true;
    return err;
}

void GLViewer::render()
{
    if (available)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(bckgrnd_clr.r, bckgrnd_clr.g, bckgrnd_clr.b, 1.f);
        glLineWidth(2.f);
        glPointSize(1.f);
        update();
        draw();
        glutSwapBuffers();
        glutPostRedisplay();
    }
}

void GLViewer::updatePointCloud(sl::Mat &matXYZRGBA)
{
    pointCloud_.mutexData.lock();
    pointCloud_.pushNewPC(matXYZRGBA);
    pointCloud_.mutexData.unlock();
}

void GLViewer::update()
{
    if (keyStates_['q'] == KEY_STATE::UP || keyStates_['Q'] == KEY_STATE::UP || keyStates_[27] == KEY_STATE::UP)
    {
        pointCloud_.close();
        currentInstance_->exit();
        return;
    }

    // Rotate camera with mouse
    if (mouseButton_[MOUSE_BUTTON::LEFT])
    {
        camera_.rotate(sl::Rotation((float)mouseMotion_[1] * MOUSE_R_SENSITIVITY, camera_.getRight()));
        camera_.rotate(sl::Rotation((float)mouseMotion_[0] * MOUSE_R_SENSITIVITY, camera_.getVertical() * -1.f));
    }

    // Translate camera with mouse
    if (mouseButton_[MOUSE_BUTTON::RIGHT])
    {
        camera_.translate(camera_.getUp() * (float)mouseMotion_[1] * MOUSE_T_SENSITIVITY);
        camera_.translate(camera_.getRight() * (float)mouseMotion_[0] * MOUSE_T_SENSITIVITY);
    }

    // Zoom in with mouse wheel
    if (mouseWheelPosition_ != 0)
    {
        float distance = sl::Translation(camera_.getOffsetFromPosition()).norm();
        if (mouseWheelPosition_ > 0 && distance > camera_.getZNear())
        { // zoom
            camera_.setOffsetFromPosition(camera_.getOffsetFromPosition() * MOUSE_UZ_SENSITIVITY);
        }
        else if (distance < camera_.getZFar())
        { // unzoom
            camera_.setOffsetFromPosition(camera_.getOffsetFromPosition() * MOUSE_DZ_SENSITIVITY);
        }
    }

    // Update point cloud buffers
    pointCloud_.mutexData.lock();
    pointCloud_.update();
    pointCloud_.mutexData.unlock();
    camera_.update();
    clearInputs();
}

void GLViewer::draw()
{
    const sl::Transform vpMatrix = camera_.getViewProjectionMatrix();

    // Simple 3D shader for simple 3D objects
    glUseProgram(shader_.getProgramId());

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Axis
    glUniformMatrix4fv(shMVPMatrixLoc_, 1, GL_FALSE, sl::Transform::transpose(vpMatrix * frustum.getModelMatrix()).m);
    frustum.draw();
    glUseProgram(0);

    // Draw point cloud with its own shader
    pointCloud_.draw(vpMatrix);
}

void GLViewer::clearInputs()
{
    mouseMotion_[0] = mouseMotion_[1] = 0;
    mouseWheelPosition_ = 0;
    for (unsigned int i = 0; i < 256; ++i)
        if (keyStates_[i] != KEY_STATE::DOWN)
            keyStates_[i] = KEY_STATE::FREE;
}

void GLViewer::drawCallback()
{
    currentInstance_->render();
}

void GLViewer::mouseButtonCallback(int button, int state, int x, int y)
{
    if (button < 5)
    {
        if (button < 3)
        {
            currentInstance_->mouseButton_[button] = state == GLUT_DOWN;
        }
        else
        {
            currentInstance_->mouseWheelPosition_ += button == MOUSE_BUTTON::WHEEL_UP ? 1 : -1;
        }
        currentInstance_->mouseCurrentPosition_[0] = x;
        currentInstance_->mouseCurrentPosition_[1] = y;
        currentInstance_->previousMouseMotion_[0] = x;
        currentInstance_->previousMouseMotion_[1] = y;
    }
}

void GLViewer::mouseMotionCallback(int x, int y)
{
    currentInstance_->mouseMotion_[0] = x - currentInstance_->previousMouseMotion_[0];
    currentInstance_->mouseMotion_[1] = y - currentInstance_->previousMouseMotion_[1];
    currentInstance_->previousMouseMotion_[0] = x;
    currentInstance_->previousMouseMotion_[1] = y;
    glutPostRedisplay();
}

void GLViewer::reshapeCallback(int width, int height)
{
    glViewport(0, 0, width, height);
    float hfov = (180.0f / M_PI) * (2.0f * atan(width / (2.0f * 500)));
    float vfov = (180.0f / M_PI) * (2.0f * atan(height / (2.0f * 500)));
    currentInstance_->camera_.setProjection(hfov, vfov, currentInstance_->camera_.getZNear(), currentInstance_->camera_.getZFar());
}

void GLViewer::keyPressedCallback(unsigned char c, int x, int y)
{
    currentInstance_->keyStates_[c] = KEY_STATE::DOWN;
    glutPostRedisplay();
}

void GLViewer::keyReleasedCallback(unsigned char c, int x, int y)
{
    currentInstance_->keyStates_[c] = KEY_STATE::UP;
}

void GLViewer::idle()
{
    glutPostRedisplay();
}
