#include "io/StepIO.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Writer.hxx>

bool exportSTEP(const TopoDS_Shape& shape, const std::string& filename) {
    if (shape.IsNull()) return false;
    STEPControl_Writer writer;
    if (writer.Transfer(shape, STEPControl_AsIs) != IFSelect_RetDone) return false;
    return writer.Write(filename.c_str()) == IFSelect_RetDone;
}

TopoDS_Shape importSTEP(const std::string& filename) {
    STEPControl_Reader reader;
    if (reader.ReadFile(filename.c_str()) != IFSelect_RetDone) return {};
    reader.TransferRoots();
    return reader.OneShape();
}

bool exportSTL(const TopoDS_Shape& shape, const std::string& filename) {
    if (shape.IsNull()) return false;
    BRepMesh_IncrementalMesh mesher(shape, 0.1);   // STL needs a triangulation
    StlAPI_Writer writer;
    return writer.Write(shape, filename.c_str());
}
