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

// Common
static dmui::localize::LocalizeString lsGeneralCategory("$GeneralCategory", "General");
static dmui::localize::LocalizeString lsCheatsCategory("$CheatsCategory", "Cheats");
static dmui::localize::LocalizeString lsGeneralPage("$GeneralPage", "General");
static dmui::localize::LocalizeString lsExplorerPage("$ExplorerPage", "Explorer");
static dmui::localize::LocalizeString lsBasketPage("$BasketPage", "Basket");
static dmui::localize::LocalizeString lsGeneralPageSummary("$GeneralPageSummary", "Number of mods, and items that can be obtained, etc.");
static dmui::localize::LocalizeString lsExplorerPageSummary("$CheatsPageSummary", "Searching for and receiving items.");
static dmui::localize::LocalizeString lsBasketPageSummary("$BasketPageSummary", "Your shopping basket.");
static dmui::localize::LocalizeString lsAll("$All", "All");
static dmui::localize::LocalizeString lsArmor("$Armor", "Armor");
static dmui::localize::LocalizeString lsBook("$Book", "Book");
static dmui::localize::LocalizeString lsMisc("$Misc", "Misc");
static dmui::localize::LocalizeString lsWeapon("$Weapon", "Weapon");
static dmui::localize::LocalizeString lsAmmo("$Ammo", "Ammo");
static dmui::localize::LocalizeString lsKey("$Key", "Key");
static dmui::localize::LocalizeString lsAlchemy("$Alchemy", "Alchemy");
static dmui::localize::LocalizeString lsNote("$Note", "Note");
static dmui::localize::LocalizeString lsType("$Type", "Type");
static dmui::localize::LocalizeString lsFullname("$Fullname", "Full name");
static dmui::localize::LocalizeString lsSearch("$Search", "Search");

// General page
static dmui::localize::LocalizeString lsGeneralPageNumPlugins("$GeneralPageNumPlugins", "Installed plugins");
static dmui::localize::LocalizeString lsGeneralPageNumPluginsOrder("$GeneralPageNumPluginsOrder", "#");
static dmui::localize::LocalizeString lsGeneralPageNumPluginsName("$GeneralPageNumPluginsName", "Name");
static dmui::localize::LocalizeString lsGeneralPageInfoCaption("$GeneralPageInfoCaption", "Info");
static dmui::localize::LocalizeString lsGeneralPageModNoSelected("$GeneralPageModNoSelected", "Select a plugin from the list");
static dmui::localize::LocalizeString lsGeneralPageModAuthorCaption("$GeneralPageModAuthorCaption", "Author");
static dmui::localize::LocalizeString lsGeneralPageModSummaryCaption("$GeneralPageModSummaryCaption", "Summary");
static dmui::localize::LocalizeString lsGeneralPageModInfoNotSpecified("$GeneralPageModInfoNotSpecified", "Not specified");
static dmui::localize::LocalizeString lsGeneralPageModTotalCaption("$GeneralPageModTotalCaption", "Total");
static dmui::localize::LocalizeString lsGeneralPageModRegularCaption("$GeneralPageModRegularCaption", "Regular");
static dmui::localize::LocalizeString lsGeneralPageModLightCaption("$GeneralPageModLightCaption", "Light");

// Explorer page
static dmui::localize::LocalizeString lsExplorerPageBuyAll("$ExplorerPageBuyAll", "Take All");
static dmui::localize::LocalizeString lsExplorerPageBuy("$ExplorerPageBuy", "Take");
static dmui::localize::LocalizeString lsExplorerPageShowModel("$ExplorerPageShowModel", "Show model");
static dmui::localize::LocalizeString lsExplorerPageCount("$ExplorerPageCount", "Count");

// Other
static std::shared_ptr<dmui::Texture> textureArmor{};
static std::shared_ptr<dmui::Texture> textureBook{};
static std::shared_ptr<dmui::Texture> textureMisc{};
static std::shared_ptr<dmui::Texture> textureWeapon{};
static std::shared_ptr<dmui::Texture> textureAmmo{};
static std::shared_ptr<dmui::Texture> textureKey{};
static std::shared_ptr<dmui::Texture> textureAlchemy{};
static std::shared_ptr<dmui::Texture> textureNote{};
static std::shared_ptr<dmui::Texture> textureStar{};

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

