#include "meDMUIPageGeneralInfo.h"
#include "meHostLocalizeString.h"
#include "../../Resources/resource.h"

void meDMUIPageGeneralInfo::DoDrawPage()
{
	auto dataStorage = meDataStorage::GetSingleton();

	if (dmui::ui::BeginTable("##dearmodding.modexplorer.page.general.body", 2))
	{
		dmui::ui::TableNextRow();
		dmui::ui::PushStyleColor(dmui::ui::Color::kText, { .9f, .9f, .9f, 1.f });

		if (dmui::ui::TableNextColumn())
		{
			{
				dmui::FontGuard font{ *GetClient(), DMUI_FONT_ROLE_TITLE};
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
					sizeOrderColumn.x + dmui::ui::GetFontSize() * 0.25f);
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
							auto is_selected = (selectedPluginId.load() == row);

							if (is_selected)
								dmui::ui::PushStyleColor(dmui::ui::Color::kHeaderHovered, colorSelectedRow);

							dmui::ui::TableNextRow();

							if (dmui::ui::TableNextColumn())
							{
								char label[96];
								sprintf_s(label, "##dearmodding.modexplorer.page.general.body.plugins.row.%d", row);
								if (dmui::ui::Selectable(label, is_selected, dmui::ui::SelectableFlags::kSpanAllColumns))
									selectedPluginId.store(row);

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

				auto sel = selectedPluginId.load();
				auto fontSize = dmui::ui::CalcTextSize("A");
				auto plugin = sel == -1 ? nullptr : dataStorage->GetModByIndex(sel).get();

				{
					dmui::FontGuard font{ *GetClient(), DMUI_FONT_ROLE_TITLE };
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

						const float row_size = dmui::ui::GetFontSize() * 2.29f;

						for (auto itemType : meItemTypes)
						{
							dmui::ui::TableNextRow(dmui::ui::TableRowFlags::kNone, row_size);
							if (dmui::ui::TableNextColumn())
							{
								switch (itemType)
								{
								case meItemType::kArmorItem:
									DMUI_Image(GetClient(), textureArmor, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsArmor);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kBookItem:
									DMUI_Image(GetClient(), textureBook, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsBook);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kMiscItem:
									DMUI_Image(GetClient(), textureMisc, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsMisc);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kWeaponItem:
									DMUI_Image(GetClient(), textureWeapon, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsWeapon);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kAmmoItem:
									DMUI_Image(GetClient(), textureAmmo, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsAmmo);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kKeyItem:
									DMUI_Image(GetClient(), textureKey, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsKey);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kAlchemyItem:
									DMUI_Image(GetClient(), textureAlchemy, row_size, row_size);
									if (dmui::ui::BeginItemTooltip())
									{
										dmui::ui::Text(lsAlchemy);
										dmui::ui::EndTooltip();
									}
									break;
								case meItemType::kNoteItem:
									DMUI_Image(GetClient(), textureNote, row_size, row_size);
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

void meDMUIPageGeneralInfo::DoRequestUpdate()
{
	selectedPluginId.store(-1);
}

meDMUIPageGeneralInfo::meDMUIPageGeneralInfo(const std::shared_ptr<dmui::Client>& a_client) :
	meDMUIPageBased(a_client)
{
	using namespace std::literals;

	textureArmor	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_ARMOR,	"DDS");
	textureBook		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_BOOK,	"DDS");
	textureMisc		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_MISC,	"DDS");
	textureWeapon	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_WEAPON,	"DDS");
	textureAmmo		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_AMMO,	"DDS");
	textureKey		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_KEY,		"DDS");
	textureAlchemy	= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_ALCHEMY, "DDS");
	textureNote		= dmui::TextureLoader::LoadFromResource(dmui::TextureLoader::Format::kDDS, IDB_NOTE,	"DDS");

	if (!textureArmor || !textureNote || !textureBook || !textureMisc || !textureWeapon ||
		!textureAmmo || !textureKey || !textureAlchemy)
		REX::ERROR("PME::meDMUIPageGeneralInfo() failed load assets"sv);
}

int32_t meDMUIPageGeneralInfo::GetSelectedPluginId() const noexcept
{
	return selectedPluginId.load();
}

void meDMUIPageGeneralInfo::SetSelectedPluginId(int32_t a_idx) noexcept
{
	if (a_idx < 0)
	{
		selectedPluginId.store(-1);
		return;
	}

	auto dataStorage = meDataStorage::GetSingleton();
	const auto size = dataStorage->GetModCount();
	if (!size)
	{
		selectedPluginId.store(-1);
		return;
	}

	if (size <= static_cast<size_t>(a_idx))
		a_idx = static_cast<int32_t>(size) - 1;

	selectedPluginId.store(a_idx);
}