/******************************************
 *
 *            HAPI.h
 *
 *  High level function in the hologram API
 *
 *******************************************/
#ifndef _HAPIH
#define _HAPIH
#include "SceneTree.h"

#include "Node.h"
#include "GeometryNode.h"
#include "TransformationNode.h"
#include "InterferencePattern.h"
#include "StaticNode.h"
#include <chrono>
//  Channels

#define RED		1
#define GREEN	2
#define BLUE	4

// Targets

#define FILE	1
#define DEVICE	2

// Modes

#define DISPLAY	1
#define COMPUTE	2

// Algorithms

#define SIMPLE_POINT	1
#define RAINBOW_SLIT	2
#define RAY_TRACE		3
#define COMPLEX_POINT			4

void setChannels(int);
void setWavelength(int channel, double wavelength);
void setChannelPosition(int channel, double x, double y, double z);
void setTarget(int target);
void setBaseFileName(char *name);
void setPointDistance(double);
void setWorldSpace(double x1, double y1, double z1, double x2, double y2, double z2);
void setDeviceSpace(double x1, double y1, double z1, double x2, double y2, double z2);
void setAlgorithm(int algorithm);
int getAlgorithm();
void passTwo(Node * root, InterferencePattern * pattern, bool isParallel);
void traverse(Node *tree, InterferencePattern* pattern, double ctm[4][4], bool isParallel);
void compile(GeometryNode *node, double ctm[4][4]);
void computeInterference(GeometryNode *node, InterferencePattern* pattern, double lambda, double x, double y, double z, bool isParallel, double initialCtm[4][4]);
void display(Node *root, bool isParallel);
InterferencePattern* display(Node *root, int mode, bool isParallel);
double getSeconds();
#endif