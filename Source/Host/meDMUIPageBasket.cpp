#include "meDMUIPageBasket.h"
#include "meHostLocalizeString.h"
#include "meDMUIUtils.h"
#include "../meBasket.h"
#include "../../Resources/resource.h"

void meDMUIPageBasket::DoDrawPage()
{
	static int32_t selectItemIdList = GetSelectedItemId();
	auto basket = meBasket::GetSingleton();
	auto avail = dmui::ui::GetContentRegionAvail();
	auto sizeOrderColumn = dmui::ui::CalcTextSize("0xXXYYZZFF");
	auto availWidthMainCol = avail.x * .67f;
	auto availHeightTable = avail.y - (metrics.framePadding.y * 3) - sizeOrderColumn.y;

	//DoDrawPopupFailed_1();
	//DoDrawPopupFailed_2();

	if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.basket.body", 2/*,
		dmui::ui::TableFlags::kNone, { -1.f, availHeightTable-1.f }*/))
	{
		dmui::ui::TableSetupColumn("",
			dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
			availWidthMainCol);
		dmui::ui::TableSetupColumn("", dmui::ui::TableColumnFlags::kWidthStretch);

		dmui::ui::TableNextRow();
		(void)dmui::ui::TableNextColumn();

		// Define table flags with vertical scrolling and borders
		dmui::ui::TableFlags flags =
			dmui::ui::TableFlags::kScrollY |
			dmui::ui::TableFlags::kRowBg |
			dmui::ui::TableFlags::kBorders |
			dmui::ui::TableFlags::kResizable;

		dmui::ui::PushStyleVar(dmui::ui::StyleVar::kItemSpacing, dmui::ui::Vec2(0.0f, 1.0f));
		dmui::ui::PushStyleVar(dmui::ui::StyleVar::kCellPadding, dmui::ui::Vec2(8.0f, 1.0f));

		if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.basket.body.items", 5, flags/*,
			{ -1.f, availHeightTable }*/))
		{
			dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, { .0f, .0f, .0f, .0f });
			dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderActive, { .0f, .0f, .0f, .0f });

			// Freeze the first row (the header) so it stays visible while scrolling
			dmui::ui::TableSetupScrollFreeze(0, 1);

			sizeOrderColumn.x += 8.f;
			sizeOrderColumn.y += 2.f;
			auto width_2_column = sizeOrderColumn.x * 1.2f;
			auto width_3_column = width_2_column * .5f;

			// Setup columns the header row
			dmui::ui::TableSetupColumn("",
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				sizeOrderColumn.y);
			dmui::ui::TableSetupColumn("",
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				sizeOrderColumn.x);
			dmui::ui::TableSetupColumn(lsType,
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				width_2_column);
			dmui::ui::TableSetupColumn(lsExplorerPageCount,
				dmui::ui::TableColumnFlags::kWidthFixed | dmui::ui::TableColumnFlags::kNoResize,
				width_3_column);
			dmui::ui::TableSetupColumn(lsFullname,
				dmui::ui::TableColumnFlags::kWidthStretch);

			dmui::ui::TableNextRow(dmui::ui::TableRowFlags::kHeaders);
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
			dmui::ui::TextAligned(.5f, width_3_column, lsExplorerPageCount.GetValue());
			(void)dmui::ui::TableSetColumnIndex(4);
			s_header = dmui::ui::GetCursorScreenPos();
			dmui::ui::SetCursorScreenPos({ s_header.x, s_header.y + 7.f });
			dmui::ui::Text(lsFullname);

			dmui::ui::PopStyleColor(2);
			const auto colorSelectedRow = dmui::ui::GetStyleColor(dmui::ui::Color::kHeader);

			dmui::ui::ListClipper clipper;
			clipper.Begin(static_cast<int32_t>(basket->GetCount()));

			while (clipper.Step())
			{
				// Fill table with rows of data
				for (int32_t row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					auto it = basket->begin();
					std::advance(it, row);
					auto& item = it->first;

					auto is_selected = (selectItemIdList == row);
					if (is_selected)
						dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, colorSelectedRow);

					dmui::ui::TableNextRow();
					if (dmui::ui::TableNextColumn())
					{
						char label[96];
						sprintf_s(label, "##dearmodding.modexplorer.page.basket.body.plugins.row.%d", row);
						if (dmui::ui::Selectable(label, is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
						{
							selectItemIdList = row;
							SetSelectedItemId(row);
						}

						if (item.flag.all(meItemFlag::kQuestItem))
						{
							dmui::ui::SameLine();
							DMUI_Image(GetClient(), textureStar, sizeOrderColumn.y - 2.f, sizeOrderColumn.y - 2.f);
							DMUI_SetItemTooltip(GetClient(), lsStarIcon.GetValue(), true);
						}
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
						dmui::ui::TextAligned(.5f, width_3_column, std::format("{}", it->second));

					if (dmui::ui::TableNextColumn())
						dmui::ui::Text("%s", item.name.c_str());

					if (is_selected)
						dmui::ui::PopStyleColor();
				}
			}

			dmui::ui::EndTable();
		}

		(void)dmui::ui::TableNextColumn();

		if (avail.x > 50.f)
		{
			dmui::ui::PanelScope panel{ "Basket" };

			auto basket = meBasket::GetSingleton();
			auto posScreen = dmui::ui::GetCursorScreenPos();
			avail = dmui::ui::GetContentRegionAvail();
			auto scaleImg = avail.x / 249.f;
			auto sizeImg = 128.f * scaleImg;
			auto quotteRectHeight = sizeImg + metrics.framePadding.y * 5 + 
				sizeOrderColumn.y * 6;

			//DMUI_QuotteRect(posScreen,
			//	{ posScreen.x + avail.x, posScreen.y + quotteRectHeight });
			auto posX = dmui::ui::GetCursorPosX();
			dmui::ui::SetCursorPosX(posX + metrics.cellPadding.x + (avail.x - sizeImg) * .5f);
			DMUI_Image(GetClient(), textureBasket, sizeImg, sizeImg);

			(void)GetClient()->DrawSectionHeader(lsBasketPage.GetValue().c_str(), DearModdingUI::FindPhosphorSlugGlyphOrZero("basket"));

			//dmui::ui::SetCursorPosX(posX + metrics.cellPadding.x);
			if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.basket.body.basketinfo", 2))
			{
				dmui::ui::TableNextRow();
				(void)dmui::ui::TableNextColumn();
				dmui::ui::TextColored(theme.info, std::format("{}:", lsBasketPageCaps.GetValue()).c_str());
				(void)dmui::ui::TableNextColumn();
				dmui::ui::TextUnformatted(std::format("{}", basket->GetPrice()).c_str());
				dmui::ui::TableNextRow();
				(void)dmui::ui::TableNextColumn();
				dmui::ui::TextColored(theme.info, std::format("{}:", lsBasketPageWeight.GetValue()).c_str());
				(void)dmui::ui::TableNextColumn();
				dmui::ui::TextUnformatted(std::format("{:.3f}", basket->GetWeight()).c_str());

				dmui::ui::EndTable();
			}

			if (!GetClient()->DrawSectionHeader(lsBasketPagePlayer.GetValue().c_str(), DearModdingUI::FindPhosphorSlugGlyphOrZero("user")))
			{
				dmui::ui::Separator();
				//dmui::ui::SetCursorPosX(posX + metrics.cellPadding.x);
				if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.basket.body.playerinfo", 2))
				{
					dmui::ui::TableNextRow();
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextColored(theme.info, std::format("{}|{}:", lsBasketPageCaps.GetValue(), lsBasketPagePlayer.GetValue()).c_str());
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextUnformatted(std::format("{}", basket->GetPlayerCaps()).c_str());
					dmui::ui::TableNextRow();
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextColored(theme.info, std::format("{}|{}:", lsBasketPageWeight.GetValue(), lsBasketPagePlayer.GetValue()).c_str());
					(void)dmui::ui::TableNextColumn();
					auto w = basket->GetPlayerWeight();
					dmui::ui::TextUnformatted(std::format("{:.3f}/{:.3f}", w[0], w[1]).c_str());

					dmui::ui::EndTable();
				}
			}
			else
			{
			//	dmui::ui::SetCursorPosX(posX + metrics.cellPadding.x);
				if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.basket.body.playerinfo", 2))
				{
					dmui::ui::TableNextRow();
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextColored(theme.info, std::format("{}:", lsBasketPageCaps.GetValue()).c_str());
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextUnformatted(std::format("{}", basket->GetPlayerCaps()).c_str());
					dmui::ui::TableNextRow();
					(void)dmui::ui::TableNextColumn();
					dmui::ui::TextColored(theme.info, std::format("{}:", lsBasketPageWeight.GetValue()).c_str());
					(void)dmui::ui::TableNextColumn();
					auto w = basket->GetPlayerWeight();
					dmui::ui::TextUnformatted(std::format("{:.3f}/{:.3f}", w[0], w[1]).c_str());

					dmui::ui::EndTable();
				}
			}

			dmui::ui::Spacing();
			dmui::ui::Separator();
			dmui::ui::Dummy({ 1.f, 4.f });

			if (basket->IsGoodState())
			{
				if (dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.basket.body.buttons.takefree",
					lsBasketPageTakeFree.GetValue()).c_str()))
					if (!meBasket::GetSingleton()->RunOp(meBasket::TOperations::kTake))
						dmui::ui::OpenPopup(failed_id1);
				dmui::ui::SameLine();
				dmui::ui::SetCursorPosX(dmui::ui::GetCursorPosX() + 8.0f);
				if (dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.basket.body.buttons.buy",
					lsBasketPageBuy.GetValue()).c_str()))
					if (!meBasket::GetSingleton()->RunOp(meBasket::TOperations::kBuy))
						dmui::ui::OpenPopup(failed_id2);
				dmui::ui::SameLine();
				dmui::ui::SetCursorPosX(dmui::ui::GetCursorPosX() + 8.0f);
				if (dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.basket.body.buttons.clear",
					lsBasketPageClear.GetValue()).c_str()))
					meBasket::GetSingleton()->Clear();
			}
			else
			{
				dmui::ui::PushStyleColor(dmui::ui::Color::kText, theme.error);
				dmui::ui::TextWrapped(lsBasketPageNoGoodState);
				dmui::ui::PopStyleColor();
			}
		}

		dmui::ui::PopStyleVar(2);
		dmui::ui::EndTable();
	}
}

