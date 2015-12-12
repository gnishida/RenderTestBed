#pragma once

#include <glm/glm.hpp>

/**
 * This structure defines a vertex data.
 */
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	//glm::vec4 color;
	glm::vec3 color;
	glm::vec3 texCoord;
	float drawEdge;	// 0 -- exclude / 1 -- draw edge

	Vertex() {}

	Vertex(const glm::vec3& pos, const glm::vec3& n, float drawEdge = 0.0f) {
		position = pos;
		normal = n;
		this->drawEdge = drawEdge;
	}

	Vertex(const glm::vec3& pos, const glm::vec3& n, const glm::vec4& c, float drawEdge = 0.0f) {
		position = pos;
		normal = n;
		color = glm::vec3(c);
		this->drawEdge = drawEdge;
	}

	Vertex(const glm::vec3& pos, const glm::vec3& n, const glm::vec4& c, const glm::vec2& tex, float drawEdge = 0.0f) {
		position = pos;
		normal = n;
		color = glm::vec3(c);
		texCoord = glm::vec3(tex, 0);
		this->drawEdge = drawEdge;
	}
};
