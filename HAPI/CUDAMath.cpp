#include "CUDAMath.h"

const float3 operator-(float3 a, float3 b) {
	return { a.x-b.x, a.y-b.y, a.z-b.z };
}

const void operator+=(float3 &a, float3 b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

const void operator/=(float3& a, float b) {
	a.x /= b;
	a.y /= b;
	a.z /= b;
}
