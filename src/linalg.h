#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace renderer {

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;

inline const vec3 kWorldUp{0.0f, 1.0f, 0.0f};
inline const vec3 kBlack{0.0f, 0.0f, 0.0f};

inline mat4 identity() {
	return mat4(1.0f);
}

inline vec2 xy(const vec4& value) {
	return {value.x, value.y};
}

inline vec3 xyz(const vec4& value) {
	return {value.x, value.y, value.z};
}

inline vec4 point(const vec3& position) {
	return {position, 1.0f};
}

inline vec3 normalize(const vec3& value) {
	return glm::normalize(value);
}

inline vec3 cross(const vec3& lhs, const vec3& rhs) {
	return glm::cross(lhs, rhs);
}

inline float dot(const vec3& lhs, const vec3& rhs) {
	return glm::dot(lhs, rhs);
}

inline float dot(const vec4& lhs, const vec4& rhs) {
	return glm::dot(lhs, rhs);
}

inline vec3 reflect(const vec3& incident, const vec3& normal) {
	return glm::reflect(incident, normal);
}

inline float length(const vec3& value) {
	return glm::length(value);
}

inline mat4 lookAt(const vec3& position, const vec3& target, const vec3& up) {
	return glm::lookAt(position, target, up);
}

inline mat4 translate(const mat4& transform, const vec3& offset) {
	return glm::translate(transform, offset);
}

inline mat4 rotate(const mat4& transform, float angle_radians, const vec3& axis) {
	return glm::rotate(transform, angle_radians, axis);
}

inline mat4 scale(const mat4& transform, const vec3& value) {
	return glm::scale(transform, value);
}

inline mat4 perspective(float fov_radians, float aspect, float near_plane, float far_plane) {
	return glm::perspective(fov_radians, aspect, near_plane, far_plane);
}

inline float radians(float degrees) {
	return glm::radians(degrees);
}

} // namespace renderer
