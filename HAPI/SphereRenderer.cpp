#include "SphereRenderer.h"
#include "SharedRenderer.h"

#include <optix_function_table_definition.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <sutil/sutil.h>
#include <sutil/Exception.h>
#include <sutil/CUDAOutputBuffer.h>

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>

/*
 * This takes heavy inspiration from the Optix sphere example in the SDK.
 */

// This method is copy-pasted from sutil/vec_math.h because including it causes many strange errors
template <typename IntegerType>
IntegerType roundUp(IntegerType x, IntegerType y)
{
	return ((x + y - 1) / y) * y;
}

SphereRenderer::SphereRenderer() {

	// For logging purposes
	char log[2048];

	// Initialize CUDA and OptiX

	context = nullptr;
	CUDA_CHECK(cudaFree(0));
	OPTIX_CHECK(optixInit());

	CUcontext ctx = 0;
	OptixDeviceContextOptions options = {};
	options.logCallbackFunction = &optix_logger;
	options.logCallbackLevel = 4;
	OPTIX_CHECK(optixDeviceContextCreate(ctx, &options, &context));
	
	// Acceleration structure

	OptixAccelBuildOptions accel_bo = {};
	accel_bo.buildFlags = OPTIX_BUILD_FLAG_ALLOW_COMPACTION;
	accel_bo.operation = OPTIX_BUILD_OPERATION_BUILD;

	const float sphereSize = 1000000;
	OptixAabb aabb = { -sphereSize, -sphereSize, -sphereSize,
					sphereSize, sphereSize, sphereSize };
	
	CUdeviceptr aabb_buf;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&aabb_buf), sizeof(OptixAabb)));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(aabb_buf),
		&aabb,
		sizeof(OptixAabb),
		cudaMemcpyHostToDevice
	));

	OptixBuildInput aabb_input = {};
	aabb_input.type = OPTIX_BUILD_INPUT_TYPE_CUSTOM_PRIMITIVES;
	aabb_input.customPrimitiveArray.aabbBuffers = &aabb_buf;
	aabb_input.customPrimitiveArray.numPrimitives = 1;

	uint32_t aabb_flags[1] = { OPTIX_GEOMETRY_FLAG_NONE };
	aabb_input.customPrimitiveArray.flags = aabb_flags;

	aabb_input.customPrimitiveArray.numSbtRecords = 1;

	OptixAccelBufferSizes gas_sizes;
	OPTIX_CHECK(optixAccelComputeMemoryUsage(context, &accel_bo, &aabb_input, 1, &gas_sizes));

	CUdeviceptr temp_gas;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&temp_gas), gas_sizes.tempSizeInBytes));

	CUdeviceptr temp_compacted_gas;
	size_t sizeOffset = roundUp<size_t>(gas_sizes.outputSizeInBytes, 8ull); // ULL = unsigned long long
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&temp_compacted_gas),
		sizeOffset + 8
	));

	OptixAccelEmitDesc emitDesc = {};
	emitDesc.type = OPTIX_PROPERTY_TYPE_COMPACTED_SIZE;
	emitDesc.result = (CUdeviceptr)((char*)temp_compacted_gas + sizeOffset);

	OPTIX_CHECK(optixAccelBuild(
		context,
		0, // CUDA stream 0
		&accel_bo,
		&aabb_input,
		1, // Build input count
		temp_gas,
		gas_sizes.tempSizeInBytes,
		temp_compacted_gas,
		gas_sizes.outputSizeInBytes,
		&gas_handle,
		&emitDesc,
		1 // One item emitted
	));

	CUDA_CHECK(cudaFree((void*)temp_gas));
	CUDA_CHECK(cudaFree((void*)aabb_buf));

	size_t compacted_gas_size;
	CUDA_CHECK(cudaMemcpy(&compacted_gas_size, (void*)emitDesc.result, sizeof(size_t), cudaMemcpyDeviceToHost));

	if (compacted_gas_size < gas_sizes.outputSizeInBytes) {
		CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&output_gas), compacted_gas_size));
		OPTIX_CHECK(optixAccelCompact(context, 0, gas_handle, output_gas, compacted_gas_size, &gas_handle));
		CUDA_CHECK(cudaFree((void*)temp_compacted_gas));
	}
	output_gas = temp_compacted_gas;

	// Create module and pipeline for all programs

	OptixPipelineCompileOptions pipeline_co = {};
	pipeline_co.usesMotionBlur = false;
	pipeline_co.traversableGraphFlags = OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_SINGLE_GAS;
	pipeline_co.numPayloadValues = 5;
	pipeline_co.numAttributeValues = 2;
	pipeline_co.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
	pipeline_co.pipelineLaunchParamsVariableName = "params";

	OptixModuleCompileOptions module_co = {};

	// TODO: Set up cmake in VS using https://forums.developer.nvidia.com/t/how-to-generate-ptx-file-using-visual-studio/66519
	// For now, manually compile sphere.ptx

	std::string ptx_str = getPtx("sphere.ptx");
	size_t sizeof_log = sizeof(log);

	OPTIX_CHECK_LOG(optixModuleCreateFromPTX(
		context, 
		&module_co, &pipeline_co, 
		ptx_str.c_str(), ptx_str.size(),
		log, &sizeof_log,
		&sphere_module
	));

	// Initialize program groups

	OptixProgramGroupOptions program_go = {};

	// Raygen group (contains __raygen__rg)
	raygen_prog_group = nullptr;
	OptixProgramGroupDesc raygen_prog_gd = {};
	raygen_prog_gd.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;

	raygen_prog_gd.raygen.module = sphere_module;
	raygen_prog_gd.raygen.entryFunctionName = "__raygen__rg";
	
	sizeof_log = sizeof(log);
	OPTIX_CHECK_LOG(optixProgramGroupCreate(
		context,
		&raygen_prog_gd,
		1, // Number of program groups
		&program_go,
		log, &sizeof_log,
		&raygen_prog_group
	));

	// Miss group (contains __miss__ms)
	miss_prog_group = nullptr;
	OptixProgramGroupDesc miss_prog_gd = {};
	miss_prog_gd.kind = OPTIX_PROGRAM_GROUP_KIND_MISS;

	miss_prog_gd.miss.module = sphere_module;
	miss_prog_gd.miss.entryFunctionName = "__miss__ms";

	sizeof_log = sizeof(log);
	OPTIX_CHECK_LOG(optixProgramGroupCreate(
		context,
		&miss_prog_gd,
		1,
		&program_go,
		log, &sizeof_log,
		&miss_prog_group
	));

	// Hit group (contains __closesthit__ch, __intersection__sp)
	// Hit groups can contain a closest hit, any hit (unused), and intersection function
	hitgroup_prog_group = nullptr;
	OptixProgramGroupDesc hitgroup_prog_gd = {};
	hitgroup_prog_gd.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;

	hitgroup_prog_gd.hitgroup.moduleCH = sphere_module;
	hitgroup_prog_gd.hitgroup.entryFunctionNameCH = "__closesthit__ch";
	hitgroup_prog_gd.hitgroup.moduleIS = sphere_module;
	hitgroup_prog_gd.hitgroup.entryFunctionNameIS = "__intersection__sp";
	hitgroup_prog_gd.hitgroup.moduleAH = nullptr;
	hitgroup_prog_gd.hitgroup.entryFunctionNameAH = nullptr;

	sizeof_log = sizeof(log);
	OPTIX_CHECK_LOG(optixProgramGroupCreate(
		context,
		&hitgroup_prog_gd,
		1,
		&program_go,
		log, &sizeof_log,
		&hitgroup_prog_group
	));

	// Initialize pipeline

	pipeline = nullptr;
	const uint32_t max_trace_depth = 1;

	OptixProgramGroup program_groups[] = {
		raygen_prog_group,
		miss_prog_group,
		hitgroup_prog_group
	};

	OptixPipelineLinkOptions pipeline_lo = {};
	pipeline_lo.maxTraceDepth = max_trace_depth;
	pipeline_lo.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL;

	sizeof_log = sizeof(log);
	OPTIX_CHECK_LOG(optixPipelineCreate(
		context,
		&pipeline_co, &pipeline_lo,
		program_groups, sizeof(program_groups) / sizeof(program_groups[0]),
		log, &sizeof_log,
		&pipeline
	));

	OptixStackSizes stack_sizes = {};
	for (auto& prog_group : program_groups)
		OPTIX_CHECK(optixUtilAccumulateStackSizes(prog_group, &stack_sizes));

	uint32_t dc_ss_traversal; // Direct callable stack size from traversal
	uint32_t dc_ss_state; // Direct callable stack size from state
	uint32_t continuation_ss;

	OPTIX_CHECK(optixUtilComputeStackSizes(&stack_sizes, max_trace_depth,
		0, // max CC (continuation callables) depth 
		0, // max DC (direct callables) depth
		&dc_ss_traversal,
		&dc_ss_state,
		&continuation_ss
	));
	OPTIX_CHECK(optixPipelineSetStackSize(
		pipeline,
		dc_ss_traversal,
		dc_ss_state,
		continuation_ss,
		1 // Max traversal depth
	));

	sbt = {};
}

