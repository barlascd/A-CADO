<img width="1536" height="1024" alt="WhatsApp Image 2026-09-27 at 11 24 15" src="https://github.com/user-attachments/assets/dfb4d8c5-915a-4953-a8e8-83bd336f66a1" />
# AICADO

Parametric CAD application: **OpenCASCADE** (geometry) + **Qt Quick** (UI) + **Eigen** (math).
Feature-tree modelling with undo/redo, face picking, STEP/STL export, project save/load, autosave.

## Features
- Primitives: box, cylinder
- Sketch-based: extrude (rectangle/circle, add or cut), revolve, loft, sweep
- Modify: fillet, chamfer (on a picked face or all edges), holes (blind / through / counterbore / countersink), mirror, linear pattern, move / rotate / scale
- Feature tree with automatic rebuild, undo/redo, JSON project files (`.aicado`), autosave
- STEP import, STEP/STL export
- Software 3D viewport: orbit (left drag), pan (right/middle drag), zoom (wheel), click to pick a face (BVH ray cast)
- 2D sketch constraint solver (Levenberg-Marquardt), assembly + simple mates, analysis (volume, area, bbox, distance, collision)
- AI/JSON command layer: `{"operation":"create_box","width":50,"height":50,"depth":50}`
- Plugin interface, thread pool (`TaskSystem`), optional Python module and OpenCV edge detector

## Build
Requirements: CMake >= 3.21, C++20 compiler, Qt >= 6.4 (6.5+ recommended), OpenCASCADE >= 7.6, Eigen3.

Ubuntu/Debian:
```bash
sudo apt install cmake g++ libocct-foundation-dev libocct-modeling-algorithms-dev \
  libocct-modeling-data-dev libocct-data-exchange-dev libtbb-dev libeigen3-dev \
  qt6-base-dev qt6-declarative-dev qml6-module-qtquick-controls qml6-module-qtquick-layouts \
  qml6-module-qtquick-dialogs qml6-module-qtquick-window qml6-module-qtqml-workerscript
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/bin/AICADO
```
Windows: install OpenCASCADE + Eigen via vcpkg, Qt 6 via the Qt installer, pass `-DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH=<Qt>`.
macOS: `brew install opencascade eigen qt`.

Options: `-DAICADO_GUI=OFF` (core + tests only), `-DAICADO_PYTHON=ON` (pybind11 module `aicado`), `-DAICADO_OPENCV=ON`.

Tests: `ctest --test-dir build` (`core_test` covers the geometry/core, `AICADO --selftest` is a headless smoke test).

## Layout
```
src/core      Document, FeatureTree, FeatureFactory, undo/redo, assembly, mates, thread pool
src/geometry  OpenCASCADE wrappers (booleans, fillet, hole, loft, sweep, ...), meshing, analysis, gizmo
src/sketch    Sketch, constraints, solver
src/ai        AICommand / validator / CADTools
src/io        STEP/STL, project JSON, autosave
src/render    Camera, BVH
src/app       CadController + ViewportItem (QML types)
src/python    pybind11 bridge + PythonCopilot.py
qml/Main.qml  UI
extras/       Vulkan renderer interface sketch (not built)
```

## Known limits
- Viewport is a CPU/QPainter renderer (fine for moderate models); the Vulkan renderer is only an interface sketch.
- Mates: only Coincident and Distance are implemented. The Gizmo class exists but is not wired to the UI yet.
- Imported STEP bodies are stored by file path in project files.
- Tested on Ubuntu 24.04 only
