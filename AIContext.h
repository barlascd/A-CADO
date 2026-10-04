#pragma once
#include <Eigen/Dense>
#include <string>

struct CadContext {
    int selectedObject = -1;
    int selectedFace = -1;
    int selectedEdge = -1;
    Eigen::Vector3d cursorPosition{0, 0, 0};
    std::string activeFeature;
};
