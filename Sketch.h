#pragma once
#include <TopoDS_Face.hxx>
#include <gp_Ax3.hxx>
#include <vector>
#include "sketch/SketchGeometry.h"

class Sketch {
    std::vector<SketchPoint> points_;
    std::vector<SketchLine> lines_;
    std::vector<SketchCircle> circles_;
    std::vector<SketchArc> arcs_;
public:
    int addPoint(double x, double y);
    void addLine(int a, int b);
    void addCircle(int center, double radius);
    void addArc(int center, double radius, double startAngle, double endAngle);

    std::vector<SketchPoint>& points() { return points_; }
    std::vector<SketchLine>& lines() { return lines_; }
    std::vector<SketchCircle>& circles() { return circles_; }
    std::vector<SketchArc>& arcs() { return arcs_; }
    const std::vector<SketchPoint>& points() const { return points_; }
    const std::vector<SketchLine>& lines() const { return lines_; }
    const std::vector<SketchCircle>& circles() const { return circles_; }

    // Closed line loop (in insertion order) or the first circle -> planar face on the given plane.
    TopoDS_Face toFace(const gp_Ax3& plane = gp_Ax3()) const;
};