void SphereRenderer::initRaygenData(float left, float top, float sizex, float sizey, float camDist, float k, int diamond, int xMax, int yMax,
	float2* jitterBuffer) {

	//
	// Array setup
	//

	size_t jitterSize = xMax * yMax * sizeof(float2);
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&jitter_ptr), jitterSize));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(jitter_ptr),
		jitterBuffer,
		jitterSize,
		cudaMemcpyHostToDevice
	));
	
	//
	// SBT setup
	//

	size_t raygen_rs = sizeof(RayGenSbtRecord);
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&raygen_rec),
		raygen_rs
	));

	raygen_sbt.data = {
		left, top, sizex, sizey, camDist, k,
		diamond, xMax, yMax,
		reinterpret_cast<float2*>(jitter_ptr)
	};

	OPTIX_CHECK(optixSbtRecordPackHeader(raygen_prog_group, &raygen_sbt));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(raygen_rec),
		&raygen_sbt,
		raygen_rs,
		cudaMemcpyHostToDevice
	));

	sbt.raygenRecord = raygen_rec;
}

void SphereRenderer::initMissData() {
	size_t miss_rs = sizeof(MissSbtRecord);
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&miss_rec),
		miss_rs
	));

	miss_sbt.data = {};

	OPTIX_CHECK(optixSbtRecordPackHeader(miss_prog_group, &miss_sbt));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(miss_rec),
		&miss_sbt,
		miss_rs,
		cudaMemcpyHostToDevice
	));

	sbt.missRecordBase = miss_rec;
	sbt.missRecordStrideInBytes = sizeof(MissSbtRecord);
	sbt.missRecordCount = 1;
}

