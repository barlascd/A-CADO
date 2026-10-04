// Not part of the build. Interface sketch for a future Vulkan backend; the current viewport
// (src/app/ViewportItem.cpp) renders with QPainter so it runs everywhere without a GPU API.
#pragma once
#include "geometry/MeshBuilder.h"

class VulkanRenderer {
public:
    bool initialize();
    void beginFrame();
    void drawMesh(const MeshData& mesh);
    void endFrame();
    void shutdown();
};
