#include "clip.h"

static bool InsideNear(const ClipVertex& v) {
	return v.position.z >= -v.position.w;
}

static ClipVertex IntersectNear(const ClipVertex& a, const ClipVertex& b) {
	float da = a.position.z + a.position.w;
	float db = b.position.z + b.position.w;
	float t = da / (da - db);
	ClipVertex out;
	out.position = a.position + t * (b.position - a.position);
	out.color = a.color + t * (b.color - a.color);
	return out;
}

std::vector<ClipTriangle> ClipTriangleNear(const ClipVertex& v0, const ClipVertex& v1,
                                           const ClipVertex& v2) {
	const ClipVertex v[3] = {v0, v1, v2};
	const bool inside[3] = {InsideNear(v0), InsideNear(v1), InsideNear(v2)};
	int insideCount = inside[0] + inside[1] + inside[2];
	std::vector<ClipTriangle> out;
	if (insideCount == 0) {
		return out;
	}
	if (insideCount == 3) {
		out.push_back({v0, v1, v2});
		return out;
	}
	if (insideCount == 1) {
		int i = inside[0] ? 0 : (inside[1] ? 1 : 2);
		int o1 = (i + 1) % 3;
		int o2 = (i + 2) % 3;
		out.push_back({v[i], IntersectNear(v[i], v[o1]), IntersectNear(v[i], v[o2])});
		return out;
	}
	int o = inside[0] ? (inside[1] ? 2 : 1) : 0;
	int i1 = (o + 1) % 3;
	int i2 = (o + 2) % 3;
	ClipVertex p1 = IntersectNear(v[i1], v[o]);
	ClipVertex p2 = IntersectNear(v[i2], v[o]);
	out.push_back({v[i1], v[i2], p1});
	out.push_back({v[i2], p2, p1});
	return out;
}
