#pragma once

#include "linalg.h"
#include "picture.h"
#include "strong_alias.h"

#include <cstdint>
#include <string>
#include <vector>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Window;

namespace renderer {

struct FrameInput {
	vec3 camera_movement{0.0f, 0.0f, 0.0f};
	vec2 camera_turn{0.0f, 0.0f};
	vec2 camera_look_delta{0.0f, 0.0f};
	bool fast_camera_movement = false;
};

class Window {
public:
	Window(Width width, Height height, std::string title, bool use_vsync);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool isOpen() const;
	FrameInput pollEvents();
	void show(const Picture& picture);
	void setTitle(const std::string& title);

private:
	void close();
	void copyPicturePixels(const Picture& picture);

	int width_;
	int height_;
	SDL_Window *window_;
	SDL_Renderer *renderer_;
	SDL_Texture *texture_;
	std::vector<std::uint8_t> pixels_;
	bool is_open_;
};

} // namespace renderer
