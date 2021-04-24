/*#include "SceneTree.h"

SceneTree::SceneTree()
{
}

SceneTree::SceneTree(optix::Context context){
	this->context = context;

	optix::Acceleration rootAcceleration = this->context->createAcceleration("Trbvh");
	rootGroup = this->context->createGroup();
	rootGroup->setAcceleration(rootAcceleration);

	this->context["sysTopObject"]->set(rootGroup);
}

void SceneTree::createMaterial(optix::Program closeHit, std::string matName){

	optix::Material mat = context->createMaterial();
	mat->setClosestHitProgram(0, closeHit);
	materials[matName] = mat;
}

void SceneTree::createPlane(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName){
	optix::Geometry obj = optix::Geometry(); //TODO. MAKE IT WORK FOR HAPI
	geometries[geoName] = obj;
}

void SceneTree::createGeometry(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName) {
	optix::Geometry obj = optix::Geometry();
	geometries[geoName] = obj;
}

void SceneTree::initScene() {
	
}

optix::Group SceneTree::getRoot()
{
	return rootGroup;
}
*/