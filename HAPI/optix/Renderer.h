#pragma once
#define NOMINMAX
#include <optix.h>
#include <optixu/optixu_matrix.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sutil/sutil.h>
#include <fstream>
#include <string>
#include <iostream>
#include <map>

#include "ProgramCreator.h"
#include "GeometryGenerator.h"
class Renderer {
private:
	int WIDTH;
	int HEIGHT;
	optix::Context context;



public:
	Renderer(int width, int height, int rayCount, int entryPoints, int stackSize);

	/*
	Create a buffer that can be used to read or write from the GPU
	*/
	void createBuffer(std::string bufferName);
	
	/*
	Render the scene
	*/
	void render(int entryPoints);

	/*
	Display the data from the output buffer to the screen
	*/
	void display(int * argc, char * argv[], std::string outputBufferName);
	void cleanUp();
	optix::Context getContext();
};