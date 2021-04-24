#define NOMINMAX
#include <optix.h>
#include <optixu/optixu_matrix.h>

#include <string>
#include <map>
#include "GeometryGenerator.h";
#pragma once
class SceneTree {
private:
	optix::Context context;
	std::map<std::string, optix::Material> materials;
	std::map<std::string, optix::Geometry> geometries;

	optix::Group rootGroup;

public:

	SceneTree(optix::Context context);

	void createMaterial(optix::Program closeHit, std::string matName);

	void createPlane(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName);

	void createGeometry(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName);
	void initScene();
};