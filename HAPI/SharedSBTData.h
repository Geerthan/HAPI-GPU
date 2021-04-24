#pragma once
#include <cuda_runtime.h>

struct Ray7 {
	float3 origin, direction;
	float tmax;
};

struct ImgData {
	float2 result_1;
	float2 result_2;
	float2 result_3;
	int intersect;
};

struct RayGenData {
	float left, top, sizex, sizey, camDist, k;
	int diamond, xMax, yMax;
	float2* jitterBuffer;
};

struct MissData {};

template <typename T>
struct SbtRecord {
	__align__(OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
	T data;
};

typedef SbtRecord<RayGenData> RayGenSbtRecord;
typedef SbtRecord<MissData> MissSbtRecord;
