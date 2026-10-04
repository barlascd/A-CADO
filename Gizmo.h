#pragma once
#include <TopoDS_Shape.hxx>

// Interactive transform helper: call begin*(), feed mouse positions to update(), then apply().
class Gizmo {
public:
    enum class Mode { None, Move, Rotate, Scale };
    void beginMove()   { start(Mode::Move); }
    void beginRotate() { start(Mode::Rotate); }
    void beginScale()  { start(Mode::Scale); }
    void update(double mouseX, double mouseY);
    void apply(TopoDS_Shape& shape);
    Mode mode() const { return mode_; }
private:
    void start(Mode m) { mode_ = m; first_ = true; dx_ = dy_ = 0; }
    Mode mode_ = Mode::None;
    bool first_ = true;
    double startX_ = 0, startY_ = 0, dx_ = 0, dy_ = 0;
};
