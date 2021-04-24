/*#define NOMINMAX
#pragma once
#include <optix.h>
//#include <optixu/optixu_matrix.h>
#include <optixu/optixpp_namespace.h>

#include <string>
#include <map>
class SceneTree {
private:
	optix::Context context;
	std::map<std::string, optix::Material> materials;
	std::map<std::string, optix::Geometry> geometries;

	optix::Group rootGroup;

public:
	SceneTree();
	SceneTree(optix::Context context);

	void createMaterial(optix::Program closeHit, std::string matName);

	void createPlane(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName);

	void createGeometry(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName);
	void initScene();
	optix::Group getRoot();
};*/