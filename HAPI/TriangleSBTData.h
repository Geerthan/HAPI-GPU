#pragma once
#include "SharedSBTData.h"

struct TriangleParams {
	ImgData* image_buffer;
	OptixTraversableHandle handle;
	float3* v_buf;
	int3* i_buf;
};

struct TriangleHitGroupData {
	float3* normals;
	float3* colourBuffer;
};

typedef SbtRecord<TriangleHitGroupData> TriangleHitGroupSbtRecord;