#include "Renderer.h"

Renderer::Renderer(int width, int height, int rayCount, int entryPoints, int stackSize) {
	this->WIDTH = width;
	this->HEIGHT = height;

	context = optix::Context::create();
	context->setRayTypeCount(rayCount);
	context->setEntryPointCount(entryPoints);
	context->setStackSize(stackSize);
}

void Renderer::createBuffer(std::string bufferName) {
	optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT_OUTPUT | RT_BUFFER_GPU_LOCAL, RT_FORMAT_FLOAT4, WIDTH, HEIGHT);
	context[bufferName]->set(buffer);
}


void Renderer::render(int entryPoints) {
	try {
		context->validate();
		context->launch(entryPoints, WIDTH, HEIGHT);
	}
	catch (optix::Exception e) {
		std::cout << e.getErrorString() << std::endl;
	}
}

void Renderer::display(int* argc, char* argv[], std::string outputBufferName) {
	sutil::initGlut(argc, argv);
	sutil::displayBufferGlut("test", context[outputBufferName]->getBuffer());
}


void Renderer::cleanUp() {
	if (context) {
		context->destroy();
	}
}

optix::Context Renderer::getContext() {
	return context;
}