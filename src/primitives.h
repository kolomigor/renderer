#pragma once
#include "glm.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
};

struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;
};