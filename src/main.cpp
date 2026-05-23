#include "application.h"
#include "except.h"

int main() {
	try {
		renderer::Application app;
		app.run();
	} catch (...) {
		renderer::reactToException();
		return 1;
	}
	return 0;
}
