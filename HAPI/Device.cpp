/**************************************************
 *                Device.cpp
 *
 *  Implementation of the Device object.
 *
 *************************************************/

#include "Device.h"
#include "HAPI.h"
#include <string.h>

Device* Device::instance = 0;
char* Device::filename = 0;

void Device::loadFile(char *filename) {
	json_t *root;
	json_error_t error;
	json_t *token;

	root = json_load_file(filename, 0, &error);
	if (root == NULL) {
		printf("Error in device file at line %d, column %d\n: %s", error.line, error.column, error.text);
		exit(1);
	}

	token = json_object_get(root, "pixelsW");
	pixelsW = (int) json_integer_value(token);

	token = json_object_get(root, "pixelsH");
	pixelsH = (int) json_integer_value(token);

	token = json_object_get(root, "sizeW");
	sizeW = json_real_value(token);

	token = json_object_get(root, "sizeH");
	sizeH = json_real_value(token);

	token = json_object_get(root, "minDepth");
	_minDepth = json_real_value(token);

	token = json_object_get(root, "maxDepth");
	_maxDepth = json_real_value(token);

	token = json_object_get(root, "left");
	_left = json_real_value(token);

	token = json_object_get(root, "right");
	_right = json_real_value(token);

	token = json_object_get(root, "top");
	_top = json_real_value(token);

	token = json_object_get(root, "bottom");
	_bottom = json_real_value(token);

	token = json_object_get(root, "diamond");
	if (token == NULL) {
		_diamond = false;
	}
	else {
		_diamond = json_boolean_value(token);
	}

	printf("pixels: %d %d\n", pixelsW, pixelsH);
	printf("pixel size: %f %f\n", sizeW, sizeH);
	printf("Depth: %f %f\n", _minDepth, _maxDepth);
	printf("X: %f %f\n", _left, _right);
	printf("Y: %f %f\n", _top, _bottom);
	if (_diamond) {
		printf("Diamond Pixel Arrangement\n");
	}

}

Device::Device() {

	if (filename == 0) {
		loadFile("device.halo");
	}
	else {
		loadFile(filename);
	}

	setDeviceSpace(_left, _bottom, _minDepth, _right, _top, _maxDepth);

}

Device* Device::getInstance() {
	if (instance == 0) {
		instance = new Device();
	}
	return(instance);
}

void Device::setFileName(char *name) {
	size_t len;

	len = strlen(name);
	filename = new char[len + 1];
	strcpy(filename, name);
}