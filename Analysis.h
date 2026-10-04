#pragma once
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

struct BoundingBox {
    double xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
    double dx() const { return xmax - xmin; }
    double dy() const { return ymax - ymin; }
    double dz() const { return zmax - zmin; }
};
struct MassProperties {
    double volume = 0, area = 0;
    gp_Pnt centre;
};

namespace Analysis {
double distance(const gp_Pnt& a, const gp_Pnt& b);
double minDistance(const TopoDS_Shape& a, const TopoDS_Shape& b);   // -1 on failure
BoundingBox boundingBox(const TopoDS_Shape& shape);
MassProperties massProperties(const TopoDS_Shape& shape);
bool boxesOverlap(const TopoDS_Shape& a, const TopoDS_Shape& b);    // fast AABB test
bool collides(const TopoDS_Shape& a, const TopoDS_Shape& b);        // AABB + boolean common volume
}
