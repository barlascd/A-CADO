#pragma once
#include <Standard_Failure.hxx>
#include <TopoDS_Shape.hxx>
#include <exception>
#include <string>
#include <vector>
#include "core/Parameter.h"

class Feature {
protected:
    int id;
    std::string name;
    TopoDS_Shape input;
    TopoDS_Shape result;
    std::vector<Parameter> parameters;
    bool dirty = true;
    std::string error;
public:
    Feature(int id, const std::string& name) : id(id), name(name) {}
    virtual ~Feature() = default;
    virtual void rebuild() = 0;
    virtual std::string type() const = 0;

    // Runs rebuild(); on failure keeps the input shape and stores the error.
    bool build() {
        error.clear();
        try { rebuild(); }
        catch (const Standard_Failure& e) { error = e.GetMessageString() ? e.GetMessageString() : "OCCT error"; }
        catch (const std::exception& e) { error = e.what(); }
        if (!error.empty()) result = input;
        clearDirty();
        return error.empty();
    }
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getError() const { return error; }
    const TopoDS_Shape& getShape() const { return result; }
    void setInput(const TopoDS_Shape& s) { input = s; }
    const std::vector<Parameter>& getParameters() const { return parameters; }
    double num(const std::string& key) const {
        for (const auto& p : parameters) if (p.getName() == key) return p.asDouble();
        return 0.0;
    }
    void setParameter(const std::string& key, double v) {
        for (auto& p : parameters) if (p.getName() == key) { p.set(v); markDirty(); return; }
    }
    bool isDirty() const { return dirty; }
    void markDirty() { dirty = true; }
    void clearDirty() { dirty = false; }
};
