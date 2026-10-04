#include "core/FeatureFactory.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <functional>
#include <stdexcept>

#include "geometry/Extrude.h"
#include "geometry/GeometryEngine.h"
#include "geometry/Operations.h"
#include "geometry/Transform.h"
#include "sketch/Sketch.h"

namespace {
constexpr double kPi = 3.14159265358979323846;

class ParametricFeature : public Feature {
public:
    using Builder = std::function<TopoDS_Shape(const TopoDS_Shape&, const ParametricFeature&)>;
    ParametricFeature(int id, std::string type, std::vector<Parameter> params, Builder b)
        : Feature(id, type), typeName(std::move(type)), builder(std::move(b)) {
        parameters = std::move(params);
    }
    void rebuild() override {
        result = builder(input, *this);
        if (result.IsNull()) throw std::runtime_error(typeName + ": operation failed");
    }
    std::string type() const override { return typeName; }
private:
    std::string typeName;
    Builder builder;
};

using F = ParametricFeature;
using Defaults = std::vector<std::pair<std::string, double>>;
struct Def { Defaults defaults; ParametricFeature::Builder build; };

TopoDS_Shape place(const TopoDS_Shape& s, const F& f) {
    double x = f.num("x"), y = f.num("y"), z = f.num("z");
    return (x != 0 || y != 0 || z != 0) ? Transform::translate(s, x, y, z) : s;
}
TopoDS_Shape combine(const TopoDS_Shape& in, const TopoDS_Shape& solid, const F& f) {
    if (in.IsNull()) {
        if (f.num("cut") > 0.5) throw std::runtime_error("nothing to cut from");
        return solid;
    }
    return f.num("cut") > 0.5 ? GeometryEngine::cut(in, solid) : GeometryEngine::fuse(in, solid);
}
void requireBody(const TopoDS_Shape& in) { if (in.IsNull()) throw std::runtime_error("no body yet - create a primitive first"); }

TopoDS_Wire rectWire(double w, double h, double z) {
    gp_Pnt a(-w / 2, -h / 2, z), b(w / 2, -h / 2, z), c(w / 2, h / 2, z), d(-w / 2, h / 2, z);
    BRepBuilderAPI_MakePolygon poly(a, b, c, d, true);
    return poly.Wire();
}

const std::map<std::string, Def>& registry() {
    static const std::map<std::string, Def> r = {
        {"box", {{{"width", 100}, {"height", 50}, {"depth", 20}, {"x", 0}, {"y", 0}, {"z", 0}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                return combine(in, place(GeometryEngine::createBox(f.num("width"), f.num("height"), f.num("depth")), f), f);
            }}},
        {"cylinder", {{{"radius", 20}, {"height", 40}, {"x", 0}, {"y", 0}, {"z", 0}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                return combine(in, place(GeometryEngine::createCylinder(f.num("radius"), f.num("height")), f), f);
            }}},
        {"extrude", {{{"kind", 0}, {"width", 40}, {"height", 30}, {"radius", 15}, {"distance", 20},
                      {"x", 0}, {"y", 0}, {"z", 0}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                Sketch s;
                double x = f.num("x"), y = f.num("y");
                if (f.num("kind") < 0.5) {
                    double w = f.num("width"), h = f.num("height");
                    int a = s.addPoint(x, y), b = s.addPoint(x + w, y), c = s.addPoint(x + w, y + h), d = s.addPoint(x, y + h);
                    s.addLine(a, b); s.addLine(b, c); s.addLine(c, d); s.addLine(d, a);
                } else {
                    s.addCircle(s.addPoint(x, y), f.num("radius"));
                }
                // x/y are already inside the sketch; only z lifts the sketch plane.
                gp_Ax3 plane(gp_Pnt(0, 0, f.num("z")), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0));
                TopoDS_Face face = s.toFace(plane);
                if (face.IsNull()) throw std::runtime_error("extrude: invalid profile");
                return combine(in, Extrude::create(face, f.num("distance")), f);
            }}},
        {"revolve", {{{"inner", 0}, {"outer", 20}, {"height", 40}, {"angle", 360}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                Sketch s;
                double i = f.num("inner"), o = f.num("outer"), h = f.num("height");
                int a = s.addPoint(i, 0), b = s.addPoint(o, 0), c = s.addPoint(o, h), d = s.addPoint(i, h);
                s.addLine(a, b); s.addLine(b, c); s.addLine(c, d); s.addLine(d, a);
                gp_Ax3 xz(gp_Pnt(0, 0, 0), gp_Dir(0, -1, 0), gp_Dir(1, 0, 0));   // local Y -> world Z
                TopoDS_Face face = s.toFace(xz);
                if (face.IsNull()) throw std::runtime_error("revolve: invalid profile");
                return combine(in, Ops::revolve(face, gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), f.num("angle") * kPi / 180.0), f);
            }}},
        {"loft", {{{"w1", 60}, {"h1", 60}, {"w2", 20}, {"h2", 20}, {"height", 50}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                std::vector<TopoDS_Wire> wires{rectWire(f.num("w1"), f.num("h1"), 0), rectWire(f.num("w2"), f.num("h2"), f.num("height"))};
                return combine(in, Ops::loft(wires), f);
            }}},
        {"sweep", {{{"radius", 5}, {"length1", 60}, {"length2", 40}, {"cut", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                BRepBuilderAPI_MakePolygon path;
                path.Add(gp_Pnt(0, 0, 0)); path.Add(gp_Pnt(0, 0, f.num("length1"))); path.Add(gp_Pnt(f.num("length2"), 0, f.num("length1")));
                gp_Circ circle(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), f.num("radius"));
                TopoDS_Wire profile = BRepBuilderAPI_MakeWire(BRepBuilderAPI_MakeEdge(circle).Edge()).Wire();
                return combine(in, Ops::sweep(profile, path.Wire()), f);
            }}},
        {"fillet", {{{"radius", 3}, {"face", -1}},
            [](const TopoDS_Shape& in, const F& f) {
                requireBody(in);
                return Ops::filletEdges(in, Ops::edgesOf(in, (int)f.num("face")), f.num("radius"));
            }}},
        {"chamfer", {{{"distance", 2}, {"face", -1}},
            [](const TopoDS_Shape& in, const F& f) {
                requireBody(in);
                return Ops::chamferEdges(in, Ops::edgesOf(in, (int)f.num("face")), f.num("distance"));
            }}},
        {"hole", {{{"x", 10}, {"y", 10}, {"diameter", 10}, {"depth", 20}, {"throughAll", 0},
                   {"counterbore", 0}, {"countersink", 0}, {"counterDiameter", 0}, {"counterDepth", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                requireBody(in);
                HoleParameters p;
                p.diameter = f.num("diameter"); p.depth = f.num("depth");
                p.throughAll = f.num("throughAll") > 0.5;
                p.counterbore = f.num("counterbore") > 0.5; p.countersink = f.num("countersink") > 0.5;
                p.counterDiameter = f.num("counterDiameter"); p.counterDepth = f.num("counterDepth");
                return Ops::makeHole(in, gp_Pnt(f.num("x"), f.num("y"), 0), p);
            }}},
        {"mirror", {{{"plane", 0}, {"offset", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                requireBody(in);
                int p = (int)f.num("plane"); double o = f.num("offset");
                gp_Pnt origin(p == 0 ? o : 0, p == 1 ? o : 0, p == 2 ? o : 0);
                gp_Dir normal(p == 0 ? 1 : 0, p == 1 ? 1 : 0, p == 2 ? 1 : 0);
                return GeometryEngine::fuse(in, Ops::mirror(in, gp_Ax2(origin, normal)));
            }}},
        {"pattern", {{{"count", 3}, {"spacing", 30}, {"axis", 0}},
            [](const TopoDS_Shape& in, const F& f) {
                requireBody(in);
                return Ops::linearPatternFused(in, (int)f.num("count"), f.num("spacing"), (int)f.num("axis"));
            }}},
        {"move", {{{"x", 0}, {"y", 0}, {"z", 0}},
            [](const TopoDS_Shape& in, const F& f) { requireBody(in); return Transform::translate(in, f.num("x"), f.num("y"), f.num("z")); }}},
        {"rotate", {{{"angle", 90}},
            [](const TopoDS_Shape& in, const F& f) { requireBody(in); return Transform::rotate(in, f.num("angle") * kPi / 180.0); }}},
        {"scale", {{{"factor", 1.5}},
            [](const TopoDS_Shape& in, const F& f) { requireBody(in); return Transform::scale(in, f.num("factor")); }}},
    };
    return r;
}
}  // namespace

std::unique_ptr<Feature> FeatureFactory::create(int id, const std::string& type, const std::map<std::string, double>& values) {
    auto it = registry().find(type);
    if (it == registry().end()) return nullptr;
    std::vector<Parameter> params;
    for (const auto& [key, def] : it->second.defaults) {
        auto v = values.find(key);
        params.emplace_back(key, v != values.end() ? v->second : def);
    }
    return std::make_unique<ParametricFeature>(id, type, std::move(params), it->second.build);
}

std::vector<std::string> FeatureFactory::types() {
    std::vector<std::string> out;
    for (const auto& [k, _] : registry()) out.push_back(k);
    return out;
}
