#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QUrl>
#include <cstring>
#include <iostream>
#include "core/Document.h"
#include "geometry/Analysis.h"
#include "geometry/GeometryEngine.h"
#include "plugins/PluginSystem.h"

// Headless smoke test: `AICADO --selftest`
static int selfTest() {
    std::cout << "AICADO starting ...\n";
    TopoDS_Shape box = GeometryEngine::createBox(100, 50, 20);
    if (box.IsNull()) { std::cerr << "Failed to create box\n"; return 1; }
    std::cout << "Box created successfully\n";

    Document doc;
    std::string err;
    if (!doc.addFeature("box", {{"width", 100}, {"height", 50}, {"depth", 20}}, &err)) { std::cerr << err << "\n"; return 1; }
    if (!doc.addFeature("fillet", {{"radius", 3}}, &err)) { std::cerr << "fillet: " << err << "\n"; return 1; }
    if (!doc.addFeature("hole", {{"x", 50}, {"y", 25}, {"diameter", 10}, {"throughAll", 1}}, &err)) { std::cerr << "hole: " << err << "\n"; return 1; }
    double v = Analysis::massProperties(doc.shape()).volume;
    std::cout << "Volume after fillet+hole: " << v << "\n";
    if (!(v > 0 && v < 100.0 * 50 * 20)) return 1;
    if (!doc.undo() || !doc.redo()) return 1;
    std::cout << "Self test passed\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::strcmp(argv[1], "--selftest") == 0) return selfTest();

    QGuiApplication app(argc, argv);
    app.setApplicationName("AICADO");
    PluginManager::instance().registerPlugin(std::make_unique<BuiltinInfoPlugin>());
    PluginManager::instance().initializeAll();

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    engine.loadFromModule("AICADO", "Main");
#else
    engine.load(QUrl(QStringLiteral("qrc:/AICADO/Main.qml")));
#endif
    if (engine.rootObjects().isEmpty()) return 1;
    int rc = app.exec();
    PluginManager::instance().shutdownAll();
    return rc;
}
