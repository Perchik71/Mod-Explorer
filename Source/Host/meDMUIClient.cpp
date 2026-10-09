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


// Other
static std::atomic_bool no_once_load_items = true;
static std::atomic_bool updateFrame = false;
static std::atomic_bool updatePlugins = false;
static std::atomic_bool updateSearchPlugins = false;
static std::atomic_bool done = true;
static std::atomic_bool terminated = false;
static std::atomic_bool needItemsUpdate = false;
static std::atomic_int32_t selectedShopPluginId = -1;
static std::atomic_int8_t selectedShopTypeId = -1;

static meModSortedList pluginSearchShopList{};

static std::array<char, 128> textSearchItem{};
static std::vector<int32_t> SearchPlugins{};
static std::vector<int32_t> SearchItems{};

using namespace std::literals;

namespace dmui
{
	static void Image(Client* a_client, const std::shared_ptr<dmui::Texture> a_texture, float a_w, float a_h) noexcept
	{
		if (!a_client || !a_texture)
			return;

		a_texture->SendOnceDmuiRequest(a_client);

		if (a_w == -1.f)
			a_w = static_cast<float>(a_texture->GetWidth());

		if (a_h == -1.f)
			a_h = static_cast<float>(a_texture->GetHeight());

		(void)dmui::ui::Image(a_texture->GetHandle(), { a_w, a_h });
	}

	static double GetTime() noexcept
	{
		// Get the duration elapsed since the clock's epoch
		auto duration_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
		// Convert that duration to a double in seconds
		std::chrono::duration<double> seconds_duration = duration_since_epoch;
		// Extract the double value
		return seconds_duration.count();
	}

	namespace spinner
	{
		static void SpinnerFadeBars(float w, const dmui::ui::Vec4& color = { 1.f, 1.f, 1.f, 1.f },
			float speed = 2.8f, size_t bars = 3, bool scale = false) noexcept
		{
			const float radius = (w * 0.5f) * bars;
			dmui::ui::Vec2 pos = dmui::ui::GetCursorScreenPos(), size{radius * 2, radius * 2}, centre{radius, radius};

			const float nextItemKoeff = 1.5f;
			const float yOffsetKoeftt = 0.8f;
			const float heightSpeed = 0.8f;
			const float start = static_cast<float>(GetTime()) * speed;
			const float offset = static_cast<float>(std::numbers::pi) / bars;

			for (size_t i = 0; i < bars; i++)
			{
				float a = start + (static_cast<float>(std::numbers::pi) - i * offset);
				dmui::ui::Vec4 c = { color.x, color.y, color.z, std::max(0.1f, std::sinf(a * heightSpeed)) };
				float h = (scale ? (0.6f + 0.4f * c.w) : 1.f) * size.y * .5f;

				dmui::ui::WindowDrawList().AddRectFilled(
					{ pos.x + 2.f + i * (w * nextItemKoeff) - w * .5f, pos.y + (centre.y - h * yOffsetKoeftt) },
					{ pos.x + 2.f + i * (w * nextItemKoeff) + w * .5f, pos.y + (centre.y + h * yOffsetKoeftt) },
					dmui::ui::ColorConvertFloat4ToU32(c));
			}
		}
	}
}

namespace UITools
{
	static void SpinnerThink(dmui::Client* a_client, const char* a_label)
	{
		// Bullshit wrapper for imgui dimmy window
		if (dmui::ui::BeginTable(a_label, 1))
		{
			dmui::ui::TableNextRow();

			constexpr auto w = 40.f;
			auto wndRect = dmui::ui::GetContentRegionAvail();
			auto posScreen = dmui::ui::GetCursorScreenPos();
			dmui::ui::SetCursorScreenPos({ posScreen.x + (wndRect.x - w * 2) * .5f, posScreen.y + (wndRect.y - w * 2) * .5f });
			
			DMUI_Vec4 color = { 1.f, 1.f, 1.f, 1.f };
			DMUI_ThemeColors theme{};
			auto themeOptional = a_client->GetThemeColors();
			if (themeOptional.has_value())
				color = themeOptional->info;
				
			dmui::spinner::SpinnerFadeBars(w, color, 5.6f, 4, true);
			dmui::ui::EndTable();
		}
	}
}

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
	updateFrame.store(true);


	dmui::ui::Text("Hello World!");

	updateFrame.store(false);
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

