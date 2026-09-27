#include "meDMUIClient.h"
#include "../meUtils.h"
#include "../mePluginInfo.h"
#include "../meDataStorage.h"
#include "../../Resources/resource.h"

#include <RE/S/Setting.h>
#include <atomic>

// Common
static dmui::localize::LocalizeString lsGeneralCategory("$GeneralCategory", "General");
static dmui::localize::LocalizeString lsCheatsCategory("$CheatsCategory", "Cheats");
static dmui::localize::LocalizeString lsGeneralPage("$GeneralPage", "General");
static dmui::localize::LocalizeString lsExplorerPage("$ExplorerPage", "Explorer");
static dmui::localize::LocalizeString lsBasketPage("$BasketPage", "Basket");
static dmui::localize::LocalizeString lsGeneralPageSummary("$GeneralPageSummary", "Number of mods, and items that can be obtained, etc.");
static dmui::localize::LocalizeString lsExplorerPageSummary("$CheatsPageSummary", "Searching for and receiving items.");
static dmui::localize::LocalizeString lsBasketPageSummary("$BasketPageSummary", "Your shopping basket.");

// General page
static dmui::localize::LocalizeString lsGeneralPageNumPlugins("$GeneralPageNumPlugins", "Number of installed plugins (Total/Regular/Light)");
static dmui::localize::LocalizeString lsGeneralPageNumPluginsOrder("$GeneralPageNumPluginsOrder", "#");
static dmui::localize::LocalizeString lsGeneralPageNumPluginsName("$GeneralPageNumPluginsName", "Name");
static dmui::localize::LocalizeString lsGeneralPageInfoCaption("$GeneralPageInfoCaption", "Info");
static dmui::localize::LocalizeString lsGeneralPageModNoSelected("$GeneralPageModNoSelected", "Select a plugin from the list");
static dmui::localize::LocalizeString lsGeneralPageModAuthorCaption("$GeneralPageModAuthorCaption", "Author");
static dmui::localize::LocalizeString lsGeneralPageModSummaryCaption("$GeneralPageModSummaryCaption", "Summary");
static dmui::localize::LocalizeString lsGeneralPageModInfoNotSpecified("$GeneralPageModInfoNotSpecified", "Not specified");

static std::atomic_bool failedAssetsLoad{};
static std::shared_ptr<dmui::Texture> textureArmor{};
static std::shared_ptr<dmui::Texture> textureBook{};
static std::shared_ptr<dmui::Texture> textureMisc{};
static std::shared_ptr<dmui::Texture> textureWeapon{};
static std::shared_ptr<dmui::Texture> textureAmmo{};
static std::shared_ptr<dmui::Texture> textureKey{};
static std::shared_ptr<dmui::Texture> textureAlchemy{};
static std::shared_ptr<dmui::Texture> textureNote{};

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

		const DMUI_ImageDrawOptions options
		{
			DMUI_IMAGE_DRAW_OPTIONS_0_1_SIZE,
			{ a_w, a_h },
			{ 0.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f, 1.0f },
			1u,
			0u
		};

		(void)a_client->DrawImage(a_texture->GetHandle(), options);
	}
}

