#include <iostream>
#include <filesystem>
#include <fstream>
#include "Core/Project/ProjectManager.h"
#include "Core/Scripting/External/ExternalScriptManager.h"
#include "Core/Logging/Logger.h"

using namespace GameEngine;

int main() {
    Logger::Info("Starting project I/O demo test");

    const std::string tmpDir = "./_tmp_project_io_demo";
    try {
        if (std::filesystem::exists(tmpDir)) {
            std::filesystem::remove_all(tmpDir);
        }
        std::filesystem::create_directories(tmpDir);
    } catch (const std::exception& e) {
        std::cerr << "Failed to prepare temp directory: " << e.what() << std::endl;
        return 1;
    }

    auto& pm = ProjectManager::Instance();
    if (!pm.CreateProject(tmpDir, "MockProject")) {
        std::cerr << "CreateProject failed" << std::endl;
        return 1;
    }

    pm.AddScene("Assets/Scenes/main.scene");
    if (!pm.SaveProject()) {
        std::cerr << "SaveProject failed" << std::endl;
        return 1;
    }
    pm.CloseProject();

    if (!pm.LoadProject(tmpDir)) {
        std::cerr << "LoadProject failed" << std::endl;
        return 1;
    }

    const std::string scriptsDir = pm.GetAssetsDirectory() + "/Scripts";
    auto& esm = ExternalScriptManager::Instance();
    if (!esm.Initialize(scriptsDir)) {
        std::cerr << "ExternalScriptManager Initialize failed" << std::endl;
        return 1;
    }

#ifndef _WIN32
    {
        const std::string testScript = scriptsDir + "/DemoPrint.cpp";
        try {
            std::filesystem::create_directories(scriptsDir);
            std::ofstream out(testScript);
            out <<
R"(#include "Core/Scripting/External/ExternalScript.h"
using namespace GameEngine;
class Demo : public IExternalScript {
public:
    void OnStart(World*, Entity) override {}
    void OnUpdate(World*, Entity, float) override {}
    void OnDestroy(World*, Entity) override {}
};
extern "C" IExternalScript* CreateScript() { return new Demo(); }
extern "C" void DestroyScript(IExternalScript* s) { delete s; })";
            out.close();
        } catch (const std::exception& e) {
            Logger::Warning(std::string("Failed to write test script: ") + e.what());
        }

        if (!esm.CompileScript(testScript)) {
            Logger::Warning("CompileScript failed (this is optional in demo)");
        } else {
            if (!esm.LoadCompiledScript("DemoPrint")) {
                Logger::Warning("LoadCompiledScript failed (optional)");
            }
        }
    }
#endif

    const std::string settingsPath = tmpDir + "/project.json";
    if (!std::filesystem::exists(settingsPath)) {
        std::cerr << "project.json not found after save/load roundtrip" << std::endl;
        return 1;
    }

    Logger::Info("Project I/O demo test finished OK");
    return 0;
}