meDMUIClient::~meDMUIClient()
{
	terminated.store(true);
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

	//std::thread([]() {
	//	REX::FTimer timer;
	//	auto dataStorage = meDataStorage::GetSingleton();
	//	while (!terminated.load())
	//	{
	//		// don't load the process too much
	//		std::this_thread::sleep_for(50ms);

	//		if (!done.load())
	//		{
	//			if (needItemsUpdate.load())
	//			{
	//				meDataStorageAutoLock guard(dataStorage);
	//				timer.Start();

	//				try
	//				{
	//					auto typeId = selectedShopTypeId.load();
	//					auto pluginId = selectedShopPluginId.load();
	//					if (pluginId == -1)
	//					{
	//						switch (typeId)
	//						{
	//						case 0:
	//							dataStorage->GetAllItems(meItemType::kArmorItem, itemShopList, false);
	//							break;
	//						case 1:
	//							dataStorage->GetAllItems(meItemType::kBookItem, itemShopList, false);
	//							break;
	//						case 2:
	//							dataStorage->GetAllItems(meItemType::kMiscItem, itemShopList, false);
	//							break;
	//						case 3:
	//							dataStorage->GetAllItems(meItemType::kWeaponItem, itemShopList, false);
	//							break;
	//						case 4:
	//							dataStorage->GetAllItems(meItemType::kAmmoItem, itemShopList, false);
	//							break;
	//						case 5:
	//							dataStorage->GetAllItems(meItemType::kKeyItem, itemShopList, false);
	//							break;
	//						case 6:
	//							dataStorage->GetAllItems(meItemType::kAlchemyItem, itemShopList, false);
	//							break;
	//						case 7:
	//							dataStorage->GetAllItems(meItemType::kNoteItem, itemShopList, false);
	//							break;
	//						default:
	//							dataStorage->GetAllItems(meItemType::kMax, itemShopList, false);
	//							break;
	//						}
	//					}
	//					else
	//					{
	//						auto plugin = dataStorage->GetModForShopByIndex(pluginId);
	//						if (plugin)
	//						{
	//							switch (typeId)
	//							{
	//							case 0:
	//								plugin->GetAllItems(meItemType::kArmorItem, itemShopList, false);
	//								break;
	//							case 1:
	//								plugin->GetAllItems(meItemType::kBookItem, itemShopList, false);
	//								break;
	//							case 2:
	//								plugin->GetAllItems(meItemType::kMiscItem, itemShopList, false);
	//								break;
	//							case 3:
	//								plugin->GetAllItems(meItemType::kWeaponItem, itemShopList, false);
	//								break;
	//							case 4:
	//								plugin->GetAllItems(meItemType::kAmmoItem, itemShopList, false);
	//								break;
	//							case 5:
	//								plugin->GetAllItems(meItemType::kKeyItem, itemShopList, false);
	//								break;
	//							case 6:
	//								plugin->GetAllItems(meItemType::kAlchemyItem, itemShopList, false);
	//								break;
	//							case 7:
	//								plugin->GetAllItems(meItemType::kNoteItem, itemShopList, false);
	//								break;
	//							default:
	//								plugin->GetAllItems(meItemType::kMax, itemShopList, false);
	//								break;
	//							}
	//						}
	//					}
	//				}
	//				catch (...)
	//				{}

	//				timer.Stop();

	//				// protection against epileptics
	//				auto duration = timer.GetDuration<std::chrono::milliseconds>();
	//				if (duration < 750)
	//					std::this_thread::sleep_for((750 - duration) * 1ms);

	//				needItemsUpdate.store(false);
	//				done.store(true);
	//			}
	//			if (updateSearchPlugins.load())
	//			{
	
	//			}
	//		}
	//	}
	//	}).detach();

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
	pluginSearchShopList.clear();
	auto managerPages = meDMUIPageManager::GetSingleton();
	managerPages->SetProcessingUpdate(false);
	globalPageExplorer->SetUpdateState(false);
	globalPageGeneralInfo->SetUpdateState(false);
}
