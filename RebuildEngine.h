#pragma once
#include "core/FeatureTree.h"

class RebuildEngine {
public:
    // Re-runs dirty features (and everything downstream of them). Returns the final shape.
    static TopoDS_Shape rebuild(FeatureTree& tree, const TopoDS_Shape& base = TopoDS_Shape()) {
        TopoDS_Shape previous = base;
        bool upstreamChanged = false;
        for (auto& feature : tree.getFeatures()) {
            feature->setInput(previous);
            if (feature->isDirty() || upstreamChanged) {
                feature->build();
                upstreamChanged = true;
            }
            previous = feature->getShape();
        }
        return previous;
    }
};