static meItemList itemShopList;
static std::array<char, 128> textExplorerSearchPlugin{};
static std::array<char, 128> textExplorerSearchPluginDone{};

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

namespace PageHelper
{
	static void DrawPluginTableForShop(dmui::Client* a_client, int32_t& selectPluginIdList, const meModSortedList* a_list = nullptr)
	{
		DMUI_Vec4 color = { 1.f, 1.f, 1.f, 1.f };
		DMUI_Vec4 colorDisable = { .5f, .5f, .5f, 1.f };
		DMUI_ThemeColors theme{};
		auto themeOptional = a_client->GetThemeColors();
		if (themeOptional.has_value())
		{
			color = themeOptional->accent;
			colorDisable = themeOptional->statusDisable;
		}

		auto dataStorage = meDataStorage::GetSingleton();

		// Define table flags with vertical scrolling and borders
		dmui::ui::TableFlags flags =
			dmui::ui::TableFlags::kScrollY |
			dmui::ui::TableFlags::kRowBg |
			dmui::ui::TableFlags::kBorders |
			dmui::ui::TableFlags::kResizable |
			dmui::ui::TableFlags::kSizingFixedFit;

		dmui::ui::PushStyleVar(dmui::ui::StyleVar::kItemSpacing, dmui::ui::Vec2(0.0f, 1.0f));
		dmui::ui::PushStyleVar(dmui::ui::StyleVar::kCellPadding, dmui::ui::Vec2(8.0f, 1.0f));

		if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.explorer.body.plugins", 2, flags))
		{
			dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, { .0f, .0f, .0f, .0f });
			dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderActive, { .0f, .0f, .0f, .0f });

			// Freeze the first row (the header) so it stays visible while scrolling
			dmui::ui::TableSetupScrollFreeze(0, 1);

			auto sizeOrderColumn = dmui::ui::CalcTextSize("0xFFFFFZ");

