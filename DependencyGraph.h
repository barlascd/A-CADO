#pragma once
#include <unordered_map>
#include <vector>

class DependencyGraph {
    std::unordered_map<int, std::vector<int>> graph;
public:
    void addDependency(int feature, int dependency) { graph[feature].push_back(dependency); }
    void removeFeature(int feature) { graph.erase(feature); }
    void clear() { graph.clear(); }
    const std::vector<int>& getDependencies(int feature) const {
        static const std::vector<int> empty;
        auto it = graph.find(feature);
        return it == graph.end() ? empty : it->second;
    }
    std::vector<int> getDependents(int feature) const {
        std::vector<int> out;
        for (const auto& [f, deps] : graph)
            for (int d : deps) if (d == feature) out.push_back(f);
        return out;
    }
};
