#pragma once
#include <TopoDS_Shape.hxx>

class GeometryEngine {
public:
    static TopoDS_Shape createBox(double width, double height, double depth);
    static TopoDS_Shape createCylinder(double radius, double height);
    static TopoDS_Shape fuse(const TopoDS_Shape& a, const TopoDS_Shape& b);
    static TopoDS_Shape cut(const TopoDS_Shape& a, const TopoDS_Shape& b);
    static TopoDS_Shape common(const TopoDS_Shape& a, const TopoDS_Shape& b);
};
