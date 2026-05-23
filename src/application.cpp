#include "application.h"

#include "image_io.h"

#include <cassert>
#include <stdexcept>

namespace renderer {
namespace {

constexpr Width kFrameWidth{800};
constexpr Height kFrameHeight{600};
constexpr Fov kCameraFov{60.0f};
constexpr NearPlane kCameraNearPlane{0.1f};
constexpr FarPlane kCameraFarPlane{100.0f};
const vec3 kCameraPosition{2.5f, 2.0f, 2.0f};
const vec3 kCameraTarget{0.0f, 0.0f, -6.0f};
const vec3 kCubeCenter{0.0f, 0.0f, -6.0f};
const vec3 kRed{1.0f, 0.0f, 0.0f};
const vec3 kGreen{0.0f, 1.0f, 0.0f};
const vec3 kBlue{0.0f, 0.0f, 1.0f};
const vec3 kYellow{1.0f, 1.0f, 0.0f};
const vec3 kCyan{0.0f, 1.0f, 1.0f};
const vec3 kMagenta{1.0f, 0.0f, 1.0f};

Aspect makeAspect(Width width, Height height) {
	return Aspect{static_cast<float>(width) / static_cast<float>(height)};
}

Triangle makeTriangle(const vec3& a, const vec3& b, const vec3& c, const vec3& ca, const vec3& cb,
                      const vec3& cc) {
	return {makeVertex(a, ca), makeVertex(b, cb), makeVertex(c, cc)};
}

void addTriangle(World *world, const vec3& a, const vec3& b, const vec3& c, const vec3& ca,
                 const vec3& cb, const vec3& cc) {
	assert(world != nullptr);
	world->addTriangle(makeTriangle(a, b, c, ca, cb, cc));
}

Camera makeDefaultCamera() {
	return {kCameraFov,       makeAspect(kFrameWidth, kFrameHeight),
	        kCameraNearPlane, kCameraFarPlane,
	        kCameraPosition,  kCameraTarget};
}

World makeDefaultWorld() {
	World world;
	const float half_side = 1.0f;
	const vec3 v000 = kCubeCenter + vec3{-half_side, -half_side, -half_side};
	const vec3 v001 = kCubeCenter + vec3{-half_side, -half_side, half_side};
	const vec3 v010 = kCubeCenter + vec3{-half_side, half_side, -half_side};
	const vec3 v011 = kCubeCenter + vec3{-half_side, half_side, half_side};
	const vec3 v100 = kCubeCenter + vec3{half_side, -half_side, -half_side};
	const vec3 v101 = kCubeCenter + vec3{half_side, -half_side, half_side};
	const vec3 v110 = kCubeCenter + vec3{half_side, half_side, -half_side};
	const vec3 v111 = kCubeCenter + vec3{half_side, half_side, half_side};

	addTriangle(&world, v001, v101, v111, kRed, kRed, kRed);
	addTriangle(&world, v001, v111, v011, kRed, kRed, kRed);
	addTriangle(&world, v000, v010, v110, kGreen, kGreen, kGreen);
	addTriangle(&world, v000, v110, v100, kGreen, kGreen, kGreen);
	addTriangle(&world, v000, v001, v011, kBlue, kBlue, kBlue);
	addTriangle(&world, v000, v011, v010, kBlue, kBlue, kBlue);
	addTriangle(&world, v100, v110, v111, kYellow, kYellow, kYellow);
	addTriangle(&world, v100, v111, v101, kYellow, kYellow, kYellow);
	addTriangle(&world, v010, v011, v111, kCyan, kCyan, kCyan);
	addTriangle(&world, v010, v111, v110, kCyan, kCyan, kCyan);
	addTriangle(&world, v000, v100, v101, kMagenta, kMagenta, kMagenta);
	addTriangle(&world, v000, v101, v001, kMagenta, kMagenta, kMagenta);
	return world;
}

} // namespace

Application::Application()
    : world_(makeDefaultWorld()), camera_(makeDefaultCamera()),
      renderer_(kFrameWidth, kFrameHeight) {
}

void Application::run() {
	const Picture picture = renderer_.render(world_, camera_);
	if (!savePPM(picture, "output.ppm")) {
		throw std::runtime_error("failed to write output.ppm");
	}
}

} // namespace renderer
