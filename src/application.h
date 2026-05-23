#pragma once

#include "renderer.h"
#include "scene.h"

namespace renderer {

class Application {
public:
	Application();
	void run();

private:
	Scene scene_;
	Renderer renderer_;
};

} // namespace renderer
