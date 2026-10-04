#pragma once
#include <TopoDS_Shape.hxx>

class Extrude {
public:
    static TopoDS_Shape create(const TopoDS_Shape& face, double distance);   // along world Z
};
