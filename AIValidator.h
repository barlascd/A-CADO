#pragma once
#include <initializer_list>
#include "ai/AICommand.h"

class AIValidator {
    static bool required(const AICommand& c, std::initializer_list<const char*> keys) {
        for (auto k : keys) { auto it = c.numbers.find(k); if (it == c.numbers.end() || !(it->second > 0)) return false; }
        return true;
    }
    static bool optionalPositive(const AICommand& c, std::initializer_list<const char*> keys) {
        for (auto k : keys) { auto it = c.numbers.find(k); if (it != c.numbers.end() && !(it->second > 0)) return false; }
        return true;
    }
public:
    static bool validate(const AICommand& c) {
        switch (c.type) {
            case CommandType::CreateBox:      return required(c, {"width", "height", "depth"});
            case CommandType::CreateCylinder: return required(c, {"radius", "height"});
            case CommandType::Fillet:         return required(c, {"radius"});
            case CommandType::Chamfer:        return required(c, {"distance"});
            case CommandType::Hole:           return required(c, {"diameter"}) && optionalPositive(c, {"depth"});
            case CommandType::Extrude: {
                auto it = c.numbers.find("distance");
                return it != c.numbers.end() && it->second != 0 && optionalPositive(c, {"width", "height", "radius"});
            }
            case CommandType::Revolve:        return optionalPositive(c, {"outer", "height", "angle"});
            case CommandType::Pattern: {
                auto it = c.numbers.find("count");
                return it == c.numbers.end() || it->second >= 2;
            }
            default: return true;
        }
    }
};
