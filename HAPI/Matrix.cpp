/************************************
 *
 *     Matrix.cpp
 *
 *  Implementation of simple matrix
 *  library
 *
 ***********************************/

#include "Matrix.h"
#include <stdio.h>

void identity(double m[4][4]) {
	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++) 
			if(i == j)
				m[i][j] = 1.0;
			else
				m[i][j] = 0.0;
}

void assign(double m1[4][4], double m2[4][4]) {
	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			m1[i][j] = m2[i][j];
}

void MxV(double m[4][4], double vIn[4], double vOut[4]) {
	int i,j;

	for(i=0; i<4; i++) {
		vOut[i] = 0.0;
		for(j=0; j<4; j++) 
			vOut[i] += m[i][j] * vIn[j];
	}
}

void MxM(double m1[4][4], double m2[4][4], double r[4][4]) {
	int i,j,k;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++) {
			r[i][j] = 0.0;
			for(k=0; k<4; k++)
				r[i][j] += m1[i][k] * m2[k][j];
		}
}

MatrixStack::MatrixStack() {

	_top = 0;

}

MatrixStack::~MatrixStack() {

}

void MatrixStack::push(double m[4][4]) {

	assign(_stack[_top], m);
	_top++;

}

void MatrixStack::pop(double m[4][4]) {

	_top--;
	assign(m, _stack[_top]);

}