/*****************************************
 *
 *             GeometryNode.cpp
 *
 *  Implementation of the geometry node
 *  object.
 *
 *****************************************/

#include "GeometryNode.h"

GeometryNode::GeometryNode(int type) : Node(GEOMETRYNODE) {

	_primitiveType = type;
	_dirty = 0;

}

GeometryNode::~GeometryNode() {

}

void GeometryNode::addPoint(double x1, double y1, double z1) {

	if(_primitiveType != POINT)
		return;

	Point *p = new Point();
	p->x = x1;
	p->y = y1;
	p->z = z1;
	_pointList.push_back(p);
	_dirty = 1;

}

void GeometryNode::addLine(double x1, double y1, double z1, double x2, double y2, double z2) {

	if(_primitiveType != LINE)
		return;
	Line *l = new Line();
	l->x1 = x1;
	l->y1 = y1;
	l->z1 = z1;
	l->x2 = x2;
	l->y2 = y2;
	l->z2 = z2;
	_lineList.push_back(l);
	_dirty = 1;

}

void GeometryNode::addSphere(double x1, double y1, double z1, double r, double c) {

	if (_primitiveType != SPHERE)
		return;

	Point *p = new Point();
	p->x = x1;
	p->y = y1;
	p->z = z1;
	_pointList.push_back(p);
	_radius.push_back(r);
	_colourList.push_back(c);
	_dirty = 1;

}

int GeometryNode::count() {

	if(_primitiveType == POINT)
		return(_pointList.size());
	if(_primitiveType == LINE)
		return(_lineList.size());
	if (_primitiveType == SPHERE)
		return(_pointList.size());

}

void GeometryNode::setLightPoints(std::vector<Point*> points) {
	int i;

	_lightPoints.clear();

	for (i = 0; i < points.size(); i++) {
		_lightPoints.push_back(points[i]);
	}

}

void GeometryNode::setTransRadius(std::vector<double> r) {
	int i;

	_transRadius.clear();

	for (i = 0; i < r.size(); i++) {
		_transRadius.push_back(r[i]);
	}
}