#include "geometry/Extrude.h"
#include <BRepPrimAPI_MakePrism.hxx>
#include <gp_Vec.hxx>

TopoDS_Shape Extrude::create(const TopoDS_Shape& face, double distance) {
    return BRepPrimAPI_MakePrism(face, gp_Vec(0, 0, distance)).Shape();
}
