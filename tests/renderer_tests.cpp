#include "camera.h"
#include "clip.h"
#include "picture.h"
#include "rasterizer.h"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr float kEpsilon = 0.0001f;

renderer::Material testMaterial() {
	return {{1.0f, 1.0f, 1.0f}, 0.0f, 1.0f, 0.0f, 1.0f};
}

renderer::Vertex vertex(renderer::vec4 position, renderer::vec3 color = {1.0f, 1.0f, 1.0f}) {
	return {position, color};
}

renderer::Triangle triangle(renderer::Vertex a, renderer::Vertex b, renderer::Vertex c) {
	return {a, b, c, testMaterial()};
}

void fail(const std::string& message) {
	throw std::runtime_error(message);
}

void expectTrue(bool condition, const std::string& message) {
	if (!condition) {
		fail(message);
	}
}

void expectNear(float actual, float expected, const std::string& message) {
	if (std::abs(actual - expected) > kEpsilon) {
		fail(message + ": expected " + std::to_string(expected) + ", got " +
		     std::to_string(actual));
	}
}

void expectVec3Near(const renderer::vec3& actual, const renderer::vec3& expected,
                    const std::string& message) {
	expectNear(actual.x, expected.x, message + ".x");
	expectNear(actual.y, expected.y, message + ".y");
	expectNear(actual.z, expected.z, message + ".z");
}

bool isInsideClipVolume(const renderer::Vertex& v) {
	return v.position.w > 0.0f && v.position.x >= -v.position.w - kEpsilon &&
	       v.position.x <= v.position.w + kEpsilon && v.position.y >= -v.position.w - kEpsilon &&
	       v.position.y <= v.position.w + kEpsilon && v.position.z >= -v.position.w - kEpsilon &&
	       v.position.z <= v.position.w + kEpsilon;
}

void expectTriangleInsideClipVolume(const renderer::Triangle& t) {
	expectTrue(isInsideClipVolume(t.v0), "v0 is outside clip volume");
	expectTrue(isInsideClipVolume(t.v1), "v1 is outside clip volume");
	expectTrue(isInsideClipVolume(t.v2), "v2 is outside clip volume");
}

void testFrustumClippingKeepsInsideTriangle() {
	const renderer::Triangle input =
	    triangle(vertex({-0.25f, -0.25f, 0.0f, 1.0f}), vertex({0.25f, -0.25f, 0.0f, 1.0f}),
	             vertex({0.0f, 0.25f, 0.0f, 1.0f}));

	const std::vector<renderer::Triangle> clipped = renderer::clipTriangleByFrustum(input);

	expectTrue(clipped.size() == 1, "inside triangle should survive unchanged");
	expectTriangleInsideClipVolume(clipped.front());
}

void testFrustumClippingRejectsOutsideTriangle() {
	const renderer::Triangle input =
	    triangle(vertex({-2.0f, -0.5f, 0.0f, 1.0f}), vertex({-1.5f, 0.5f, 0.0f, 1.0f}),
	             vertex({-1.2f, -0.2f, 0.0f, 1.0f}));

	const std::vector<renderer::Triangle> clipped = renderer::clipTriangleByFrustum(input);

	expectTrue(clipped.empty(), "triangle outside left plane should be rejected");
}

void testFrustumClippingCutsTriangle() {
	const renderer::Triangle input =
	    triangle(vertex({0.0f, 0.0f, 0.0f, 1.0f}), vertex({-2.0f, 0.5f, 0.0f, 1.0f}),
	             vertex({-2.0f, -0.5f, 0.0f, 1.0f}));

	const std::vector<renderer::Triangle> clipped = renderer::clipTriangleByFrustum(input);

	expectTrue(!clipped.empty(), "partially visible triangle should produce clipped geometry");
	for (const renderer::Triangle& part : clipped) {
		expectTriangleInsideClipVolume(part);
	}
}

void testRasterizerWritesPixels() {
	renderer::Picture picture(renderer::Width{10}, renderer::Height{10});
	const renderer::Triangle input = triangle(vertex({1.0f, 1.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}),
	                                          vertex({8.0f, 1.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}),
	                                          vertex({1.0f, 8.0f, 0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}));

	renderer::Rasterizer::rasterizeTriangle(input, &picture);

	expectVec3Near(picture.colorAt(renderer::PixelX{2}, renderer::PixelY{2}), {1.0f, 0.0f, 0.0f},
	               "rasterizer should color an interior pixel");
}

