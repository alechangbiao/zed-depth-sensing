#include "gl_viewer.h"
#include "utils.h"

// Using std and sl namespaces
using namespace std;
using namespace sl;

int main(int argc, char **argv)
{
    Camera zed;
    // Set configuration parameters for the ZED
    InitParameters init_parameters;
    init_parameters.depth_mode = DEPTH_MODE::ULTRA;
    init_parameters.coordinate_system = COORDINATE_SYSTEM::RIGHT_HANDED_Y_UP; // OpenGL's coordinate system is right_handed
    parseArgs(argc, argv, init_parameters);

    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != ERROR_CODE::SUCCESS)
    {
        print("Camera Open", returned_state, "Exit program.");
        return EXIT_FAILURE;
    }

    auto camera_config = zed.getCameraInformation().camera_configuration;

    // Point cloud viewer
    GLViewer viewer;
    // Initialize point cloud viewer
    GLenum errgl = viewer.init(argc, argv, camera_config.calibration_parameters.left_cam);
    if (errgl != GLEW_OK)
    {
        print("Error OpenGL: " + std::string((char *)glewGetErrorString(errgl)));
        return EXIT_FAILURE;
    }

    RuntimeParameters runParameters;
    // Setting the depth confidence parameters
    runParameters.confidence_threshold = 50;
    runParameters.texture_confidence_threshold = 100;

    // Allocation of 4 channels of float on GPU
    Mat point_cloud(camera_config.resolution, MAT_TYPE::F32_C4, MEM::GPU);

    // Main Loop
    while (viewer.isAvailable())
    {
        // Check that a new image is successfully acquired
        if (zed.grab(runParameters) == ERROR_CODE::SUCCESS)
        {
            // retrieve the current 3D coloread point cloud in GPU
            zed.retrieveMeasure(point_cloud, MEASURE::XYZRGBA, MEM::GPU);
            viewer.updatePointCloud(point_cloud);
        }
    }
    // free allocated memory before closing the ZED
    point_cloud.free();

    // close the ZED
    zed.close();

    return EXIT_SUCCESS;
}
