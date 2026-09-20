#pragma once

#include <REX/REX.h>
#include <DearModdingUI/Client.h>

#include <memory>

class meDMUIClient :
	public REX::TSingleton<meDMUIClient>
{
	static constexpr const auto kClientId			= "dearmodding.modexplorer";
	static constexpr const auto kClientDisplayName	= "Mod Explorer";
	static constexpr const auto kClientIcon			= "coffee";

	static constexpr const auto kCategoryGeneralId	= "dearmodding.modexplorer.category.general";
	static constexpr const auto kCategoryCheatsId	= "dearmodding.modexplorer.category.cheats";

	static constexpr const auto kPageGeneralId = "dearmodding.modexplorer.page.general";
	static constexpr const auto kPageExplorerId = "dearmodding.modexplorer.page.explorer";
	static constexpr const auto kPageBasketId = "dearmodding.modexplorer.page.basket";

	std::unique_ptr<dmui::Client> client{};

	static void RendererGeneralPage() noexcept;
	static void RendererExplorerPage() noexcept;
	static void RendererBasketPage() noexcept;
public:
	meDMUIClient() = default;

	bool Connect() noexcept;
};