			// Setup columns the header row
			dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsOrder,
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				sizeOrderColumn.x);
			dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsName,
				dmui::ui::TableColumnFlags::kWidthStretch);
			dmui::ui::TableHeadersRow();

			dmui::ui::PopStyleColor(2);
			const auto colorSelectedRow = dmui::ui::GetStyleColor(dmui::ui::Color::kHeader);

			if (!a_list)
			{
				meDataStorageAutoLock guard(dataStorage);

				try
				{
					dmui::ui::ListClipper clipper;
					clipper.Begin(dataStorage->GetModForShopCount() + 1);

					while (clipper.Step())
					{
						// Fill table with rows of data
						for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
						{
							auto is_selected = (selectPluginIdList == row);
							if (is_selected)
								dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, colorSelectedRow);

							if (!row)
							{
								dmui::ui::TableNextRow();

								if (dmui::ui::TableNextColumn())
								{
									if (dmui::ui::Selectable("##dearmodding.modexplorer.page.explorer.body.plugins.row.all",
										is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
										if (!needItemsUpdate.load())
										{
											done.store(false);
											selectPluginIdList = row;
											selectedShopPluginId.store(row - 1);
											needItemsUpdate.store(true);
										}

									dmui::ui::SameLine(.0f, .0f);

									constexpr static auto ALL_INDEX = "-";

									if (is_selected)
										dmui::ui::Text(ALL_INDEX);
									else
										dmui::ui::TextColored(color, ALL_INDEX);
								}

								if (dmui::ui::TableNextColumn())
									dmui::ui::Text("- - - %s - - -", lsAll.GetValue());
							}
							else
							{
								auto plugin = dataStorage->GetModForShopByIndex(row - 1);

								dmui::ui::TableNextRow();

								if (dmui::ui::TableNextColumn())
								{
									char label[96];
									sprintf_s(label, "##dearmodding.modexplorer.page.explorer.body.plugins.row.%d", row);
									if (dmui::ui::Selectable(label, is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
										if (!needItemsUpdate.load())
										{
											done.store(false);
											selectPluginIdList = row;
											selectedShopPluginId.store(row - 1);
											needItemsUpdate.store(true);
										}

									dmui::ui::SameLine(.0f, .0f);

									if (is_selected)
										dmui::ui::Text("0x%X", plugin->GetIndex().value());
									else
									{
										if (plugin->GetFile()->IsLight())
											dmui::ui::TextColored(colorDisable, "0x%X", plugin->GetIndex().value());
										else
											dmui::ui::TextColored(color, "0x%X", plugin->GetIndex().value());
									}
								}

								if (dmui::ui::TableNextColumn())
									dmui::ui::Text(plugin->GetFileName().c_str());
							}

							if (is_selected)
								dmui::ui::PopStyleColor();
						}
					}
				}
				catch (...)
				{}
			}
			else
			{
				meDataStorageAutoLock guard(dataStorage);

				dmui::ui::ListClipper clipper;
				clipper.Begin(static_cast<int32_t>(a_list->size()));

				while (clipper.Step())
				{
					// Fill table with rows of data
					for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						auto is_selected = (selectPluginIdList == row);
						if (is_selected)
							dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, colorSelectedRow);

						auto& plugin = a_list->at(row);

						dmui::ui::TableNextRow();
						if (dmui::ui::TableNextColumn())
						{
							char label[96];
							sprintf_s(label, "##dearmodding.modexplorer.page.explorer.body.plugins.row.%d", row);
							if (dmui::ui::Selectable(label, is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
								if (!needItemsUpdate.load())
								{
									done.store(false);
									selectPluginIdList = row;
									const auto& items = dataStorage->GetModShopList();
									auto& mod = a_list->at(selectPluginIdList);
									auto it = std::find_if(items.cbegin(), items.cend(), [&mod](const std::shared_ptr<meModStorage>& ptr) {
										return ptr && ptr->GetFile() == mod->GetFile();
										});

									if (it == items.cend())
										selectedShopPluginId.store(-1);
									else
										selectedShopPluginId.store(static_cast<int32_t>(std::distance(items.cbegin(), it)));

									needItemsUpdate.store(true);
								}

							dmui::ui::SameLine(.0f, .0f);

							if (is_selected)
								dmui::ui::Text("0x%X", plugin->GetIndex().value());
							else
							{
								if (plugin->GetFile()->IsLight())
									dmui::ui::TextColored(colorDisable, "0x%X", plugin->GetIndex().value());
								else
									dmui::ui::TextColored(color, "0x%X", plugin->GetIndex().value());
							}
						}

						if (dmui::ui::TableNextColumn())
							dmui::ui::Text(plugin->GetFileName().c_str());

						if (is_selected)
							dmui::ui::PopStyleColor();
					}
				}
			}

			dmui::ui::EndTable();
		}

		dmui::ui::PopStyleVar(2);
	}
}

