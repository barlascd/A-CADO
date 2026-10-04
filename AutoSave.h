#pragma once
#include <QDir>
#include <QStandardPaths>
#include "io/ProjectIO.h"

class AutoSave {
public:
    static QString defaultPath() {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dir);
        return dir + "/project.autosave";
    }
    static bool save(const Document& document, const QString& path = defaultPath()) {
        return ProjectIO::save(document, path);
    }
};
