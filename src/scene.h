#pragma once

#include "camera.h"
#include "world.h"

#include <filesystem>

namespace renderer {

struct Scene {
	World world;
	Camera camera;
};

Scene loadScene(const std::filesystem::path& filename, Width width, Height height);
Scene loadObjScene(const std::filesystem::path& filename, Width width, Height height);

} // namespace renderer
