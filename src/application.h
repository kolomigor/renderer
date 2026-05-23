#pragma once

#include "camera.h"
#include "renderer.h"
#include "world.h"

namespace renderer {

class Application {
public:
	Application();
	void run();

private:
	World world_;
	Camera camera_;
	Renderer renderer_;
};

} // namespace renderer
