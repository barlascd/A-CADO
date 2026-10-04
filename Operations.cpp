#include "geometry/Operations.h"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_TransitionMode.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <Bnd_Box.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include "geometry/GeometryEngine.h"

namespace Ops {

TopoDS_Shape revolve(const TopoDS_Shape& profile, const gp_Ax1& axis, double angle) {
    return BRepPrimAPI_MakeRevol(profile, axis, angle).Shape();
}

TopoDS_Shape fillet(const TopoDS_Shape& shape, const TopoDS_Edge& edge, double radius) {
    return filletEdges(shape, {edge}, radius);
}
TopoDS_Shape filletEdges(const TopoDS_Shape& shape, const std::vector<TopoDS_Edge>& edges, double radius) {
    BRepFilletAPI_MakeFillet maker(shape);
    for (const auto& e : edges) maker.Add(radius, e);
    maker.Build();
    if (!maker.IsDone()) return {};
    return maker.Shape();
}
TopoDS_Shape chamferEdges(const TopoDS_Shape& shape, const std::vector<TopoDS_Edge>& edges, double distance) {
    BRepFilletAPI_MakeChamfer maker(shape);
    for (const auto& e : edges) maker.Add(distance, e);
    maker.Build();
    if (!maker.IsDone()) return {};
    return maker.Shape();
}
std::vector<TopoDS_Edge> edgesOf(const TopoDS_Shape& shape, int faceIndex) {
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(shape, TopAbs_FACE, faces);
    const TopoDS_Shape& scope = (faceIndex >= 1 && faceIndex <= faces.Extent()) ? faces.FindKey(faceIndex) : shape;
    TopTools_IndexedMapOfShape edges;
    TopExp::MapShapes(scope, TopAbs_EDGE, edges);
    std::vector<TopoDS_Edge> out;
    for (int i = 1; i <= edges.Extent(); ++i) out.push_back(TopoDS::Edge(edges.FindKey(i)));
    return out;
}

TopoDS_Shape makeHole(const TopoDS_Shape& body, const gp_Pnt& position, const HoleParameters& p) {
    Bnd_Box box;
    BRepBndLib::Add(body, box);
    double x0, y0, z0, x1, y1, z1;
    box.Get(x0, y0, z0, x1, y1, z1);
    const double x = position.X(), y = position.Y(), top = z1;
    const gp_Dir down(0, 0, -1);

    double startZ = p.throughAll ? top + 1.0 : top;
    double depth = p.throughAll ? (z1 - z0) + 2.0 : p.depth;
    TopoDS_Shape tool = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x, y, startZ), down), p.diameter / 2.0, depth).Shape();
    TopoDS_Shape result = GeometryEngine::cut(body, tool);

    if (!result.IsNull() && p.counterbore && p.counterDiameter > p.diameter) {
        TopoDS_Shape cb = BRepPrimAPI_MakeCylinder(gp_Ax2(gp_Pnt(x, y, top + 1.0), down), p.counterDiameter / 2.0, p.counterDepth + 1.0).Shape();
        result = GeometryEngine::cut(result, cb);
    } else if (!result.IsNull() && p.countersink && p.counterDiameter > p.diameter) {
        TopoDS_Shape cs = BRepPrimAPI_MakeCone(gp_Ax2(gp_Pnt(x, y, top), down), p.counterDiameter / 2.0, p.diameter / 2.0, p.counterDepth).Shape();
        result = GeometryEngine::cut(result, cs);
    }
    return result;
}

TopoDS_Shape loft(const std::vector<TopoDS_Wire>& profiles) {
    BRepOffsetAPI_ThruSections maker(true, false);
    for (const auto& w : profiles) maker.AddWire(w);
    maker.Build();
    if (!maker.IsDone()) return {};
    return maker.Shape();
}

TopoDS_Shape sweep(const TopoDS_Wire& profile, const TopoDS_Wire& path) {
    BRepOffsetAPI_MakePipeShell pipe(path);
    pipe.SetTransitionMode(BRepBuilderAPI_RoundCorner);
    pipe.Add(profile);
    pipe.Build();
    if (!pipe.IsDone()) return {};
    pipe.MakeSolid();
    return pipe.Shape();
}

TopoDS_Shape mirror(const TopoDS_Shape& shape, const gp_Ax2& plane) {
    gp_Trsf t;
    t.SetMirror(plane);
    return BRepBuilderAPI_Transform(shape, t).Shape();
}

std::vector<TopoDS_Shape> linearPattern(const TopoDS_Shape& shape, int count, double spacing, int axis) {
    std::vector<TopoDS_Shape> result;
    for (int i = 0; i < count; ++i) {
        gp_Trsf t;
        double d = i * spacing;
        t.SetTranslation(gp_Vec(axis == 0 ? d : 0, axis == 1 ? d : 0, axis == 2 ? d : 0));
        result.push_back(BRepBuilderAPI_Transform(shape, t).Shape());
    }
    return result;
}
TopoDS_Shape linearPatternFused(const TopoDS_Shape& shape, int count, double spacing, int axis) {
    TopoDS_Shape out;
    for (const auto& s : linearPattern(shape, count, spacing, axis)) out = GeometryEngine::fuse(out, s);
    return out;
}
}  // namespace Ops
