#pragma once
#include <TopoDS_Shape.hxx>
#include <string>

class CadObject {
    int id;
    std::string name;
    TopoDS_Shape shape;
public:
    CadObject(int id, const std::string& name);
    int getId() const;
    const std::string& getName() const;
    void setShape(const TopoDS_Shape& newShape);
    const TopoDS_Shape& getShape() const;
    bool isValid() const;
};
