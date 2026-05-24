# Renderer

A small CPU software renderer with an SDL window, JSON scene loading, textured OBJ mesh loading,
perspective projection, frustum clipping, back-face culling, unlit material rendering, and a
depth buffer.

## Build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If `CMAKE_BUILD_TYPE` is not set, the project defaults to `Release` for single-config
generators.

## Run

From the project root:

```sh
./build/renderer
```

The default scene is resolved from the current directory or from the executable location, so
this also works from another directory:

```sh
/Users/kolomigor/renderer/build/renderer
```

Use an explicit frame limit:

```sh
./build/renderer --fps 60
./build/renderer --fps 120
```

Load a custom scene:

```sh
./build/renderer --scene assets/scenes/default.json
```

## Textures

Materials can reference ASCII or binary PPM textures with a `texture` field:

```json
{
  "albedo": [1.0, 1.0, 1.0],
  "texture": "../textures/ruby.ppm",
  "ambient": 0.32,
  "diffuse": 0.9,
  "specular": 0.35,
  "shininess": 40.0
}
```

OBJ meshes should provide `vt` texture coordinates and face indices such as `f 1/1 2/2 3/3`.
Texture paths are resolved relative to the scene JSON file.

Show command-line help:

```sh
./build/renderer --help
```

## Controls

- `W/A/S/D`: move camera horizontally
- `Space` or `E`: move up
- `Ctrl` or `Q`: move down
- `Shift`: move faster
- Arrow keys: turn camera
- Right mouse button + move mouse: look around
- `Esc`: quit

## Test

```sh
ctest --test-dir build --output-on-failure
```