void meDMUIPageBasket::DoRequestUpdate()
{
	meBasket::GetSingleton()->Clear();
}

void meDMUIPageBasket::DoDrawPopupFailed_1() noexcept
{
	if (dmui::ui::BeginPopup(failed_id1))
	{
		dmui::ui::PushTextWrapPos(dmui::ui::GetFontSize() * 35.f);
		dmui::ui::TextColored(theme.error, "FAILED");
		dmui::ui::PopTextWrapPos();
		dmui::ui::Separator();
		if (dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.basket.popup.failed", "tttt").c_str()))
		{
			openedPopups[0] = false;
			dmui::ui::CloseCurrentPopup();
		}

		dmui::ui::EndPopup();
	}
}

void meDMUIPageBasket::DoDrawPopupFailed_2() noexcept
{
	static bool f = true;
	if (dmui::ui::BeginPopupModal(failed_id2, f))
	{
		dmui::ui::PushTextWrapPos(dmui::ui::GetFontSize() * 35.f);
		dmui::ui::TextColored(theme.error, "FAILED");
		dmui::ui::PopTextWrapPos();
		dmui::ui::Separator();
		/*if (dmui::ui::Button(std::format("{}##dearmodding.modexplorer.page.basket.popup.failed", "tttt").c_str()))
		{
			openedPopups[0] = false;
			dmui::ui::CloseCurrentPopup();
		}*/

		dmui::ui::EndPopup();
	}
}

meDMUIPageBasket::meDMUIPageBasket(const std::shared_ptr<dmui::Client>& a_client) :
	meDMUIPageBased(a_client)
{
	using namespace std::literals;

	textureStar = dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_STAR, "DDS");
	textureBasket = dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_BASKET, "DDS");
	if (!textureStar || !textureBasket)
		REX::ERROR("PME::meDMUIPageExplorer() failed load assets"sv);
}

int32_t meDMUIPageBasket::GetSelectedItemId() const noexcept
{
	return selectedItemId.load();
}

void meDMUIPageBasket::SetSelectedItemId(int32_t a_idx) noexcept
{
	auto basket = meBasket::GetSingleton();

	if (basket->GetCount() >= static_cast<uint32_t>(a_idx))
		selectedItemId.store(a_idx < 0 ? -1 : a_idx);
	else
		selectedItemId.store(-1);
}
