/************************************************
 *
 *               TransformationNode.h
 *
 *  The transformation node is used to represent
 *  transformations in the scene graph.  It contains
 *  a 4x4 transformation matrix that is constructed
 *  by calls to its methods.
 *
 ************************************************/
#ifndef _TRANSFORMATIONNODE
#define _TRANSFORMATIONNODE
#include "Node.h"

class TransformationNode: public Node {
public:
	TransformationNode();
	~TransformationNode();
	void translate(double dx, double dy, double dz);
	void scale(double dx, double dy, double dz);
	void rotateX(double angle);
	void rotateY(double angle);
	void rotateZ(double angle);
	void concat(double matrix[4][4]);
protected:
	double _matrix[4][4];
};
#endif