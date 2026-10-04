#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "core/Feature.h"

// Creates parametric features by type name ("box", "fillet", ...).
// Missing parameters fall back to the defaults listed in FeatureFactory.cpp.
class FeatureFactory {
public:
    static std::unique_ptr<Feature> create(int id, const std::string& type,
                                           const std::map<std::string, double>& values);
    static std::vector<std::string> types();
};
