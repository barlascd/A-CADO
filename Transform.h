#pragma once
#include <TopoDS_Shape.hxx>

class Transform {
public:
    static TopoDS_Shape translate(const TopoDS_Shape& shape, double x, double y, double z);
    static TopoDS_Shape rotate(const TopoDS_Shape& shape, double angleRadians);   // about world Z
    static TopoDS_Shape scale(const TopoDS_Shape& shape, double factor);          // about origin
};
