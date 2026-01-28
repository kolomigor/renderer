#pragma once
#include "camera.h"
#include "lights.h"
#include "picture.h"
#include "renderer.h"
#include "world.h"

class Application {
public:
	Application();
	void Run();

private:
	World world_;
	Camera camera_;
	Lights lights_;
	Renderer renderer_;
	Picture picture_;
};
