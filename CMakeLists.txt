cmake_minimum_required(VERSION 3.21)
project(AICADO VERSION 0.1 LANGUAGES CXX)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # QML module dir 'AICADO/' would clash with the exe name
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(AICADO_GUI    "Build the Qt Quick application" ON)
option(AICADO_PYTHON "Build the pybind11 'aicado' module" OFF)
option(AICADO_OPENCV "Build the optional OpenCV edge detector" OFF)

find_package(OpenCASCADE REQUIRED)
find_package(Eigen3 REQUIRED)

# ---- Core library (no Qt) ---------------------------------------------------
add_library(aicado_core STATIC
    src/core/CadObject.cpp src/core/Document.cpp src/core/Assembly.cpp
    src/core/FeatureFactory.cpp
    src/geometry/GeometryEngine.cpp src/geometry/Transform.cpp src/geometry/Extrude.cpp
    src/geometry/Operations.cpp src/geometry/Analysis.cpp src/geometry/MeshBuilder.cpp
    src/geometry/Gizmo.cpp
    src/sketch/Sketch.cpp src/sketch/ConstraintSolver.cpp
    src/ai/CADTools.cpp
    src/io/StepIO.cpp
    src/render/SpatialIndex.cpp
)
target_include_directories(aicado_core PUBLIC src ${OpenCASCADE_INCLUDE_DIR})
target_link_directories(aicado_core PUBLIC ${OpenCASCADE_LIBRARY_DIR})
# Link only the toolkits we use (OpenCASCADE_LIBRARIES also lists Draw/test toolkits that are often not installed).
# Names differ between OCCT 7.6 (TKSTEP, TKSTL) and 7.8+ (TKDESTEP, TKDESTL); missing ones are skipped.
set(AICADO_OCCT_TK TKernel TKMath TKG2d TKG3d TKGeomBase TKBRep TKGeomAlgo TKTopAlgo TKPrim TKBO TKBool
    TKFillet TKOffset TKMesh TKShHealing TKXSBase TKService
    TKSTEP TKSTEPBase TKSTEPAttr TKSTEP209 TKSTL TKDESTEP TKDESTL TKDE)
set(AICADO_OCCT_LIBS "")
foreach(tk ${AICADO_OCCT_TK})
    find_library(AICADO_LIB_${tk} NAMES ${tk} HINTS ${OpenCASCADE_LIBRARY_DIR} ${OpenCASCADE_BINARY_DIR})
    if(AICADO_LIB_${tk})
        list(APPEND AICADO_OCCT_LIBS ${AICADO_LIB_${tk}})
    endif()
endforeach()
target_link_libraries(aicado_core PUBLIC Eigen3::Eigen ${AICADO_OCCT_LIBS})

if(AICADO_OPENCV)
    find_package(OpenCV REQUIRED)
    target_sources(aicado_core PRIVATE src/vision/EdgeDetector.cpp)
    target_compile_definitions(aicado_core PUBLIC AICADO_HAS_OPENCV)
    target_link_libraries(aicado_core PUBLIC ${OpenCV_LIBS})
endif()

enable_testing()
add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE aicado_core)
add_test(NAME core_test COMMAND core_test)

# ---- Qt Quick application ---------------------------------------------------
if(AICADO_GUI)
    find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Quick Qml QuickControls2)
    qt_standard_project_setup()
    qt_add_executable(AICADO
        src/main.cpp src/app/CadController.cpp src/app/ViewportItem.cpp src/io/ProjectIO.cpp)
    target_include_directories(AICADO PRIVATE src/app)   # qmltyperegistrar includes headers by bare name
    set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
    qt_add_qml_module(AICADO URI AICADO VERSION 1.0 QML_FILES qml/Main.qml)
    target_link_libraries(AICADO PRIVATE aicado_core Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml Qt6::QuickControls2)
    add_test(NAME selftest COMMAND AICADO --selftest)
endif()

# ---- Python module ----------------------------------------------------------
if(AICADO_PYTHON)
    find_package(pybind11 REQUIRED)
    pybind11_add_module(aicado src/python/PythonBridge.cpp)
    target_link_libraries(aicado PRIVATE aicado_core)
endif()
