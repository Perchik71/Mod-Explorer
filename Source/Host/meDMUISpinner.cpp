#include "meDMUISpinner.h"

#include <atomic>
#include <numbers>
#include <cmath>

void meDMUISpinner::Draw() const noexcept
{
	auto wndRect = dmui::ui::GetContentRegionAvail();
	auto posScreen = dmui::ui::GetCursorScreenPos();

	DMUI_StyleMetrics metrics{};
	(void)dmui::ui::GetStyleMetrics(metrics);

	// test
	//dmui::ui::WindowDrawList().AddRect(posScreen, posScreen + wndRect, 0xFFFFFFFF);

	const float w = (radius * 2) * bars;
	const float nextItemKoeff = 1.5f;
	const float yOffsetKoeftt = .8f;
	const float heightSpeed = .8f;
	const float width = w / (bars * nextItemKoeff);
	const float start = static_cast<float>(GetTime()) * speed;
	const float offset = static_cast<float>(std::numbers::pi) / bars;
	const float bar_p = width * .5f;
	const float y_centre = w * .5f;

	DmVec2 size{ w, w };
	DmVec2 off{ (wndRect.x - w) * .5f, (wndRect.y - w) * .5f };
	DmVec2 pos = posScreen + off;

	//dmui::ui::WindowDrawList().AddRectFilled(pos, pos + size, 0xFF00FFFF);

	for (size_t i = 0; i < bars; i++)
	{
		float a = start + (static_cast<float>(std::numbers::pi) - i * offset);
		DmVec4 c = { color.x, color.y, color.z, std::max(.1f, std::sinf(a * heightSpeed)) };
		float h = (.6f + .4f * c.w) * size.y * .5f;

		dmui::ui::WindowDrawList().AddRectFilled(
			{ pos.x + metrics.framePadding.x + bar_p + i * (width * nextItemKoeff) - bar_p, pos.y + (y_centre - h * yOffsetKoeftt) },
			{ pos.x + metrics.framePadding.x + bar_p + i * (width * nextItemKoeff) + bar_p, pos.y + (y_centre + h * yOffsetKoeftt) },
			DMUI_ColorConvertFloat4ToU32(c));
	}
}

uint32_t meDMUISpinner::GetBars() const noexcept
{
	return bars;
}

void meDMUISpinner::SetBars(uint32_t a_bars) noexcept
{
	bars = a_bars;
}

float meDMUISpinner::GetSpeed() const noexcept
{
	return speed;
}

void meDMUISpinner::SetSpeed(float a_speed) noexcept
{
	speed = a_speed;
}

float meDMUISpinner::GetRadius() const noexcept
{
	return radius;
}

void meDMUISpinner::SetRadius(float a_radius) noexcept
{
	radius = a_radius;
}

uint32_t meDMUISpinner::GetColorU32() const noexcept
{
	return DMUI_ColorConvertFloat4ToU32(color);
}

void meDMUISpinner::SetColorU32(uint32_t a_color) noexcept
{
	color = DMUI_ColorConvertU32ToFloat4(a_color);
}

DmVec4 meDMUISpinner::GetColor() const noexcept
{
	return color;
}

void meDMUISpinner::SetColor(const DmVec4& a_color) noexcept
{
	color = a_color;
}