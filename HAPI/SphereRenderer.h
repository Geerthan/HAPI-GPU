#pragma once
#include <optix.h>

#include <string>
#include <vector>

#include "SphereSBTData.h"

class SphereRenderer {
public:
	OptixDeviceContext context;
	OptixPipeline pipeline;
	OptixTraversableHandle gas_handle;
	OptixProgramGroup raygen_prog_group, miss_prog_group, hitgroup_prog_group, exception_prog_group;
	OptixModule sphere_module;
	CUdeviceptr output_gas;
	OptixShaderBindingTable sbt;
	SphereParams params;

	CUdeviceptr raygen_rec, miss_rec, hitgroup_rec;

	RayGenSbtRecord raygen_sbt;
	MissSbtRecord miss_sbt;
	SphereHitGroupSbtRecord hitgroup_sbt;

	// Raygen
	CUdeviceptr jitter_ptr;

	// Hitgroup
	CUdeviceptr point_ptr, radius_ptr, colour_ptr;

	// Params
	CUdeviceptr result_ptr, intersect_ptr;

	SphereRenderer();
	void initRaygenData(float left, float top, float sizex, float sizey, float camDist, float k, int diamond, int xMax, int yMax, 
		float2* jitterBuffer);
	void initMissData();
	void initHitGroupData(float3* pointBuffer, int points, float* radiusBuffer, int radii, float* colourBuffer, int colours);
	std::vector<ImgData> launch(int width, int height);
};

std::string getPtx(std::string ptx_name);