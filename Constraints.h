#pragma once

// geometryA/geometryB are point indices (Coincident, Distance, Fixed), line indices
// (Horizontal, Vertical, Parallel, Perpendicular, Equal, Angle) or circle indices (Radius, Diameter).
// Tangent: geometryA = line, geometryB = circle.
enum class ConstraintType {
    Coincident, Horizontal, Vertical, Parallel, Perpendicular, Tangent,
    Equal, Distance, Angle, Radius, Diameter, Fixed
};
struct Constraint {
    ConstraintType type = ConstraintType::Fixed;
    int geometryA = -1;
    int geometryB = -1;
    double value = 0.0;     // length / radians / radius depending on type
};
