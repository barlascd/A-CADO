#include "geometry/Gizmo.h"
#include <algorithm>
#include "geometry/Transform.h"

void Gizmo::update(double mouseX, double mouseY) {
    if (first_) { startX_ = mouseX; startY_ = mouseY; first_ = false; }
    dx_ = mouseX - startX_;
    dy_ = mouseY - startY_;
}
void Gizmo::apply(TopoDS_Shape& shape) {
    if (shape.IsNull()) return;
    switch (mode_) {
        case Mode::Move:   shape = Transform::translate(shape, dx_, -dy_, 0.0); break;
        case Mode::Rotate: shape = Transform::rotate(shape, dx_ * 0.01); break;
        case Mode::Scale:  shape = Transform::scale(shape, std::max(0.01, 1.0 + dx_ * 0.005)); break;
        case Mode::None:   break;
    }
    mode_ = Mode::None;
}