void meDMUIClient::RendererGeneralPage()
{
	auto dataStorage = meDataStorage::GetSingleton();
	auto uiClient = meDMUIClient::GetSingleton();
	auto dmuiPlatform = uiClient->client.get();

	DMUI_ThemeColors theme{};
	auto themeOptional = dmuiPlatform->GetThemeColors();
	if (themeOptional.has_value())
		theme = themeOptional.value();

	static int32_t selectedPluginId = -1;

	if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.general.body", 2))
	{
		dmui::ui::TableNextRow();
		dmui::ui::PushStyleColor(dmui::ui::Color::kText, { .9f, .9f, .9f, 1.f });
		
		if (dmui::ui::TableNextColumn())
		{
			{
				dmui::FontGuard font{ *dmuiPlatform, DMUI_FONT_ROLE_TITLE };
				dmui::ui::TextUnformatted(lsGeneralPageNumPlugins);
			}
			dmui::ui::TextUnformatted("("); dmui::ui::SameLine();
			dmui::ui::TextColored(theme.success, "%u", dataStorage->GetModCount()); dmui::ui::SameLine();
			dmui::ui::TextUnformatted("/"); dmui::ui::SameLine();
			dmui::ui::TextColored(theme.success, "%u", dataStorage->GetDefaultModCount()); dmui::ui::SameLine();
			dmui::ui::TextUnformatted("/"); dmui::ui::SameLine();
			dmui::ui::TextColored(theme.success, "%u", dataStorage->GetLightModCount()); dmui::ui::SameLine();
			dmui::ui::TextUnformatted(")");

			// Define table flags with vertical scrolling and borders
			dmui::ui::TableFlags flags =
				dmui::ui::TableFlags::kScrollY |
				dmui::ui::TableFlags::kRowBg |
				dmui::ui::TableFlags::kBorders |
				dmui::ui::TableFlags::kResizable |
				dmui::ui::TableFlags::kSizingFixedFit;

			dmui::ui::PushStyleVar(dmui::ui::StyleVar::kItemSpacing, dmui::ui::Vec2(0.0f, 1.0f));
			dmui::ui::PushStyleVar(dmui::ui::StyleVar::kCellPadding, dmui::ui::Vec2(8.0f, 1.0f));

			if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.general.body.plugins", 2, flags))
			{
				dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, { .0f, .0f, .0f, .0f });
				dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderActive, { .0f, .0f, .0f, .0f });

				// Freeze the first row (the header) so it stays visible while scrolling
				dmui::ui::TableSetupScrollFreeze(0, 1);

				auto sizeOrderColumn = dmui::ui::CalcTextSize("0xFFFFF");	
				
				// Setup columns the header row
				dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsOrder,
					dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
					sizeOrderColumn.x);
				dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsName, 
					dmui::ui::TableColumnFlags::kWidthStretch);
				dmui::ui::TableHeadersRow();

				dmui::ui::PopStyleColor(2);
				const auto colorSelectedRow = dmui::ui::GetStyleColor(dmui::ui::Color::kHeader);

				try
				{
					dataStorage->Lock();

					dmui::ui::ListClipper clipper;
					clipper.Begin(dataStorage->GetModCount());

					while (clipper.Step())
					{
						// Fill table with rows of data
						for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							auto plugin = dataStorage->GetModByIndex(row);
							auto is_selected = (selectedPluginId == row);

							if (is_selected)
								dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, colorSelectedRow);

							dmui::ui::TableNextRow();

							if (dmui::ui::TableNextColumn())
							{
								char label[96];
								sprintf_s(label, "##dearmodding.modexplorer.page.general.body.plugins.row.%d", row);
								if (dmui::ui::Selectable(label, is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
									selectedPluginId = row;

								dmui::ui::SameLine(.0f, .0f);

								if (is_selected)
									dmui::ui::Text("0x%X", plugin->GetIndex().value());
								else
								{
									if (plugin->GetFile()->IsLight())
										dmui::ui::TextColored(theme.statusDisable, "0x%X", plugin->GetIndex().value());
									else
										dmui::ui::TextColored(theme.accent, "0x%X", plugin->GetIndex().value());
								}
							}

							if (dmui::ui::TableNextColumn())
								dmui::ui::Text(plugin->GetFileName().c_str());

							if (is_selected)
								dmui::ui::PopStyleColor();
						}
					}
				}
				catch (...)
				{
					dataStorage->Unlock();
				}

				dmui::ui::EndTable();
			}

			dmui::ui::PopStyleVar(2);
		}
		if (dmui::ui::TableNextColumn())
		{
			try
			{
				constexpr auto colorHeader = dmui::ui::Vec4(1.f, .85f, .1f, 1.f);

				dataStorage->Lock();

				auto plugin = selectedPluginId == -1 ? nullptr :
					dataStorage->GetModByIndex(selectedPluginId).get();

				{
					dmui::FontGuard font{ *dmuiPlatform, DMUI_FONT_ROLE_TITLE };
					dmui::ui::TextUnformatted(lsGeneralPageInfoCaption);
				}

				dmui::ui::NewLine();

				if (!plugin)
				{
					dmui::ui::PushStyleColor(dmui::ui::Color::kText, theme.error);
					dmui::ui::TextWrapped(lsGeneralPageModNoSelected);
					dmui::ui::PopStyleColor();
				}
				else
				{
					auto GetStr = [&](const std::string& a_str, const char* a_default)
						{
							return (a_str.empty() || !a_str.length()) ? a_default : a_str.c_str();
						};

					dmui::ui::TextColored(colorHeader, "%s: ", lsGeneralPageModAuthorCaption.GetValue().c_str());
					dmui::ui::SameLine();
					dmui::ui::TextWrapped(GetStr(plugin->GetAuthor(), lsGeneralPageModInfoNotSpecified));
					dmui::ui::TextColored(colorHeader, "%s: ", lsGeneralPageModSummaryCaption.GetValue().c_str());
					dmui::ui::SameLine();
					dmui::ui::TextWrapped(GetStr(plugin->GetSummary(), lsGeneralPageModInfoNotSpecified));

					// Define table flags with vertical scrolling and borders
					dmui::ui::TableFlags flags =
						dmui::ui::TableFlags::kScrollY |
						dmui::ui::TableFlags::kSizingFixedFit;

					if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.general.body.plugininfo", 2, flags))
					{
						// Setup columns the header row
						dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsOrder,
							dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize, 96);
						dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsName,
							dmui::ui::TableColumnFlags::kWidthStretch);

						for (auto itemType : meItemTypes)
						{
							dmui::ui::TableNextRow();
							if (dmui::ui::TableNextColumn())
							{
								switch (itemType)
								{
								case meItemType::kArmorItem:
									dmui::Image(dmuiPlatform, textureArmor, 64.f, 64.f);
									break;
								case meItemType::kBookItem:
									dmui::Image(dmuiPlatform, textureBook, 64.f, 64.f);
									break;
								case meItemType::kMiscItem:
									dmui::Image(dmuiPlatform, textureMisc, 64.f, 64.f);
									break;
								case meItemType::kWeaponItem:
									dmui::Image(dmuiPlatform, textureWeapon, 64.f, 64.f);
									break;
								case meItemType::kAmmoItem:
									dmui::Image(dmuiPlatform, textureAmmo, 64.f, 64.f);
									break;
								case meItemType::kKeyItem:
									dmui::Image(dmuiPlatform, textureKey, 64.f, 64.f);
									break;
								case meItemType::kAlchemyItem:
									dmui::Image(dmuiPlatform, textureAlchemy, 64.f, 64.f);
									break;
								case meItemType::kNoteItem:
									dmui::Image(dmuiPlatform, textureNote, 64.f, 64.f);
									break;
								default:
									break;
								}
							}
							if (dmui::ui::TableNextColumn())
							{
								//dmui::ui::Al
								auto fontSize = dmui::ui::CalcTextSize("A");
								fontSize.y = (66.f - fontSize.y) * .5f;

								dmui::ui::PushStyleVar(dmui::ui::StyleVar::kCellPadding, { 1.f, fontSize.y });
								//ImGui::AlignTextToFramePadding();

								auto num = plugin->GetItemCount(itemType);
								if (num)
									dmui::ui::Text("%u", num);
								else
									dmui::ui::TextColored(theme.statusDisable, "-");

								dmui::ui::PopStyleVar();
							}
						}

						dmui::ui::EndTable();
					}
				}
			}
			catch (...)
			{
				dataStorage->Unlock();
			}
		}
		
		dmui::ui::PopStyleColor();
		dmui::ui::EndTable();
	}
}

