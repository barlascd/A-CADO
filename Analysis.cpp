#include "geometry/Analysis.h"
#include <BRepBndLib.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include "geometry/GeometryEngine.h"

namespace Analysis {
double distance(const gp_Pnt& a, const gp_Pnt& b) { return a.Distance(b); }

double minDistance(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    BRepExtrema_DistShapeShape d(a, b);
    return d.IsDone() ? d.Value() : -1.0;
}
BoundingBox boundingBox(const TopoDS_Shape& shape) {
    BoundingBox r;
    if (shape.IsNull()) return r;
    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    if (box.IsVoid()) return r;
    box.Get(r.xmin, r.ymin, r.zmin, r.xmax, r.ymax, r.zmax);
    return r;
}
MassProperties massProperties(const TopoDS_Shape& shape) {
    MassProperties m;
    if (shape.IsNull()) return m;
    GProp_GProps v, s;
    BRepGProp::VolumeProperties(shape, v);
    BRepGProp::SurfaceProperties(shape, s);
    m.volume = v.Mass();
    m.centre = v.CentreOfMass();
    m.area = s.Mass();
    return m;
}
bool boxesOverlap(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    Bnd_Box ba, bb;
    BRepBndLib::Add(a, ba);
    BRepBndLib::Add(b, bb);
    return !ba.IsOut(bb);
}
bool collides(const TopoDS_Shape& a, const TopoDS_Shape& b) {
    if (!boxesOverlap(a, b)) return false;
    TopoDS_Shape common = GeometryEngine::common(a, b);
    return !common.IsNull() && massProperties(common).volume > 1e-6;
}
}  // namespace Analysis
