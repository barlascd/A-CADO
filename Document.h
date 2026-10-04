#pragma once
#include <map>
#include <string>
#include "core/CadObject.h"
#include "core/Command.h"
#include "core/DependencyGraph.h"
#include "core/FeatureTree.h"

class Document {
    FeatureTree featureTree;
    DependencyGraph dependencyGraph;
    CommandManager commands;
    TopoDS_Shape baseShape;      // e.g. an imported STEP body
    std::string importedFile;
    int nextId = 1;
public:
    FeatureTree& getFeatureTree() { return featureTree; }
    const FeatureTree& getFeatureTree() const { return featureTree; }
    DependencyGraph& getDependencyGraph() { return dependencyGraph; }

    // Builds the feature against the current shape first; only commits it if it works.
    bool addFeature(const std::string& type, const std::map<std::string, double>& values, std::string* error = nullptr);
    bool removeLastFeature();
    bool undo();
    bool redo();
    bool canUndo() const { return commands.canUndo(); }
    bool canRedo() const { return commands.canRedo(); }
    void clearHistory() { commands.clear(); }

    void rebuild();
    TopoDS_Shape shape() const;
    void clear();
    void setBaseShape(const TopoDS_Shape& s, const std::string& file) { baseShape = s; importedFile = file; }
    const std::string& getImportedFile() const { return importedFile; }
    CadObject body() const;
};
