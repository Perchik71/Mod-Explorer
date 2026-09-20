#include "meDMUIClient.h"
#include "../mePluginInfo.h"

static dmui::localize::LocalizeString lsGeneralCategory("$GeneralCategory", "General");
static dmui::localize::LocalizeString lsCheatsCategory("$CheatsCategory", "Cheats");
static dmui::localize::LocalizeString lsGeneralPage("$GeneralPage", "General");
static dmui::localize::LocalizeString lsExplorerPage("$ExplorerPage", "Explorer");
static dmui::localize::LocalizeString lsBasketPage("$BasketPage", "Basket");
static dmui::localize::LocalizeString lsGeneralPageSummary("$GeneralPageSummary", "Number of mods, and items that can be obtained, etc.");
static dmui::localize::LocalizeString lsExplorerPageSummary("$CheatsPageSummary", "Searching for and receiving items.");
static dmui::localize::LocalizeString lsBasketPageSummary("$BasketPageSummary", "Your shopping basket.");

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
		REX::ERROR("meDMUIClient::Connect() Internal error");
		return false;
	}
	
	if (!client->Connect())
	{
		if (!client->HostPresent())
			REX::WARN("meDMUIClient::Connect() No dmui host is loaded; no menu this session");
		else
			REX::ERROR("meDMUIClient::Connect() dmui registration failed, {}", DMUI_ResultToString(client->LastResult()));

		return false;
	}

	auto localizeStringManager = dmui::localize::LocalizationManager::GetSingleton();
	localizeStringManager->Init("Data/F4SE/Plugins/DearModdingUI/Translation/Mod Explorer/ModExplorer.txt", true);
	if (localizeStringManager->Exists())
		localizeStringManager->Load();

	if (!client->AddCategory({ 
		.id = kCategoryGeneralId,
		.displayName = lsGeneralCategory,
		.sortKey = 0,
		.iconName = "info",
		}) && (DMUI_RESULT_DUPLICATE_CATEGORY_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() dmui add category failed, {}", DMUI_ResultToString(client->LastResult()));
		return false;
	}

	if (!client->AddCategory({
		.id = kCategoryCheatsId,
		.displayName = lsCheatsCategory,
		.sortKey = 1,
		.iconName = "bug-beetle",
		}) && (DMUI_RESULT_DUPLICATE_CATEGORY_ID != client->LastResult()))
	{
		REX::ERROR("meDMUIClient::Connect() dmui add category failed, {}", DMUI_ResultToString(client->LastResult()));
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
		REX::ERROR("meDMUIClient::Connect() Page 'General' registration failed, {}", DMUI_ResultToString(client->LastResult()));
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
		REX::ERROR("meDMUIClient::Connect() Page 'Explorer' registration failed, {}", DMUI_ResultToString(client->LastResult()));
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
		REX::ERROR("meDMUIClient::Connect() Page 'Basket' registration failed, {}", DMUI_ResultToString(client->LastResult()));
		return false;
	}

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host", kClientId);

	return true;
}