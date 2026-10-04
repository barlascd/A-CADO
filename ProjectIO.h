#pragma once
#include <QString>
#include "core/Document.h"

// JSON project files: {"version":1,"import":"part.step","features":[{"type":"box","params":{...}}]}
namespace ProjectIO {
bool save(const Document& doc, const QString& path, QString* error = nullptr);
bool load(Document& doc, const QString& path, QString* error = nullptr);
}
