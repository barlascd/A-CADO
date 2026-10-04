#include "io/ProjectIO.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "io/StepIO.h"

namespace ProjectIO {
bool save(const Document& doc, const QString& path, QString* error) {
    QJsonArray features;
    for (const auto& f : doc.getFeatureTree().getFeatures()) {
        QJsonObject params;
        for (const auto& p : f->getParameters()) params[QString::fromStdString(p.getName())] = p.asDouble();
        features.append(QJsonObject{{"type", QString::fromStdString(f->type())}, {"params", params}});
    }
    QJsonObject root{{"version", 1}, {"import", QString::fromStdString(doc.getImportedFile())}, {"features", features}};
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { if (error) *error = file.errorString(); return false; }
    file.write(QJsonDocument(root).toJson());
    return true;
}

bool load(Document& doc, const QString& path, QString* error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { if (error) *error = file.errorString(); return false; }
    QJsonParseError pe;
    QJsonDocument jd = QJsonDocument::fromJson(file.readAll(), &pe);
    if (!jd.isObject()) { if (error) *error = pe.errorString(); return false; }
    QJsonObject root = jd.object();
    doc.clear();
    QString imp = root["import"].toString();
    if (!imp.isEmpty()) {
        TopoDS_Shape s = importSTEP(imp.toStdString());
        if (s.IsNull()) { if (error) *error = "cannot import " + imp; return false; }
        doc.setBaseShape(s, imp.toStdString());
    }
    for (const QJsonValue& v : root["features"].toArray()) {
        QJsonObject o = v.toObject();
        std::map<std::string, double> values;
        QJsonObject params = o["params"].toObject();
        for (auto it = params.begin(); it != params.end(); ++it) values[it.key().toStdString()] = it.value().toDouble();
        std::string err;
        if (!doc.addFeature(o["type"].toString().toStdString(), values, &err)) {
            if (error) *error = QString::fromStdString(err);
            return false;
        }
    }
    doc.clearHistory();
    return true;
}
}  // namespace ProjectIO
