#pragma once

#include "camera.h"
#include "lights.h"
#include "world.h"

#include <filesystem>

namespace renderer {

struct Scene {
	World world;
	Camera camera;
	Lights lights;
};

Scene loadScene(const std::filesystem::path& filename, Width width, Height height);

} // namespace renderer
