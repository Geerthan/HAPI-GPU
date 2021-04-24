#include "helper.h"

double rho = 0.15;
double noise = 1.2;
int rx = 99;
int ry = 99;

void setrho(double r) {
	rho = r;
}

double getrho() {
	return(rho);
}

void setnoise(double n) {
	noise = n;
}

double getnoise() {
	return(noise);
}

void setres(int x, int y) {
	rx = x;
	ry = y;
}

void getres(int& x, int& y) {
	x = rx;
	y = ry;
}