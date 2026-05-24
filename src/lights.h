#pragma once

#include "linalg.h"

#include <vector>

namespace renderer {

struct DirectionalLight {
	vec3 direction_to_light;
	vec3 color;
	float intensity;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
	float linear_attenuation;
	float quadratic_attenuation;
};

class Lights {
public:
	Lights(vec3 ambient_color, float ambient_intensity);

	void addDirectionalLight(DirectionalLight light);
	void addPointLight(PointLight light);

	const vec3& ambientColor() const;
	float ambientIntensity() const;
	const std::vector<DirectionalLight>& directionalLights() const;
	const std::vector<PointLight>& pointLights() const;

private:
	vec3 ambient_color_;
	float ambient_intensity_;
	std::vector<DirectionalLight> directional_lights_;
	std::vector<PointLight> point_lights_;
};

} // namespace renderer
