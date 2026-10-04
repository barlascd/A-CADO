#pragma once
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>
#include "core/Document.h"
#include "geometry/MeshBuilder.h"

class CadController : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString info READ info NOTIFY modelChanged)
    Q_PROPERTY(QStringList features READ features NOTIFY modelChanged)
    Q_PROPERTY(int selectedFace READ selectedFace WRITE setSelectedFace NOTIFY selectionChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY modelChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY modelChanged)
public:
    explicit CadController(QObject* parent = nullptr);

    Q_INVOKABLE void createBox(double width, double height, double depth);
    Q_INVOKABLE void createCylinder(double radius, double height);
    // Generic entry point: type is a FeatureFactory name ("fillet", "hole", "extrude", ...).
    Q_INVOKABLE void addFeature(const QString& type, const QVariantMap& params);
    Q_INVOKABLE void runCommand(const QString& json);   // {"operation":"create_box","width":10,...}
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void deleteLast();
    Q_INVOKABLE void newProject();
    Q_INVOKABLE void save(const QUrl& file);
    Q_INVOKABLE void open(const QUrl& file);
    Q_INVOKABLE void openAutosave();
    Q_INVOKABLE void importStep(const QUrl& file);
    Q_INVOKABLE void exportStep(const QUrl& file);
    Q_INVOKABLE void exportStl(const QUrl& file);
    Q_INVOKABLE void clearSelection() { setSelectedFace(-1); }

    QString status() const { return status_; }
    QString info() const { return info_; }
    QStringList features() const { return features_; }
    int selectedFace() const { return selectedFace_; }
    void setSelectedFace(int f);
    bool canUndo() const { return doc_.canUndo(); }
    bool canRedo() const { return doc_.canRedo(); }
    const MeshData& mesh() const { return mesh_; }

signals:
    void statusChanged();
    void modelChanged();
    void selectionChanged();

private:
    void refresh();
    void setStatus(const QString& s);
    static QString localPath(const QUrl& u);

    Document doc_;
    MeshData mesh_;
    QStringList features_;
    QString status_, info_;
    int selectedFace_ = -1;
    QTimer autosave_;
};
