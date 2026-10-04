#include "core/CadObject.h"

CadObject::CadObject(int id, const std::string& name) : id(id), name(name) {}
int CadObject::getId() const { return id; }
const std::string& CadObject::getName() const { return name; }
void CadObject::setShape(const TopoDS_Shape& newShape) { shape = newShape; }
const TopoDS_Shape& CadObject::getShape() const { return shape; }
bool CadObject::isValid() const { return !shape.IsNull(); }
