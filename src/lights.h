#pragma once

#include "linalg.h"

#include <vector>

namespace renderer {

struct DirectionalLight {
	vec3 direction_to_light;
	vec3 color;
	float intensity;
};

class Lights {
public:
	Lights(vec3 ambient_color, float ambient_intensity);

	void addDirectionalLight(DirectionalLight light);

	const vec3& ambientColor() const;
	float ambientIntensity() const;
	const std::vector<DirectionalLight>& directionalLights() const;

private:
	vec3 ambient_color_;
	float ambient_intensity_;
	std::vector<DirectionalLight> directional_lights_;
};

} // namespace renderer
