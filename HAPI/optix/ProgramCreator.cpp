#include "ProgramCreator.h"


ProgramCreator::ProgramCreator() {}

ProgramCreator::ProgramCreator(optix::Context context) {
	this->context = context;
}

std::string ProgramCreator::readFile(std::string fileName){
	std::string ptxString;
	std::ifstream ptxFile(fileName);

	std::string line;
	if (ptxFile.is_open()) {
		std::cout << "Accesed file: " << fileName << std::endl;
		while (std::getline(ptxFile, line)) {
			ptxString += line;
			ptxString += '\n';
		}
		ptxFile.close();
	}
	else {
		std::cout << "ERROR: Cannot open file! File Name: " << fileName << std::endl;
	}
	return ptxString;
}

optix::Program ProgramCreator::createProgram(std::string fileName, std::string programName){
	const std::string ptxString = readFile(fileName);
	const char* ptx = ptxString.c_str();
	optix::Program program;
	try {
		program = context->createProgramFromPTXString(ptx, programName);
	}
	catch (optix::Exception& e) {
		std::cout << e.getErrorString() << std::endl;
		
	}
	return program;
}

void ProgramCreator::createRaygenProgram(std::string fileName, std::string programName, int entryPt){
	optix::Program program = createProgram(fileName, programName);

	context->setRayGenerationProgram(entryPt, program);
}

void ProgramCreator::createExceptionProgram(std::string fileName, std::string programName, int entryPt){
	optix::Program program = createProgram(fileName, programName);
	context->setExceptionProgram(entryPt, program);
}

void ProgramCreator::createMissProgram(std::string fileName, std::string programName, int entryPt){
	optix::Program program = createProgram(fileName, programName);
	context->setMissProgram(entryPt, program);
}

void ProgramCreator::createProgramVariable3f(std::string programName, std::string variableName, float x, float y, float z){
	context[variableName]->setFloat(x, y, z);
}

void ProgramCreator::createProgramVariable1i(std::string programName, std::string variableName, int x) {
	context[variableName]->setInt(x);
}
