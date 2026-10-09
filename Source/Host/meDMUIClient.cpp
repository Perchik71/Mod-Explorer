#include "meDMUIClient.h"
#include "../meUtils.h"
#include "../mePluginInfo.h"
#include "../meDataStorage.h"
#include "../../Resources/resource.h"

#include <RE/S/Setting.h>
#include <atomic>
#include <numbers>
#include <shlwapi.h>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_SIZE

#include "meDMUIPageManager.h"
#include "meDMUIPageGeneralInfo.h"
#include "meDMUIPageExplorer.h"
#include "meHostLocalizeString.h"

static std::shared_ptr<meDMUIPageGeneralInfo> globalPageGeneralInfo{};
static std::shared_ptr<meDMUIPageExplorer> globalPageExplorer{};

using namespace std::literals;

void meDMUIClient::RendererGeneralPage()
{
	meDMUIPageManager::GetSingleton()->DrawPage(0);
}

void meDMUIClient::RendererExplorerPage() noexcept
{
	meDMUIPageManager::GetSingleton()->DrawPage(1);
}

void meDMUIClient::RendererBasketPage() noexcept
{
	dmui::ui::Text("Hello World!");
}

std::string meDMUIClient::GetLocalizeFileName() const noexcept
{
	std::string lfile = meUtils::GetRuntimeDirectory() + "Data/F4SE/Plugins/DearModdingUI/Translations/Mod Explorer/ModExplorer.txt";

	// Retrieve the global collection of INI settings
	auto settings = RE::INISettingCollection::GetSingleton();
	if (!settings)
	{
		REX::ERROR("RE::INISettingCollection::GetSingleton return nullptr"sv);
		return "";
	}

	// Look up the SLanguage:General setting
	// Yeah, exactly SLanguage:General this Bethesda
	auto setting = settings->GetSetting("SLanguage:General"); 
	if (!setting) 
		setting = settings->GetSetting("sLanguage:General");

	if (setting && (setting->GetType() == RE::Setting::SETTING_TYPE::kString))
	{
		std::string lang = setting->GetString().data();
		lang.insert(0, "_");

		auto it = lfile.find_last_of('.');
		if (it != std::string::npos)
			lfile.insert(it, lang);
		else
			lfile += lang;
	}
	else
	{
		REX::ERROR("RE::INISettingCollection::GetSetting no found \"SLanguage:General\" setting"sv);
		return "";
	}

	return lfile;
}

bool meDMUIClient::Connect() noexcept
{
	client = std::make_shared<dmui::Client>(
		kClientId,
		kClientDisplayName,
		dmui::Version{ VERSION_MAJOR, VERSION_MINOR },
		kClientIcon);
	if (!client)
	{
		REX::ERROR("meDMUIClient::Connect() Internal error"sv);
		return false;
	}
	
	if (!client->Connect())
	{
		if (!client->HostPresent())
			REX::WARN("meDMUIClient::Connect() No dmui host is loaded; no menu this session"sv);
		else
			REX::ERROR("meDMUIClient::Connect() dmui registration failed, {}"sv, DMUI_ResultToString(client->LastResult()));

		return false;
	}

	auto lfile = GetLocalizeFileName();
	if (lfile.length() > 0)
	{
		auto localizeStringManager = dmui::localize::LocalizationManager::GetSingleton();
		localizeStringManager->Init(lfile);
		if (localizeStringManager->Exists())
			localizeStringManager->Load();
	}

	if (!client->AddCategory({
			.id = kCategoryGeneralId,
			.displayName = lsGeneralCategory,
			.sortKey = 0,
			.iconName = "info",
		}) && (DMUI_RESULT_DUPLICATE_CATEGORY_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() dmui add category failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	if (!client->AddCategory({
		.id = kCategoryCheatsId,
		.displayName = lsCheatsCategory,
		.sortKey = 1,
		.iconName = "bug-beetle",
		}) && (DMUI_RESULT_DUPLICATE_CATEGORY_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() dmui add category failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	if (!client->AddPage({ 
		.id = kPageGeneralId,
		.displayName = lsGeneralPage,
		.categoryId = kCategoryGeneralId,
		.summary = lsGeneralPageSummary,
		.iconName = "gear", },
		std::addressof(RendererGeneralPage)) && (DMUI_RESULT_DUPLICATE_PAGE_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() Page 'General' registration failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	if (!client->AddPage({
		.id = kPageExplorerId,
		.displayName = lsExplorerPage,
		.categoryId = kCategoryCheatsId,
		.summary = lsExplorerPageSummary,
		.sortKey = 0,
		.iconName = "hand-deposit", },
		std::addressof(RendererExplorerPage)) && (DMUI_RESULT_DUPLICATE_PAGE_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() Page 'Explorer' registration failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	if (!client->AddPage({
		.id = kPageBasketId,
		.displayName = lsBasketPage,
		.categoryId = kCategoryCheatsId,
		.summary = lsBasketPageSummary,
		.sortKey = 1,
		.iconName = "basket", },
		std::addressof(RendererBasketPage)) && (DMUI_RESULT_DUPLICATE_PAGE_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() Page 'Basket' registration failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	globalPageGeneralInfo = std::make_shared<meDMUIPageGeneralInfo>(client);
	globalPageExplorer = std::make_shared<meDMUIPageExplorer>(client);
	meDMUIPageManager::GetSingleton()->RegisterPage(globalPageGeneralInfo);
	meDMUIPageManager::GetSingleton()->RegisterPage(globalPageExplorer);

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host"sv, kClientId);

	return true;
}

void meDMUIClient::BeginUpdate() noexcept
{
	auto managerPages = meDMUIPageManager::GetSingleton();
	while (managerPages->IsProcessingDraw()) { std::this_thread::yield(); }
	globalPageGeneralInfo->SetUpdateState(true);
	globalPageGeneralInfo->SetSelectedPluginId(-1);
	globalPageExplorer->SetUpdateState(true);
	globalPageExplorer->SetSelectedPluginId(-1);
	managerPages->SetProcessingUpdate(true);
}

void meDMUIClient::EndUpdate() noexcept
{
	auto managerPages = meDMUIPageManager::GetSingleton();
	managerPages->SetProcessingUpdate(false);
	globalPageExplorer->SetUpdateState(false);
	globalPageGeneralInfo->SetUpdateState(false);
}
