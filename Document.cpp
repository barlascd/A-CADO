#include "core/Document.h"
#include "core/FeatureFactory.h"
#include "core/RebuildEngine.h"

namespace {
void pushFeature(Document& d, std::unique_ptr<Feature>& f) {
    auto& tree = d.getFeatureTree();
    int id = f->getId();
    int prev = tree.empty() ? -1 : tree.getFeatures().back()->getId();
    tree.add(std::move(f));
    if (prev >= 0) d.getDependencyGraph().addDependency(id, prev);
}
void popFeature(Document& d, std::unique_ptr<Feature>& f) {
    f = d.getFeatureTree().pop();
    if (f) d.getDependencyGraph().removeFeature(f->getId());
}
struct AddFeatureCommand : Command {
    Document& d; std::unique_ptr<Feature> f;
    AddFeatureCommand(Document& d, std::unique_ptr<Feature> f) : d(d), f(std::move(f)) {}
    void execute() override { pushFeature(d, f); }
    void undo() override { popFeature(d, f); }
};
struct RemoveLastCommand : Command {
    Document& d; std::unique_ptr<Feature> f;
    explicit RemoveLastCommand(Document& d) : d(d) {}
    void execute() override { popFeature(d, f); }
    void undo() override { pushFeature(d, f); }
};
}  // namespace

bool Document::addFeature(const std::string& type, const std::map<std::string, double>& values, std::string* error) {
    auto f = FeatureFactory::create(nextId, type, values);
    if (!f) { if (error) *error = "unknown feature type: " + type; return false; }
    f->setInput(shape());
    if (!f->build()) { if (error) *error = f->getError(); return false; }
    ++nextId;
    commands.execute(std::make_unique<AddFeatureCommand>(*this, std::move(f)));
    return true;
}
bool Document::removeLastFeature() {
    if (featureTree.empty()) return false;
    commands.execute(std::make_unique<RemoveLastCommand>(*this));
    return true;
}
bool Document::undo() { return commands.undo(); }
bool Document::redo() { return commands.redo(); }
void Document::rebuild() { RebuildEngine::rebuild(featureTree, baseShape); }
TopoDS_Shape Document::shape() const {
    return featureTree.empty() ? baseShape : featureTree.getFeatures().back()->getShape();
}
void Document::clear() {
    featureTree.clear(); dependencyGraph.clear(); commands.clear();
    baseShape = TopoDS_Shape(); importedFile.clear(); nextId = 1;
}
CadObject Document::body() const {
    CadObject o(1, "Body");
    o.setShape(shape());
    return o;
}
