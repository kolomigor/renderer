#pragma once

#include "picture.h"

#include <filesystem>

namespace renderer {

bool savePPM(const Picture& picture, const std::filesystem::path& filename);

} // namespace renderer
