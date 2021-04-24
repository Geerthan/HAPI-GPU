#include "SharedRenderer.h"

std::string getPtx(std::string ptx_name) {

	std::string ptx_string;
	std::ifstream ptx_ifstream("../HAPI/" + ptx_name);

	std::string line;
	if (ptx_ifstream.is_open()) {
		std::cout << "Accessed file: " << ptx_name << std::endl;
		while (std::getline(ptx_ifstream, line)) {
			ptx_string += line;
			ptx_string += '\n';
		}
		ptx_ifstream.close();
	}
	else {
		std::cout << "ERROR: Cannot open file! Filename: " << ptx_name << std::endl;
	}
	return ptx_string;
}

void optix_logger(unsigned int level, const char* tag, const char* message, void* cbdata) {
	std::cerr << "\n[OptiX log message] \nLevel: " << level << "\nTag: " << tag << "\nMessage:\n" << message << "\n[End of message]\n\n";
}