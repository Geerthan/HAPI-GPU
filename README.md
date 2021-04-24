# HAPI - The Holography Library

This is a simple library for computing holograms.  It supports multiple algorithms 
the most interesting of which is ray tracing.  The program in the ray directory is a basic 
test of this.  The current system only ray traces spheres, since it is used for testing 
the underlying algorithm.  The HAPI directory contains the library, which is statically 
linked with the test programs.

The two external dependencies are jansson and freeimage, you may need to change project 
settings to correctly link to them.

Each program needs a display.halo file which must be in the same directory as the .exe 
file.  Several sample ones are included.

device.halo - the same as device512.halo\
device512.halo - a 512x512 abstract device, good for testing\
device6500.halo - device file for the LightCrafter 6500 device\
devicelightcrafter.halo - device file for the LightCrafter 3000 device\
deviceSquare.halo - a 1024x1024 abstract device, good for testing

# Triangle Mesh Framework

This specific version of HAPI builds on the last version of HAPI-GPU, using GPU acceleration
in order to create a framework for triangle mesh renders.

## Instructions

The Visual Studio project requires a few pre-requisites in order to work. You need to download
and link CUDA 11.1 as well as OptiX 7.2 (newer versions may work for both). Additionally, 
the CUDA files need to be built using the NVCC compiler outside of Visual Studio. NVCC 
is packaged with CUDA and should be added to your path. It is then possible to use the build 
scripts for the tmesh and sphere programs, if you edit the scripts to contain the correct 
Visual Studio and OptiX locations. The build scripts are the .bat files located in the 
HAPI subfolder. 