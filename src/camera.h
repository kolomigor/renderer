#pragma once

#include "linalg.h"
#include "strong_alias.h"

namespace renderer {

class Camera {
public:
	Camera(Fov fov, Aspect aspect, NearPlane near_plane, FarPlane far_plane, const vec3& position,
	       const vec3& target);

	mat4 viewMatrix() const;
	mat4 projectionMatrix() const;
	mat4 viewProjectionMatrix() const;
	void moveLocal(const vec3& movement);
	void turn(float yaw_delta, float pitch_delta);
	const vec3& position() const;

private:
	void updateBasis();

	vec3 position_;
	vec3 forward_;
	vec3 right_;
	vec3 up_;
	float yaw_;
	float pitch_;
	float fov_;
	float aspect_;
	float near_;
	float far_;
};

} // namespace renderer
