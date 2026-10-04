#include "app/CadController.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <map>
#include "ai/AIValidator.h"
#include "ai/CADTools.h"
#include "geometry/Analysis.h"
#include "io/AutoSave.h"
#include "io/ProjectIO.h"
#include "io/StepIO.h"

CadController::CadController(QObject* parent) : QObject(parent) {
    autosave_.setSingleShot(true);
    autosave_.setInterval(5000);
    connect(&autosave_, &QTimer::timeout, this, [this] { AutoSave::save(doc_); });
    refresh();
    setStatus(tr("Ready - create a box or a cylinder to start"));
}

QString CadController::localPath(const QUrl& u) { return u.isLocalFile() ? u.toLocalFile() : u.toString(); }
void CadController::setStatus(const QString& s) { status_ = s; emit statusChanged(); }
void CadController::setSelectedFace(int f) {
    if (f == selectedFace_) return;
    selectedFace_ = f;
    emit selectionChanged();
    if (f > 0) setStatus(tr("Face %1 selected - fillet/chamfer will use its edges").arg(f));
}

void CadController::refresh() {
    TopoDS_Shape shape = doc_.shape();
    mesh_ = MeshBuilder::build(shape);
    selectedFace_ = -1;
    features_.clear();
    int n = 1;
    for (const auto& f : doc_.getFeatureTree().getFeatures()) {
        QStringList ps;
        for (const auto& p : f->getParameters()) ps << QString("%1=%2").arg(QString::fromStdString(p.getName())).arg(p.asDouble(), 0, 'g', 5);
        features_ << QString("%1. %2").arg(n++).arg(QString::fromStdString(f->type())) + "  (" + ps.join(", ") + ")";
    }
    if (shape.IsNull()) info_ = tr("No geometry");
    else {
        try {
            auto b = Analysis::boundingBox(shape);
            auto m = Analysis::massProperties(shape);
            info_ = tr("Volume %1 | Area %2 | Size %3 x %4 x %5 | %6 triangles")
                        .arg(m.volume, 0, 'f', 1).arg(m.area, 0, 'f', 1)
                        .arg(b.dx(), 0, 'f', 1).arg(b.dy(), 0, 'f', 1).arg(b.dz(), 0, 'f', 1).arg(mesh_.triangleCount());
        } catch (...) { info_ = tr("Analysis failed"); }
    }
    emit modelChanged();
    emit selectionChanged();
    if (doc_.getFeatureTree().size() > 0) autosave_.start();
}

void CadController::createBox(double w, double h, double d) { addFeature("box", {{"width", w}, {"height", h}, {"depth", d}}); }
void CadController::createCylinder(double r, double h) { addFeature("cylinder", {{"radius", r}, {"height", h}}); }

void CadController::addFeature(const QString& type, const QVariantMap& params) {
    std::map<std::string, double> v;
    for (auto it = params.begin(); it != params.end(); ++it) v[it.key().toStdString()] = it.value().toDouble();
    if ((type == "fillet" || type == "chamfer") && !v.count("face")) v["face"] = selectedFace_;
    std::string err;
    if (doc_.addFeature(type.toStdString(), v, &err)) { refresh(); setStatus(tr("Added %1").arg(type)); }
    else setStatus(tr("%1 failed: %2").arg(type, QString::fromStdString(err)));
}

void CadController::runCommand(const QString& json) {
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(json.toUtf8(), &pe);
    if (!jd.isObject()) { setStatus(tr("Invalid JSON: %1").arg(pe.errorString())); return; }
    QJsonObject o = jd.object();
    auto type = commandTypeFromString(o["operation"].toString().toStdString());
    if (!type) { setStatus(tr("Unknown operation")); return; }
    AICommand c;
    c.type = *type;
    for (auto it = o.begin(); it != o.end(); ++it) {
        if (it.key() == "operation") continue;
        if (it.value().isString()) c.strings[it.key().toStdString()] = it.value().toString().toStdString();
        else c.numbers[it.key().toStdString()] = it.value().toDouble();
    }
    CadContext ctx;
    ctx.selectedFace = selectedFace_;
    CADTools tools(doc_);
    if (tools.execute(c, ctx)) { refresh(); setStatus(tr("Command executed")); }
    else setStatus(tr("Command failed: %1").arg(QString::fromStdString(tools.lastError())));
}

void CadController::undo() { if (doc_.undo()) { refresh(); setStatus(tr("Undo")); } }
void CadController::redo() { if (doc_.redo()) { refresh(); setStatus(tr("Redo")); } }
void CadController::deleteLast() { if (doc_.removeLastFeature()) { refresh(); setStatus(tr("Last feature deleted (Undo restores it)")); } }
void CadController::newProject() { doc_.clear(); refresh(); setStatus(tr("New project")); }

void CadController::save(const QUrl& file) {
    QString err;
    setStatus(ProjectIO::save(doc_, localPath(file), &err) ? tr("Saved %1").arg(localPath(file)) : tr("Save failed: %1").arg(err));
}
void CadController::open(const QUrl& file) {
    QString err;
    bool ok = ProjectIO::load(doc_, localPath(file), &err);
    refresh();
    setStatus(ok ? tr("Opened %1").arg(localPath(file)) : tr("Open failed: %1").arg(err));
}
void CadController::openAutosave() { open(QUrl::fromLocalFile(AutoSave::defaultPath())); }

void CadController::importStep(const QUrl& file) {
    TopoDS_Shape s = importSTEP(localPath(file).toStdString());
    if (s.IsNull()) { setStatus(tr("STEP import failed")); return; }
    doc_.clear();
    doc_.setBaseShape(s, localPath(file).toStdString());
    refresh();
    setStatus(tr("Imported %1").arg(localPath(file)));
}
void CadController::exportStep(const QUrl& file) {
    setStatus(exportSTEP(doc_.shape(), localPath(file).toStdString()) ? tr("STEP exported") : tr("STEP export failed"));
}
void CadController::exportStl(const QUrl& file) {
    setStatus(exportSTL(doc_.shape(), localPath(file).toStdString()) ? tr("STL exported") : tr("STL export failed"));
}
