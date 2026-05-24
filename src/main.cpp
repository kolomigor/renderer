#include "application.h"
#include "except.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

const std::filesystem::path kDefaultScenePath{"assets/scenes/default.json"};

struct ProgramOptions {
	std::optional<int> target_fps;
	std::filesystem::path scene_path;
	bool show_help = false;
};

std::string usage() {
	return "usage: renderer [--fps 60|120] [--scene PATH] [--help]";
}

std::string helpText() {
	return usage() +
	       "\n\n"
	       "Options:\n"
	       "  --fps 60|120   Limit rendering to 60 or 120 FPS. Without this, VSync is used.\n"
	       "  --scene PATH   Load a scene JSON file. Defaults to assets/scenes/default.json.\n"
	       "  -h, --help     Show this help message.\n"
	       "\n"
	       "Controls:\n"
	       "  W/A/S/D        Move camera horizontally.\n"
	       "  Space/E        Move up.\n"
	       "  Ctrl/Q         Move down.\n"
	       "  Shift          Move faster.\n"
	       "  Arrow keys     Turn camera.\n"
	       "  Right mouse    Look around.\n"
	       "  Esc            Quit.";
}

std::filesystem::path absoluteIfPossible(const std::filesystem::path& path) {
	std::error_code error;
	const std::filesystem::path absolute_path = std::filesystem::absolute(path, error);
	if (error) {
		return path;
	}
	return absolute_path;
}

std::filesystem::path canonicalIfPossible(const std::filesystem::path& path) {
	std::error_code error;
	const std::filesystem::path canonical_path = std::filesystem::weakly_canonical(path, error);
	if (error) {
		return path;
	}
	return canonical_path;
}

std::vector<std::filesystem::path> executablePathCandidates(const char *argv0) {
	std::vector<std::filesystem::path> candidates;
	if (argv0 == nullptr || std::string(argv0).empty()) {
		return candidates;
	}

	const std::filesystem::path executable(argv0);
	if (executable.is_absolute() || executable.has_parent_path()) {
		candidates.push_back(absoluteIfPossible(executable));
		return candidates;
	}

	if (const char *path_env = std::getenv("PATH")) {
		std::string paths(path_env);
		std::size_t start = 0;
		while (start <= paths.size()) {
			const std::size_t separator = paths.find(':', start);
			const std::size_t count =
			    separator == std::string::npos ? std::string::npos : separator - start;
			const std::string directory = paths.substr(start, count);
			if (!directory.empty()) {
				candidates.push_back(std::filesystem::path(directory) / executable);
			}
			if (separator == std::string::npos) {
				break;
			}
			start = separator + 1;
		}
	}
	candidates.push_back(absoluteIfPossible(executable));
	return candidates;
}

std::filesystem::path executableDirectory(const char *argv0) {
	for (const std::filesystem::path& candidate : executablePathCandidates(argv0)) {
		std::error_code error;
		if (std::filesystem::exists(candidate, error)) {
			return canonicalIfPossible(candidate).parent_path();
		}
	}
	return std::filesystem::current_path();
}

std::filesystem::path defaultScenePath(const char *argv0) {
	const std::filesystem::path executable_dir = executableDirectory(argv0);
	const std::vector<std::filesystem::path> candidates{
	    std::filesystem::current_path() / kDefaultScenePath,
	    executable_dir / kDefaultScenePath,
	    executable_dir.parent_path() / kDefaultScenePath,
	};

	for (const std::filesystem::path& candidate : candidates) {
		std::error_code error;
		if (std::filesystem::exists(candidate, error)) {
			return canonicalIfPossible(candidate);
		}
	}
	return executable_dir.parent_path() / kDefaultScenePath;
}

int parseFps(const std::string& value) {
	std::size_t parsed = 0;
	int fps = 0;
	try {
		fps = std::stoi(value, &parsed);
	} catch (const std::exception&) {
		throw std::runtime_error("invalid FPS value: " + value + "\n" + usage());
	}
	if (parsed != value.size() || (fps != 60 && fps != 120)) {
		throw std::runtime_error("invalid FPS value: " + value + "\n" + usage());
	}
	return fps;
}

std::string requireValue(int argc, char *argv[], int *index, const std::string& option) {
	if (*index + 1 >= argc) {
		throw std::runtime_error("missing value for " + option + "\n" + usage());
	}
	++(*index);
	return argv[*index];
}

ProgramOptions parseOptions(int argc, char *argv[]) {
	ProgramOptions options;
	options.scene_path = defaultScenePath(argc > 0 ? argv[0] : nullptr);

	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument == "-h" || argument == "--help") {
			options.show_help = true;
		} else if (argument == "--fps") {
			options.target_fps = parseFps(requireValue(argc, argv, &index, argument));
		} else if (argument == "--scene") {
			options.scene_path = requireValue(argc, argv, &index, argument);
		} else {
			throw std::runtime_error("unknown option: " + argument + "\n" + usage());
		}
	}

	return options;
}

} // namespace

int main(int argc, char *argv[]) {
	try {
		const ProgramOptions options = parseOptions(argc, argv);
		if (options.show_help) {
			std::cout << helpText() << '\n';
			return 0;
		}

		renderer::Application app({options.scene_path, options.target_fps});
		app.run();
	} catch (...) {
		renderer::reactToException();
		return 1;
	}
	return 0;
}
