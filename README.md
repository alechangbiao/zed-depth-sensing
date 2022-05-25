# ZED SDK - Depth Sensing

This sample shows how to retreive the current point cloud.

## Getting Started

- Get the latest [ZED SDK](https://www.stereolabs.com/developers/release/)
- Check the [Documentation](https://www.stereolabs.com/docs/)

## Build the program

```
mkdir build && cd build
cmake ..
make
```

## Run the program

- Navigate to the build directory and launch the executable
- Or open a terminal in the build directory and run the sample :

      ./ZED_Depth_Sensing

### Features

- Camera live point cloud is retreived
- An OpenGL windows displays it in 3D

## Study path

utils -> cameral_gl -> shader -> simple_3d_object -> point_cloud -> gl_viewer -> main

## Summarize

Simple3DObject -> Model Class
CameraGL -> Camera Projection Class
