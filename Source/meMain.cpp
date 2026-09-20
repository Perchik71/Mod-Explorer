#include <F4SE/Impl/PCH.h>
#include <F4SE/F4SE.h>

#include "mePlugin.h"
#include "mePluginInfo.h"
#include "meUtils.h"

#include <windows.h>

// F4SE NP requirement
F4SE_PLUGIN_VERSION = []() noexcept
    {
        F4SE::PluginVersionData data{};
        data.PluginVersion({ VERSION_MAJOR, VERSION_MINOR, 0, VERSION_BUILD });
        data.PluginName(PLUGIN_NAME);
        data.AuthorName(PLUGIN_AUTHOR);
        data.UsesAddressLibrary(true);
        data.UsesAddressLibraryNG(true);
        data.UsesSigScanning(false);
        data.IsLayoutDependent(true);
        data.IsLayoutDependentNG(true);
        data.HasNoStructUse(false);

        return data;
    }();

// For F4SE OG
F4SE_PLUGIN_QUERY(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
    if (!a_f4se)
        return false;

    if (!a_info)
        return false;

    if (a_f4se->RuntimeVersion() < REL::Version(F4SE::RUNTIME_1_10_163))
        return false;

    if (const auto data = F4SE::PluginVersionData::GetSingleton())
    {
        a_info->infoVersion = F4SE::PluginInfo::kVersion;
        a_info->version = data->GetPluginVersion().pack();
        a_info->name = data->GetPluginName().data();
    }

    if (!std::filesystem::exists(std::format("{}Data\\F4SE\\Plugins\\version-1-10-163-0.bin", 
        meUtils::GetRuntimeDirectory())))
    {
        MessageBoxA(nullptr, "" PLUGIN_NAME ": disabled, address library needs to be updated", 
            "Warnings", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);

        return false;
    }

    return true;
}

static bool meInitUnsafe(const F4SE::LoadInterface* a_f4se)
{
    return mePlugin::GetSingleton()->Init(a_f4se);
}

static bool meInitSafe(const F4SE::LoadInterface* a_f4se)
{
    __try
    {
        return meInitUnsafe(a_f4se);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool mePreloadInitUnsafe(const F4SE::PreLoadInterface* a_preloadf4se)
{
    // run patches after LoadLibrary
    return mePlugin::GetSingleton()->PreInit(a_preloadf4se);
}

static bool mePreloadInitSafe(const F4SE::PreLoadInterface* a_preloadf4se)
{
    __try
    {
        return mePreloadInitUnsafe(a_preloadf4se);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

#define meInit meInitSafe
#define mePreloadInit mePreloadInitSafe

// No supported OG
F4SE_PLUGIN_PRELOAD(const F4SE::PreLoadInterface* a_preloadf4se)
{
    return mePreloadInit(a_preloadf4se);
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
    return meInit(a_f4se);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, [[maybe_unused]] LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        mePlugin::GetSingleton()->SetHandleCurrentDll(reinterpret_cast<REX::W32::HMODULE>(hModule));
        break;
    case DLL_PROCESS_DETACH:
        mePlugin::GetSingleton()->Kill();
        break;
    }
    return TRUE;
}
