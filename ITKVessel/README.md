# README

## Build the project

Create a directory alongside *ITKVessel* named *ITKVessel_build*. In the *ITKVessel_build* directory, run the CMake executable, making sure that the location of the ITK build directory is specified:

    cmake -DITK_DIR=~/ITK/ITKbin ../ITKVessel
    make
