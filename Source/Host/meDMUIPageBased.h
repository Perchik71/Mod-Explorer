#pragma once

#include "meDMUISpinner.h"

class meDMUIPageBased
{
	std::shared_ptr<dmui::Client> client{};
	std::string lastError{};
	std::atomic_bool update = false;
	std::atomic_bool drawing = false;

	meDMUIPageBased(const meDMUIPageBased&) = delete;
	meDMUIPageBased(meDMUIPageBased&&) = delete;
	meDMUIPageBased& operator=(const meDMUIPageBased&) = delete;
	meDMUIPageBased& operator=(meDMUIPageBased&&) = delete;
protected:
	std::unique_ptr<meDMUISpinner> spinner;
	DMUI_ThemeColors theme{};
	DMUI_StyleMetrics metrics{};

	virtual void DoRequestUpdate() = 0;
	virtual void DoDrawPage() = 0;
	virtual dmui::Client* GetClient() const noexcept;
public:
	using TNotifyEvent = void(const meDMUIPageBased&);

	meDMUIPageBased(const std::shared_ptr<dmui::Client>& a_client);
	~meDMUIPageBased() = default;

	virtual void DrawPage();
	virtual void RequestUpdate();
	virtual void SetUpdateState(bool a_value) noexcept;

	std::function<TNotifyEvent> OnBeginDraw{};
	std::function<TNotifyEvent> OnEndDraw{};
};