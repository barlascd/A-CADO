#include "geometry/MeshBuilder.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <algorithm>
#include <cmath>

MeshData MeshBuilder::build(const TopoDS_Shape& shape, double deflection) {
    MeshData out;
    if (shape.IsNull()) return out;
    BRepMesh_IncrementalMesh mesher(shape, deflection);
    TopTools_IndexedMapOfShape faces;
    TopExp::MapShapes(shape, TopAbs_FACE, faces);

    for (int fi = 1; fi <= faces.Extent(); ++fi) {
        TopoDS_Face face = TopoDS::Face(faces.FindKey(fi));
        TopLoc_Location loc;
        opencascade::handle<Poly_Triangulation> tri = BRep_Tool::Triangulation(face, loc);
        if (tri.IsNull()) continue;
        const gp_Trsf tr = loc.Transformation();
        const bool reversed = face.Orientation() == TopAbs_REVERSED;
        for (int t = 1; t <= tri->NbTriangles(); ++t) {
            int a, b, c;
            tri->Triangle(t).Get(a, b, c);
            if (reversed) std::swap(b, c);
            gp_Pnt p[3] = {tri->Node(a).Transformed(tr), tri->Node(b).Transformed(tr), tri->Node(c).Transformed(tr)};
            gp_Vec n = gp_Vec(p[0], p[1]).Crossed(gp_Vec(p[0], p[2]));
            double len = n.Magnitude();
            if (len > 1e-12) n.Divide(len);
            for (const gp_Pnt& q : p) {
                out.indices.push_back((uint32_t)out.vertices.size());
                out.vertices.push_back({(float)q.X(), (float)q.Y(), (float)q.Z(),
                                        (float)n.X(), (float)n.Y(), (float)n.Z(), 0.f, 0.f});
            }
            out.triangleFace.push_back(fi);
        }
    }
    return out;
}