void SphereRenderer::initHitGroupData(float3* pointBuffer, int points, float* radiusBuffer, int radii, float* colourBuffer, int colours) {

	//
	// Array setup
	//

	size_t pointSize = points * sizeof(float3);
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&point_ptr), pointSize));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(point_ptr),
		pointBuffer,
		pointSize,
		cudaMemcpyHostToDevice
	));

	size_t radiusSize = radii * sizeof(float);
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&radius_ptr), radiusSize));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(radius_ptr),
		radiusBuffer,
		radiusSize,
		cudaMemcpyHostToDevice
	));

	size_t colourSize = colours * sizeof(float);
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&colour_ptr), colourSize));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(colour_ptr),
		colourBuffer,
		colourSize,
		cudaMemcpyHostToDevice
	));

	//
	// SBT setup
	//

	size_t hitgroup_rs = sizeof(SphereHitGroupSbtRecord);
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&hitgroup_rec),
		hitgroup_rs
	));

	hitgroup_sbt.data = {
		reinterpret_cast<float3*>(point_ptr),
		reinterpret_cast<float*>(radius_ptr),
		reinterpret_cast<float*>(colour_ptr)
	};

	OPTIX_CHECK(optixSbtRecordPackHeader(hitgroup_prog_group, &hitgroup_sbt));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(hitgroup_rec),
		&hitgroup_sbt,
		hitgroup_rs,
		cudaMemcpyHostToDevice
	));

	sbt.hitgroupRecordBase = hitgroup_rec;
	sbt.hitgroupRecordStrideInBytes = sizeof(SphereHitGroupSbtRecord);
	sbt.hitgroupRecordCount = 1;
}

std::vector<ImgData> SphereRenderer::launch(int width, int height) {

	sutil::CUDAOutputBuffer<ImgData> image_buffer(sutil::CUDAOutputBufferType::CUDA_DEVICE, width, height);

	params = {
		image_buffer.map(),
		gas_handle
	};

	CUstream stream;
	CUDA_CHECK(cudaStreamCreate(&stream));

	CUdeviceptr param_ptr;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&param_ptr), sizeof(SphereParams)));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(param_ptr),
		&params,
		sizeof(params),
		cudaMemcpyHostToDevice
	));

	OPTIX_CHECK(optixLaunch(pipeline, stream, param_ptr, sizeof(SphereParams), &sbt, width, height, 1)); // Depth = 1

	CUDA_SYNC_CHECK();

	image_buffer.unmap();

	ImgData* image_ptr = image_buffer.getHostPointer();

	std::vector<ImgData> image_vec;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			image_vec.push_back(image_ptr[i * width + j]);
	}

	return image_vec;

	// Memory cleanup
	CUDA_CHECK(cudaFree(reinterpret_cast<void*>(sbt.raygenRecord)));
	CUDA_CHECK(cudaFree(reinterpret_cast<void*>(sbt.missRecordBase)));
	CUDA_CHECK(cudaFree(reinterpret_cast<void*>(sbt.hitgroupRecordBase)));
	CUDA_CHECK(cudaFree(reinterpret_cast<void*>(output_gas)));

	OPTIX_CHECK(optixPipelineDestroy(pipeline));
	OPTIX_CHECK(optixProgramGroupDestroy(raygen_prog_group));
	OPTIX_CHECK(optixProgramGroupDestroy(miss_prog_group));
	OPTIX_CHECK(optixProgramGroupDestroy(hitgroup_prog_group));
	OPTIX_CHECK(optixModuleDestroy(sphere_module));
	OPTIX_CHECK(optixDeviceContextDestroy(context));
}
