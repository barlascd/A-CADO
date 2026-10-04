#include "geometry/Transform.h"
#include <BRepBuilderAPI_Transform.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

TopoDS_Shape Transform::translate(const TopoDS_Shape& shape, double x, double y, double z) {
    gp_Trsf t; t.SetTranslation(gp_Vec(x, y, z));
    return BRepBuilderAPI_Transform(shape, t).Shape();
}
TopoDS_Shape Transform::rotate(const TopoDS_Shape& shape, double angle) {
    gp_Trsf t; t.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), angle);
    return BRepBuilderAPI_Transform(shape, t).Shape();
}
TopoDS_Shape Transform::scale(const TopoDS_Shape& shape, double factor) {
    gp_Trsf t; t.SetScale(gp_Pnt(0, 0, 0), factor);
    return BRepBuilderAPI_Transform(shape, t).Shape();
}
