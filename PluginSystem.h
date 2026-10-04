#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Plugin {
public:
    virtual ~Plugin() = default;
    virtual std::string name() const = 0;
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
};

class PluginManager {
    std::vector<std::unique_ptr<Plugin>> plugins_;
public:
    static PluginManager& instance() { static PluginManager m; return m; }
    void registerPlugin(std::unique_ptr<Plugin> p) { plugins_.push_back(std::move(p)); }
    void initializeAll() { for (auto& p : plugins_) p->initialize(); }
    void shutdownAll() { for (auto it = plugins_.rbegin(); it != plugins_.rend(); ++it) (*it)->shutdown(); }
    std::vector<std::string> names() const { std::vector<std::string> n; for (auto& p : plugins_) n.push_back(p->name()); return n; }
};

// Example plugin; copy it as a template for your own.
class BuiltinInfoPlugin : public Plugin {
public:
    std::string name() const override { return "BuiltinInfo"; }
    void initialize() override { std::cout << "[plugin] " << name() << " initialized\n"; }
    void shutdown() override { std::cout << "[plugin] " << name() << " shut down\n"; }
};
