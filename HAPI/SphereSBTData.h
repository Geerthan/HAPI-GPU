#pragma once
#include "SharedSBTData.h"

struct SphereParams {
	ImgData* image_buffer;
	OptixTraversableHandle handle;
};

struct SphereHitGroupData {
	float3* pointBuffer;
	float* radiusBuffer;
	float* colourBuffer;
};

typedef SbtRecord<SphereHitGroupData> SphereHitGroupSbtRecord;