void testRasterizerDepthTestKeepsNearestPixel() {
	renderer::Picture picture(renderer::Width{10}, renderer::Height{10});
	const renderer::Triangle far_triangle =
	    triangle(vertex({1.0f, 1.0f, 0.8f, 1.0f}, {1.0f, 0.0f, 0.0f}),
	             vertex({8.0f, 1.0f, 0.8f, 1.0f}, {1.0f, 0.0f, 0.0f}),
	             vertex({1.0f, 8.0f, 0.8f, 1.0f}, {1.0f, 0.0f, 0.0f}));
	const renderer::Triangle near_triangle =
	    triangle(vertex({1.0f, 1.0f, 0.2f, 1.0f}, {0.0f, 1.0f, 0.0f}),
	             vertex({8.0f, 1.0f, 0.2f, 1.0f}, {0.0f, 1.0f, 0.0f}),
	             vertex({1.0f, 8.0f, 0.2f, 1.0f}, {0.0f, 1.0f, 0.0f}));

	renderer::Rasterizer::rasterizeTriangle(far_triangle, &picture);
	renderer::Rasterizer::rasterizeTriangle(near_triangle, &picture);

	expectVec3Near(picture.colorAt(renderer::PixelX{2}, renderer::PixelY{2}), {0.0f, 1.0f, 0.0f},
	               "nearer triangle should overwrite farther triangle");
}

void testCameraProjectionMapsTargetToCenter() {
	const renderer::Camera camera(renderer::Fov{60.0f}, renderer::Aspect{1.0f},
	                              renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                              {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});
	const renderer::vec4 clip_position =
	    camera.viewProjectionMatrix() * renderer::point({0.0f, 0.0f, -5.0f});
	const renderer::vec3 ndc = renderer::xyz(clip_position) / clip_position.w;

	expectNear(ndc.x, 0.0f, "target point should project to horizontal center");
	expectNear(ndc.y, 0.0f, "target point should project to vertical center");
	expectTrue(ndc.z >= -1.0f && ndc.z <= 1.0f, "target point should be inside depth range");
}

void testCameraProjectionRespondsToAspect() {
	const renderer::Camera square_camera(renderer::Fov{60.0f}, renderer::Aspect{1.0f},
	                                     renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                                     {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});
	const renderer::Camera wide_camera(renderer::Fov{60.0f}, renderer::Aspect{2.0f},
	                                   renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                                   {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});

	const renderer::vec4 square_clip =
	    square_camera.viewProjectionMatrix() * renderer::point({1.0f, 0.0f, -5.0f});
	const renderer::vec4 wide_clip =
	    wide_camera.viewProjectionMatrix() * renderer::point({1.0f, 0.0f, -5.0f});
	const float square_ndc_x = square_clip.x / square_clip.w;
	const float wide_ndc_x = wide_clip.x / wide_clip.w;

	expectTrue(std::abs(wide_ndc_x) < std::abs(square_ndc_x),
	           "wider aspect should reduce horizontal NDC magnitude");
}

using Test = void (*)();

void runTest(const std::string& name, Test test) {
	test();
	std::cout << "[PASS] " << name << '\n';
}

} // namespace

int main() {
	try {
		runTest("frustum clipping keeps inside triangle", testFrustumClippingKeepsInsideTriangle);
		runTest("frustum clipping rejects outside triangle",
		        testFrustumClippingRejectsOutsideTriangle);
		runTest("frustum clipping cuts triangle", testFrustumClippingCutsTriangle);
		runTest("rasterizer writes pixels", testRasterizerWritesPixels);
		runTest("rasterizer depth test keeps nearest pixel",
		        testRasterizerDepthTestKeepsNearestPixel);
		runTest("camera projection maps target to center", testCameraProjectionMapsTargetToCenter);
		runTest("camera projection responds to aspect", testCameraProjectionRespondsToAspect);
	} catch (const std::exception& exception) {
		std::cerr << "[FAIL] " << exception.what() << '\n';
		return 1;
	}
	return 0;
}
