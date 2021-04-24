/****************************************
 *
 *          InterferencePattern.cpp
 *
 *  Implementation of the inteference pattern
 *  object.
 *
 *****************************************/

#include "InterferencePattern.h"
#include <FreeImage.h>
#include <stdio.h>

InterferencePattern::InterferencePattern() {
	int i;
	int pixels;

	device = Device::getInstance();

	width = device->pixelsWidth();
	height = device->pixelsHeight();
	pixels = width*height;

	max = -1.0e10;
	min = 1.0e10;

	image = new double[pixels];
	for(i=0; i<pixels; i++)
		image[i] = 0.0;

}

InterferencePattern::~InterferencePattern() {

}

void InterferencePattern::set(int x, int y, double value) {

	image[y*width+x] = value;

}

void InterferencePattern::add(int x, int y, double value) {

	/*
	nv = image[x][y] + value;
	image[x][y] = nv;
	*/

	image[y * width + x] += value;

}

void InterferencePattern::add(InterferencePattern& ip) {
	int i,j;

	for(i=0; i<width; i++)
		for (j = 0; j<height; j++) {
			image[j*width+i] += ip.image[j*width+i];
		}

}

void InterferencePattern::clear() {
	int i;
	int pixels;

	max = -1.0e10;
	min = 1.0e10;
	pixels = width*height;
	for(i=0; i<pixels; i++)
		image[i] = 0.0;

}

void InterferencePattern::save(char *filename) {
	int i;
	int j;
	int k;
	FIBITMAP *bitmap;
	BYTE *bits;
	double scale;
	double value;
	int pixels;

	pixels = width*height;
	for (i = 0; i < pixels; i++) {
		if (image[i] > max)
			max = image[i];
		if (image[i] < min)
			min = image[i];
	}
	scale = max-min;
	printf("%f %f %s\n",min,max,filename);
	bitmap = FreeImage_Allocate(width, height, 8);
	for(j=0; j<height; j++) {
		bits = FreeImage_GetScanLine(bitmap,j);
		k = j*width;
		for(i=0; i<width; i++) {
			value = 1.0 - (image[i+k]-min)/scale;
			bits[FI_RGBA_RED] = (BYTE) (255.0*value);
			bits += 1;
		}
	}
	FreeImage_Save(FIF_BMP, bitmap, filename, BMP_DEFAULT);
}
