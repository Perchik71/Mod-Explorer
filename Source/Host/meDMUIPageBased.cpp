#include "meDMUIPageBased.h"
#include "meDMUIPageManager.h"

dmui::Client* meDMUIPageBased::GetClient() const noexcept
{
	return client.get();
}

meDMUIPageBased::meDMUIPageBased(const std::shared_ptr<dmui::Client>& a_client) :
	client(a_client), spinner(std::make_unique<meDMUISpinner>())
{}

void meDMUIPageBased::DrawPage()
{
	try
	{
		auto themeOptional = client->GetThemeColors();
		if (themeOptional.has_value())
			theme = themeOptional.value();
		(void)dmui::ui::GetStyleMetrics(metrics);
		if (spinner)
		{
			spinner->SetColor(theme.info);
			spinner->SetRadius(dmui::ui::GetFontSize());
		}

		drawing.store(true);

		if (OnBeginDraw)
			OnBeginDraw(*this);

		auto managerPages = meDMUIPageManager::GetSingleton();
		if (!managerPages->IsProcessingUpdate())
			DoDrawPage();
		else if (spinner)
			spinner->Draw();

		if (OnEndDraw)
			OnEndDraw(*this);

		drawing.store(false);
	}
	catch (const std::exception& e)
	{
		lastError = e.what();

		if (OnEndDraw)
			OnEndDraw(*this);

		drawing.store(false);
	}
}

void meDMUIPageBased::RequestUpdate()
{
	try
	{
		SetUpdateState(true);
		while (drawing.load()) { std::this_thread::yield(); }
		DoRequestUpdate();
		SetUpdateState(false);
	}
	catch (const std::exception& e)
	{
		lastError = e.what();
		SetUpdateState(false);
	}
}

void meDMUIPageBased::SetUpdateState(bool a_value) noexcept
{
	update.store(a_value);
}
