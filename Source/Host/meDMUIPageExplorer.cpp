#include "meDMUIPageExplorer.h"
#include "meHostLocalizeString.h"
#include "../../Resources/resource.h"

#include <DearModdingUI/Client.h>
#include <format>

#include <shlwapi.h>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_SIZE

static std::atomic_bool no_once_load_items = true;

namespace PageHelper
{
	static void DrawPluginTableForShop(dmui::Client* a_client, meDMUIPageExplorer* a_sender, 
		int32_t& selectPluginIdList, const meModSortedList* a_list = nullptr)
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
										if (!a_sender->IsProcessingUpdateItems())
										{
											selectPluginIdList = row;
											a_sender->SetSelectedPluginId(row - 1);
											a_sender->RequestAsyncUpdateItem();
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
										if (!a_sender->IsProcessingUpdateItems())
										{
											selectPluginIdList = row;
											a_sender->SetSelectedPluginId(row - 1);
											a_sender->RequestAsyncUpdateItem();
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
								if (!a_sender->IsProcessingUpdateItems())
								{
									selectPluginIdList = row;
									const auto& items = dataStorage->GetModShopList();
									auto& mod = a_list->at(selectPluginIdList);
									auto it = std::find_if(items.cbegin(), items.cend(), [&mod](const std::shared_ptr<meModStorage>& ptr) {
										return ptr && ptr->GetFile() == mod->GetFile();
										});

									if (it == items.cend())
										a_sender->SetSelectedPluginId(-1);
									else
										a_sender->SetSelectedPluginId(static_cast<int32_t>(std::distance(items.cbegin(), it)));

									a_sender->RequestAsyncUpdateItem();
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

void meDMUIPageExplorer::DoDrawPage()
{
	static uint16_t countForBuy = 1;
	static uint16_t countMinForBuy = 1;
	static uint16_t countMaxForBuy = 500;
	static int32_t selectPluginIdList = -1;

	static dmui::ui::Vec2 sizeSearchText{};
	static dmui::ui::Vec2 sizeCountText{};

	if (no_once_load_items.load())
	{
		no_once_load_items.store(false);
		sizeSearchText = dmui::ui::CalcTextSize(std::format("{}: ", lsSearch.GetValue()));
		sizeCountText = dmui::ui::CalcTextSize(std::format("{}: ", lsExplorerPageCount.GetValue()));
		selectPluginIdList = 0;

		RequestAsyncUpdateItem();
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
				selectedPluginId.store(-1);
				itemShopList.clear();

				memcpy_s(textExplorerSearchPluginDone.data(), textExplorerSearchPluginDone.size(),
					textExplorerSearchPlugin.data(), textExplorerSearchPlugin.size());

				if (!textExplorerSearchPluginDone[0])
				{
					selectPluginIdList = 0;
					textExplorerSearchItem[0] = '\0';
					textExplorerSearchItemDone[0] = '\0';
					RequestAsyncUpdateItem();
				}
				else
				{
					textExplorerSearchItem[0] = '\0';
					textExplorerSearchItemDone[0] = '\0';
					RequestAsyncUpdateItem(TRequestAsync::kSearchPlugins);
				}
			}

			// dmui::ui::Text("selectedShopPluginId: %d", selectedShopPluginId.load());

			if (textExplorerSearchPluginDone[0])
			{
				if (IsProcessingSearchPlugins())
				{
					if (spinner)
						spinner->Draw();
				}
				else
					PageHelper::DrawPluginTableForShop(GetClient(), this, selectPluginIdList, std::addressof(pluginSearchShopList));
			}
			else
				PageHelper::DrawPluginTableForShop(GetClient(), this, selectPluginIdList);
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
					selectedShopTypeId.store(a_typeId);
					RequestAsyncUpdateItem();
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
				textExplorerSearchItem.data(), textExplorerSearchItem.size()))
			{

				//printf("Text changed to: %s\n", text_buffer);
			}

			if (!IsProcessingSearchPlugins())
			{
				if (!IsProcessingUpdateItems())
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
									DMUI_Image(GetClient(), textureStar, sizeOrderColumn.y, sizeOrderColumn.y);

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
				else if (IsProcessingUpdateItems() && spinner)
					spinner->Draw();
			}
		}

		dmui::ui::PopStyleColor();
		dmui::ui::EndTable();
	}
}

void meDMUIPageExplorer::DoRequestUpdate()
{
}

