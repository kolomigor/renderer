#pragma once

#include "renderer.h"
#include "scene.h"

#include <filesystem>
#include <optional>

namespace renderer {

struct ApplicationConfig {
	std::filesystem::path scene_path;
	std::optional<int> target_fps = std::nullopt;
};

class Application {
public:
	explicit Application(ApplicationConfig config);
	void run();

private:
	Scene scene_;
	Renderer renderer_;
	std::optional<int> target_fps_;
	vec3 camera_movement_velocity_{0.0f, 0.0f, 0.0f};
	vec2 camera_turn_velocity_{0.0f, 0.0f};
};

} // namespace renderer
