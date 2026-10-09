#pragma once

#include <DearModdingUI/Client.h>
#include "TextureLoader.h"

using DmU8		= uint8_t;
using DmU16		= uint16_t;
using DmU32		= uint32_t;
using DmVec2	= dmui::ui::Vec2;
using DmVec4	= dmui::ui::Vec4;

#ifndef DMUI_COL32_R_SHIFT
#define DMUI_COL32_R_SHIFT	24
#define DMUI_COL32_G_SHIFT	16
#define DMUI_COL32_B_SHIFT	8
#define DMUI_COL32_A_SHIFT	0
#endif // !DMUI_COL32_R_SHIFT

[[nodiscard]] constexpr static float DMUI_Saturate(float f) noexcept
{
	return (f < .0f) ? .0f : (f > 1.f) ? 1.f : f;
}

#ifndef DMUI_COL32
#define DMUI_COL32(R,G,B,A)			((static_cast<DmU32>(A)<<DMUI_COL32_A_SHIFT) | (static_cast<DmU32>(B)<<DMUI_COL32_B_SHIFT) | (static_cast<DmU32>(G)<<DMUI_COL32_G_SHIFT) | (static_cast<DmU32>(R)<<DMUI_COL32_R_SHIFT))
#define DMUI_COL32_WHITE			DMUI_COL32(255,255,255,255)	// Opaque white = 0xFFFFFFFF
#define DMUI_COL32_BLACK			DMUI_COL32(0,0,0,255)		// Opaque black
#define DMUI_COL32_BLACK_TRANS		DMUI_COL32(0,0,0,0)			// Transparent black = 0x00000000
#define DMUI_F32_TO_INT8_SAT(_VAL)	((int)(DMUI_Saturate(_VAL) * 255.0f + 0.5f))
#endif // !DMUI_COL32

[[nodiscard]] DmVec4 DMUI_ColorConvertU32ToFloat4(DmU32 in) noexcept;
[[nodiscard]] DmU32 DMUI_ColorConvertFloat4ToU32(const DmVec4& in) noexcept;

void DMUI_Image(dmui::Client* a_client, const std::shared_ptr<dmui::Texture> a_texture, float a_w, float a_h) noexcept;

[[nodiscard]] static inline DmVec2 operator+(const DmVec2& lhs, const DmVec2& rhs) noexcept
{
	return DmVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

[[nodiscard]] static inline DmVec2& operator+=(DmVec2& lhs, const DmVec2& rhs) noexcept
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

[[nodiscard]] static inline DmVec2 operator-(const DmVec2& lhs, const DmVec2& rhs) noexcept
{
	return DmVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}

[[nodiscard]] static inline DmVec2& operator-=(DmVec2& lhs, const DmVec2& rhs) noexcept
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

[[nodiscard]] static inline DmVec2 operator*(const DmVec2& lhs, float rhs) noexcept
{
	return DmVec2(lhs.x * rhs, lhs.y * rhs);
}

[[nodiscard]] static inline DmVec2& operator*=(DmVec2& lhs, float rhs) noexcept
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	return lhs;
}

[[nodiscard]] static inline DmVec2 operator/(const DmVec2& lhs, float rhs) noexcept
{
	return DmVec2(lhs.x / rhs, lhs.y / rhs);
}

[[nodiscard]] static inline DmVec2& operator/=(DmVec2& lhs, float rhs) noexcept
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

[[nodiscard]] double GetTime() noexcept;