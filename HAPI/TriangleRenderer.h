#pragma once
#include <optix.h>

#include <string>
#include <vector>

#include "TriangleSBTData.h"

class TriangleRenderer {
public:
	OptixDeviceContext context;
	OptixPipeline pipeline;
	OptixTraversableHandle gas_handle;
	OptixProgramGroup raygen_prog_group, hitgroup_prog_group;
	OptixModule triangle_module;
	CUdeviceptr output_gas;
	OptixShaderBindingTable sbt;
	TriangleParams params;

	CUdeviceptr raygen_rec, miss_rec, hitgroup_rec;

	RayGenSbtRecord raygen_sbt;
	TriangleHitGroupSbtRecord hitgroup_sbt;

	// Raygen
	CUdeviceptr jitter_ptr;

	// Hitgroup
	CUdeviceptr normal_ptr, colour_ptr;

	// Params
	CUdeviceptr result_ptr, intersect_ptr;
	CUdeviceptr v_buf, i_buf;

	TriangleRenderer(double initialCtm[4][4], float3* vertices, int nv, int3* indices, int ni);
	void initRaygenData(float left, float top, float sizex, float sizey, float camDist, float k, int diamond, int xMax, int yMax,
		float2* jitterBuffer);
	void initHitGroupData(float3* normalBuffer, int nn, float3* colourBuffer, int colours);
	std::vector<ImgData> launch(int width, int height);
};

std::string getPtx(std::string ptx_name);