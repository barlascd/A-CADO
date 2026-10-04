#pragma once
enum class MateType { Coincident, Concentric, Distance, Angle, Parallel, Perpendicular, Tangent };

struct Mate {
    MateType type = MateType::Coincident;
    int componentA = 0;
    int componentB = 0;
    double value = 0.0;
};