void meDMUIPageExplorer::DoRequestUpdateItems()
{
	std::thread([](meDMUIPageExplorer* a_sender) {
		using namespace std::literals;

		REX::FTimer timer;
		auto dataStorage = meDataStorage::GetSingleton();
		meDataStorageAutoLock guard(dataStorage);
		timer.Start();

		try
		{
			auto typeId = a_sender->GetSelectedShopTypeId();
			auto pluginId = a_sender->GetSelectedPluginId();
			auto& itemShopList = a_sender->GetItemShopList();
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

		a_sender->ResetState();
		}, this).detach();
}

void meDMUIPageExplorer::DoRequestSearchPlugins()
{
	std::thread([](meDMUIPageExplorer* a_sender) {
		using namespace std::literals;

		REX::FTimer timer;
		auto dataStorage = meDataStorage::GetSingleton();
		auto& list = a_sender->GetPluginSearchShopList();
		meDataStorageAutoLock guard(dataStorage);
		timer.Start();
		list.clear();

		try
		{
			for (auto& mod : dataStorage->GetModShopList())
				if (StrStrIA(mod->GetFileName().c_str(), a_sender->GetTextSearchPlugin()))
					list.push_back(mod);
		}
		catch (...)
		{}

		timer.Stop();

		// protection against epileptics
		auto duration = timer.GetDuration<std::chrono::milliseconds>();
		if (duration < 750)
			std::this_thread::sleep_for((750 - duration) * 1ms);

		a_sender->ResetState();
		}, this).detach();
}

void meDMUIPageExplorer::DoRequestSearchItems()
{
	std::thread([](meDMUIPageExplorer* a_sender) {
		using namespace std::literals;

		a_sender->ResetState();
		}, this).detach();
}

meDMUIPageExplorer::meDMUIPageExplorer(const std::shared_ptr<dmui::Client>& a_client) :
	meDMUIPageBased(a_client)
{
	using namespace std::literals;

	textureStar = dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_STAR, "DDS");
	if (!textureStar)
		REX::ERROR("PME::meDMUIPageExplorer() failed load assets"sv);
}

int32_t meDMUIPageExplorer::GetSelectedPluginId() const noexcept
{
	return selectedPluginId.load();
}

void meDMUIPageExplorer::SetSelectedPluginId(int32_t a_idx) noexcept
{
	selectedPluginId.store(a_idx);
}

int8_t meDMUIPageExplorer::GetSelectedShopTypeId() const noexcept
{
	return selectedShopTypeId.load();
}

void meDMUIPageExplorer::SetSelectedShopTypeId(int8_t a_idx) noexcept
{
	selectedShopTypeId.store(a_idx);
}

meItemList& meDMUIPageExplorer::GetItemShopList() noexcept
{
	return itemShopList;
}

meModSortedList& meDMUIPageExplorer::GetPluginSearchShopList() noexcept
{
	return pluginSearchShopList;
}

const char* meDMUIPageExplorer::GetTextSearchPlugin() const noexcept
{
	return textExplorerSearchPluginDone.data();
}

const char* meDMUIPageExplorer::GetTextSearchItem() const noexcept
{
	return textExplorerSearchItemDone.data();
}

void meDMUIPageExplorer::RequestAsyncUpdateItem(TRequestAsync a_requestEvent)
{
	switch (a_requestEvent)
	{
	case meDMUIPageExplorer::TRequestAsync::kUpdateItems:
		currentEvent.store(TRequestAsync::kUpdateItems);
		DoRequestUpdateItems();
		break;
	case meDMUIPageExplorer::TRequestAsync::kSearchPlugins:
		currentEvent.store(TRequestAsync::kSearchPlugins);
		DoRequestSearchPlugins();
		break;
	case meDMUIPageExplorer::TRequestAsync::kSearchItems:
		currentEvent.store(TRequestAsync::kSearchItems);
		DoRequestSearchItems();
		break;
	default:
		break;
	}
}

bool meDMUIPageExplorer::IsProcessingUpdateItems() const noexcept
{
	return currentEvent.load() == TRequestAsync::kUpdateItems;
}

bool meDMUIPageExplorer::IsProcessingSearchPlugins() const noexcept
{
	return currentEvent.load() == TRequestAsync::kSearchPlugins;
}

bool meDMUIPageExplorer::IsProcessingSearchItems() const noexcept
{
	return currentEvent.load() == TRequestAsync::kSearchItems;
}

void meDMUIPageExplorer::ResetState()
{
	currentEvent.store(TRequestAsync::kNone);
}
