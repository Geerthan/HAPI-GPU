/********************************************
 *
 *                 Matrix.h
 *
 *  Simple matrix library for 4x4 matrices
 *  and vectors.
 *
 *******************************************/

void identity(double m[4][4]);
void assign(double m1[4][4], double m2[4][4]);
void MxM(double m1[4][4], double m2[4][4], double result[4][4]);
void MxV(double m[4][4], double vIn[4], double vOut[4]);

class MatrixStack {
public:
	MatrixStack();
	~MatrixStack();
	void push(double ctm[4][4]);
	void pop(double ctm[4][4]);
private:
	int _top;
	double _stack[10][4][4];
};