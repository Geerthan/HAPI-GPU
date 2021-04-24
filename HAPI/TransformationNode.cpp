/********************************************
 *
 *           TransformatioNode.cpp
 *
 *  Implementation of the transformation node.
 *
 ********************************************/

#include "TransformationNode.h"
#include "Matrix.h"
#include <math.h>

TransformationNode::TransformationNode() : Node(TRANSFORMATIONNODE) {

	identity(_matrix);

}

TransformationNode::~TransformationNode() {

}

void TransformationNode::translate(double dx, double dy, double dz) {
	double tmatrix[4][4], temp[4][4];

	identity(tmatrix);
	tmatrix[0][3] = dx;
	tmatrix[1][3] = dy;
	tmatrix[2][3] = dz;
	MxM(_matrix, tmatrix, temp);
	assign(_matrix, temp);

}

void TransformationNode::scale(double sx, double sy, double sz) {
	double smatrix[4][4], temp[4][4];

	identity(smatrix);
	smatrix[0][0] = sx;
	smatrix[1][1] = sy;
	smatrix[2][2] = sz;
	MxM(_matrix, smatrix, temp);
	assign(_matrix, temp);

}

void TransformationNode::rotateX(double angle) {
	double rmatrix[4][4], temp[4][4];

	identity(rmatrix);
	rmatrix[1][1] = cos(angle);
	rmatrix[1][2] = -sin(angle);
	rmatrix[2][1] = sin(angle);
	rmatrix[2][2] = cos(angle);
	MxM(_matrix, rmatrix, temp);
	assign(_matrix, temp);

}

void TransformationNode::rotateY(double angle) {
	double rmatrix[4][4], temp[4][4];

	identity(rmatrix);
	rmatrix[0][0] = cos(angle);
	rmatrix[0][2] = sin(angle);
	rmatrix[2][0] = -sin(angle);
	rmatrix[2][2] = cos(angle);
	MxM(_matrix, rmatrix, temp);
	assign(_matrix, temp);

}

void TransformationNode::rotateZ(double angle) {
	double rmatrix[4][4], temp[4][4];

	identity(rmatrix);
	rmatrix[0][0] = cos(angle);
	rmatrix[0][1] = -sin(angle);
	rmatrix[1][0] = sin(angle);
	rmatrix[1][1] = cos(angle);
	MxM(_matrix, rmatrix, temp);
	assign(_matrix, temp);

}

void TransformationNode::concat(double matrix[4][4]) {
	double temp[4][4];

	MxM(matrix, _matrix, temp);
	assign(matrix, temp);

}