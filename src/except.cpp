#include "except.h"

#include <exception>
#include <iostream>

namespace renderer {

void reactToException() {
	try {
		throw;
	} catch (const std::exception& exception) {
		std::cerr << "renderer error: " << exception.what() << '\n';
	} catch (...) {
		std::cerr << "unknown renderer error\n";
	}
}

} // namespace renderer
