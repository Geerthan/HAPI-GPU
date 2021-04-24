/********************************************
 *
 *            Hapi.cpp
 *
 *  Implementation of the basic Hapi functions
 *
 ********************************************/

#include <Windows.h>
#include "HAPI.h"
#include "Internal.h"
#include <string.h>
#include <stack>
#include "Matrix.h"

double mx, my, mz;
double cx, cy, cz;
double pointDistance = 250.0;

int nChannels = 0;
double wavelengths[3] = { 0, 0, 0 };
double cpx[3] = { 40000.0, 40000.0, 40000.0 };
//double cpx[3] = { 0.0, 0.0, 0.0 };
double cpy[3] = { 0.0, 0.0, 0.0 };
//double cpz[3] = { 0.0, 0.0, 0.0 };
double cpz[3] = { 70000.0, 70000.0, 70000.0 };
int myTarget = -1;
char *baseFileName;

//  initial world space
double wx1 = 0.0;
double wy1 = 0.0;
double wz1 = 0.0;

double wx2 = 1.0;
double wy2 = 1.0;
double wz2 = 1.0;

//  initial device space
double dx1 = 0.0;
double dy1 = 0.0;
double dz1 = 0.0;

double dx2 = 1.0;
double dy2 = 1.0;
double dz2 = 1.0;

static int currentAlgorithm = SIMPLE_POINT;

void setChannels(int flag) {

	if(flag & RED) {
		nChannels++;
		wavelengths[0] = 0.650;
	}
	if(flag & GREEN) {
		nChannels++;
		wavelengths[1] = 0.532;
	}
	if(flag & BLUE) {
		nChannels++;
		wavelengths[2] = 0.405;
	}
}

void setWavelength(int channel, double wavelength) {

	if(channel == RED)
		wavelengths[0] = wavelength/1000.0;
	if(channel == GREEN)
		wavelengths[1] = wavelength/1000.0;
	if(channel == BLUE)
		wavelengths[2] = wavelength/1000.0;

}

void setChannelPosition(int channel, double x, double y, double z) {

	cpx[channel] = x;
	cpy[channel] = y;
	cpz[channel] = z;

}

void setPointDistance(double d) {

	pointDistance = d;

}

void setTarget(int target) {

	myTarget = target;

}

void setBaseFileName(char *name) {
	int len;

	len = strlen(name);
	baseFileName = new char[len+1];
	strcpy(baseFileName,name);

}

void setAlgorithm(int algorithm) {

	currentAlgorithm = algorithm;

}

int getAlgorithm() {

	return(currentAlgorithm);

}

void computeSpaceTransform() {
	double t;

	t = (dx2 - dx1) / (wx2 - wx1);
	mx = t;
	cx = dx1 - wx1*t;
	printf("X: %f %f\n",mx,cx);

	t = (dy2 - dy1) / (wy2 - wy1);
	my = t;
	cy = dy1 - wy1*t;
	printf("Y: %f %f\n",my,cy);

	t = (dz2 - dz1) / (wz2 - wz1);
	mz = t;
	cz = dz1 - wz1*t;

}

void setWorldSpace(double x1, double y1, double z1, double x2, double y2, double z2) {

	wx1 = x1;
	wy1 = y1;
	wz1 = z1;
	wx2 = x2;
	wy2 = y2;
	wz2 = z2;

	computeSpaceTransform();

}

void setDeviceSpace(double x1, double y1, double z1, double x2, double y2, double z2) {

	dx1 = x1 * 10000.0;
	dy1 = y1 * 10000.0;
	dz1 = z1 * 10000.0;
	dx2 = x2 * 10000.0;
	dy2 = y2 * 10000.0;
	dz2 = z2 * 10000.0;

	computeSpaceTransform();

}

void passOne(Node *root, double initialCtm[4][4]) {
	double ctm[4][4];
	MatrixStack stack;
	std::stack<Node*> nodeStack;
	Node *current;
	InterferencePattern *pat;
	int i;

	assign(ctm, initialCtm);
	nodeStack.push(root);
	while (nodeStack.size() != 0) {
		current = nodeStack.top();
		nodeStack.pop();
		switch(current->getType()) {
		case STATICNODE:
			if (((StaticNode*)current)->isDirty()) {
				pat = new InterferencePattern();
				stack.push(ctm);
				traverse(current, pat, ctm, false);
				stack.pop(ctm);
				((StaticNode*)current)->setInterferencePattern(*pat);
			}
			break;
		case TRANSFORMATIONNODE:
			stack.push(ctm);
			((TransformationNode*)current)->concat(ctm);
			for (i = 0; i<current->count(); i++)
				passOne(current->getChild(i), ctm);
			stack.pop(ctm);
			break;
		case GEOMETRYNODE:
			compile((GeometryNode*)current, ctm);
			break;
		default:
			for (i = 0; i < current->count(); i++)
				nodeStack.push(current->getChild(i));
		}
	}
}

void passTwo(Node *root, InterferencePattern* pattern, bool isParallel, double initialCtm[4][4]) {
	std::stack<Node*> nodeStack;
	Node *current;
	int i;
	InterferencePattern *pat;

	nodeStack.push(root);
	while (nodeStack.size() != 0) {
		current = nodeStack.top();
		nodeStack.pop();
		switch (current->getType()) {
		case STATICNODE:
			pat = &((StaticNode*)current)->getInterferencePattern(); //You don't need to recompute this again. 
			pattern->add(*pat); 
			break;
		case GEOMETRYNODE:
			// Render the pattern
			// TODO: Remove Ctm from this when HAPI scene graph transformations are implemented for triangle meshes
			computeInterference((GeometryNode*)current, pattern, wavelengths[0], cpx[0], cpy[0], cpz[0], isParallel, initialCtm);
			break;
		default:
			for (i = 0; i < current->count(); i++)
				nodeStack.push(current->getChild(i));
		}
	}
}

void traverse(Node *root, InterferencePattern* pattern, double initialCtm[4][4], bool isParallel) {
	/*
	 *  first pass - convert all the geometry
	 *  to point light sources
	 */
	passOne(root, initialCtm);

	/*
	 *  second pass - add all the point light sources to
	 *  the interference pattern
	 */
	
	// TODO: Remove Ctm from this when HAPI scene graph transformations are implemented for triangle meshes
	passTwo(root, pattern, isParallel, initialCtm); 

}

void display(Node *root, bool isParallel) {
	double ctm[4][4];
	InterferencePattern *pattern = new InterferencePattern();
	char buffer[256];

	identity(ctm);
	traverse(root, pattern, ctm, isParallel);

	if(myTarget == FILE) {
		sprintf(buffer,"%s.bmp",baseFileName);
		pattern->save(buffer);
	}

}

InterferencePattern* display(Node *root, int mode, bool isParallel) {
	InterferencePattern *pattern = new InterferencePattern();
	double ctm[4][4];
	char buffer[256];

	identity(ctm);
	traverse(root, pattern, ctm, isParallel);

	if(mode == COMPUTE)
		return(pattern);
	if(myTarget == FILE) {
		sprintf(buffer,"%s.bmp",baseFileName);
		pattern->save(buffer);
	}

}

double getSeconds() {
    LARGE_INTEGER freq, val;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&val);
    return (double)val.QuadPart / (double)freq.QuadPart;
}