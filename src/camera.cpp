#include "camera.h"

namespace renderer {
namespace {

vec3 makeForward(const vec3& position, const vec3& target) {
	return normalize(target - position);
}

vec3 makeRight(const vec3& forward) {
	return normalize(cross(forward, kWorldUp));
}

vec3 makeUp(const vec3& right, const vec3& forward) {
	return normalize(cross(right, forward));
}

} // namespace

Camera::Camera(Fov fov, Aspect aspect, NearPlane near_plane, FarPlane far_plane,
               const vec3& position, const vec3& target)
    : position_(position), forward_(makeForward(position, target)), right_(makeRight(forward_)),
      up_(makeUp(right_, forward_)), fov_(fov), aspect_(aspect), near_(near_plane),
      far_(far_plane) {
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

} // namespace renderer
