#pragma once

#include <string>
#include <iostream>
#include <fstream>

std::string getPtx(std::string ptx_name);
void optix_logger(unsigned int level, const char* tag, const char* message, void* cbdata);