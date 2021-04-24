#include "TriangleRenderer.h"
#include "SharedRenderer.h"

#include <optix_function_table_definition.h>
#include <optix_stack_size.h>
#include <optix_stubs.h>

#include <sutil/sutil.h>
#include <sutil/Exception.h>
#include <sutil/CUDAOutputBuffer.h>
//#include <sutil/vec_math.h> This include is broken

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Matrix.h"
#include "Internal.h"

/*
 * This takes heavy inspiration from the Optix SDK examples, notably the sphere example. 
 */

 // This method is copy-pasted from sutil/vec_math.h because including it causes many strange errors
template <typename IntegerType>
IntegerType roundUp(IntegerType x, IntegerType y) {
	return ((x + y - 1) / y) * y;
}

TriangleRenderer::TriangleRenderer(double initialCtm[4][4], float3* vertices, int nv, int3* indices, int ni) {

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

	// First transform triangle data, normally supposed to be in Matrix.cpp (MxV) and Compile.cpp (compile)
	double v1[4], v2[4];
	for (int i = 0; i < nv; i++) {
		v1[0] = vertices[i].x;
		v1[1] = vertices[i].y;
		v1[2] = vertices[i].z;
		MxV(initialCtm, v1, v2);
		vertices[i].x = mx * v2[0] + cx;
		vertices[i].y = my * v2[1] + cy;
		vertices[i].z = mz * v2[2] + cz;
	}

	size_t v_size = nv * sizeof(float3);
	CUdeviceptr v_acc_buf;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&v_acc_buf), v_size));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(v_acc_buf),
		vertices,
		v_size,
		cudaMemcpyHostToDevice
	));

	// Copy into params for normals
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&v_buf), v_size));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(v_buf),
		vertices,
		v_size,
		cudaMemcpyHostToDevice
	));

	size_t i_size = ni * sizeof(int3);
	CUdeviceptr i_acc_buf;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&i_acc_buf), i_size));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(i_acc_buf),
		indices,
		i_size,
		cudaMemcpyHostToDevice
	));

	// Copy into params for normals
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&i_buf), i_size));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(i_buf),
		indices,
		i_size,
		cudaMemcpyHostToDevice
	));

	OptixBuildInput triangle_input = {};
	triangle_input.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;

	triangle_input.triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
	// Refer to https://github.com/ingowald/optix7course/blob/master/example04_firstTriangleMesh/SampleRenderer.cpp
	triangle_input.triangleArray.vertexStrideInBytes = 0; // May have to be changed later
	triangle_input.triangleArray.numVertices = nv;
	triangle_input.triangleArray.vertexBuffers = &v_acc_buf;

	triangle_input.triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
	triangle_input.triangleArray.numIndexTriplets = ni;
	triangle_input.triangleArray.indexBuffer = i_acc_buf;

	uint32_t triangle_flags[1] = { OPTIX_GEOMETRY_FLAG_NONE }; // Add per-primitive materials here

	triangle_input.triangleArray.flags = triangle_flags;
	triangle_input.triangleArray.numSbtRecords = 1;
	triangle_input.triangleArray.sbtIndexOffsetBuffer = 0; // These three lines may be optional
	triangle_input.triangleArray.sbtIndexOffsetSizeInBytes = 0;
	triangle_input.triangleArray.sbtIndexOffsetStrideInBytes = 0;

	OptixAccelBufferSizes gas_sizes;
	OPTIX_CHECK(optixAccelComputeMemoryUsage(context, &accel_bo, &triangle_input, 1, &gas_sizes));

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
		&triangle_input,
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
	CUDA_CHECK(cudaFree((void*)v_acc_buf)); // v_buf throws an error if it is not freed here (?)
	CUDA_CHECK(cudaFree((void*)i_acc_buf));

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
	pipeline_co.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE; // Can also use OPTIX_EXCEPTION_FLAG_STACK_OVERFLOW
	pipeline_co.pipelineLaunchParamsVariableName = "params";

	OptixModuleCompileOptions module_co = {};

	// TODO: Set up cmake in VS using https://forums.developer.nvidia.com/t/how-to-generate-ptx-file-using-visual-studio/66519
	// For now, manually compile ptx

	std::string ptx_str = getPtx("tmesh.ptx");
	size_t sizeof_log = sizeof(log);

	OPTIX_CHECK_LOG(optixModuleCreateFromPTX(
		context,
		&module_co, &pipeline_co,
		ptx_str.c_str(), ptx_str.size(),
		log, &sizeof_log,
		&triangle_module
	));

	// Initialize program groups

	OptixProgramGroupOptions program_go = {};

	// Raygen group (contains __raygen__rg)
	raygen_prog_group = nullptr;
	OptixProgramGroupDesc raygen_prog_gd = {};
	raygen_prog_gd.kind = OPTIX_PROGRAM_GROUP_KIND_RAYGEN;

	raygen_prog_gd.raygen.module = triangle_module;
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

	// Hit group (contains __closesthit__ch, __intersection__sp)
	// Hit groups can contain a closest hit, any hit (unused), and intersection function
	hitgroup_prog_group = nullptr;
	OptixProgramGroupDesc hitgroup_prog_gd = {};
	hitgroup_prog_gd.kind = OPTIX_PROGRAM_GROUP_KIND_HITGROUP;

	hitgroup_prog_gd.hitgroup.moduleCH = triangle_module;
	hitgroup_prog_gd.hitgroup.entryFunctionNameCH = "__closesthit__ch";
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
		hitgroup_prog_group
	};

	OptixPipelineLinkOptions pipeline_lo = {};
	pipeline_lo.maxTraceDepth = max_trace_depth;
	pipeline_lo.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_FULL; // Can be reduced

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

	//
	// SBT setup
	//

	size_t miss_rs = sizeof(MissSbtRecord);
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&miss_rec),
		miss_rs
	));

	sbt.missRecordBase = miss_rec;
	sbt.missRecordStrideInBytes = sizeof(MissSbtRecord);
	sbt.missRecordCount = 1;
	
}

