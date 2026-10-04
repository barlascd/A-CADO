#include "core/Assembly.h"
#include <BRep_Builder.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include "geometry/Analysis.h"

namespace {
TopoDS_Shape placed(const Component& c) { return c.shape.Moved(TopLoc_Location(c.transform)); }
}

bool Assembly::solveMates() {
    bool all = true;
    for (const Mate& m : mates_) {
        Component *a = nullptr, *b = nullptr;
        for (auto& c : components_) { if (c.id == m.componentA) a = &c; if (c.id == m.componentB) b = &c; }
        if (!a || !b) { all = false; continue; }
        gp_Pnt ca = Analysis::massProperties(placed(*a)).centre;
        gp_Pnt cb = Analysis::massProperties(placed(*b)).centre;
        gp_Vec delta;
        if (m.type == MateType::Coincident) delta = gp_Vec(cb, ca);
        else if (m.type == MateType::Distance) delta = gp_Vec(cb, gp_Pnt(ca.X() + m.value, ca.Y(), ca.Z()));
        else { all = false; continue; }
        gp_Trsf t; t.SetTranslation(delta);
        b->transform = t.Multiplied(b->transform);
    }
    return all;
}

TopoDS_Compound Assembly::compound() const {
    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);
    for (const auto& c : components_) builder.Add(comp, placed(c));
    return comp;
}
