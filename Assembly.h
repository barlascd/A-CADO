#pragma once
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <string>
#include <vector>
#include "core/Mates.h"

struct Component {
    int id = 0;
    std::string name;
    TopoDS_Shape shape;
    gp_Trsf transform;
};

class Assembly {
    std::vector<Component> components_;
    std::vector<Mate> mates_;
public:
    void add(const Component& component) { components_.push_back(component); }
    void addMate(const Mate& m) { mates_.push_back(m); }
    const std::vector<Component>& components() const { return components_; }
    // Implemented: Coincident (centres of mass) and Distance (offset along X). Others return false.
    bool solveMates();
    TopoDS_Compound compound() const;
};
