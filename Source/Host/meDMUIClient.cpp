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
static dmui::localize::LocalizeString lsArmor("$Armor", "Armor");
static dmui::localize::LocalizeString lsBook("$Book", "Book");
static dmui::localize::LocalizeString lsMisc("$Misc", "Misc");
static dmui::localize::LocalizeString lsWeapon("$Weapon", "Weapon");
static dmui::localize::LocalizeString lsAmmo("$Ammo", "Ammo");
static dmui::localize::LocalizeString lsKey("$Key", "Key");
static dmui::localize::LocalizeString lsAlchemy("$Alchemy", "Alchemy");
static dmui::localize::LocalizeString lsNote("$Note", "Note");

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
static std::shared_ptr<dmui::Texture> textureStar{};

static std::atomic_bool done = true;
static std::atomic_bool terminated = false;
static std::atomic_bool needItemsUpdate = false;
static std::atomic_int32_t selectedShopPluginId = -1;
static std::atomic_int8_t selectedShopTypeId = -1;
static meItemList itemShopList;

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
				{}

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

void meDMUIClient::RendererExplorerPage() noexcept
{
	auto dataStorage = meDataStorage::GetSingleton();
	auto uiClient = meDMUIClient::GetSingleton();
	auto dmuiPlatform = uiClient->client.get();

	DMUI_ThemeColors theme{};
	auto themeOptional = dmuiPlatform->GetThemeColors();
	if (themeOptional.has_value())
		theme = themeOptional.value();

	static uint16_t countForBuy = 1;
	static uint16_t countMinForBuy = 1;
	static uint16_t countMaxForBuy = 500;
	static uint32_t selectPluginIdList = -1;

	auto widgetRect = dmui::ui::GetContentRegionAvail();

	if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.explorer.body", 2))
	{
		dmui::ui::PushStyleColor(dmui::ui::Color::kText, { .9f, .9f, .9f, 1.f });

		dmui::ui::TableSetupColumn("##dearmodding.modexplorer.page.explorer.body.column_plugings",
			dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
			widgetRect.x * .33f);
		dmui::ui::TableSetupColumn("##dearmodding.modexplorer.page.explorer.body.column_shop",
			dmui::ui::TableColumnFlags::kWidthStretch | dmui::ui::TableColumnFlags::kNoResize);

		dmui::ui::TableNextRow();
		if (dmui::ui::TableNextColumn())
		{
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
										dmui::ui::TextColored(theme.accent, ALL_INDEX);
								}

								if (dmui::ui::TableNextColumn())
									dmui::ui::Text("- - - All - - -");
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
											dmui::ui::TextColored(theme.statusDisable, "0x%X", plugin->GetIndex().value());
										else
											dmui::ui::TextColored(theme.accent, "0x%X", plugin->GetIndex().value());
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

				dmui::ui::EndTable();
			}

			dmui::ui::PopStyleVar(2);
		}

		if (dmui::ui::TableNextColumn())
		{
			auto clientRect = dmui::ui::GetContentRegionAvail();
	
			auto prevColor = dmui::ui::GetStyleColor(dmui::ui::Color::kButton);
			dmui::ui::PushStyleColor(dmui::ui::Color::kButton, theme.statusDisable);

			auto currentBtnType = selectedShopTypeId.load();
			auto createBtn = [&](const char* a_id, std::int8_t a_typeId) {
				if (currentBtnType == a_typeId)
					dmui::ui::PushStyleColor(dmui::ui::Color::kButton, prevColor);
				if (dmui::ui::Button(a_id))
				{
					done.store(false);
					selectedShopTypeId.store(a_typeId);
					needItemsUpdate.store(true);
				}
				if (currentBtnType == a_typeId)
					dmui::ui::PopStyleColor();
				};

			createBtn("All##dearmodding.modexplorer.page.explorer.body.buttons.all", -1); dmui::ui::SameLine();
			createBtn("Armor##dearmodding.modexplorer.page.explorer.body.buttons.armor", 0); dmui::ui::SameLine();
			createBtn("Book##dearmodding.modexplorer.page.explorer.body.buttons.book", 1); dmui::ui::SameLine();
			createBtn("Misc##dearmodding.modexplorer.page.explorer.body.buttons.misc", 2); dmui::ui::SameLine();
			createBtn("Weapon##dearmodding.modexplorer.page.explorer.body.buttons.weapon", 3); dmui::ui::SameLine();
			createBtn("Ammo##dearmodding.modexplorer.page.explorer.body.buttons.ammo", 4); dmui::ui::SameLine();
			createBtn("Key##dearmodding.modexplorer.page.explorer.body.buttons.key", 5); dmui::ui::SameLine();
			createBtn("Alchemy##dearmodding.modexplorer.page.explorer.body.buttons.alchemy", 6); dmui::ui::SameLine();
			createBtn("Note##dearmodding.modexplorer.page.explorer.body.buttons.note", 7);

			dmui::ui::PopStyleColor();

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
					{ -1.f, avail.y - (clientRect.y - avail.y) }))
				{
					dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, { .0f, .0f, .0f, .0f });
					dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderActive, { .0f, .0f, .0f, .0f });

					// Freeze the first row (the header) so it stays visible while scrolling
					dmui::ui::TableSetupScrollFreeze(0, 1);

					auto sizeOrderColumn = dmui::ui::CalcTextSize("0xXXYYZZFFZ");
					sizeOrderColumn.x += 8.f;
					sizeOrderColumn.y += 2.f;

					// Setup columns the header row
					dmui::ui::TableSetupColumn("",
						dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
						30.f);
					dmui::ui::TableSetupColumn("FormID",
						dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
						sizeOrderColumn.x);
					dmui::ui::TableSetupColumn("Type",
						dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
						sizeOrderColumn.x * 1.2f);
					dmui::ui::TableSetupColumn("Full name",
						dmui::ui::TableColumnFlags::kWidthStretch);
					dmui::ui::TableHeadersRow();

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
								dmui::ui::TextAligned(.5f, sizeOrderColumn.x, std::format("0x{:08X}", item.formId));

							if (dmui::ui::TableNextColumn())
							{
								dmui::ui::PushStyleColor(dmui::ui::Color::kText, theme.info);
								
								auto w = sizeOrderColumn.x * 1.2f;
								switch (item.type)
								{
								case meItemType::kArmorItem:
									dmui::ui::TextAligned(.5f, w, lsArmor.GetValue());
									break;
								case meItemType::kBookItem:
									dmui::ui::TextAligned(.5f, w, lsBook.GetValue());
									break;
								case meItemType::kMiscItem:
									dmui::ui::TextAligned(.5f, w, lsMisc.GetValue());
									break;
								case meItemType::kWeaponItem:
									dmui::ui::TextAligned(.5f, w, lsWeapon.GetValue());
									break;
								case meItemType::kAmmoItem:
									dmui::ui::TextAligned(.5f, w, lsAmmo.GetValue());
									break;
								case meItemType::kKeyItem:
									dmui::ui::TextAligned(.5f, w, lsKey.GetValue());
									break;
								case meItemType::kAlchemyItem:
									dmui::ui::TextAligned(.5f, w, lsAlchemy.GetValue());
									break;
								case meItemType::kNoteItem:
									dmui::ui::TextAligned(.5f, w, lsNote.GetValue());
									break;
								default:
									dmui::ui::TextAligned(.5f, w, "-");
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

				(void)dmui::ui::Button("All##dearmodding.modexplorer.page.explorer.body.buttons.buy");			dmui::ui::SameLine();
				(void)dmui::ui::SliderScalar("Count##dearmodding.modexplorer.page.explorer.body.buttons.count",
					std::addressof(countForBuy), std::addressof(countMinForBuy), std::addressof(countMaxForBuy));

				dmui::ui::PopStyleVar(2);
			}
			else
			{
				dmui::ui::Text("Please wait...");
			}
		}

		dmui::ui::PopStyleColor();
		dmui::ui::EndTable();
	}
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
	textureStar		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kWIC, IDB_STAR,	"PNG");
	if (!textureArmor || !textureNote || !textureBook || !textureMisc || !textureWeapon || 
		!textureAmmo || !textureKey || !textureAlchemy || !textureStar)
		REX::ERROR("meDMUIClient::Connect() failed load assets"sv);

	REX::INFO("meDMUIClient::Connect() Registered as '{}' with the dmui host"sv, kClientId);

	std::thread([]() {
		auto dataStorage = meDataStorage::GetSingleton();
		while (!terminated.load())
		{
			std::this_thread::sleep_for(20ms);

			if (!done.load())
			{
				if (needItemsUpdate.load())
				{
					meDataStorageAutoLock guard(dataStorage);

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

					needItemsUpdate.store(false);
					done.store(true);
				}
			}
		}
		}).detach();

	return true;
}