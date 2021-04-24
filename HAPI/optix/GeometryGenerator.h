#pragma once
#define NOMINMAX


#include <optix.h>
#include <optixu/optixpp_namespace.h>
#include "tiny_obj_loader.h"
#include <iostream>
struct VertexData {
	optix::float3 position;
	optix::float3 normal;
	optix::float2 texCoords;
};
//OPTIX USES triangles -> Counter clockwise
class GeomteryGenerator {
	//The optix::make_float3 and optix::make_float2 functions in optixu/optixpp_math_namespace.h wasn't work. Re wrote it in here.
	inline static optix::float2 makeFloat2(float x, float y);
	inline static optix::float3 makeFloat3(float x, float y, float z);
public:
	/*Creates the vertex information for a plane and returns a geometry object of a plane */
	static optix::Geometry createPlane(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg, float width, float height);
	/*Takes in vertex and indice information and returns an a geometry object of the mesh*/
	static optix::Geometry createGeometry(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg, std::vector<VertexData> vertices, std::vector<unsigned int> indices, int numTri);
	

	static optix::Geometry loadMesh(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg, std::string fileName);
};