#include "renderer.h"
#include "clip.h"
#include "primitives.h"
#include "rasterizer.h"

static bool IsBackFaceCCW(const ScreenVertex& a, const ScreenVertex& b, const ScreenVertex& c) {
	const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	return area >= 0.f;
}

static ScreenVertex ProjectVertex(const ClipVertex& clip, int width, int height) {
	assert(clip.position.w > 0);
	assert(clip.position.z >= -clip.position.w);
	const glm::vec3 ndc = glm::vec3(clip.position) / clip.position.w;
	ScreenVertex out;
	out.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(width);
	out.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * static_cast<float>(height);
	out.depth = ndc.z;
	out.color = clip.color;
	return out;
}

void Renderer::Render(const World& world, const Camera& camera, Picture& picture) {
	picture.Clear({0.0f, 0.0f, 0.0f});
	picture.ClearDepth();
	const glm::mat4 vp = camera.Projection() * camera.View();
	const int w = picture.Width();
	const int h = picture.Height();
	for (const Triangle& t : world.GetTriangles()) {
		ClipVertex v0 = {vp * glm::vec4(t.v0.position, 1.0f), t.v0.color};
		ClipVertex v1 = {vp * glm::vec4(t.v1.position, 1.0f), t.v1.color};
		ClipVertex v2 = {vp * glm::vec4(t.v2.position, 1.0f), t.v2.color};
		auto clipped_triangles = ClipTriangleNear(v0, v1, v2);
		for (const ClipTriangle& tr : clipped_triangles) {
			ScreenVertex c0 = ProjectVertex(tr[0], w, h);
			ScreenVertex c1 = ProjectVertex(tr[1], w, h);
			ScreenVertex c2 = ProjectVertex(tr[2], w, h);
			if (IsBackFaceCCW(c0, c1, c2)) {
				continue;
			}
			Rasterizer::RasterizeTriangle(c0, c1, c2, picture);
		}
	}
}
