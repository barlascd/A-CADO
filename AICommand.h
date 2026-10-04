#pragma once
#include <optional>
#include <string>
#include <unordered_map>

enum class CommandType { CreateBox, CreateCylinder, Extrude, Revolve, Fillet, Chamfer, Hole, Mirror, Pattern, Delete, Move };

struct AICommand {
    CommandType type = CommandType::CreateBox;
    std::unordered_map<std::string, double> numbers;
    std::unordered_map<std::string, std::string> strings;
};

// Maps the "operation" string used by the Python copilot / JSON commands.
inline std::optional<CommandType> commandTypeFromString(const std::string& s) {
    static const std::unordered_map<std::string, CommandType> m = {
        {"create_box", CommandType::CreateBox}, {"create_cylinder", CommandType::CreateCylinder},
        {"extrude", CommandType::Extrude}, {"revolve", CommandType::Revolve},
        {"fillet", CommandType::Fillet}, {"chamfer", CommandType::Chamfer}, {"hole", CommandType::Hole},
        {"mirror", CommandType::Mirror}, {"pattern", CommandType::Pattern},
        {"delete", CommandType::Delete}, {"move", CommandType::Move}};
    auto it = m.find(s);
    if (it == m.end()) return std::nullopt;
    return it->second;
}
