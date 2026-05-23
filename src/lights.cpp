#include "lights.h"

#include <cassert>

namespace renderer {
namespace {

DirectionalLight normalizedLight(DirectionalLight light) {
	const float direction_length = length(light.direction_to_light);
	assert(direction_length > 0.0f);
	if (direction_length == 0.0f) {
		light.direction_to_light = kWorldUp;
	} else {
		light.direction_to_light = light.direction_to_light / direction_length;
	}
	return light;
}

} // namespace

Lights::Lights(vec3 ambient_color, float ambient_intensity)
    : ambient_color_(ambient_color), ambient_intensity_(ambient_intensity) {
	assert(ambient_intensity_ >= 0.0f);
}

void Lights::addDirectionalLight(DirectionalLight light) {
	assert(light.intensity >= 0.0f);
	directional_lights_.push_back(normalizedLight(light));
}

const vec3& Lights::ambientColor() const {
	return ambient_color_;
}

float Lights::ambientIntensity() const {
	return ambient_intensity_;
}

const std::vector<DirectionalLight>& Lights::directionalLights() const {
	return directional_lights_;
}

} // namespace renderer
