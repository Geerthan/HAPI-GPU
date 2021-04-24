/********************************************
*
*            ray
*
*  Test program for the HAPI library. This
*  program has three optional parameters that
*  can be used to move the hologram on the
*  display surface and depth.  The x and y
*  displacements are used to avoid artifacts
*  caused by the interaction between the laser
*  and the DMD pixels.  The z displacement can
*  be used to control the distance between the
*  DMD surface and where the hologram appears.
*
********************************************/
#define _USE_MATH_DEFINES
#include <math.h>
#include "HAPI.h"
#include <stdio.h>
#include "Matrix.h"
#include <vector>
#include <iostream>
#include "helper.h"

int main(int argc, char **argv) {
	Node *node = new Node(NONE);
	GeometryNode *gnode;
	TransformationNode *tnode;
	int i;
	char buffer[256];
	double t1;
	double dx, dy, dz;

	struct Settings {
		double rho;
		double noise;
		int rx;
		int ry;
	};

	/*
		Sphere rho values were 0.1 and 0.4.
		Tested triangle mesh rho values included 0.1, 0.01, and 0.0001.
		The visual effect that should be observed by rho differences can be seen by using the same rho value in both tests.
		A good example of this is using 0.0001 rho in both.
	*/
	struct Settings settings[] = {
		0.1, 1.0, 25, 25,
		0.4, 1.0, 25, 25
	};

	int NSettings = sizeof(settings) / sizeof(Settings);

	dx = dy = dz = 0.0;
	if (argc == 4) {
		dx = atof(argv[1]);
		dy = atof(argv[2]);
		dz = atof(argv[3]);
		printf("dz: %f\n", dz);
	}

	t1 = getSeconds();
	/*
	*  Compute only the red channel and
	*  use a wavelength of 600 nm for red
	*  (this has since been edited to 532nm)
	*/
	setChannels(RED);

	setWavelength(RED, 532.0);
	setChannelPosition(RED, 0.0, 0.0, 37.5);
	/*
	*  Store the resulting interference
	*  pattern in a file, the base name
	*  for the files constructed is "test"
	*/
	setTarget(FILE);
	setBaseFileName("test");
	/*
	*  The world space defines the coordinate system
	*  used in the application.  Use whatever units
	*  you like.
	*/
	setWorldSpace(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
	/*
	*  The device space specifies where the hologram
	*  will appear in the real world.  The units used
	*  are cm.  These values are a good start for our
	*  prototype display
	*/
	//setDeviceSpace(-0.4+dx, -0.4+dy, 17.75+dz, 0.4+dx, 0.4+dy, 18.25+dz);

	/*
	*  Create the lines for the truncated pyramid
	*/
	gnode = new GeometryNode(SPHERE);
	gnode->addSphere(0.0, 0.0, 0.0, 0.6, 1.0);

	/*
	*  Set up the transformation for rotating the
	*  truncated pyramid
	*/
	tnode = new TransformationNode();
	tnode->rotateZ(0.0);
	/*
	*  Link up the nodes in the scene graph
	*/
	node->addChild(gnode);
//	tnode->addChild(gnode);
	/*
	*  Generate the interference patterns
	*/
	setPointDistance(250.0);
	setAlgorithm(RAY_TRACE);

	int choice;
	bool isParallel;

	std::cout << "Parallel -> 1 \nSerial ->2 \nInput: ";
	std::cin >> choice;

	if (choice == 1) {
		isParallel = true;
	}
	else {
		isParallel = false;
	}
	printf("Settings: %d\n", NSettings);

	for (i = 0; i < NSettings; i++) {
		printf("===================================================\n");
		printf("     Setting %d\n", i);
		printf("   %f %f %d %d\n", settings[i].rho, settings[i].noise, settings[i].rx, settings[i].ry);
		printf("====================================================\n");
		setres(settings[i].rx, settings[i].ry);
		setrho(settings[i].rho);
		setnoise(settings[i].noise);
		sprintf(buffer, "setting%d", i);
		setBaseFileName(buffer);
		display(node, isParallel);
//		tnode->rotateZ(0.1);
	}

	printf("Total time for all runs: %f\n", getSeconds() - t1);
}