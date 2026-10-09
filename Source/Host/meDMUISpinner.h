#pragma once

#include <DearModdingUI/Client.h>
#include "meDMUIUtils.h"

class meDMUISpinner
{
	float speed = 5.6f;
	float radius = 40.f;
	uint32_t bars = 4;
	DmVec4 color = { 1.f, 1.f, 1.f, 1.f };
	std::shared_ptr<dmui::Client> client{};

	meDMUISpinner(const meDMUISpinner&) = delete;
	meDMUISpinner(meDMUISpinner&&) = delete;
	meDMUISpinner& operator=(const meDMUISpinner&) = delete;
	meDMUISpinner& operator=(meDMUISpinner&&) = delete;
public:
	meDMUISpinner() = default;
	~meDMUISpinner() = default;

	void Draw() const noexcept;

	[[nodiscard]] uint32_t GetBars() const noexcept;
	void SetBars(uint32_t a_bars) noexcept;
	[[nodiscard]] float GetSpeed() const noexcept;
	void SetSpeed(float a_speed) noexcept;
	[[nodiscard]] float GetRadius() const noexcept;
	void SetRadius(float a_radius) noexcept;
	[[nodiscard]] uint32_t GetColorU32() const noexcept;
	void SetColorU32(uint32_t a_color) noexcept;
	[[nodiscard]] DmVec4 GetColor() const noexcept;
	void SetColor(const DmVec4& a_color) noexcept;
};