#include "GeometryGenerator.h"


optix::float3 GeomteryGenerator::makeFloat3(float x, float y, float z)
{
	optix::float3 f;
	f.x = x;
	f.y = y;
	f.z = z;
	return f;
}

optix::float2 GeomteryGenerator::makeFloat2(float x, float y)
{
	optix::float2 f;
	f.x = x;
	f.y = y;
	return f;
}

optix::Geometry GeomteryGenerator::createPlane(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg,
											   float width, float height) {
	std::vector<VertexData> vertices;
	VertexData botLeft;
	botLeft.position = makeFloat3(-1.0, 0, -1.0f);
	botLeft.normal = makeFloat3(1, 0, 0.0f);
	botLeft.texCoords = makeFloat2(0, 0);
	vertices.push_back(botLeft);


	VertexData botRight;
	botRight.position = makeFloat3(1.0, 0.0f, -1.0f);
	botRight.normal = makeFloat3(0, 1, 0.0f);
	botRight.texCoords = makeFloat2(1, 0);
	vertices.push_back(botRight);

	VertexData topRight;
	topRight.position = makeFloat3(1, 0, 1.0);
	topRight.normal = makeFloat3(0, 0, 1.0f);
	topRight.texCoords = makeFloat2(1, 1);
	vertices.push_back(topRight);

	VertexData topLeft;
	topLeft.position = makeFloat3(-1.0f, 0, 1.0f);
	topLeft.normal = makeFloat3(1, 1, 0.0f);
	topLeft.texCoords = makeFloat2(0, 1);
	vertices.push_back(topLeft);



	std::vector<unsigned int> indicies;
	//Bot right triangle
	indicies.push_back(0);
	indicies.push_back(1);
	indicies.push_back(2);

	indicies.push_back(2);
	indicies.push_back(3);
	indicies.push_back(0);
	return createGeometry(context, boundBoxPorg, intersectProg, vertices, indicies, 4);
}

optix::Geometry GeomteryGenerator::createGeometry(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg,
													std::vector<VertexData> vertices, std::vector<unsigned int> indices, int numTri){
	optix::Geometry geometry;
	geometry = context->createGeometry();

	//Create a buffer for vertices and allocated the required memory
	optix::Buffer vertexBuffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_USER);
	vertexBuffer->setElementSize(sizeof(VertexData));
	vertexBuffer->setSize(vertices.size());
	//Access the buffer and store the vertex data into it. 
	void *vertexLoc = vertexBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
	memcpy(vertexLoc, vertices.data(), sizeof(VertexData) * vertices.size());
	vertexBuffer->unmap();


	
	//Same as above but for indices
	optix::Buffer indexBuffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT3, numTri);
	
	void *indexLoc = indexBuffer->map(0, RT_BUFFER_MAP_WRITE_DISCARD);
	memcpy(indexLoc, indices.data(), sizeof(optix::uint3) * numTri);
	
	indexBuffer->unmap();	


	geometry->setBoundingBoxProgram(boundBoxPorg);
	geometry->setIntersectionProgram(intersectProg);
	//Where the vertex and index buffers are stored in the kernel program
	geometry["vertexBuffer"]->setBuffer(vertexBuffer);
	geometry["indexBuffer"]->setBuffer(indexBuffer);
	geometry->setPrimitiveCount(numTri);
	return geometry;
}

optix::Geometry GeomteryGenerator::loadMesh(optix::Context context, optix::Program boundBoxPorg, optix::Program intersectProg, std::string fileName){
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string objName = fileName + ".obj";
	std::string err = tinyobj::LoadObj(shapes, materials, objName.c_str(), 0);

	if (!err.empty()) {
		std::cout << "Failed to read from: " << objName << std::endl;
		std::cerr << err << std::endl;
		return NULL;
	}
	else {
		std::cout << "Reading From: " << objName << std::endl;
 	}

	std::vector<VertexData> vertices;
	std::vector<unsigned int> indicies;
	
	int numVert = (int)shapes[0].mesh.positions.size();
	for (int i = 0; i < numVert; i+=3) {
		VertexData data;
		data.position.x = shapes[0].mesh.positions[i];
		data.position.y = shapes[0].mesh.positions[i+1];
		data.position.z = shapes[0].mesh.positions[i+2];
		
		
		
		data.normal.x = shapes[0].mesh.normals[i];
		data.normal.y = shapes[0].mesh.normals[i+1];
		data.normal.z = shapes[0].mesh.normals[i+2];
		

		
		data.texCoords.x = 0;
		data.texCoords.y = 0;

		
		vertices.push_back(data);
	}
	int numIndicies = (int)shapes[0].mesh.indices.size();
	int numTri = 0;
	for (int i = 0; i < numIndicies; i++) {
		indicies.push_back(shapes[0].mesh.indices[i]);
		if (i % 3 == 2) {
			numTri++;
		}
	}

	return createGeometry(context, boundBoxPorg, intersectProg, vertices, indicies, numTri);

}
