#include "application.h"
#include "except.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

const std::filesystem::path kDefaultScenesDirectory{"assets/scenes"};
const std::string kDefaultSceneName{"default"};

struct ProgramOptions {
	std::optional<int> target_fps;
	std::filesystem::path scenes_directory;
	std::filesystem::path scene_path;
	std::optional<std::filesystem::path> obj_path;
	std::optional<std::string> scene_spec;
	bool list_scenes = false;
	bool select_scene = false;
	bool show_help = false;
};

std::string usage() {
	return "usage: renderer [--fps 60|120] [--scene NAME|PATH] [--obj PATH] [--scenes-dir PATH] "
	       "[--list-scenes] [--select-scene] [--help]";
}

std::string helpText() {
	return usage() +
	       "\n\n"
	       "Options:\n"
	       "  --fps 60|120   Limit rendering to 60 or 120 FPS. Without this, VSync is used.\n"
	       "  --scene NAME   Load a scene from assets/scenes by name, for example default.\n"
	       "  --scene PATH   Load a scene JSON file by path, or auto-generate a scene for OBJ.\n"
	       "  --obj PATH     Auto-generate a scene for an OBJ model, without writing JSON.\n"
	       "  --scenes-dir PATH\n"
	       "                 Directory used for named scenes. Defaults to assets/scenes.\n"
	       "  --list-scenes  Print available scenes and exit.\n"
	       "  --select-scene Show a numbered scene picker before opening the renderer.\n"
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

std::filesystem::path defaultScenesDirectory(const char *argv0) {
	const std::filesystem::path executable_dir = executableDirectory(argv0);
	const std::vector<std::filesystem::path> candidates{
	    std::filesystem::current_path() / kDefaultScenesDirectory,
	    executable_dir / kDefaultScenesDirectory,
	    executable_dir.parent_path() / kDefaultScenesDirectory,
	};

	for (const std::filesystem::path& candidate : candidates) {
		std::error_code error;
		if (std::filesystem::is_directory(candidate, error)) {
			return canonicalIfPossible(candidate);
		}
	}
	return executable_dir.parent_path() / kDefaultScenesDirectory;
}

std::string sceneNameFromPath(const std::filesystem::path& path) {
	return path.stem().string();
}

std::vector<std::filesystem::path> sceneFiles(const std::filesystem::path& scenes_directory) {
	std::vector<std::filesystem::path> scenes;
	std::error_code error;
	if (!std::filesystem::is_directory(scenes_directory, error)) {
		return scenes;
	}
	for (const std::filesystem::directory_entry& entry :
	     std::filesystem::directory_iterator(scenes_directory, error)) {
		if (error) {
			break;
		}
		if (entry.is_regular_file(error) && entry.path().extension() == ".json") {
			scenes.push_back(canonicalIfPossible(entry.path()));
		}
	}
	std::sort(scenes.begin(), scenes.end());
	return scenes;
}

void printScenes(const std::vector<std::filesystem::path>& scenes) {
	if (scenes.empty()) {
		std::cout << "No scenes found.\n";
		return;
	}
	for (const std::filesystem::path& scene : scenes) {
		std::cout << sceneNameFromPath(scene) << "  " << scene.string() << '\n';
	}
}

std::filesystem::path resolveScenePath(const std::filesystem::path& scenes_directory,
                                       const std::string& scene_spec) {
	const std::filesystem::path requested(scene_spec);
	if (requested.is_absolute() || requested.has_parent_path()) {
		return canonicalIfPossible(requested);
	}

	std::filesystem::path candidate = scenes_directory / requested;
	if (!candidate.has_extension()) {
		candidate.replace_extension(".json");
	}
	std::error_code error;
	if (std::filesystem::exists(candidate, error)) {
		return canonicalIfPossible(candidate);
	}
	return canonicalIfPossible(requested);
}

bool hasExtension(const std::filesystem::path& path, const std::string& expected) {
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
		return static_cast<char>(std::tolower(value));
	});
	return extension == expected;
}

bool isObjPath(const std::filesystem::path& path) {
	return hasExtension(path, ".obj");
}

std::filesystem::path selectSceneInteractively(const std::filesystem::path& scenes_directory) {
	const std::vector<std::filesystem::path> scenes = sceneFiles(scenes_directory);
	if (scenes.empty()) {
		throw std::runtime_error("no scenes found in " + scenes_directory.string());
	}

	std::cout << "Available scenes:\n";
	for (std::size_t index = 0; index < scenes.size(); ++index) {
		std::cout << "  " << index + 1 << ". " << sceneNameFromPath(scenes[index]) << '\n';
	}
	std::cout << "Choose scene [1-" << scenes.size() << "]: ";

	std::string value;
	std::getline(std::cin, value);
	std::size_t parsed = 0;
	int selection = 0;
	try {
		selection = std::stoi(value, &parsed);
	} catch (const std::exception&) {
		throw std::runtime_error("invalid scene selection: " + value);
	}
	if (parsed != value.size() || selection < 1 ||
	    selection > static_cast<int>(scenes.size())) {
		throw std::runtime_error("invalid scene selection: " + value);
	}
	return scenes[static_cast<std::size_t>(selection - 1)];
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
	options.scenes_directory = defaultScenesDirectory(argc > 0 ? argv[0] : nullptr);

	for (int index = 1; index < argc; ++index) {
		const std::string argument = argv[index];
		if (argument == "-h" || argument == "--help") {
			options.show_help = true;
		} else if (argument == "--fps") {
			options.target_fps = parseFps(requireValue(argc, argv, &index, argument));
		} else if (argument == "--scene") {
			options.scene_spec = requireValue(argc, argv, &index, argument);
		} else if (argument == "--obj") {
			options.obj_path = requireValue(argc, argv, &index, argument);
		} else if (argument == "--scenes-dir") {
			options.scenes_directory = requireValue(argc, argv, &index, argument);
		} else if (argument == "--list-scenes") {
			options.list_scenes = true;
		} else if (argument == "--select-scene") {
			options.select_scene = true;
		} else {
			throw std::runtime_error("unknown option: " + argument + "\n" + usage());
		}
	}

	options.scenes_directory = canonicalIfPossible(options.scenes_directory);
	if (options.obj_path.has_value() &&
	    (options.scene_spec.has_value() || options.select_scene || options.list_scenes)) {
		throw std::runtime_error("--obj cannot be combined with scene selection options\n" + usage());
	}
	if (options.obj_path.has_value()) {
		if (!isObjPath(*options.obj_path)) {
			throw std::runtime_error("--obj expects an .obj file: " + options.obj_path->string());
		}
		options.obj_path = canonicalIfPossible(*options.obj_path);
	} else {
		options.scene_path =
		    resolveScenePath(options.scenes_directory, options.scene_spec.value_or(kDefaultSceneName));
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
		if (options.list_scenes) {
			printScenes(sceneFiles(options.scenes_directory));
			return 0;
		}

		const std::filesystem::path scene_path =
		    options.select_scene ? selectSceneInteractively(options.scenes_directory)
		                         : options.scene_path;
		renderer::ApplicationConfig config;
		config.target_fps = options.target_fps;
		if (options.obj_path.has_value()) {
			config.obj_path = options.obj_path;
		} else if (isObjPath(scene_path)) {
			config.obj_path = scene_path;
		} else {
			config.scene_path = scene_path;
		}
		renderer::Application app(config);
		app.run();
	} catch (...) {
		renderer::reactToException();
		return 1;
	}
	return 0;
}
