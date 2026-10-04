// Qt-free regression test of the core library.
#include <cmath>
#include <iostream>
#include "ai/CADTools.h"
#include "core/Assembly.h"
#include "geometry/Analysis.h"
#include "geometry/GeometryEngine.h"
#include "geometry/MeshBuilder.h"
#include "io/StepIO.h"
#include "render/SpatialIndex.h"
#include "sketch/ConstraintSolver.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::cerr << "FAIL line " << __LINE__ << ": " #c "\n"; ++failures; } } while (0)

int main() {
    Document doc; std::string err;
    CHECK(doc.addFeature("box", {{"width", 100}, {"height", 50}, {"depth", 20}}, &err));
    CHECK(std::abs(Analysis::massProperties(doc.shape()).volume - 100000) < 1e-3);
    CHECK(doc.addFeature("fillet", {{"radius", 3}}, &err));
    CHECK(doc.addFeature("hole", {{"x", 50}, {"y", 25}, {"diameter", 10}, {"throughAll", 1}}, &err));
    double v = Analysis::massProperties(doc.shape()).volume;
    CHECK(v > 0 && v < 100000);
    CHECK(doc.undo()); CHECK(doc.undo()); CHECK(doc.redo());
    CHECK(doc.getFeatureTree().size() == 2);
    CHECK(!doc.addFeature("fillet", {{"radius", 500}}, &err));   // must fail cleanly
    CHECK(doc.getFeatureTree().size() == 2);

    for (const char* t : {"extrude", "revolve", "loft", "sweep", "mirror", "pattern", "move", "rotate", "scale", "cylinder"}) {
        Document d; d.addFeature("box", {}, &err);
        bool ok = d.addFeature(t, {}, &err);
        if (!ok) std::cerr << t << ": " << err << "\n";
        CHECK(ok);
    }
    { Document d; CHECK(d.addFeature("extrude", {{"kind", 1}, {"radius", 10}, {"distance", 30}}, &err)); CHECK(Analysis::massProperties(d.shape()).volume > 9000); }

    MeshData m = MeshBuilder::build(doc.shape());
    CHECK(m.triangleCount() > 12);
    std::vector<Triangle> tris;
    for (size_t t = 0; t < m.triangleCount(); ++t) {
        auto g = [&](int k) { auto& q = m.vertices[m.indices[3 * t + k]]; return Eigen::Vector3f(q.x, q.y, q.z); };
        tris.push_back({g(0), g(1), g(2), (int)t});
    }
    BVH bvh; bvh.build(tris);
    CHECK(!bvh.raycast({{50, 25, 100}, {0, 0, -1}}).empty());
    CHECK(bvh.raycast({{500, 500, 100}, {0, 0, -1}}).empty());

    CADTools tools(doc); AICommand c; c.type = CommandType::CreateCylinder; c.numbers = {{"radius", 5}, {"height", 10}};
    CHECK(tools.execute(c));
    AICommand bad; bad.type = CommandType::CreateBox; bad.numbers = {{"width", -1}, {"height", 1}, {"depth", 1}};
    CHECK(!tools.execute(bad));

    CHECK(exportSTEP(doc.shape(), "/tmp/t.step")); CHECK(!importSTEP("/tmp/t.step").IsNull());
    CHECK(exportSTL(doc.shape(), "/tmp/t.stl"));

    Sketch s; int a = s.addPoint(0, 0), b = s.addPoint(9, 3), cc = s.addPoint(8, 8); s.addLine(a, b); s.addLine(b, cc);
    ConstraintSolver solver;
    CHECK(solver.solve(s, {{ConstraintType::Fixed, a}, {ConstraintType::Horizontal, 0}, {ConstraintType::Distance, a, b, 10}, {ConstraintType::Vertical, 1}}));
    CHECK(std::abs(s.points()[b].position.y()) < 1e-5 && std::abs(s.points()[b].position.x() - 10) < 1e-5);
    CHECK(std::abs(s.points()[cc].position.x() - 10) < 1e-5);

    Assembly as; Component c1, c2; c1.id = 1; c1.shape = GeometryEngine::createBox(10, 10, 10); c2.id = 2; c2.shape = GeometryEngine::createBox(4, 4, 4);
    as.add(c1); as.add(c2); as.addMate({MateType::Coincident, 1, 2, 0});
    CHECK(as.solveMates());
    CHECK(Analysis::collides(c1.shape, c1.shape));

    std::cout << (failures ? "FAILED\n" : "all core tests passed\n");
    return failures;
}
