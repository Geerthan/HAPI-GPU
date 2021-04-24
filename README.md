HAPI - The Holography Library

This is a simple library for computing holograms.  It supports multiple algorithms 
the most interesting of which is ray tracing.  The program in the ray directory is a basic 
test of this.  The current system only ray traces spheres, since it is used for testing 
the underlying algorithm.  The HAPI directory contains the library, which is statically 
linked with the test programs.

The two external dependencies are jansson and freeimage, you may need to change project 
settings to correctly link to them.

Each program needs a display.halo file which must be in the same directory as the .exe 
file.  Several sample ones are included.

device.halo - the same as device512.halo
device512.halo - a 512x512 abstract device, good for testing
device6500.halo - device file for the LightCrafter 6500 device
devicelightcrafter.halo - device file for the LightCrafter 3000 device
deviceSquare.halo - a 1024x1024 abstract device, good for testing