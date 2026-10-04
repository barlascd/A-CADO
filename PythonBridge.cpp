#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ai/AIValidator.h"
#include "ai/CADTools.h"
#include "core/Document.h"
#include "geometry/Analysis.h"
#include "io/StepIO.h"

namespace py = pybind11;

class PyCad {
    Document doc;
    CADTools tools{doc};
public:
    bool create_box(double w, double h, double d) { return tools.createBox(w, h, d); }
    bool create_cylinder(double r, double h) { return tools.createCylinder(r, h); }
    // command: {"operation": "fillet", "radius": 2.0, ...}
    bool execute(const py::dict& command) {
        AICommand c;
        auto type = commandTypeFromString(py::str(command["operation"]));
        if (!type) throw std::invalid_argument("Unknown operation");
        c.type = *type;
        for (auto item : command) {
            std::string key = py::str(item.first);
            if (key == "operation") continue;
            if (py::isinstance<py::str>(item.second)) c.strings[key] = py::str(item.second);
            else c.numbers[key] = item.second.cast<double>();
        }
        return tools.execute(c);
    }
    std::string last_error() const { return tools.lastError(); }
    size_t feature_count() const { return const_cast<Document&>(doc).getFeatureTree().size(); }
    double volume() const { return Analysis::massProperties(doc.shape()).volume; }
    bool undo() { return doc.undo(); }
    bool redo() { return doc.redo(); }
    bool export_step(const std::string& p) const { return exportSTEP(doc.shape(), p); }
    bool export_stl(const std::string& p) const { return exportSTL(doc.shape(), p); }
};

PYBIND11_MODULE(aicado, m) {
    py::class_<PyCad>(m, "Cad")
        .def(py::init<>())
        .def("create_box", &PyCad::create_box)
        .def("create_cylinder", &PyCad::create_cylinder)
        .def("execute", &PyCad::execute)
        .def("last_error", &PyCad::last_error)
        .def("feature_count", &PyCad::feature_count)
        .def("volume", &PyCad::volume)
        .def("undo", &PyCad::undo)
        .def("redo", &PyCad::redo)
        .def("export_step", &PyCad::export_step)
        .def("export_stl", &PyCad::export_stl);
}
