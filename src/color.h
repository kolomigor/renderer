#pragma once

#include <algorithm>
#include <cstdint>

namespace renderer {

inline std::uint8_t toColorChannel(float color) {
	return static_cast<std::uint8_t>(std::clamp(color, 0.0f, 1.0f) * 255.0f);
}

} // namespace renderer
