/*
nvcc -ptx -ccbin "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.14.26428\bin\Hostx64\x64" -I "C:\ProgramData\NVIDIA Corporation\OptiX SDK 6.0.0\include" bounding_box.cu

http://blog.three-eyed-games.com/2018/05/03/gpu-ray-tracing-in-unity-part-1/
*/

#include <string>
#include "Renderer.h"
#include "SceneTree.h"
#include <optixu/optixu_matrix.h>
int main(int argc, char* argv[]) {
	optix::float3 camPos;
	camPos.x = 0.0f;
	camPos.y = 0.0f;
	camPos.z = 2.0f;

	optix::float3 camU;
	camU.x = 1.0f;
	camU.y = 0.0f;
	camU.z = 0.0f;

	optix::float3 camV;
	camV.x = 0.0f;
	camV.y = 1.0f;
	camV.z = 0.0f;

	optix::float3 camW;
	camW.x = 0.0f;
	camW.y = 0.0f;
	camW.z = -1.0f;


	int width = 512u;
	int height = 384u;
	int rayCount = 1;
	int entryPoint = 1;
	std::string outputBuffer = "result_buffer";
	Renderer renderer(width, height, rayCount, entryPoint, 800);

	ProgramCreator progCreator(renderer.getContext());
	SceneTree sceneTree(renderer.getContext());

	renderer.createBuffer(outputBuffer);
	progCreator.createRaygenProgram("ray_generation.ptx", "rayGeneration", 0);
	progCreator.createMissProgram("miss.ptx", "miss_environment_constant", 0);

	progCreator.createProgramVariable3f("rayGeneration", "sysCameraPosition", camPos.x, camPos.y, camPos.z);
	progCreator.createProgramVariable3f("rayGeneration", "sysCameraU", camU.x, camU.y, camU.z);
	progCreator.createProgramVariable3f("rayGeneration", "sysCameraV", camV.x, camV.y, camV.z);
	progCreator.createProgramVariable3f("rayGeneration", "sysCameraW", camW.x, camW.y, camW.z);
	progCreator.createProgramVariable1i("rayGeneration", "maxDepth", 2);

	optix::Program boundBox = progCreator.createProgram("bounding_box.ptx", "boundbox_triangle_indexed");
	optix::Program intersectProg = progCreator.createProgram("intersection.ptx", "intersect_triangle_indexed");
	optix::Program closeHitProg = progCreator.createProgram("closest_hit.ptx", "closestHit");


	optix::Program redHit = progCreator.createProgram("red_hit.ptx", "closestHit");

	sceneTree.createMaterial(closeHitProg, "firstMat");
	sceneTree.createGeometry(boundBox, intersectProg, width, height, "sphere");

	sceneTree.createMaterial(redHit, "secondMat");
	sceneTree.createGeometry(boundBox, intersectProg, width, height, "monkey");
	sceneTree.initScene();

	renderer.render(0);
	
	renderer.display(&argc,argv, outputBuffer);
	
	renderer.cleanUp();

}