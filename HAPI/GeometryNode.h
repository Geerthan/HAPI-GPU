/************************************
 *
 *            GeometryNode.h
 *
 *  Geometry node class.
 *
 **************************************/
#ifndef _GEOMETRYNODEH
#define _GEOMETRYNODEH
#include "Node.h"

#define POINT	1
#define LINE	2
#define POLYGON	3
#define SPHERE	4

struct Point {
	double x;
	double y;
	double z;
};

struct Line {
	double x1, y1, z1;
	double x2, y2, z2;
};

class GeometryNode: public Node {
public:
	GeometryNode(int type);
	~GeometryNode();
	void addPoint(double x, double y, double z);
	void addLine(double x1, double y1, double z1, double x2, double y2, double z2);
	void addSphere(double x, double y, double z, double r, double c);
	int count();
	int getPrimitiveType() { return(_primitiveType); };
	int isDirty() { return(_dirty); };
	void setLightPoints(std::vector<Point*> points);
	void setTransRadius(std::vector<double> r);
	std::vector<Point*> getLightPoints() { return(_lightPoints); };
	std::vector<Point*> getPoints() { return(_pointList); };
	std::vector<Line*> getLines() { return(_lineList); };
	std::vector<double> getRadius() { return(_radius); };
	std::vector<double> getTransRadius() { return(_transRadius); };
	std::vector<double> getColour() { return (_colourList); };
protected:
	int _primitiveType;
	int _dirty;
	std::vector<Point*> _pointList;
	std::vector<Line*> _lineList;
	std::vector<Point*> _points;
	std::vector<Point*> _lightPoints;
	std::vector<double> _radius;
	std::vector<double> _transRadius;
	std::vector<double> _colourList;
};
#endif