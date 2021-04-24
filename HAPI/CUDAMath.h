#pragma once
#include <cuda_runtime.h>

const float3 operator-(float3 a, float3 b);
const void operator+=(float3 &a, float3 b);
const void operator/=(float3& a, float b);