void meDMUIClient::RendererExplorerPage() noexcept
{
	dmui::ui::TextUnformatted("Hello World!");
}

void meDMUIClient::RendererBasketPage() noexcept
{
	dmui::ui::TextUnformatted("Hello World!");
}

std::string meDMUIClient::GetLocalizeFileName() const noexcept
{
	std::string lfile = meUtils::GetRuntimeDirectory() + "Data/F4SE/Plugins/DearModdingUI/Translation/Mod Explorer/ModExplorer.txt";

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

	auto lfile = GetLocalizeFileName();
	if (lfile.length() > 0)
	{
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

	textureArmor	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_ARMOR,	"PNG");
	textureBook		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_BOOK,	"PNG");
	textureMisc		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_MISC,	"PNG");
	textureWeapon	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_WEAPON,	"PNG");
	textureAmmo		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_AMMO,	"PNG");
	textureKey		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_KEY,		"PNG");
	textureAlchemy	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_ALCHEMY,	"PNG");
	textureNote		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_NOTE,	"PNG");
	if (!textureArmor || !textureNote || !textureBook || !textureMisc || !textureWeapon || 
		!textureAmmo || !textureKey || !textureAlchemy)
		REX::ERROR("meDMUIClient::Connect() failed load assets"sv);

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host"sv, kClientId);

	return true;
}