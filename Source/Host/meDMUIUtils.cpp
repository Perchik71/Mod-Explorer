#include "meDMUIUtils.h"
#include "meHostLocalizeString.h"

DmVec4 DMUI_ColorConvertU32ToFloat4(DmU32 in) noexcept
{
	constexpr static float s = 1.0f / 255.0f;
	return DmVec4(
		((in >> DMUI_COL32_R_SHIFT) & 0xFF) * s,
		((in >> DMUI_COL32_G_SHIFT) & 0xFF) * s,
		((in >> DMUI_COL32_B_SHIFT) & 0xFF) * s,
		((in >> DMUI_COL32_A_SHIFT) & 0xFF) * s);
}

DmU32 DMUI_ColorConvertFloat4ToU32(const DmVec4& in) noexcept
{
	DmU32 out;
	out  = (static_cast<DmU32>(DMUI_F32_TO_INT8_SAT(in.x))) << DMUI_COL32_R_SHIFT;
	out |= (static_cast<DmU32>(DMUI_F32_TO_INT8_SAT(in.y))) << DMUI_COL32_G_SHIFT;
	out |= (static_cast<DmU32>(DMUI_F32_TO_INT8_SAT(in.z))) << DMUI_COL32_B_SHIFT;
	out |= (static_cast<DmU32>(DMUI_F32_TO_INT8_SAT(in.w))) << DMUI_COL32_A_SHIFT;
	return out;
}

void DMUI_Image(dmui::Client* a_client, const std::shared_ptr<dmui::Texture> a_texture, float a_w, float a_h) noexcept
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

void DMUI_QuotteRect(DmVec2 a_min, DmVec2 a_max) noexcept
{
	dmui::ui::WindowDrawList().AddRect(a_min, a_max, DMUI_COL32(255, 255, 255, 114), 4.f);
	dmui::ui::WindowDrawList().AddRectFilled(a_min, a_max, DMUI_COL32(255, 255, 255, 24), 4.f);
}

void DMUI_SetItemTooltip(dmui::Client* a_client, const std::string_view& a_desc, bool a_warning) noexcept
{
	if (dmui::ui::BeginItemTooltip())
	{
		auto themeOptional = a_client->GetThemeColors();
		if (a_warning)
			dmui::ui::TextColored(themeOptional.has_value() ? themeOptional.value().error : DmVec4{ 1.f, .3f, .3f, 1.f },
				std::format("{}:", lsWarning.GetValue()).c_str());

		dmui::ui::PushTextWrapPos(dmui::ui::GetFontSize() * 35.0f);
		dmui::ui::TextUnformatted(a_desc);
		dmui::ui::PopTextWrapPos();
		dmui::ui::EndTooltip();
	}
}

double GetTime() noexcept
{
	// Get the duration elapsed since the clock's epoch
	auto duration_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
	// Convert that duration to a double in seconds
	std::chrono::duration<double> seconds_duration = duration_since_epoch;
	// Extract the double value
	return seconds_duration.count();
}
