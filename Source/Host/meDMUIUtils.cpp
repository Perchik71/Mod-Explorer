#include "meDMUIUtils.h"

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

double GetTime() noexcept
{
	// Get the duration elapsed since the clock's epoch
	auto duration_since_epoch = std::chrono::steady_clock::now().time_since_epoch();
	// Convert that duration to a double in seconds
	std::chrono::duration<double> seconds_duration = duration_since_epoch;
	// Extract the double value
	return seconds_duration.count();
}
