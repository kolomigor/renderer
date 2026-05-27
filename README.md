# Renderer

Небольшой программный CPU-рендерер с окном SDL, каталогом JSON-сцен, загрузкой текстурированных OBJ-моделей, перспективной проекцией, отсечением по пирамиде видимости, отсечением невидимых граней, рендерингом материалов без освещения и буфером глубины.

## Сборка

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Если `CMAKE_BUILD_TYPE` не задан, проект по умолчанию использует `Release` для одноконфигурационных генераторов.

## Запуск

Из корня проекта:

```sh
./build/renderer
```

Сцена по умолчанию ищется относительно текущей директории или относительно расположения исполняемого файла, поэтому это также работает из другой директории:

```sh
/Users/kolomigor/renderer/build/renderer
```

Использовать явное ограничение FPS:

```sh
./build/renderer --fps 60
./build/renderer --fps 120
```

Сцены лежат отдельно в `assets/scenes`. Посмотреть доступные сцены:

```sh
./build/renderer --list-scenes
```

Выбрать сцену по имени из `assets/scenes`:

```sh
./build/renderer --scene default
./build/renderer --scene gallery
./build/renderer --scene leather-bag
./build/renderer --scene performance
```

Показать интерактивный список сцен перед запуском:

```sh
./build/renderer --select-scene
```

Загрузить сцену из другого каталога или по прямому пути:

```sh
./build/renderer --scenes-dir path/to/scenes --scene custom
./build/renderer --scene assets/scenes/default.json
```

Открыть OBJ-модель без JSON-сцены:

```sh
./build/renderer --obj path/to/model.obj
./build/renderer --scene path/to/model.obj
```

В этом режиме рендерер сам создает сцену в памяти: загружает OBJ/MTL, задает дефолтный материал, считает границы модели и ставит камеру так, чтобы объект попал в кадр.

## Текстуры

Материалы могут ссылаться на JPEG, PNG, ASCII- или бинарные PPM-текстуры через поле `texture`:

```json
{
  "albedo": [1.0, 1.0, 1.0],
  "texture": "../textures/ruby.ppm"
}
```

OBJ-модели должны содержать текстурные координаты `vt` и индексы граней вида `f 1/1 2/2 3/3`.

Пути к текстурам разрешаются относительно JSON-файла сцены.

Показать справку по аргументам командной строки:

```sh
./build/renderer --help
```

## Управление

- `W/A/S/D`: перемещение камеры по горизонтали
- `Space` или `E`: движение вверх
- `Ctrl` или `Q`: движение вниз
- `Shift`: двигаться быстрее
- Стрелки: поворот камеры
- Правая кнопка мыши + движение мыши: осмотр вокруг
- `Esc`: выйти

## Тесты

```sh
ctest --test-dir build --output-on-failure
```
