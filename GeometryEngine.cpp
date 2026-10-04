#include "geometry/GeometryEngine.h"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>

TopoDS_Shape GeometryEngine::createBox(double w, double h, double d) { return BRepPrimAPI_MakeBox(w, h, d).Shape(); }
TopoDS_Shape GeometryEngine::createCylinder(double r, double h) { return BRepPrimAPI_MakeCylinder(r, h).Shape(); }

TopoDS_Shape GeometryEngine::fuse(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    if (a.IsNull()) return b;
    if (b.IsNull()) return a;
    BRepAlgoAPI_Fuse op(a, b);
    return op.IsDone() ? op.Shape() : TopoDS_Shape();
}
TopoDS_Shape GeometryEngine::cut(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    if (a.IsNull() || b.IsNull()) return a;
    BRepAlgoAPI_Cut op(a, b);
    return op.IsDone() ? op.Shape() : TopoDS_Shape();
}
TopoDS_Shape GeometryEngine::common(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    if (a.IsNull() || b.IsNull()) return {};
    BRepAlgoAPI_Common op(a, b);
    return op.IsDone() ? op.Shape() : TopoDS_Shape();
}
