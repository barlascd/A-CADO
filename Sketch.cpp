#include "sketch/Sketch.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Circ.hxx>
#include <gp_Pnt.hxx>

int Sketch::addPoint(double x, double y) {
    points_.push_back({Eigen::Vector2d(x, y)});
    return static_cast<int>(points_.size() - 1);
}
void Sketch::addLine(int a, int b) { lines_.push_back({a, b}); }
void Sketch::addCircle(int center, double radius) { circles_.push_back({center, radius}); }
void Sketch::addArc(int center, double radius, double s, double e) { arcs_.push_back({center, radius, s, e}); }

TopoDS_Face Sketch::toFace(const gp_Ax3& pl) const {
    auto world = [&](const Eigen::Vector2d& p) {
        gp_XYZ o = pl.Location().XYZ() + p.x() * pl.XDirection().XYZ() + p.y() * pl.YDirection().XYZ();
        return gp_Pnt(o);
    };
    if (!lines_.empty()) {
        BRepBuilderAPI_MakeWire wire;
        for (const auto& l : lines_) {
            BRepBuilderAPI_MakeEdge e(world(points_[l.pointA].position), world(points_[l.pointB].position));
            if (!e.IsDone()) return TopoDS_Face();
            wire.Add(e.Edge());
        }
        if (!wire.IsDone()) return TopoDS_Face();
        BRepBuilderAPI_MakeFace face(wire.Wire(), true);
        return face.IsDone() ? face.Face() : TopoDS_Face();
    }
    if (!circles_.empty()) {
        const auto& c = circles_.front();
        gp_Circ circ(gp_Ax2(world(points_[c.centerPoint].position), pl.Direction(), pl.XDirection()), c.radius);
        BRepBuilderAPI_MakeWire wire(BRepBuilderAPI_MakeEdge(circ).Edge());
        BRepBuilderAPI_MakeFace face(wire.Wire(), true);
        return face.IsDone() ? face.Face() : TopoDS_Face();
    }
    return TopoDS_Face();
}
