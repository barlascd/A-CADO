#pragma once
#include <string>
#include "ai/AICommand.h"
#include "ai/AIContext.h"
#include "core/Document.h"

class CADTools {
    Document& document;
    std::string lastError_;
public:
    explicit CADTools(Document& document) : document(document) {}
    bool createBox(double width, double height, double depth);
    bool createCylinder(double radius, double height);
    // Validates and runs a command. Command number keys equal the feature parameter names
    // (see FeatureFactory.cpp). Fillet/chamfer use ctx.selectedFace, hole uses ctx.cursorPosition by default.
    bool execute(const AICommand& command, const CadContext& ctx = CadContext());
    const std::string& lastError() const { return lastError_; }
};
