#include "window.h"

#include "color.h"

#include <SDL.h>

#include <cassert>
#include <stdexcept>

namespace renderer {
namespace {

constexpr int kColorChannels = 3;

std::string sdlErrorMessage(const std::string& operation) {
	return operation + ": " + SDL_GetError();
}

void throwSdlError(const std::string& operation) {
	throw std::runtime_error(sdlErrorMessage(operation));
}

template <class T> T *checkSdlPointer(T *pointer, const std::string& operation) {
	if (pointer == nullptr) {
		throwSdlError(operation);
	}
	return pointer;
}

SDL_Renderer *createRenderer(SDL_Window *window) {
	SDL_Renderer *renderer =
	    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer != nullptr) {
		return renderer;
	}
	return checkSdlPointer(SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE),
	                       "failed to create SDL renderer");
}

bool isPressed(const std::uint8_t *keyboard, SDL_Scancode key) {
	return keyboard[key] != 0;
}

float keyAxis(bool positive, bool negative) {
	return (positive ? 1.0f : 0.0f) - (negative ? 1.0f : 0.0f);
}

} // namespace

Window::Window(Width width, Height height, std::string title)
    : width_(width), height_(height), window_(nullptr), renderer_(nullptr), texture_(nullptr),
      pixels_(width_ * height_ * kColorChannels), is_open_(true) {
	assert(width_ > 0);
	assert(height_ > 0);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throwSdlError("failed to initialize SDL video");
	}

	try {
		window_ = checkSdlPointer(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
		                                           SDL_WINDOWPOS_CENTERED, width_, height_,
		                                           SDL_WINDOW_RESIZABLE),
		                          "failed to create SDL window");
		renderer_ = createRenderer(window_);
		texture_ = checkSdlPointer(SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB24,
		                                             SDL_TEXTUREACCESS_STREAMING, width_, height_),
		                           "failed to create SDL texture");
		if (SDL_RenderSetLogicalSize(renderer_, width_, height_) != 0) {
			throwSdlError("failed to set SDL logical size");
		}
	} catch (...) {
		close();
		SDL_Quit();
		throw;
	}
}

Window::~Window() {
	close();
	SDL_Quit();
}

bool Window::isOpen() const {
	return is_open_;
}

FrameInput Window::pollEvents() {
	FrameInput input;
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			is_open_ = false;
		}
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
			is_open_ = false;
		}
		if (event.type == SDL_MOUSEMOTION && (event.motion.state & SDL_BUTTON_RMASK) != 0) {
			input.camera_look_delta.x += static_cast<float>(event.motion.xrel);
			input.camera_look_delta.y += static_cast<float>(event.motion.yrel);
		}
	}

	const std::uint8_t *keyboard = SDL_GetKeyboardState(nullptr);
	input.camera_movement.x =
	    keyAxis(isPressed(keyboard, SDL_SCANCODE_D), isPressed(keyboard, SDL_SCANCODE_A));
	input.camera_movement.y =
	    keyAxis(isPressed(keyboard, SDL_SCANCODE_SPACE) || isPressed(keyboard, SDL_SCANCODE_E),
	            isPressed(keyboard, SDL_SCANCODE_LCTRL) ||
	                isPressed(keyboard, SDL_SCANCODE_RCTRL) || isPressed(keyboard, SDL_SCANCODE_Q));
	input.camera_movement.z =
	    keyAxis(isPressed(keyboard, SDL_SCANCODE_W), isPressed(keyboard, SDL_SCANCODE_S));
	input.camera_turn.x =
	    keyAxis(isPressed(keyboard, SDL_SCANCODE_RIGHT), isPressed(keyboard, SDL_SCANCODE_LEFT));
	input.camera_turn.y =
	    keyAxis(isPressed(keyboard, SDL_SCANCODE_UP), isPressed(keyboard, SDL_SCANCODE_DOWN));
	input.fast_camera_movement =
	    isPressed(keyboard, SDL_SCANCODE_LSHIFT) || isPressed(keyboard, SDL_SCANCODE_RSHIFT);
	return input;
}

void Window::show(const Picture& picture) {
	assert(picture.width() == width_);
	assert(picture.height() == height_);

	copyPicturePixels(picture);
	if (SDL_UpdateTexture(texture_, nullptr, pixels_.data(), width_ * kColorChannels) != 0) {
		throwSdlError("failed to update SDL texture");
	}
	if (SDL_RenderClear(renderer_) != 0) {
		throwSdlError("failed to clear SDL renderer");
	}
	if (SDL_RenderCopy(renderer_, texture_, nullptr, nullptr) != 0) {
		throwSdlError("failed to copy SDL texture");
	}
	SDL_RenderPresent(renderer_);
}

void Window::close() {
	if (texture_ != nullptr) {
		SDL_DestroyTexture(texture_);
		texture_ = nullptr;
	}
	if (renderer_ != nullptr) {
		SDL_DestroyRenderer(renderer_);
		renderer_ = nullptr;
	}
	if (window_ != nullptr) {
		SDL_DestroyWindow(window_);
		window_ = nullptr;
	}
}

void Window::copyPicturePixels(const Picture& picture) {
	for (int y = 0; y < height_; ++y) {
		for (int x = 0; x < width_; ++x) {
			const vec3& color = picture.colorAt(PixelX{x}, PixelY{y});
			const int offset = (y * width_ + x) * kColorChannels;
			pixels_[offset] = toColorChannel(color.r);
			pixels_[offset + 1] = toColorChannel(color.g);
			pixels_[offset + 2] = toColorChannel(color.b);
		}
	}
}

} // namespace renderer
