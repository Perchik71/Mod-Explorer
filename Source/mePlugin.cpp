#include "Host/meDMUIClient.h"
#include "mePlugin.h"
#include "mePluginInfo.h"
#include "meUtils.h"

#include <RE/T/TESDataHandler.h>
#include <RE/T/TESFormUtil.h>
#include <RE/T/TESForm.h>

void mePlugin::RegisterWidgetInDMUIPlatform() noexcept
{
	if (meDMUIClient::GetSingleton()->Connect())
		state.set(true, State::kRendererInit);
	else
		state.set(true, State::kRendererFailed);
}

void mePlugin::PostInitImportantSystem() noexcept
{
	//meImgUI::GetSingleton()->Initialize();
}

void mePlugin::PostUpdateData() noexcept
{
	//RE::TESForm:()
}

void mePlugin::F4SEMessageListener(F4SE::MessagingInterface::Message* a_msg) noexcept
{
	if (a_msg && (a_msg->type == F4SE::MessagingInterface::kGameDataReady))
	{
		auto plugin = mePlugin::GetSingleton();
		if (!plugin->state.any(State::kRendererInit))
			plugin->RegisterWidgetInDMUIPlatform();
	}
}

void mePlugin::SetHandleCurrentDll(REX::W32::HMODULE a_handleCurrentDll) noexcept
{
	handleCurrentDll = a_handleCurrentDll;
}

REX::W32::HMODULE mePlugin::GetHandleCurrentDll() const noexcept
{
	return handleCurrentDll;
}

bool mePlugin::Init(const F4SE::LoadInterface* a_f4se) noexcept
{
	if (state.any(State::kInit))
		return true;

	bool initResult = true;
	try
	{
		// Init
		F4SE::Init(a_f4se);

		if (state.none(State::kPreInit))
		{
			auto game_ver = a_f4se->RuntimeVersion();
			REX::INFO("" PLUGIN_NAME " mod (ver: " VER_FILE_VERSION_STR ") Initializing..."sv);
			REX::INFO("Game version: {}.{}.{}.{}"sv, game_ver.major(), game_ver.minor(), game_ver.patch(), game_ver.build());

			// Load the Config
			const auto config = REX::TOML::SettingStore::GetSingleton();
			config->Init("Data/F4SE/Plugins/" PLUGIN_NAME ".toml", "Data/F4SE/Plugins/" PLUGIN_NAME "Custom.toml");
			config->Load();

			state.set(true, State::kPreInit);
		}

		// Listen for Messages (to Install PostInit Patches)
		auto MessagingInterface = F4SE::GetMessagingInterface();
		if (MessagingInterface->RegisterListener(F4SEMessageListener))
			REX::INFO("Started Listening for F4SE Message Callbacks."sv);

		state.set(true, State::kInit);
	}
	catch (...)
	{
		initResult = false;
	}

	return initResult;
}

bool mePlugin::PreInit(const F4SE::PreLoadInterface* a_preloadf4se) noexcept
{
	if (state.any(State::kPreInit))
		return true;

	bool initResult = true;
	try
	{
		// Preload Init
		F4SE::Init(a_preloadf4se);

		auto game_ver = a_preloadf4se->RuntimeVersion();
		REX::INFO("" PLUGIN_NAME " mod (ver: " VER_FILE_VERSION_STR ") Initializing..."sv);
		REX::INFO("Game version: {}.{}.{}.{}"sv, game_ver.major(), game_ver.minor(), game_ver.patch(), game_ver.build());

		// Load the Config
		const auto config = REX::TOML::SettingStore::GetSingleton();
		config->Init("Data/F4SE/Plugins/" PLUGIN_NAME ".toml", "Data/F4SE/Plugins/" PLUGIN_NAME "Custom.toml");
		config->Load();

		state.set(true, State::kPreInit);
	}
	catch (...)
	{
		initResult = false;
	}

	return initResult;
}

void mePlugin::Kill() noexcept
{
	if (state.any(State::kShutdown))
		return;

	state.set(true, State::kShutdown);
}