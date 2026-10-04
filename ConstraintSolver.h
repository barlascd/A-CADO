#pragma once
#include <vector>
#include "sketch/Constraints.h"
#include "sketch/Sketch.h"

// Levenberg-Marquardt solver with a numeric Jacobian. Moves points and circle radii in-place.
class ConstraintSolver {
public:
    bool solve(Sketch& sketch, const std::vector<Constraint>& constraints);
};
