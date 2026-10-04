#pragma once
#include <memory>
#include <vector>
#include "core/Feature.h"

class FeatureTree {
    std::vector<std::unique_ptr<Feature>> features;
public:
    void add(std::unique_ptr<Feature> feature) { features.push_back(std::move(feature)); }
    std::unique_ptr<Feature> pop() {
        if (features.empty()) return nullptr;
        auto f = std::move(features.back());
        features.pop_back();
        return f;
    }
    auto& getFeatures() { return features; }
    const auto& getFeatures() const { return features; }
    size_t size() const { return features.size(); }
    bool empty() const { return features.empty(); }
    void clear() { features.clear(); }
};
