#include <optix.h>
#include <optix_function_table_definition.h>

#include <cuda_runtime.h>
#include <cuda/helpers.h>

#include <stdio.h>
#include <thrust/complex.h>

#include "SphereSBTData.h"

struct Angle {
	double x;
	double y;
};
struct Point {
	double x;
	double y;
	double z;
};

struct Payload {
	float3 colour;
	float diff;
	float t;
};

extern "C" {
	__constant__ SphereParams params;
}

extern "C" __global__ void __raygen__rg() {

	const uint3 launchIdx = optixGetLaunchIndex();
	const uint3 launchDim = optixGetLaunchDimensions();

	const RayGenData* rtData = (RayGenData*)optixGetSbtDataPointer();

	const float left	= rtData->left;
	const float top		= rtData->top;
	const float sizex	= rtData->sizex;
	const float sizey	= rtData->sizey;
	const float camDist	= rtData->camDist;

	const int diamond	= rtData->diamond;
	const int xMax		= rtData->xMax;
	const int yMax		= rtData->yMax;

	const float k		= rtData->k;


	float x = left + launchIdx.x * sizex;
	if (diamond && ((launchIdx.y % 2) == 0)) {
		x += sizex / 2.0f;
	}

	float y = top - launchIdx.y * sizey;

	Payload payload;
	payload.colour = make_float3(0.0f);
	payload.diff = 0.0f;
	payload.t = 0.0f;
	unsigned int p0, p1, p2, p3, p4;

	//Compute the reference of the sphere
	//NOTE: thrust::complex example(realNumber, ImagNumber);
	thrust::complex<float> finalComplex(0.0, 0.0);

	Ray7 currentRay;
	currentRay.origin.x = x;
	currentRay.origin.y = y;
	currentRay.origin.z = -camDist;
	currentRay.tmax = 1.e27f; // The value of RT_DEFAULT_MAX
	float3 dirTotal = make_float3(0.0f);

	int numIntersect = 0;
	for (int i = 0; i < xMax; i++) {
		for (int j = 0; j < yMax; j++) {

			int index = i * xMax + j;

			double dx = rtData->jitterBuffer[index].x;
			double dy = rtData->jitterBuffer[index].y;
			double dz = 1.0;
			double len = sqrt(dx * dx + dy * dy + dz * dz);

			currentRay.direction.x = dx / len;
			currentRay.direction.y = dy / len;
			currentRay.direction.z = dz / len;

			p0 = float_as_int(payload.colour.x);
			p1 = float_as_int(payload.colour.y);
			p2 = float_as_int(payload.colour.z);
			p3 = float_as_int(payload.diff);
			p4 = float_as_int(payload.t);

			optixTrace(
				params.handle,
				currentRay.origin,
				currentRay.direction,
				0.0f, // Intersection dist (min/max)
				1e16f,
				0.0f, // 0 ray-time = 0 motion blur
				OptixVisibilityMask(255),
				OPTIX_RAY_FLAG_NONE,
				0, // SBT offset / ray type program id
				0, // SBT stride / total number of ray types
				0, // Miss index / active miss program
				p0, p1, p2, p3, p4
			);

			payload.colour.x = int_as_float(p0);
			payload.colour.y = int_as_float(p1);
			payload.colour.z = int_as_float(p2);
			payload.diff = int_as_float(p3);
			payload.t = int_as_float(p4);

			if (payload.t >= 0.0) {

				payload.t -= camDist;

				thrust::complex<float> complexNum(0.0, (float)(k * payload.t));
				complexNum = thrust::exp<float>(complexNum);

				thrust::complex<float> complexColour = thrust::complex<float>((float)payload.colour.x, 0.0);
				complexNum = (complexColour * complexNum) / thrust::complex<float>((float)payload.t, 0.0);
				finalComplex += complexNum;

				numIntersect += 1;
			}
		}
	}

	// Can convert this to a single int for slight optimization
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_1.x = finalComplex.real();
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_1.y = finalComplex.imag();
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].intersect = numIntersect;
}

extern "C" __global__ void __closesthit__ch() {
	float diff = int_as_float(optixGetAttribute_0());
	float colour = int_as_float(optixGetAttribute_1());

	float ambient = 0.2;
	float diffuse = 0.8;
	float result = diff * diffuse * colour + ambient;

	float3 payloadCol = make_float3(result);
	optixSetPayload_0(float_as_int(payloadCol.x));
	optixSetPayload_1(float_as_int(payloadCol.y));
	optixSetPayload_2(float_as_int(payloadCol.z));
}

extern "C" __global__ void __intersection__sp(int primitiveIndex) {

	primitiveIndex = 0;

	const SphereHitGroupData* rtData = (SphereHitGroupData*)optixGetSbtDataPointer();

	float3 rayOrigin = optixGetWorldRayOrigin();
	float3 rayDirection = optixGetWorldRayDirection();

	//Sphere intersection
	float3 point = rtData->pointBuffer[primitiveIndex];
	double radius = rtData->radiusBuffer[primitiveIndex];

	//Quadratic equation
	double A = 1.0;
	double B;
	double C;
	float3 ec;
	
	//Calculate the discriminant of the quadratic equation
	ec.x = rayOrigin.x - point.x;
	ec.y = rayOrigin.y - point.y;
	ec.z = rayOrigin.z - point.z;
	B = 2 * (rayDirection.x * ec.x + rayDirection.y * ec.y + rayDirection.z * ec.z);

	C = ec.x * ec.x + ec.y * ec.y + ec.z * ec.z - radius * radius;
	double discriminant = B * B - 4.0 * A * C;
	
	optixSetPayload_0(float_as_int(0.0f));
	optixSetPayload_1(float_as_int(0.0f));
	optixSetPayload_2(float_as_int(0.0f));
	optixSetPayload_4(float_as_int(-1.0f));

	float lx = 0.0;
	float ly = 0.0;
	float lz = -0.5;
	float len = sqrt(lx * lx + ly * ly + lz * lz);
	lx /= len;
	ly /= len;
	lz /= len; //lz = -1
	//FIX ME. GET RID OF THE IF STATEMENTS
	float finalT = -1.0;
	if (discriminant >= 0) { //The ray hit the sphere
		discriminant = sqrt(discriminant);
		//two roots of the quadratic equation
		float t1 = (-B - discriminant) / (2 * A);
		float t2 = (-B + discriminant) / (2 * A);
		if (t1 > 0.0f) {
			finalT = t1;
		}
		if (t2 > 0.0f && t2 < t1) {
			finalT = t2;
		}

		float nx = rayOrigin.x + finalT * rayDirection.x - point.x;
		float ny = rayOrigin.y + finalT * rayDirection.y - point.y;
		float nz = rayOrigin.z + finalT * rayDirection.z - point.z;

		len = sqrt(nx * nx + ny * ny + nz * nz);
		nx /= len;
		ny /= len;
		nz /= len;
		float diff = nx * lx + ny * ly + nz * lz;
		if (diff < 0.0) {
			diff = 0.0;
		}
		float colour = rtData->colourBuffer[primitiveIndex];
		
		if (optixReportIntersection(finalT, 0, float_as_int(diff), float_as_int(colour))) {
			optixSetPayload_4(float_as_int(finalT));
		}
	}
}

extern "C" __global__ void __miss__ms() {

}
