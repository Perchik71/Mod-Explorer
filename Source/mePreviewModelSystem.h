#pragma once

#include <REX/REX.h>

#include <RE/N/NiPoint4.h>
#include <RE/N/NiCamera.h>
#include <RE/N/NiPointer.h>
#include <RE/N/NiNode.h>
#include <RE/N/NiUpdateData.h>

#include <RE/T/TESForm.h>

#include <RE/E/ExtraDataList.h>


#include <RE/I/Interface3D.h>
#include <RE/B/BSGraphics.h>
#include <RE/I/Inventory3DManager.h>

#include <atomic>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_SIZE

class Inventory3DManager :
	public RE::Inventory3DManager
{
public:
	static Inventory3DManager* Create() noexcept;
	static void Release(Inventory3DManager* a_mgr) noexcept;

	void LoadInventoryItem(RE::TESForm* a_form, RE::ExtraDataList* a_extras, std::uint32_t a_unk) noexcept;
};

class meRenderer :
	public REX::TSingleton<meRenderer>
{
private:
	const float kFallbackCameraAspect = 16.f / 9.f;
	const std::string_view RENDER_NAME = "ModExplorer_Preview3D";
	
	RE::Interface3D::Renderer* renderer{ nullptr };

	



	RE::NiPointer<RE::NiAVObject> displayRoot{};
	RE::NiPoint4 appliedDisplayBounds{};

	void ApplyCameraFOV(RE::NiCamera* a_camera, float a_configFOV, float a_aspectOverride = 0.0f) noexcept;

public:
	meRenderer();
	~meRenderer();

	[[nodiscard]] float GetRenderWindowAspect() const noexcept;
	

	[[nodiscard]] bool Init() noexcept;
	void ApplyRendererFOV(float a_configFOV) noexcept;
};



class mePreviewModelSystem :
	public REX::TSingleton<mePreviewModelSystem>
{
	std::atomic_bool busy{ false }, terminated{ false };
	Inventory3DManager* Inventory3Dmgr{ nullptr };

	RE::NiPointer<RE::NiNode> loadedModel{ nullptr };
	RE::Interface3D::Renderer* renderer{ nullptr };
	std::atomic<RE::TESForm*> currentForm{ nullptr };
	

	[[nodiscard]] bool LoadModel() noexcept;
	[[nodiscard]] bool DrawModel() noexcept;
	[[nodiscard]] bool SafeDrawModel(RE::NiNode* a_node, RE::NiUpdateData& a_data);
public:
	mePreviewModelSystem() = default;
	~mePreviewModelSystem();

	bool Init() noexcept;
	void Shutdown() noexcept;
	constexpr bool IsInit() const noexcept { return Inventory3Dmgr; }

	bool Renderer(RE::TESForm* a_form) noexcept;
};