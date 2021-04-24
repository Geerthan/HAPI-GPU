/****************************************
 *
 *           compile.cpp
 *
 *  Compile a geometry node into a
 *  list of point light sources.
 *
 ***************************************/

#include "HAPI.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Internal.h"
#include "Matrix.h"
#include <stdio.h>

void compile(GeometryNode *node, double ctm[4][4]) {
	std::vector<Point*> plist;
	std::vector<Line*> list;
	std::vector<Point*> points;
	std::vector<double> r;
	std::vector<double> tr;
	Line *l;
	Point *p, *p1;
	double x, y, z;
	double x1, y1, z1;
	double x2, y2, z2;
	double v1[4], v2[4];
	double dx, dy, dz;
	double len;
	int axis;
	int i, j;
	int np;
	double d;

	if(node->getPrimitiveType() == POINT) {
		points = node->getPoints();
		for (i = 0; i < points.size(); i++) {
			p = points[i];
			v1[0] = p->x;
			v1[1] = p->y;
			v1[2] = p->z;
			MxV(ctm, v1, v2);
			p1 = new Point();
			p1->x = mx*v2[0]+cx;
			p1->y = my*v2[1]+cy;
			p1->z = mz*v2[2]+cz;
			plist.push_back(p1);
		}
		node->setLightPoints(plist);
		return;
	}

	if (node->getPrimitiveType() == SPHERE) {
		points = node->getPoints();
		for (i = 0; i < points.size(); i++) {
			p = points[i];
			v1[0] = p->x;
			v1[1] = p->y;
			v1[2] = p->z;
			MxV(ctm, v1, v2);
			p1 = new Point();
			p1->x = mx*v2[0] + cx;
			p1->y = my*v2[1] + cy;
			p1->z = mz*v2[2] + cz;
			plist.push_back(p1);
		}
		node->setLightPoints(plist);
		r = node->getRadius();
		// apply scaling in the x axis to radius, something to be aware of
		for (i = 0; i < r.size(); i++) {
			d = r[i];
			tr.push_back(ctm[0][0] * mx*d);
		}
		node->setTransRadius(tr);
		printf("Transformed Sphere\n");
		std::vector<Point*> lp;
		lp = node->getLightPoints();
		tr = node->getTransRadius();
		for (i = 0; i < points.size(); i++) {
			printf("  %f %f %f %f\n", lp[i]->x, lp[i]->y, lp[i]->z, tr[i]);
		}
		return;
	}
	
	list = node->getLines();
	for(i=0; i<list.size(); i++) {
		l = list[i];
		v1[0] = l->x1;
		v1[1] = l->y1;
		v1[2] = l->z1;
		v1[3] = 1.0;
		MxV(ctm,v1,v2);
		x1 = mx*v2[0]+cx;
		y1 = my*v2[1]+cy;
		z1 = mz*v2[2]+cz;
		v1[0] = l->x2;
		v1[1] = l->y2;
		v1[2] = l->z2;
		MxV(ctm,v1,v2);
		x2 = mx*v2[0]+cx;
		y2 = my*v2[1]+cy;
		z2 = mz*v2[2]+cz;

		dx = fabs(x2 - x1);
		dy = fabs(y2 - y1);
		dz = fabs(z2 - z1);
		len = dx;
		axis = 1;
		if(dy > len) {
			len = dy;
			axis = 2;
		}
		if(dz > len) {
			len = dz;
			axis = 3;
		}
		switch(axis) {
			case 1:
				if(x2 > x1)
					dx = pointDistance;
				else
					dx = -pointDistance;
				np = fabs(x2 - x1)/pointDistance;
				dy = (y2 - y1)/len*pointDistance;
				dz = (z2 - z1)/len*pointDistance;
				break;
			case 2:
				if(y2 > y1)
					dy = pointDistance;
				else
					dy = -pointDistance;
				np = fabs(y2 - y1)/pointDistance;
				dx = (x2 - x1)/len*pointDistance;
				dz = (z2 - z1)/len*pointDistance;
				break;
			case 3:
				if(z2 > z1)
					dz = pointDistance;
				else
					dz = -pointDistance;
				np = fabs(z2 - z1)/pointDistance;
				dx = (x2 - x1)/len*pointDistance;
				dy = (y2 - y1)/len*pointDistance;
				break;
		}
		x = x1;
		y = y1;
		z = z1;
		for(j=0; j<np; j++) {
			p = new Point();
			p->x = x;
			p->y = y;
			p->z = z;
			plist.push_back(p);
			x += dx;
			y += dy;
			z += dz;
		}
		printf("line: %f %f %f %f %f %f\n", x1, y1, z1, x2, y2, z2);
		printf("line: %d %d %f %f %f %f\n", np, axis, len, dx, dy, dz);
	}
	node->setLightPoints(plist);
}