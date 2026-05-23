#include "application.h"

#include "window.h"

#include <cassert>
#include <chrono>
#include <filesystem>

namespace renderer {
namespace {

constexpr Width kFrameWidth{800};
constexpr Height kFrameHeight{600};
constexpr float kCameraMoveSpeed = 4.0f;
constexpr float kFastCameraMoveMultiplier = 3.0f;
constexpr float kKeyboardTurnSpeed = 1.75f;
constexpr float kMouseLookSensitivity = 0.004f;
const std::filesystem::path kDefaultScenePath{"assets/scenes/default.json"};

using Clock = std::chrono::steady_clock;

float secondsSince(Clock::time_point start, Clock::time_point end) {
	return std::chrono::duration<float>(end - start).count();
}

vec3 normalizedOrZero(const vec3& movement) {
	const float movement_length = length(movement);
	if (movement_length == 0.0f) {
		return {0.0f, 0.0f, 0.0f};
	}
	return movement / movement_length;
}

void applyCameraInput(Camera *camera, const FrameInput& input, float frame_seconds) {
	assert(camera != nullptr);

	const float speed =
	    kCameraMoveSpeed * (input.fast_camera_movement ? kFastCameraMoveMultiplier : 1.0f);
	camera->moveLocal(normalizedOrZero(input.camera_movement) * speed * frame_seconds);

	const float yaw_delta = input.camera_turn.x * kKeyboardTurnSpeed * frame_seconds +
	                        input.camera_look_delta.x * kMouseLookSensitivity;
	const float pitch_delta = input.camera_turn.y * kKeyboardTurnSpeed * frame_seconds -
	                          input.camera_look_delta.y * kMouseLookSensitivity;
	camera->turn(yaw_delta, pitch_delta);
}

std::filesystem::path defaultScenePath() {
	if (std::filesystem::exists(kDefaultScenePath)) {
		return kDefaultScenePath;
	}
	const std::filesystem::path parent_scene_path = ".." / kDefaultScenePath;
	if (std::filesystem::exists(parent_scene_path)) {
		return parent_scene_path;
	}
	return kDefaultScenePath;
}

} // namespace

Application::Application()
    : scene_(loadScene(defaultScenePath(), kFrameWidth, kFrameHeight)),
      renderer_(kFrameWidth, kFrameHeight) {
}

void Application::run() {
	Window window(kFrameWidth, kFrameHeight, "Renderer");
	Clock::time_point previous_frame = Clock::now();
	while (window.isOpen()) {
		const Clock::time_point current_frame = Clock::now();
		const float frame_seconds = secondsSince(previous_frame, current_frame);
		previous_frame = current_frame;

		const FrameInput input = window.pollEvents();
		if (!window.isOpen()) {
			break;
		}
		applyCameraInput(&scene_.camera, input, frame_seconds);
		const Picture picture = renderer_.render(scene_.world, scene_.camera, scene_.lights);
		window.show(picture);
	}
}

} // namespace renderer
