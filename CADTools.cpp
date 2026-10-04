#include "ai/CADTools.h"
#include <map>
#include "ai/AIValidator.h"

bool CADTools::createBox(double w, double h, double d) {
    return document.addFeature("box", {{"width", w}, {"height", h}, {"depth", d}}, &lastError_);
}
bool CADTools::createCylinder(double r, double h) {
    return document.addFeature("cylinder", {{"radius", r}, {"height", h}}, &lastError_);
}

bool CADTools::execute(const AICommand& c, const CadContext& ctx) {
    if (!AIValidator::validate(c)) { lastError_ = "invalid command parameters"; return false; }
    if (c.type == CommandType::Delete) {
        if (document.removeLastFeature()) return true;
        lastError_ = "nothing to delete";
        return false;
    }
    std::map<std::string, double> v(c.numbers.begin(), c.numbers.end());
    const char* type = "";
    switch (c.type) {
        case CommandType::CreateBox:      type = "box"; break;
        case CommandType::CreateCylinder: type = "cylinder"; break;
        case CommandType::Extrude:        type = "extrude"; break;
        case CommandType::Revolve:        type = "revolve"; break;
        case CommandType::Fillet:         type = "fillet"; break;
        case CommandType::Chamfer:        type = "chamfer"; break;
        case CommandType::Hole:           type = "hole"; break;
        case CommandType::Mirror:         type = "mirror"; break;
        case CommandType::Pattern:        type = "pattern"; break;
        case CommandType::Move:           type = "move"; break;
        default: break;
    }
    if ((c.type == CommandType::Fillet || c.type == CommandType::Chamfer) && !v.count("face") && ctx.selectedFace > 0)
        v["face"] = ctx.selectedFace;
    if (c.type == CommandType::Hole) {
        if (!v.count("x")) v["x"] = ctx.cursorPosition.x();
        if (!v.count("y")) v["y"] = ctx.cursorPosition.y();
    }
    return document.addFeature(type, v, &lastError_);
}
