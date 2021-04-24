#include "SceneTree.h"

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
	optix::Geometry plane = GeomteryGenerator::createPlane(context, boundBox, intersect, WIDTH, HEIGHT); //Create the geometry. You don't need to create multiple geos, just multi instances
	geometries[geoName] = plane;
}

void SceneTree::createGeometry(optix::Program boundBox, optix::Program intersect, int WIDTH, int HEIGHT, std::string geoName) {
	optix::Geometry obj = GeomteryGenerator::loadMesh(context, boundBox, intersect, geoName);
	geometries[geoName] = obj;
}

void SceneTree::initScene() {
	unsigned int count = 0;
	optix::GeometryInstance planeInstance = context->createGeometryInstance();
	planeInstance->setGeometry(geometries["sphere"]); //Create and instance of it and provide a material to it
	planeInstance->setMaterialCount(1);
	planeInstance->setMaterial(0, materials["firstMat"]); //TODO create material. Kinda like a texture map

	optix::Acceleration planeAcceleration = context->createAcceleration("Trbvh");
	optix::GeometryGroup planeGroup = context->createGeometryGroup(); //Put the instance of the geometry into the group
	planeGroup->setAcceleration(planeAcceleration);
	planeGroup->setChildCount(1);
	planeGroup->setChild(0, planeInstance);


	float trafoPlane[16] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	optix::Matrix4x4 matrixPlane(trafoPlane);
	optix::float3 translate;
	translate.x = -1.0f;
	translate.y = 0.0f;
	translate.z = -1.0f;

	matrixPlane = matrixPlane.translate(translate);

	optix::float3 xAxis;
	xAxis.x = 1.0f;
	xAxis.y = 0.0f;
	xAxis.z = 0.0f;

	//matrixPlane = matrixPlane.rotate(1.5708f, xAxis);
	optix::Transform trPlane = context->createTransform(); //Create a transformation
	trPlane->setChild(planeGroup); //Set the transformation as a parent to the plane
	trPlane->setMatrix(false, matrixPlane.getData(), matrixPlane.inverse().getData());

	count = rootGroup->getChildCount();
	rootGroup->setChildCount(1 + count);
	rootGroup->setChild(count, trPlane);

	optix::GeometryInstance secondInstance = context->createGeometryInstance();
	secondInstance->setGeometry(geometries["sphere"]); //Create and instance of it and provide a material to it
	secondInstance->setMaterialCount(1);
	secondInstance->setMaterial(0, materials["secondMat"]); //TODO create material. Kinda like a texture map

	optix::GeometryGroup secondGroup = context->createGeometryGroup(); //Put the instance of the geometry into the group
	secondGroup->setAcceleration(planeAcceleration);
	secondGroup->setChildCount(1);
	secondGroup->setChild(0, secondInstance);


	translate.z = -2.0f;
	translate.x = 1.0f;
	matrixPlane = matrixPlane.translate(translate);
	optix::Transform triSecond = context->createTransform();
	triSecond->setChild(secondGroup);
	triSecond->setMatrix(false, matrixPlane.getData(), matrixPlane.inverse().getData());

	count = rootGroup->getChildCount();
	rootGroup->setChildCount(1 + count);
	rootGroup->setChild(count, triSecond);
}
