#include "camera.h"
#include "clip.h"
#include "picture.h"
#include "rasterizer.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"

#include <cmath>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr float kEpsilon = 0.0001f;

renderer::Material testMaterial() {
	return {{1.0f, 1.0f, 1.0f}};
}

renderer::Vertex vertex(renderer::vec4 position, renderer::vec3 color = {1.0f, 1.0f, 1.0f},
                        renderer::vec2 texcoord = {0.0f, 0.0f}) {
	return {position, color, texcoord};
}

renderer::Triangle triangle(renderer::Vertex a, renderer::Vertex b, renderer::Vertex c) {
	return {a, b, c, testMaterial()};
}

std::filesystem::path sourcePath(const std::filesystem::path& relative_path) {
	return std::filesystem::path(RENDERER_SOURCE_DIR) / relative_path;
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

bool hasNonBlackPixel(const renderer::Picture& picture) {
	for (int y = 0; y < picture.height(); ++y) {
		for (int x = 0; x < picture.width(); ++x) {
			const renderer::vec3& color =
			    picture.colorAt(renderer::PixelX{x}, renderer::PixelY{y});
			if (color.r > kEpsilon || color.g > kEpsilon || color.b > kEpsilon) {
				return true;
			}
		}
	}
	return false;
}

bool hasBluishPixel(const renderer::Picture& picture) {
	for (int y = 0; y < picture.height(); ++y) {
		for (int x = 0; x < picture.width(); ++x) {
			const renderer::vec3& color =
			    picture.colorAt(renderer::PixelX{x}, renderer::PixelY{y});
			if (color.b > color.r + kEpsilon && color.b > color.g + kEpsilon) {
				return true;
			}
		}
	}
	return false;
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

void testRasterizerInterpolatesDepthLinearly() {
	renderer::Picture picture(renderer::Width{10}, renderer::Height{10});
	const renderer::Triangle constant_depth_triangle =
	    triangle(vertex({0.0f, 0.0f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}),
	             vertex({9.0f, 0.0f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}),
	             vertex({0.0f, 9.0f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}));
	const renderer::Triangle varying_depth_triangle =
	    triangle(vertex({0.0f, 0.0f, 0.1f, 10.0f}, {1.0f, 0.0f, 0.0f}),
	             vertex({9.0f, 0.0f, 0.9f, 1.0f}, {1.0f, 0.0f, 0.0f}),
	             vertex({0.0f, 9.0f, 0.9f, 1.0f}, {1.0f, 0.0f, 0.0f}));

	renderer::Rasterizer::rasterizeTriangle(constant_depth_triangle, &picture);
	renderer::Rasterizer::rasterizeTriangle(varying_depth_triangle, &picture);

	expectVec3Near(picture.colorAt(renderer::PixelX{2}, renderer::PixelY{2}), {0.0f, 1.0f, 0.0f},
	               "depth should be interpolated linearly in screen space");
}

void testRasterizerSamplesTexture() {
	renderer::Picture picture(renderer::Width{10}, renderer::Height{10});
	renderer::Material material = testMaterial();
	material.texture = std::make_shared<renderer::Texture>(
	    2, 2, std::vector<renderer::vec3>{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
	                                      {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}});
	const renderer::Triangle input{
	    vertex({1.0f, 1.0f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}),
	    vertex({8.0f, 1.0f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}),
	    vertex({1.0f, 8.0f, 0.5f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}),
	    material};

	renderer::Rasterizer::rasterizeTriangle(input, &picture);

	expectVec3Near(picture.colorAt(renderer::PixelX{2}, renderer::PixelY{2}), {0.0f, 0.0f, 1.0f},
	               "textured triangle should sample its material texture");
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

void testCameraMoveKeepsForwardPointCentered() {
	renderer::Camera camera(renderer::Fov{60.0f}, renderer::Aspect{1.0f},
	                        renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                        {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});

	camera.moveLocal({0.0f, 0.0f, 1.0f});
	const renderer::vec4 clip_position =
	    camera.viewProjectionMatrix() * renderer::point({0.0f, 0.0f, -5.0f});
	const renderer::vec3 ndc = renderer::xyz(clip_position) / clip_position.w;

	expectNear(ndc.x, 0.0f, "forward point should remain horizontally centered after camera move");
	expectNear(ndc.y, 0.0f, "forward point should remain vertically centered after camera move");
}

void testRendererCullsBackFaces() {
	const renderer::Material material{{1.0f, 0.0f, 0.0f}};
	const renderer::Camera camera(renderer::Fov{60.0f}, renderer::Aspect{1.0f},
	                              renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                              {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});
	const renderer::Renderer software_renderer(renderer::Width{20}, renderer::Height{20});

	renderer::World front_world;
	front_world.addTriangle(renderer::makeTriangle({-0.8f, -0.8f, -3.0f},
	                                               {0.8f, -0.8f, -3.0f},
	                                               {0.0f, 0.8f, -3.0f}, material));
	const renderer::Picture front_picture = software_renderer.render(front_world, camera);
	expectTrue(hasNonBlackPixel(front_picture), "front-facing triangle should be rendered");

	renderer::World back_world;
	back_world.addTriangle(renderer::makeTriangle({-0.8f, -0.8f, -3.0f},
	                                              {0.0f, 0.8f, -3.0f},
	                                              {0.8f, -0.8f, -3.0f}, material));
	const renderer::Picture back_picture = software_renderer.render(back_world, camera);
	expectTrue(!hasNonBlackPixel(back_picture), "back-facing triangle should be culled");
}

void testRendererPreservesMaterialTexture() {
	renderer::Material material{{1.0f, 1.0f, 1.0f}};
	material.texture = std::make_shared<renderer::Texture>(
	    1, 1, std::vector<renderer::vec3>{{0.0f, 0.0f, 1.0f}});
	const renderer::Camera camera(renderer::Fov{60.0f}, renderer::Aspect{1.0f},
	                              renderer::NearPlane{0.1f}, renderer::FarPlane{100.0f},
	                              {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f});
	const renderer::Renderer software_renderer(renderer::Width{20}, renderer::Height{20});

	renderer::World world;
	world.addTriangle(renderer::makeTriangle({-0.8f, -0.8f, -3.0f},
	                                         {0.8f, -0.8f, -3.0f},
	                                         {0.0f, 0.8f, -3.0f}, material));

	const renderer::Picture picture = software_renderer.render(world, camera);
	expectTrue(hasBluishPixel(picture),
	           "renderer should preserve material texture through projection");
}

void testAutoObjSceneRendersModel() {
	const renderer::Scene scene =
	    renderer::loadObjScene(sourcePath("assets/models/cube.obj"), renderer::Width{20},
	                           renderer::Height{20});
	const renderer::Renderer software_renderer(renderer::Width{20}, renderer::Height{20});

	const renderer::Picture picture = software_renderer.render(scene.world, scene.camera);

	expectTrue(hasNonBlackPixel(picture), "auto OBJ scene should render a visible model");
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
		runTest("rasterizer interpolates depth linearly",
		        testRasterizerInterpolatesDepthLinearly);
		runTest("rasterizer samples texture", testRasterizerSamplesTexture);
		runTest("camera projection maps target to center", testCameraProjectionMapsTargetToCenter);
		runTest("camera projection responds to aspect", testCameraProjectionRespondsToAspect);
		runTest("camera move keeps forward point centered",
		        testCameraMoveKeepsForwardPointCentered);
		runTest("renderer culls back faces", testRendererCullsBackFaces);
		runTest("renderer preserves material texture", testRendererPreservesMaterialTexture);
		runTest("auto OBJ scene renders model", testAutoObjSceneRendersModel);
	} catch (const std::exception& exception) {
		std::cerr << "[FAIL] " << exception.what() << '\n';
		return 1;
	}
	return 0;
}
