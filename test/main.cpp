/********************************************
 *
 *            test
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

int main(int argc, char **argv) {
	Node *node = new Node(NONE);
	GeometryNode *gnode;
	TransformationNode *tnode;
	int i;
	char buffer[256];
	double t1;
	double dx, dy, dz;

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
	 *  use a wavelenght of 650 nm for red
	 */
	setChannels(RED);
	setWavelength(RED, 650.0);
	setChannelPosition(RED, 0.5, 0.5, 17.5);
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
//	setDeviceSpace(-0.4+dx, -0.4+dy, 17.75+dz, 0.4+dx, 0.4+dy, 18.25+dz);

	/*
	 *  Create the lines for the truncated pyramid
	 */
	gnode = new GeometryNode(LINE);
	/* Front face*/
	
	gnode->addLine(-0.5, -0.5, 1.0, 0.5, -0.5, 1.0);
	gnode->addLine(0.5, -0.5, 1.0, 0.5, 0.5, 1.0);
	gnode->addLine(0.5, 0.5, 1.0, -0.5, 0.5, 1.0);
	gnode->addLine(-0.5, 0.5, 1.0, -0.5, -0.5, 1.0);
	

	/* Back Face */
	gnode->addLine(-1.0, -1.0, -1.0, 1.0, -1.0, -1.0);
	gnode->addLine(1.0, -1.0, -1.0, 1.0, 1.0, -1.0);
	gnode->addLine(1.0, 1.0, -1.0, -1.0, 1.0, -1.0);
	gnode->addLine(-1.0, 1.0, -1.0, -1.0, -1.0, -1.0);

	/* Lines connecting the faces*/
	
	gnode->addLine(-0.5, -0.5, 1.0, -1.0, -1.0, -1.0);
	gnode->addLine(0.5, -0.5, 1.0, 1.0, -1.0, -1.0);
	gnode->addLine(0.5, 0.5, 1.0, 1.0, 1.0, -1.0);
	gnode->addLine(-0.5, 0.5, 1.0, -1.0, 1.0, -1.0);
	

	/*
	*  Set up the transformation for rotating the
	*  truncated pyramid
	*/
	tnode = new TransformationNode();
	tnode->rotateZ(0.0);

	/*
	*  Link up the nodes in the scene graph
	*/
	node->addChild(tnode);
	tnode->addChild(gnode);
	/*
	 *  Generate the interference patterns
	 */
	setAlgorithm(COMPLEX_POINT);
	setPointDistance(10.0);
	for(i=0; i<1; i++) {
		sprintf(buffer,"ptest%d",i);
		setBaseFileName(buffer);
		display(node, false);
		tnode->rotateZ(0.1);
	}
	printf("total time: %f\n", getSeconds() - t1);
}