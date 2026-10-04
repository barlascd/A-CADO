#pragma once
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <vector>

struct HoleParameters {
    double diameter = 10.0;
    double depth = 20.0;
    bool throughAll = false;
    bool counterbore = false;
    bool countersink = false;
    double counterDiameter = 0.0;
    double counterDepth = 0.0;
};

namespace Ops {
TopoDS_Shape revolve(const TopoDS_Shape& profile, const gp_Ax1& axis, double angleRadians);
TopoDS_Shape fillet(const TopoDS_Shape& shape, const TopoDS_Edge& edge, double radius);
TopoDS_Shape filletEdges(const TopoDS_Shape& shape, const std::vector<TopoDS_Edge>& edges, double radius);
TopoDS_Shape chamferEdges(const TopoDS_Shape& shape, const std::vector<TopoDS_Edge>& edges, double distance);
// faceIndex is 1-based (same numbering as MeshBuilder); <1 means "all edges of the shape".
std::vector<TopoDS_Edge> edgesOf(const TopoDS_Shape& shape, int faceIndex);
// Hole is drilled downwards from the top (max Z) of the body at position.X(), position.Y().
TopoDS_Shape makeHole(const TopoDS_Shape& body, const gp_Pnt& position, const HoleParameters& p);
TopoDS_Shape loft(const std::vector<TopoDS_Wire>& profiles);
TopoDS_Shape sweep(const TopoDS_Wire& profile, const TopoDS_Wire& path);
TopoDS_Shape mirror(const TopoDS_Shape& shape, const gp_Ax2& plane);
std::vector<TopoDS_Shape> linearPattern(const TopoDS_Shape& shape, int count, double spacing, int axis = 0);
TopoDS_Shape linearPatternFused(const TopoDS_Shape& shape, int count, double spacing, int axis = 0);
}
