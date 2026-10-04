#pragma once
#include <Eigen/Dense>
#include <vector>

struct Triangle { Eigen::Vector3f a, b, c; int id = 0; };
struct Ray { Eigen::Vector3f origin, direction; };

// Bounding volume hierarchy for ray picking.
class BVH {
    struct Node { Eigen::Vector3f min, max; int left = -1, right = -1, start = 0, count = 0; };
    std::vector<Triangle> tris_;
    std::vector<int> order_;
    std::vector<Node> nodes_;
    int buildNode(int begin, int end);
public:
    void build(const std::vector<Triangle>& triangles);
    // Triangle ids hit by the ray, nearest first.
    std::vector<int> raycast(const Ray& ray) const;
};
