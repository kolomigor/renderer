#include "camera.h"

#include <algorithm>
#include <cmath>

namespace renderer {
namespace {

constexpr float kHalfPi = 1.57079632679f;
constexpr float kPitchLimit = kHalfPi - 0.01f;

vec3 makeForward(const vec3& position, const vec3& target) {
	return normalize(target - position);
}

vec3 makeForward(float yaw, float pitch) {
	const float cos_pitch = std::cos(pitch);
	return normalize({std::sin(yaw) * cos_pitch, std::sin(pitch), -std::cos(yaw) * cos_pitch});
}

vec3 makeRight(const vec3& forward) {
	return normalize(cross(forward, kWorldUp));
}

vec3 makeUp(const vec3& right, const vec3& forward) {
	return normalize(cross(right, forward));
}

float makeYaw(const vec3& forward) {
	return std::atan2(forward.x, -forward.z);
}

float makePitch(const vec3& forward) {
	return std::asin(std::clamp(forward.y, -1.0f, 1.0f));
}

} // namespace

Camera::Camera(Fov fov, Aspect aspect, NearPlane near_plane, FarPlane far_plane,
               const vec3& position, const vec3& target)
    : position_(position), forward_(makeForward(position, target)), right_(makeRight(forward_)),
      up_(makeUp(right_, forward_)), yaw_(makeYaw(forward_)), pitch_(makePitch(forward_)),
      fov_(fov), aspect_(aspect), near_(near_plane), far_(far_plane) {
}

mat4 Camera::viewMatrix() const {
	return lookAt(position_, position_ + forward_, up_);
}

mat4 Camera::projectionMatrix() const {
	return perspective(radians(fov_), aspect_, near_, far_);
}

mat4 Camera::viewProjectionMatrix() const {
	return projectionMatrix() * viewMatrix();
}

void Camera::moveLocal(const vec3& movement) {
	position_ += right_ * movement.x + kWorldUp * movement.y + forward_ * movement.z;
}

void Camera::turn(float yaw_delta, float pitch_delta) {
	yaw_ += yaw_delta;
	pitch_ = std::clamp(pitch_ + pitch_delta, -kPitchLimit, kPitchLimit);
	updateBasis();
}

void Camera::updateBasis() {
	forward_ = makeForward(yaw_, pitch_);
	right_ = makeRight(forward_);
	up_ = makeUp(right_, forward_);
}

const vec3& Camera::position() const {
	return position_;
}

} // namespace renderer
