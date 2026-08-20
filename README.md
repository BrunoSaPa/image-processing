# Computer Vision Course Exercises

This repository contains small C++ exercises built with CMake and OpenCV. Each exercise is an independent executable.

## Prerequisites

- CMake 3.20 or newer
- A C++17 compiler
- OpenCV 4 (including the highgui module for exercises that open windows)

On macOS with Homebrew:

```sh
brew install cmake opencv
```

On Debian or Ubuntu:

```sh
sudo apt update
sudo apt install cmake g++ libopencv-dev
```

If CMake cannot find a Homebrew OpenCV installation, provide its CMake package directory while configuring:

```sh
cmake --preset debug \
  -DOpenCV_DIR="$(brew --prefix opencv)/lib/cmake/opencv4"
```

For an `opencv@4` formula, use `brew --prefix opencv@4` instead.

## Configure and build

Run these commands from the repository root:

```sh
cmake --preset debug
cmake --build --preset debug
```

The debug executables are written to `build/debug/`. For an optimized build, use the release preset:

```sh
cmake --preset release
cmake --build --preset release
```

To build only one exercise:

```sh
cmake --build --preset debug --target exercise_01
```

To run it, execute the binary from the repository root so relative output paths behave as expected:

```sh
./build/debug/exercise_01
```

Exercises that use `cv::imshow` need a graphical desktop session and wait for a key press before exiting. Generated images are written under `images/`, which is intentionally ignored by Git.

## Add a new exercise

Create a directory whose name starts with `exercise_` and put its entry point at `main.cpp`:

```text
exercises/
├── exercise_01/
│   └── main.cpp
├── exercise_02/
│   └── main.cpp
└── exercise_03/
    └── main.cpp
```

For example:

```sh
mkdir -p exercises/exercise_03
cp exercises/exercise_02/main.cpp exercises/exercise_03/main.cpp
```

Then build as usual:

```sh
cmake --build --preset debug
```

CMake automatically discovers `exercises/exercise_03/main.cpp` and creates an `exercise_03` target. If a generator does not notice the new file immediately, re-run `cmake --preset debug` and build again.

The top-level `CMakeLists.txt` is the only build file that needs to be maintained. Keep each exercise self-contained; shared code can be added later as a library under a separate source directory and linked from `add_cv_exercise`.

## Useful commands

```sh
# List configured targets
cmake --build --preset debug --target help

# Reconfigure after changing CMake settings
cmake --preset debug

# Build with verbose compiler/linker commands
cmake --build --preset debug --verbose
```

## Project layout

```text
.
├── CMakeLists.txt       # Automatic exercise discovery and OpenCV linking
├── CMakePresets.json    # Debug and release configurations
├── exercises/           # One independent executable per exercise
├── resources/           # Input images and other source data
├── images/              # Generated locally when an exercise writes images
└── build/               # Generated CMake build trees
```
