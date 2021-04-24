/********************************************
 *
 *            test
 *
 *  Test program for the HAPI library.
 *
 ********************************************/
#define _USE_MATH_DEFINES
#include <math.h>
#include "HAPI.h"
#include <stdio.h>
#include "Matrix.h"
#include <vector>

int main(int argc, char **argv) {
	Node *node = new Node(NONE);
	GeometryNode *gnode;
	GeometryNode *fan;
	TransformationNode *tnode;
	int i;
	char buffer[256];
	double t1;
	double dx, dy, dz;
	double theta;
	double dtheta;
	int n;
	double x, y, z;

	dx = atof(argv[1]);
	dy = atof(argv[2]);
	dz = atof(argv[3]);
	printf("dz: %f\n",dz);

	t1 = getSeconds();
	setChannels(RED);
	setWavelength(RED, 650.0);
	setTarget(FILE);
	setBaseFileName("test");
	setWorldSpace(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
//	setDeviceSpace(-0.25, -0.25, 17.75, 0.25, 0.25, 18.25);
	setDeviceSpace(-0.4+dx, -0.4+dy, 17.75+dz, 0.4+dx, 0.4+dy, 18.25+dz);

	gnode = new GeometryNode(LINE);
	gnode->addLine(-0.5, -0.5, 1.0, 0.5, -0.5, 1.0);
	gnode->addLine(0.5, -0.5, 1.0, 0.5, 0.5, 1.0);
	gnode->addLine(0.5,0.5, 1.0, -0.5, 0.5, 1.0);
	gnode->addLine(-0.5, 0.5, 1.0, -0.5, -0.5, 1.0);

	gnode->addLine(-1.0, -1.0, -1.0, 1.0, -1.0, -1.0);
	gnode->addLine(1.0, -1.0, -1.0, 1.0, 1.0, -1.0);
	gnode->addLine(1.0, 1.0, -1.0, -1.0, 1.0, -1.0);
	gnode->addLine(-1.0, 1.0, -1.0, -1.0, -1.0, -1.0);

	gnode->addLine(-0.5, -0.5, 1.0, -1.0, -1.0, -1.0);
	gnode->addLine(0.5, -0.5, 1.0, 1.0, -1.0, -1.0);
	gnode->addLine(0.5, 0.5, 1.0, 1.0, 1.0, -1.0);
	gnode->addLine(-0.5, 0.5, 1.0, -1.0, 1.0, -1.0);

	tnode = new TransformationNode();
	tnode->rotateZ(0.0);

	fan = new GeometryNode(LINE);
	n = 42;
	theta = 0;
	dtheta = 2*M_PI/n;
	z = 0.5;
//	printf("%f %f\n", 0.2*cos(0.0), 0.2*sin(0.0));
//	printf("%f %f\n", 0.2*cos(dtheta), 0.2*sin(dtheta));
	for(i=0; i<n; i++) {
		x = cos(theta);
		y = sin(theta);
		fan->addLine(0.2*cos(theta), 0.2*sin(theta), z, x, y, z);
		theta += dtheta;
	}
	node->addChild(gnode);
//	tnode->addChild(gnode);
	setAlgorithm(RAINBOW_SLIT);
	for(i=0; i<1; i++) {
//		setPointDistance(200.0 + i*50.0);
//		printf("%d %f\n",i,200.0+i*50.0);
		sprintf(buffer,"ptest%d",i);
		setBaseFileName(buffer);
		display(node, false);
//		tnode->rotateZ(0.1);
	}
	printf("total time: %f\n", getSeconds() - t1);
}