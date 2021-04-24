#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <map>

#include <optix.h>
#include <optixpp_namespace.h>
class ProgramCreator {
private:
	optix::Context context;
	std::string readFile(std::string fileName);
	std::map<std::string, optix::Program> programs;
public:
	ProgramCreator();
	ProgramCreator(optix::Context context);

	/*The follow reads in a PTX program and creates a corresponding program*/
	optix::Program createProgram(std::string fileName, std::string programName);
	void createRaygenProgram(std::string fileName, std::string programName, int entryPt);
	void createExceptionProgram(std::string fileName, std::string programName, int entryPt);
	void createMissProgram(std::string fileName, std::string programName, int entryPt);

	/*Sets a variable in the given program. Very similiar to Uniform variables in OpenGL*/
	void createProgramVariable3f(std::string programName, std::string variableName, float x, float y, float z);
	void createProgramVariable1i(std::string programName, std::string variableName, int x);
};