void TriangleRenderer::initRaygenData(float left, float top, float sizex, float sizey, float camDist, float k, int diamond, int xMax, int yMax,
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

void TriangleRenderer::initHitGroupData(float3* normalBuffer, int nn, float3* colourBuffer, int colours) {

	//
	// Array setup
	//

	size_t normalSize = nn * sizeof(float3);
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&normal_ptr), normalSize));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(normal_ptr),
		normalBuffer,
		normalSize,
		cudaMemcpyHostToDevice
	));


	size_t colourSize = colours * sizeof(float3);
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

	size_t hitgroup_rs = sizeof(TriangleHitGroupSbtRecord);
	CUDA_CHECK(cudaMalloc(
		reinterpret_cast<void**>(&hitgroup_rec),
		hitgroup_rs
	));

	hitgroup_sbt.data = {
		reinterpret_cast<float3*>(normal_ptr),
		reinterpret_cast<float3*>(colour_ptr)
	};

	OPTIX_CHECK(optixSbtRecordPackHeader(hitgroup_prog_group, &hitgroup_sbt));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(hitgroup_rec),
		&hitgroup_sbt,
		hitgroup_rs,
		cudaMemcpyHostToDevice
	));

	sbt.hitgroupRecordBase = hitgroup_rec;
	sbt.hitgroupRecordStrideInBytes = sizeof(TriangleHitGroupSbtRecord);
	sbt.hitgroupRecordCount = 1;
}

std::vector<ImgData> TriangleRenderer::launch(int width, int height) {

	sutil::CUDAOutputBuffer<ImgData> image_buffer(sutil::CUDAOutputBufferType::CUDA_DEVICE, width, height);

	params = {
		image_buffer.map(),
		gas_handle,
		reinterpret_cast<float3*>(v_buf),
		reinterpret_cast<int3*>(i_buf)
	};

	CUstream stream;
	CUDA_CHECK(cudaStreamCreate(&stream));

	CUdeviceptr param_ptr;
	CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&param_ptr), sizeof(TriangleParams)));
	CUDA_CHECK(cudaMemcpy(
		reinterpret_cast<void*>(param_ptr),
		&params,
		sizeof(params),
		cudaMemcpyHostToDevice
	));

	OPTIX_CHECK(optixLaunch(pipeline, stream, param_ptr, sizeof(TriangleParams), &sbt, width, height, 1)); // Depth = 1

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
	OPTIX_CHECK(optixProgramGroupDestroy(hitgroup_prog_group));
	OPTIX_CHECK(optixModuleDestroy(triangle_module));
	OPTIX_CHECK(optixDeviceContextDestroy(context));
}
