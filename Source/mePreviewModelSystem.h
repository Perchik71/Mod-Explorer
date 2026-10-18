#pragma once

#include <REX/REX.h>

#include <RE/T/TESForm.h>
#include <RE/N/NiPointer.h>
#include <RE/N/NiNode.h>
#include <RE/I/Interface3D.h>
#include <RE/B/BSGraphics.h>

#include <atomic>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_SIZE

class mePreviewModelSystem :
	public REX::TSingleton<mePreviewModelSystem>
{
	std::atomic_bool done{ false };
	RE::NiPointer<RE::NiNode> loadedModel{ nullptr };
	RE::Interface3D::Renderer* renderer{ nullptr };
	std::atomic<RE::TESForm*> currentForm{ nullptr };
public:
	mePreviewModelSystem() = default;
	~mePreviewModelSystem();

	void Init() noexcept;
	void Shutdown() noexcept;
	constexpr bool IsInit() const noexcept { return renderer; }

	[[nodiscard]] bool LoadModel(RE::TESForm* a_form) noexcept;
	[[nodiscard]] void Draw(RE::TESForm* a_form) noexcept;
};