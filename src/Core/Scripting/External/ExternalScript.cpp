#include "ExternalScript.h"
#include "../../Logging/Logger.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace GameEngine {

ExternalScript::ExternalScript(const std::string& name, void* libraryHandle)
    : m_name(name), m_libraryHandle(libraryHandle), m_scriptInstance(nullptr),
      m_createFunc(nullptr), m_destroyFunc(nullptr) {
}

ExternalScript::~ExternalScript() {
    if (m_scriptInstance && m_destroyFunc) {
        m_destroyFunc(m_scriptInstance);
        m_scriptInstance = nullptr;
    }
    
    if (m_libraryHandle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(m_libraryHandle));
#else
        dlclose(m_libraryHandle);
#endif
        m_libraryHandle = nullptr;
    }
}

bool ExternalScript::Initialize() {
    if (!m_libraryHandle) {
        Logger::Error("No library handle for script: " + m_name);
        return false;
    }
    
#ifdef _WIN32
    m_createFunc = reinterpret_cast<CreateScriptFunc>(
        GetProcAddress(static_cast<HMODULE>(m_libraryHandle), "CreateScript"));
    m_destroyFunc = reinterpret_cast<DestroyScriptFunc>(
        GetProcAddress(static_cast<HMODULE>(m_libraryHandle), "DestroyScript"));
#else
    m_createFunc = reinterpret_cast<CreateScriptFunc>(
        dlsym(m_libraryHandle, "CreateScript"));
    m_destroyFunc = reinterpret_cast<DestroyScriptFunc>(
        dlsym(m_libraryHandle, "DestroyScript"));
#endif
    
    if (!m_createFunc || !m_destroyFunc) {
        Logger::Error("Failed to find required functions in script: " + m_name);
        return false;
    }
    
    m_scriptInstance = m_createFunc();
    if (!m_scriptInstance) {
        Logger::Error("Failed to create script instance: " + m_name);
        return false;
    }
    
    Logger::Info("Successfully initialized script: " + m_name);
    return true;
}

void ExternalScript::ExecuteStart(World* world, Entity entity) {
    if (m_scriptInstance) {
        try {
            m_scriptInstance->OnStart(world, entity);
        } catch (const std::exception& e) {
            Logger::Error("Exception in script OnStart (" + m_name + "): " + e.what());
        }
    }
}

void ExternalScript::ExecuteUpdate(World* world, Entity entity, float deltaTime) {
    if (m_scriptInstance) {
        try {
            m_scriptInstance->OnUpdate(world, entity, deltaTime);
        } catch (const std::exception& e) {
            Logger::Error("Exception in script OnUpdate (" + m_name + "): " + e.what());
        }
    }
}

void ExternalScript::ExecuteDestroy(World* world, Entity entity) {
    if (m_scriptInstance) {
        try {
            m_scriptInstance->OnDestroy(world, entity);
        } catch (const std::exception& e) {
            Logger::Error("Exception in script OnDestroy (" + m_name + "): " + e.what());
        }
    }
}

}
