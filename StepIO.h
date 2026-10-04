#pragma once
#include <TopoDS_Shape.hxx>
#include <string>

bool exportSTEP(const TopoDS_Shape& shape, const std::string& filename);
TopoDS_Shape importSTEP(const std::string& filename);
bool exportSTL(const TopoDS_Shape& shape, const std::string& filename);
