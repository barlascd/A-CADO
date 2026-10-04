#include "render/SpatialIndex.h"
#include <algorithm>
#include <limits>
#include <numeric>
#include <utility>

int BVH::buildNode(int begin, int end) {
    int ni = (int)nodes_.size();
    nodes_.emplace_back();
    Eigen::Vector3f mn = Eigen::Vector3f::Constant(std::numeric_limits<float>::max()), mx = -mn;
    for (int i = begin; i < end; ++i) {
        const Triangle& t = tris_[order_[i]];
        for (const auto* v : {&t.a, &t.b, &t.c}) { mn = mn.cwiseMin(*v); mx = mx.cwiseMax(*v); }
    }
    nodes_[ni].min = mn; nodes_[ni].max = mx;
    if (end - begin <= 4) { nodes_[ni].start = begin; nodes_[ni].count = end - begin; return ni; }
    int axis; (mx - mn).maxCoeff(&axis);
    int mid = (begin + end) / 2;
    std::nth_element(order_.begin() + begin, order_.begin() + mid, order_.begin() + end, [&](int x, int y) {
        return (tris_[x].a + tris_[x].b + tris_[x].c)[axis] < (tris_[y].a + tris_[y].b + tris_[y].c)[axis];
    });
    int l = buildNode(begin, mid), r = buildNode(mid, end);
    nodes_[ni].left = l; nodes_[ni].right = r;
    return ni;
}

void BVH::build(const std::vector<Triangle>& triangles) {
    tris_ = triangles; nodes_.clear();
    order_.resize(tris_.size());
    std::iota(order_.begin(), order_.end(), 0);
    if (!tris_.empty()) { nodes_.reserve(tris_.size()); buildNode(0, (int)tris_.size()); }
}

static bool hitBox(const Eigen::Vector3f& mn, const Eigen::Vector3f& mx, const Ray& r, float tmax) {
    float t0 = 0, t1 = tmax;
    for (int i = 0; i < 3; ++i) {
        if (std::abs(r.direction[i]) < 1e-12f) { if (r.origin[i] < mn[i] || r.origin[i] > mx[i]) return false; continue; }
        float inv = 1.f / r.direction[i], a = (mn[i] - r.origin[i]) * inv, b = (mx[i] - r.origin[i]) * inv;
        if (a > b) std::swap(a, b);
        t0 = std::max(t0, a); t1 = std::min(t1, b);
        if (t0 > t1) return false;
    }
    return true;
}
static bool hitTri(const Triangle& t, const Ray& r, float& out) {   // Moller-Trumbore, double sided
    Eigen::Vector3f e1 = t.b - t.a, e2 = t.c - t.a, p = r.direction.cross(e2);
    float det = e1.dot(p);
    if (std::abs(det) < 1e-12f) return false;
    float inv = 1.f / det;
    Eigen::Vector3f s = r.origin - t.a;
    float u = s.dot(p) * inv;
    if (u < 0 || u > 1) return false;
    Eigen::Vector3f q = s.cross(e1);
    float v = r.direction.dot(q) * inv;
    if (v < 0 || u + v > 1) return false;
    out = e2.dot(q) * inv;
    return out > 0;
}

std::vector<int> BVH::raycast(const Ray& ray) const {
    std::vector<std::pair<float, int>> hits;
    if (nodes_.empty()) return {};
    std::vector<int> stack{0};
    const float inf = std::numeric_limits<float>::max();
    while (!stack.empty()) {
        const Node& n = nodes_[stack.back()]; stack.pop_back();
        if (!hitBox(n.min, n.max, ray, inf)) continue;
        if (n.left < 0) {
            for (int i = n.start; i < n.start + n.count; ++i) {
                const Triangle& t = tris_[order_[i]];
                float d;
                if (hitTri(t, ray, d)) hits.emplace_back(d, t.id);
            }
        } else { stack.push_back(n.left); stack.push_back(n.right); }
    }
    std::sort(hits.begin(), hits.end());
    std::vector<int> ids;
    for (auto& h : hits) ids.push_back(h.second);
    return ids;
}
