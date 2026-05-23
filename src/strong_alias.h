#pragma once

namespace renderer {

template <class T, class Tag> class Alias {
public:
	explicit constexpr Alias(T value) : value_(value) {
	}

	constexpr operator T() const {
		return value_;
	}

	constexpr T value() const {
		return value_;
	}

private:
	T value_;
};

using Fov = Alias<float, struct FovTag>;
using Aspect = Alias<float, struct AspectTag>;
using NearPlane = Alias<float, struct NearPlaneTag>;
using FarPlane = Alias<float, struct FarPlaneTag>;

using Width = Alias<int, struct WidthTag>;
using Height = Alias<int, struct HeightTag>;
using PixelX = Alias<int, struct PixelXTag>;
using PixelY = Alias<int, struct PixelYTag>;

} // namespace renderer
