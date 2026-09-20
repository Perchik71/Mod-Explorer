#include "meDMUIClient.h"
#include "../meUtils.h"
#include "../mePluginInfo.h"

#include <RE/S/Setting.h>


static dmui::localize::LocalizeString lsGeneralCategory("$GeneralCategory", "General");
static dmui::localize::LocalizeString lsCheatsCategory("$CheatsCategory", "Cheats");
static dmui::localize::LocalizeString lsGeneralPage("$GeneralPage", "General");
static dmui::localize::LocalizeString lsExplorerPage("$ExplorerPage", "Explorer");
static dmui::localize::LocalizeString lsBasketPage("$BasketPage", "Basket");
static dmui::localize::LocalizeString lsGeneralPageSummary("$GeneralPageSummary", "Number of mods, and items that can be obtained, etc.");
static dmui::localize::LocalizeString lsExplorerPageSummary("$CheatsPageSummary", "Searching for and receiving items.");
static dmui::localize::LocalizeString lsBasketPageSummary("$BasketPageSummary", "Your shopping basket.");

using namespace std::literals;

void meDMUIClient::RendererGeneralPage() noexcept
{
	dmui::ui::TextUnformatted("Hello World!");
}

void meDMUIClient::RendererExplorerPage() noexcept
{
	dmui::ui::TextUnformatted("Hello World!");
}

void meDMUIClient::RendererBasketPage() noexcept
{
	dmui::ui::TextUnformatted("Hello World!");
}

bool meDMUIClient::Connect() noexcept
{
	client = std::make_unique<dmui::Client>(
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

	std::string lfile = meUtils::GetRuntimeDirectory() + "Data/F4SE/Plugins/DearModdingUI/Translation/Mod Explorer/ModExplorer.txt";

	// Retrieve the global collection of INI settings
	auto settings = RE::INISettingCollection::GetSingleton();
	if (!settings)
	{
		REX::ERROR("RE::INISettingCollection::GetSingleton return nullptr"sv);
		return false;
	}

	// Look up the SLanguage:General setting
	// Yeah, exactly SLanguage:General this Bethesda
	auto setting = settings->GetSetting("SLanguage:General");
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
		return false;
	}

	auto localizeStringManager = dmui::localize::LocalizationManager::GetSingleton();
	localizeStringManager->Init(lfile);
	if (localizeStringManager->Exists())
		localizeStringManager->Load();

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
		std::addressof(RendererExplorerPage)) && (DMUI_RESULT_DUPLICATE_PAGE_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() Page 'Basket' registration failed, {}"sv, DMUI_ResultToString(client->LastResult()));
		return false;
	}

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host"sv, kClientId);

	return true;
}