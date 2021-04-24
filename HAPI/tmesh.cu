#include <optix.h>
#include <optix_function_table_definition.h>

#include <cuda_runtime.h>
#include <cuda/helpers.h>

#include <stdio.h>
#include <thrust/complex.h>

#include "TriangleSBTData.h"

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
	__constant__ TriangleParams params;
}

extern "C" __global__ void __raygen__rg() {

	const uint3 launchIdx = optixGetLaunchIndex();
	const uint3 launchDim = optixGetLaunchDimensions();

	const RayGenData* rtData = (RayGenData*)optixGetSbtDataPointer();

	const float left = rtData->left;
	const float top = rtData->top;
	const float sizex = rtData->sizex;
	const float sizey = rtData->sizey;
	const float camDist = rtData->camDist;

	const int diamond = rtData->diamond;
	const int xMax = rtData->xMax;
	const int yMax = rtData->yMax;

	const float k = rtData->k;


	float x = left + launchIdx.x * sizex;
	if (diamond && ((launchIdx.y % 2) == 0)) {
		x += sizex / 2.0f;
	}

	float y = top - launchIdx.y * sizey;

	Payload payload;
	payload.colour = make_float3(0.0f);
	payload.diff = 0.0f;
	payload.t = -1;

	unsigned int p0, p1, p2, p3, p4;

	//Compute the reference of the sphere
	//NOTE: thrust::complex example(realNumber, ImagNumber);

	thrust::complex<float> finalComplex_1(0.0, 0.0);
	thrust::complex<float> finalComplex_2(0.0, 0.0);
	thrust::complex<float> finalComplex_3(0.0, 0.0);

	Ray7 currentRay;
	currentRay.origin.x = x;
	currentRay.origin.y = y;
	currentRay.origin.z = -camDist;
	currentRay.tmax = 1.e27f; // The value of RT_DEFAULT_MAX in older OptiX versions
	float3 dirTotal = make_float3(0.0f);

	int numIntersect = 0;
	for (int i = 0; i < yMax; i++) {
		for (int j = 0; j < xMax; j++) {

			payload.t = -1;

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
				finalComplex_1 += complexNum;

				complexColour = thrust::complex<float>((float)payload.colour.y, 0.0);
				complexNum = (complexColour * complexNum) / thrust::complex<float>((float)payload.t, 0.0);
				finalComplex_2 += complexNum;

				complexColour = thrust::complex<float>((float)payload.colour.z, 0.0);
				complexNum = (complexColour * complexNum) / thrust::complex<float>((float)payload.t, 0.0);
				finalComplex_3 += complexNum;

				numIntersect += 1;
			}
		}
	}

	// Can convert this to a single int for slight optimization
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_1.x = finalComplex_1.real();
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_1.y = finalComplex_1.imag();

	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_2.x = finalComplex_2.real();
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_2.y = finalComplex_2.imag();

	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_3.x = finalComplex_3.real();
	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].result_3.y = finalComplex_3.imag();

	params.image_buffer[launchIdx.y * launchDim.x + launchIdx.x].intersect = numIntersect;
}

extern "C" __global__ void __closesthit__ch() {
	int primitiveIndex = optixGetPrimitiveIndex();
	const TriangleHitGroupData* rtData = (TriangleHitGroupData*)optixGetSbtDataPointer();

	float3 colour = rtData->colourBuffer[primitiveIndex];

	// Calculating per-vertex normals with barycentrics
	int3 tri = params.i_buf[primitiveIndex];

	// This is a wrapper function that can be bypassed: https://raytracing-docs.nvidia.com/optix7/api/html/group__optix__device__api.html#gaa121732a59322e799fa026d0997094d4
	float2 bary_temp = optixGetTriangleBarycentrics();
	float3 barycentrics = { bary_temp.x, bary_temp.y, 1-bary_temp.x-bary_temp.y };

	float3 N = (rtData->normals[tri.x] * barycentrics.x)
		+ (rtData->normals[tri.y] * barycentrics.y)
		+ (rtData->normals[tri.z] * barycentrics.z);

	float mag = sqrt(pow(N.x, 2) + pow(N.y, 2) + pow(N.z, 2));
	N.x /= mag; N.y /= mag; N.z /= mag;

	float3 L = optixGetWorldRayDirection();

	mag = sqrt(pow(L.x, 2) + pow(L.y, 2) + pow(L.z, 2));
	L.x /= mag; L.y /= mag; L.z /= mag;

	float diff = N.x*L.x + N.y*L.y + N.z*L.z;

	float ambient = 0.2;
	float diffuse = 0.8;
	float3 result = (diff * diffuse * colour) + ambient;

	optixSetPayload_0(float_as_int(result.x));
	optixSetPayload_1(float_as_int(result.y));
	optixSetPayload_2(float_as_int(result.z));

	optixSetPayload_4(float_as_int(optixGetRayTmax()));		
}
