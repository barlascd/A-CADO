#pragma once
#include <Eigen/Dense>

struct SketchPoint { Eigen::Vector2d position; };
struct SketchLine { int pointA; int pointB; };
struct SketchCircle { int centerPoint; double radius; };
struct SketchArc { int centerPoint; double radius; double startAngle; double endAngle; };
