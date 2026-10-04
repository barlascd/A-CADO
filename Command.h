#pragma once
#include <memory>
#include <stack>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class CommandManager {
    std::stack<std::unique_ptr<Command>> undoStack, redoStack;
public:
    void execute(std::unique_ptr<Command> command) {
        command->execute();
        undoStack.push(std::move(command));
        while (!redoStack.empty()) redoStack.pop();
    }
    bool undo() {
        if (undoStack.empty()) return false;
        auto c = std::move(undoStack.top());
        undoStack.pop();
        c->undo();
        redoStack.push(std::move(c));
        return true;
    }
    bool redo() {
        if (redoStack.empty()) return false;
        auto c = std::move(redoStack.top());
        redoStack.pop();
        c->execute();
        undoStack.push(std::move(c));
        return true;
    }
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    void clear() { while (!undoStack.empty()) undoStack.pop(); while (!redoStack.empty()) redoStack.pop(); }
};