void meDMUIClient::RendererGeneralPage()
{
	updateFrame.store(true);

	auto dataStorage = meDataStorage::GetSingleton();
	auto uiClient = meDMUIClient::GetSingleton();
	auto dmuiPlatform = uiClient->client.get();

	DMUI_ThemeColors theme{};
	auto themeOptional = dmuiPlatform->GetThemeColors();
	if (themeOptional.has_value())
		theme = themeOptional.value();

	static int32_t selectedPluginId = -1;
	
	if (updatePlugins.load())
	{
		// Reset
		selectedPluginId = -1;

		UITools::SpinnerThink(dmuiPlatform, "##dearmodding.modexplorer.page.general.dimmy");
	}
	else
	{
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

				dmui::ui::Text("%s:", lsGeneralPageModTotalCaption.GetValue()); dmui::ui::SameLine();
				dmui::ui::TextColored(theme.success, "%u", dataStorage->GetModCount()); dmui::ui::SameLine();
				dmui::ui::Text("/ %s:", lsGeneralPageModRegularCaption.GetValue()); dmui::ui::SameLine();
				dmui::ui::TextColored(theme.success, "%u", dataStorage->GetDefaultModCount()); dmui::ui::SameLine();
				dmui::ui::Text("/ %s:", lsGeneralPageModLightCaption.GetValue()); dmui::ui::SameLine();
				dmui::ui::TextColored(theme.success, "%u", dataStorage->GetLightModCount());

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
						sizeOrderColumn.x + 8.f);
					dmui::ui::TableSetupColumn(lsGeneralPageNumPluginsName,
						dmui::ui::TableColumnFlags::kWidthStretch);
					dmui::ui::TableHeadersRow();

					dmui::ui::PopStyleColor(2);
					const auto colorSelectedRow = dmui::ui::GetStyleColor(dmui::ui::Color::kHeader);

					meDataStorageAutoLock guard(dataStorage);

					try
					{
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
					}

					dmui::ui::EndTable();
				}

				dmui::ui::PopStyleVar(2);
			}
			if (dmui::ui::TableNextColumn())
			{
				meDataStorageAutoLock guard(dataStorage);

				try
				{
					constexpr auto colorHeader = dmui::ui::Vec4(1.f, .85f, .1f, 1.f);

					auto fontSize = dmui::ui::CalcTextSize("A");
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

							constexpr float row_size = 64.f;

							for (auto itemType : meItemTypes)
							{
								dmui::ui::TableNextRow(dmui::ui::TableRowFlags::kNone, row_size);
								if (dmui::ui::TableNextColumn())
								{
									switch (itemType)
									{
									case meItemType::kArmorItem:
										dmui::Image(dmuiPlatform, textureArmor, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsArmor);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kBookItem:
										dmui::Image(dmuiPlatform, textureBook, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsBook);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kMiscItem:
										dmui::Image(dmuiPlatform, textureMisc, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsMisc);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kWeaponItem:
										dmui::Image(dmuiPlatform, textureWeapon, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsWeapon);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kAmmoItem:
										dmui::Image(dmuiPlatform, textureAmmo, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsAmmo);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kKeyItem:
										dmui::Image(dmuiPlatform, textureKey, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsKey);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kAlchemyItem:
										dmui::Image(dmuiPlatform, textureAlchemy, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsAlchemy);
											dmui::ui::EndTooltip();
										}
										break;
									case meItemType::kNoteItem:
										dmui::Image(dmuiPlatform, textureNote, row_size, row_size);
										if (dmui::ui::BeginItemTooltip())
										{
											dmui::ui::Text(lsNote);
											dmui::ui::EndTooltip();
										}
										break;
									default:
										break;
									}
								}

								if (dmui::ui::TableNextColumn())
								{
									// Calculate vertical offset
									float text_height = fontSize.y;
									float vertical_offset = (row_size - text_height) * 0.5f;

									// Push the cursor down by the offset inside this column
									dmui::ui::SetCursorPosY(dmui::ui::GetCursorPosY() + vertical_offset);

									//auto clientRect = dmui::ui::GetContentRegionAvail();
									auto num = plugin->GetItemCount(itemType);
									if (num)
										dmui::ui::Text("%u", num);
									else
										dmui::ui::TextColored(theme.statusDisable, "-");
								}
							}

							dmui::ui::EndTable();
						}
					}
				}
				catch (...)
				{}
			}

			dmui::ui::PopStyleColor();
			dmui::ui::EndTable();
		}
	}

	updateFrame.store(false);
}

