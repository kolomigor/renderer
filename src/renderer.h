#pragma once
#include "camera.h"
#include "picture.h"
#include "world.h"

class Renderer {
public:
	void Render(const World& world, const Camera& camera, Picture& picture);
};
