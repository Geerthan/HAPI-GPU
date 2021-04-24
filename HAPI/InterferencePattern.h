/***************************************
 *
 *        InterferencePattern.h
 *
 *  Representation of an interference pattern.
 *
 ****************************************/
#ifndef _INTERFERENCEPATTERNH
#define _INTERFERENCEPATTERNH
#include "Device.h"

class InterferencePattern {
public:
	InterferencePattern();
	~InterferencePattern();
	void set(int x, int y, double value);
	void add(int x, int y, double value);
	void add(InterferencePattern&);
	void save(char *filename);
	void clear();
	Device* getDevice() { return(device); };
protected:
	Device *device;
	int width;
	int height;
	double max;
	double min;
	double *image;
};
#endif