void meDMUIClient::RendererExplorerPage() noexcept
{
	updateFrame.store(true);

	auto dataStorage = meDataStorage::GetSingleton();
	auto uiClient = meDMUIClient::GetSingleton();
	auto dmuiPlatform = uiClient->client.get();

	DMUI_ThemeColors theme{};
	auto themeOptional = dmuiPlatform->GetThemeColors();
	if (themeOptional.has_value())
		theme = themeOptional.value();

	DMUI_StyleMetrics metrics{};
	(void)dmui::ui::GetStyleMetrics(metrics);

	static uint16_t countForBuy = 1;
	static uint16_t countMinForBuy = 1;
	static uint16_t countMaxForBuy = 500;
	static int32_t selectPluginIdList = -1;
	
	static dmui::ui::Vec2 sizeSearchText{};
	static dmui::ui::Vec2 sizeCountText{};

	if (updatePlugins.load())
	{
		// Reset
		selectPluginIdList = -1;
		selectedShopPluginId.store(-1);

		UITools::SpinnerThink(dmuiPlatform, "##dearmodding.modexplorer.page.explorer.dimmy1");
	}
	else
	{
		if (no_once_load_items.load())
		{
			no_once_load_items.store(false);
			sizeSearchText = dmui::ui::CalcTextSize(std::format("{}: ", lsSearch.GetValue()));
			sizeCountText = dmui::ui::CalcTextSize(std::format("{}: ", lsExplorerPageCount.GetValue()));
			done.store(false);
			needItemsUpdate.store(true);
			selectPluginIdList = 0;
		}

		auto widgetRect = dmui::ui::GetContentRegionAvail();

		if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.explorer.body", 2))
		{
			dmui::ui::PushStyleColor(dmui::ui::Color::kText, { .9f, .9f, .9f, 1.f });

			auto width_main_column_1 = widgetRect.x * .33f;

			dmui::ui::TableSetupColumn("##dearmodding.modexplorer.page.explorer.body.column_plugings",
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				width_main_column_1);
			dmui::ui::TableSetupColumn("##dearmodding.modexplorer.page.explorer.body.column_shop",
				dmui::ui::TableColumnFlags::kWidthStretch | dmui::ui::TableColumnFlags::kNoResize);

			dmui::ui::TableNextRow();
			if (dmui::ui::TableNextColumn())
			{
				auto safeY = dmui::ui::GetCursorPosY();
				dmui::ui::SetCursorPosY(safeY + metrics.framePadding.y);
				dmui::ui::Text("%s:", lsSearch.GetValue()); dmui::ui::SameLine();
				dmui::ui::SetCursorPosY(safeY);
				dmui::ui::SetNextItemWidth(width_main_column_1 - sizeSearchText.x);
				if (dmui::ui::InputText("##dearmodding.modexplorer.page.explorer.body.searchinput.plugins",
					textExplorerSearchPlugin.data(), textExplorerSearchPlugin.size(), dmui::ui::InputTextFlags::kEnterReturnsTrue))
				{
					selectPluginIdList = -1;
					selectedShopPluginId.store(-1);
					itemShopList.clear();

					memcpy_s(textExplorerSearchPluginDone.data(), textExplorerSearchPluginDone.size(),
						textExplorerSearchPlugin.data(), textExplorerSearchPlugin.size());

					if (!textExplorerSearchPluginDone[0])
					{
						selectPluginIdList = 0;
						done.store(false);
						needItemsUpdate.store(true);
					}
					else
					{
						done.store(false);
						updateSearchPlugins.store(true);
					}
				}

				// dmui::ui::Text("selectedShopPluginId: %d", selectedShopPluginId.load());

				if (textExplorerSearchPluginDone[0])
				{
					if (updateSearchPlugins.load() && !done.load())
						UITools::SpinnerThink(dmuiPlatform, "##dearmodding.modexplorer.page.explorer.dimmy3");
					else if (!updateSearchPlugins.load())
						PageHelper::DrawPluginTableForShop(dmuiPlatform, selectPluginIdList, std::addressof(pluginSearchShopList));			
				}
				else
					PageHelper::DrawPluginTableForShop(dmuiPlatform, selectPluginIdList);
			}

			if (dmui::ui::TableNextColumn())
			{
				auto clientRect = dmui::ui::GetContentRegionAvail();

				auto prevColor = dmui::ui::GetStyleColor(dmui::ui::Color::kButton);
				dmui::ui::PushStyleColor(dmui::ui::Color::kButton, theme.statusDisable);

				auto currentBtnType = selectedShopTypeId.load();
				auto createBtn = [&](const std::string& a_id, std::int8_t a_typeId) {
					if (currentBtnType == a_typeId)
						dmui::ui::PushStyleColor(dmui::ui::Color::kButton, prevColor);
					if (dmui::ui::Button(a_id.c_str()))
					{
						done.store(false);
						selectedShopTypeId.store(a_typeId);
						needItemsUpdate.store(true);
					}
					if (currentBtnType == a_typeId)
						dmui::ui::PopStyleColor();
					};

				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.all", lsAll.GetValue()), -1); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.armor", lsArmor.GetValue()), 0); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.book", lsBook.GetValue()), 1); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.misc", lsMisc.GetValue()), 2); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.weapon", lsWeapon.GetValue()), 3); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.ammo", lsAmmo.GetValue()), 4); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.key", lsKey.GetValue()), 5); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.alchemy", lsAlchemy.GetValue()), 6); dmui::ui::SameLine();
				createBtn(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.note", lsNote.GetValue()), 7);

				dmui::ui::PopStyleColor();

				auto safeY = dmui::ui::GetCursorPosY();
				dmui::ui::SetCursorPosY(safeY + metrics.framePadding.y);
				dmui::ui::Text("%s:", lsSearch.GetValue()); dmui::ui::SameLine();
				dmui::ui::SetCursorPosY(safeY);
				dmui::ui::SetNextItemWidth(clientRect.x - sizeSearchText.x);
				if (dmui::ui::InputText("##dearmodding.modexplorer.page.explorer.body.searchinput.items",
					textSearchItem.data(), textSearchItem.size()))
				{

					//printf("Text changed to: %s\n", text_buffer);
				}

				if (!needItemsUpdate.load() && done.load())
				{
					// Define table flags with vertical scrolling and borders
					dmui::ui::TableFlags flags =
						dmui::ui::TableFlags::kScrollY |
						dmui::ui::TableFlags::kRowBg |
						dmui::ui::TableFlags::kBorders |
						dmui::ui::TableFlags::kResizable;

					dmui::ui::PushStyleVar(dmui::ui::StyleVar::kItemSpacing, dmui::ui::Vec2(0.0f, 1.0f));
					dmui::ui::PushStyleVar(dmui::ui::StyleVar::kCellPadding, dmui::ui::Vec2(8.0f, 1.0f));

					auto avail = dmui::ui::GetContentRegionAvail();
					if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.explorer.body.items", 4, flags,
						{ -1.f, avail.y - (sizeSearchText.y + (metrics.framePadding.y * 3) + 6.f) }))
					{
						dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, { .0f, .0f, .0f, .0f });
						dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderActive, { .0f, .0f, .0f, .0f });

						// Freeze the first row (the header) so it stays visible while scrolling
						dmui::ui::TableSetupScrollFreeze(0, 1);

						auto sizeOrderColumn = dmui::ui::CalcTextSize("0xXXYYZZFF");
						sizeOrderColumn.x += 8.f;
						sizeOrderColumn.y += 2.f;
						auto width_2_column = sizeOrderColumn.x * 1.2f;

						// Setup columns the header row
						dmui::ui::TableSetupColumn("",
							dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
							30.f);
						dmui::ui::TableSetupColumn("",
							dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
							sizeOrderColumn.x);
						dmui::ui::TableSetupColumn(lsType,
							dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
							width_2_column);
						dmui::ui::TableSetupColumn(lsFullname,
							dmui::ui::TableColumnFlags::kWidthStretch);

						dmui::ui::TableNextRow(dmui::ui::TableRowFlags::kHeaders, sizeOrderColumn.y * 1.5f - 3.f);
						(void)dmui::ui::TableSetColumnIndex(1);
						auto s_header = dmui::ui::GetCursorScreenPos();
						dmui::ui::SetCursorScreenPos({ s_header.x, s_header.y + 7.f });
						dmui::ui::Text("FormID", sizeOrderColumn.y);
						(void)dmui::ui::TableSetColumnIndex(2);
						s_header = dmui::ui::GetCursorScreenPos();
						dmui::ui::SetCursorScreenPos({ s_header.x, s_header.y + 7.f });
						dmui::ui::TextAligned(.5f, width_2_column, lsType.GetValue());
						(void)dmui::ui::TableSetColumnIndex(3);
						s_header = dmui::ui::GetCursorScreenPos();
						dmui::ui::SetCursorScreenPos({ s_header.x, s_header.y + 7.f });
						dmui::ui::Text(lsFullname);

						dmui::ui::PopStyleColor(2);
						const auto colorSelectedRow = dmui::ui::GetStyleColor(dmui::ui::Color::kHeader);

						dmui::ui::ListClipper clipper;
						clipper.Begin(static_cast<int32_t>(itemShopList.size()));

						while (clipper.Step())
						{
							// Fill table with rows of data
							for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
							{
								auto& item = itemShopList[row];

								dmui::ui::TableNextRow();

								if (dmui::ui::TableNextColumn() && item.flag.all(meItemFlag::kQuestItem))
								{
									dmui::Image(dmuiPlatform, textureStar, sizeOrderColumn.y, sizeOrderColumn.y);
								}

								if (dmui::ui::TableNextColumn())
									dmui::ui::Text("0x%08X", item.formId);

								if (dmui::ui::TableNextColumn())
								{
									dmui::ui::PushStyleColor(dmui::ui::Color::kText, theme.info);

									switch (item.type)
									{
									case meItemType::kArmorItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsArmor.GetValue());
										break;
									case meItemType::kBookItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsBook.GetValue());
										break;
									case meItemType::kMiscItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsMisc.GetValue());
										break;
									case meItemType::kWeaponItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsWeapon.GetValue());
										break;
									case meItemType::kAmmoItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsAmmo.GetValue());
										break;
									case meItemType::kKeyItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsKey.GetValue());
										break;
									case meItemType::kAlchemyItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsAlchemy.GetValue());
										break;
									case meItemType::kNoteItem:
										dmui::ui::TextAligned(.5f, width_2_column, lsNote.GetValue());
										break;
									default:
										dmui::ui::TextAligned(.5f, width_2_column, "-");
										break;
									}

									dmui::ui::PopStyleColor();
								}

								if (dmui::ui::TableNextColumn())
									dmui::ui::Text("%s", item.name.c_str());
							}
						}

						dmui::ui::EndTable();
					}

					dmui::ui::PopStyleVar(2);

					dmui::ui::Dummy({ 0.f, 0.f });
					auto posXCount = dmui::ui::GetCursorPosX();
					dmui::ui::BeginDisabled();
					(void)dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.show",
						lsExplorerPageShowModel.GetValue()).c_str());
					dmui::ui::SameLine();
					dmui::ui::EndDisabled();
					dmui::ui::BeginDisabled();
					(void)dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.buyall",
						lsExplorerPageBuyAll.GetValue()).c_str());
					dmui::ui::SameLine();
					dmui::ui::EndDisabled();
					dmui::ui::BeginDisabled();
					(void)dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.explorer.body.buttons.buy",
						lsExplorerPageBuy.GetValue()).c_str());
					dmui::ui::SameLine(0, 30.f);
					posXCount = dmui::ui::GetCursorPosX() - posXCount;
					dmui::ui::Text(lsExplorerPageCount); dmui::ui::SameLine();
					dmui::ui::SetNextItemWidth(avail.x - sizeCountText.x - posXCount);
					(void)dmui::ui::SliderScalar("##dearmodding.modexplorer.page.explorer.body.buttons.count",
						std::addressof(countForBuy), std::addressof(countMinForBuy), std::addressof(countMaxForBuy));
					dmui::ui::EndDisabled();
				}
				else if (needItemsUpdate.load() && !done.load())
					UITools::SpinnerThink(dmuiPlatform, "##dearmodding.modexplorer.page.explorer.dimmy2");
			}

			dmui::ui::PopStyleColor();
			dmui::ui::EndTable();
		}
	}

	updateFrame.store(false);
}

