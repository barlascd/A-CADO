#pragma once
#include <TopoDS_Shape.hxx>
#include <cstdint>
#include <vector>

struct GpuVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};
struct MeshData {
    std::vector<GpuVertex> vertices;      // 3 per triangle (flat shading)
    std::vector<uint32_t> indices;
    std::vector<int> triangleFace;        // 1-based face index per triangle
    bool empty() const { return indices.empty(); }
    size_t triangleCount() const { return indices.size() / 3; }
};

class MeshBuilder {
public:
    static MeshData build(const TopoDS_Shape& shape, double deflection = 0.1);
};
