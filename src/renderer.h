#pragma once

#include "camera.h"
#include "lights.h"
#include "picture.h"
#include "strong_alias.h"
#include "world.h"

namespace renderer {

class Renderer {
public:
	Renderer(Width width, Height height);

	Picture render(const World& world, const Camera& camera, const Lights& lights) const;

private:
	int width_;
	int height_;
};

} // namespace renderer
