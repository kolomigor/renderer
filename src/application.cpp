#include "application.h"

#include "window.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace renderer {
namespace {

constexpr Width kFrameWidth{800};
constexpr Height kFrameHeight{600};
constexpr float kCameraMoveSpeed = 4.0f;
constexpr float kFastCameraMoveMultiplier = 3.0f;
constexpr float kKeyboardTurnSpeed = 1.75f;
constexpr float kMouseLookSensitivity = 0.004f;
constexpr float kCameraMovementSmoothness = 16.0f;
constexpr float kCameraTurnSmoothness = 18.0f;

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

float smoothingFactor(float smoothness, float frame_seconds) {
	return 1.0f - std::exp(-smoothness * frame_seconds);
}

void applyCameraInput(Camera *camera, const FrameInput& input, float frame_seconds,
                      vec3 *movement_velocity, vec2 *turn_velocity) {
	assert(camera != nullptr);
	assert(movement_velocity != nullptr);
	assert(turn_velocity != nullptr);

	const float speed =
	    kCameraMoveSpeed * (input.fast_camera_movement ? kFastCameraMoveMultiplier : 1.0f);
	const vec3 target_movement_velocity = normalizedOrZero(input.camera_movement) * speed;
	*movement_velocity +=
	    (target_movement_velocity - *movement_velocity) *
	    smoothingFactor(kCameraMovementSmoothness, frame_seconds);
	camera->moveLocal(*movement_velocity * frame_seconds);

	const vec2 target_turn_velocity = input.camera_turn * kKeyboardTurnSpeed;
	*turn_velocity += (target_turn_velocity - *turn_velocity) *
	                  smoothingFactor(kCameraTurnSmoothness, frame_seconds);
	const float yaw_delta =
	    turn_velocity->x * frame_seconds + input.camera_look_delta.x * kMouseLookSensitivity;
	const float pitch_delta =
	    turn_velocity->y * frame_seconds - input.camera_look_delta.y * kMouseLookSensitivity;
	camera->turn(yaw_delta, pitch_delta);
}

Scene loadConfiguredScene(const ApplicationConfig& config) {
	if (config.obj_path.has_value()) {
		return loadObjScene(*config.obj_path, kFrameWidth, kFrameHeight);
	}
	if (!config.scene_path.has_value()) {
		throw std::runtime_error("missing scene path");
	}
	return loadScene(*config.scene_path, kFrameWidth, kFrameHeight);
}

} // namespace

Application::Application(ApplicationConfig config)
    : scene_(loadConfiguredScene(config)),
      renderer_(kFrameWidth, kFrameHeight),
      target_fps_(config.target_fps) {
}

void Application::run() {
	Window window(kFrameWidth, kFrameHeight, "Renderer", !target_fps_.has_value());
	const std::optional<Clock::duration> target_frame_duration =
	    target_fps_.has_value()
	        ? std::optional<Clock::duration>(
	              std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(
	                  1.0 / static_cast<double>(*target_fps_))))
	        : std::nullopt;
	Clock::time_point previous_frame = Clock::now();
	Clock::time_point next_frame = previous_frame;
	Clock::time_point fps_window_start = previous_frame;
	int rendered_frames = 0;
	while (window.isOpen()) {
		const Clock::time_point current_frame = Clock::now();
		const float frame_seconds = secondsSince(previous_frame, current_frame);
		previous_frame = current_frame;

		const FrameInput input = window.pollEvents();
		if (!window.isOpen()) {
			break;
		}
		applyCameraInput(&scene_.camera, input, frame_seconds, &camera_movement_velocity_,
		                 &camera_turn_velocity_);
		const Picture picture = renderer_.render(scene_.world, scene_.camera);
		window.show(picture);
		if (target_frame_duration.has_value()) {
			next_frame += *target_frame_duration;
			const Clock::time_point after_render = Clock::now();
			if (next_frame > after_render) {
				std::this_thread::sleep_until(next_frame);
			} else {
				next_frame = after_render;
			}
		}

		++rendered_frames;
		const Clock::time_point frame_end = Clock::now();
		const float fps_window_seconds = secondsSince(fps_window_start, frame_end);
		if (fps_window_seconds >= 1.0f) {
			const int current_fps = static_cast<int>(
			    std::round(static_cast<float>(rendered_frames) / fps_window_seconds));
			window.setTitle("Renderer - " + std::to_string(current_fps) + " FPS");
			rendered_frames = 0;
			fps_window_start = frame_end;
		}
	}
}

} // namespace renderer