void meDMUIClient::RendererBasketPage() noexcept
{
	updateFrame.store(true);

	dmui::ui::TextUnformatted("Hello World!");

	updateFrame.store(false);
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

	textureArmor	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_ARMOR,	"DDS");
	textureBook		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_BOOK,	"DDS");
	textureMisc		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_MISC,	"DDS");
	textureWeapon	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_WEAPON,	"DDS");
	textureAmmo		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_AMMO,	"DDS");
	textureKey		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_KEY,		"DDS");
	textureAlchemy	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_ALCHEMY,	"DDS");
	textureNote		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_NOTE,	"DDS");
	textureStar		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_STAR,	"DDS");
	if (!textureArmor || !textureNote || !textureBook || !textureMisc || !textureWeapon || 
		!textureAmmo || !textureKey || !textureAlchemy || !textureStar)
		REX::ERROR("meDMUIClient::Connect() failed load assets"sv);

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host"sv, kClientId);

	std::thread([]() {
		REX::FTimer timer;
		auto dataStorage = meDataStorage::GetSingleton();
		while (!terminated.load())
		{
			// don't load the process too much
			std::this_thread::sleep_for(50ms);

			if (!done.load())
			{
				if (needItemsUpdate.load())
				{
					meDataStorageAutoLock guard(dataStorage);
					timer.Start();

					try
					{
						auto typeId = selectedShopTypeId.load();
						auto pluginId = selectedShopPluginId.load();
						if (pluginId == -1)
						{
							switch (typeId)
							{
							case 0:
								dataStorage->GetAllItems(meItemType::kArmorItem, itemShopList, false);
								break;
							case 1:
								dataStorage->GetAllItems(meItemType::kBookItem, itemShopList, false);
								break;
							case 2:
								dataStorage->GetAllItems(meItemType::kMiscItem, itemShopList, false);
								break;
							case 3:
								dataStorage->GetAllItems(meItemType::kWeaponItem, itemShopList, false);
								break;
							case 4:
								dataStorage->GetAllItems(meItemType::kAmmoItem, itemShopList, false);
								break;
							case 5:
								dataStorage->GetAllItems(meItemType::kKeyItem, itemShopList, false);
								break;
							case 6:
								dataStorage->GetAllItems(meItemType::kAlchemyItem, itemShopList, false);
								break;
							case 7:
								dataStorage->GetAllItems(meItemType::kNoteItem, itemShopList, false);
								break;
							default:
								dataStorage->GetAllItems(meItemType::kMax, itemShopList, false);
								break;
							}
						}
						else
						{
							auto plugin = dataStorage->GetModForShopByIndex(pluginId);
							if (plugin)
							{
								switch (typeId)
								{
								case 0:
									plugin->GetAllItems(meItemType::kArmorItem, itemShopList, false);
									break;
								case 1:
									plugin->GetAllItems(meItemType::kBookItem, itemShopList, false);
									break;
								case 2:
									plugin->GetAllItems(meItemType::kMiscItem, itemShopList, false);
									break;
								case 3:
									plugin->GetAllItems(meItemType::kWeaponItem, itemShopList, false);
									break;
								case 4:
									plugin->GetAllItems(meItemType::kAmmoItem, itemShopList, false);
									break;
								case 5:
									plugin->GetAllItems(meItemType::kKeyItem, itemShopList, false);
									break;
								case 6:
									plugin->GetAllItems(meItemType::kAlchemyItem, itemShopList, false);
									break;
								case 7:
									plugin->GetAllItems(meItemType::kNoteItem, itemShopList, false);
									break;
								default:
									plugin->GetAllItems(meItemType::kMax, itemShopList, false);
									break;
								}
							}
						}
					}
					catch (...)
					{}

					timer.Stop();

					// protection against epileptics
					auto duration = timer.GetDuration<std::chrono::milliseconds>();
					if (duration < 750)
						std::this_thread::sleep_for((750 - duration) * 1ms);

					needItemsUpdate.store(false);
					done.store(true);
				}
				if (updateSearchPlugins.load())
				{
					meDataStorageAutoLock guard(dataStorage);
					timer.Start();
					pluginSearchShopList.clear();

					try
					{
						for (auto& mod : dataStorage->GetModShopList())
							if (StrStrIA(mod->GetFileName().c_str(), textExplorerSearchPluginDone.data()))
								pluginSearchShopList.push_back(mod);
					}
					catch (...)
					{}

					timer.Stop();

					// protection against epileptics
					auto duration = timer.GetDuration<std::chrono::milliseconds>();
					if (duration < 750)
						std::this_thread::sleep_for((750 - duration) * 1ms);

					updateSearchPlugins.store(false);
					done.store(true);
				}
			}
		}
		}).detach();

	return true;
}

void meDMUIClient::BeginUpdate() noexcept
{
	while (updateFrame.load()) { std::this_thread::yield(); }
	done.store(false);
	updatePlugins.store(true);
}

void meDMUIClient::EndUpdate() noexcept
{
	pluginSearchShopList.clear();

	updatePlugins.store(false);
	done.store(true);
}
