#include "mePreviewModelSystem.h"

#include <RE/RTTI.h>
#include <RE/IDs_RTTI.h>
#include <RE/H/hkRefPtr.h>
#include <RE/P/PlayerCamera.h>
#include <RE/T/TESModel.h>
#include <RE/B/BSModelDB.h>
#include <RE/B/BSFadeNode.h>
#include <RE/S/ShadowSceneNode.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/U/UI_DEPTH_PRIORITY.h>



using namespace std::literals;

namespace ID
{
	namespace Inventory3DManager
	{
		REL::ID ctor{ 2249082 };
		REL::ID dtor{ 2249083 };

		REL::ID LoadInventoryItem2{ 2249088 };
	}

	namespace NiObject
	{
		REL::ID Clone{ 2269854 };
	}
}

namespace
{
	RE::NiPointer<RE::NiObject> CloneObject(RE::NiPointer<RE::NiNode>& a_model) noexcept
	{
		using func_t = decltype(&CloneObject);
		static REL::Relocation<func_t> func{ ID::NiObject::Clone };
		return func(a_model);
	}
}

Inventory3DManager* Inventory3DManager::Create() noexcept
{
	auto result = reinterpret_cast<Inventory3DManager*>(RE::malloc(sizeof(Inventory3DManager)));
	if (result)
	{
		using func_t = decltype(&Inventory3DManager::Release);	// just need only this for arg
		static REL::Relocation<func_t> func{ ID::Inventory3DManager::ctor };
		func(result);
	}

	return result;
}

void Inventory3DManager::Release(Inventory3DManager* a_mgr) noexcept
{
	using func_t = decltype(&Inventory3DManager::Release);
	static REL::Relocation<func_t> func{ ID::Inventory3DManager::dtor };
	func(a_mgr);
}

void Inventory3DManager::LoadInventoryItem(RE::TESForm* a_form, RE::ExtraDataList* a_extras, std::uint32_t a_unk) noexcept
{
	using func_t = decltype(&Inventory3DManager::LoadInventoryItem);
	static REL::Relocation<func_t> func{ ID::Inventory3DManager::LoadInventoryItem2 };
	func(this, a_form, a_extras, a_unk);
}


/////////////////////////////////////////////////////////////////////////////////////////////

void meRenderer::ApplyCameraFOV(RE::NiCamera* a_camera, float a_configFOV, float a_aspectOverride) noexcept
{
	constexpr auto M_PI = 3.14159265358979323846f;

	if (!a_camera)
		return;

	const auto currentWidth = a_camera->viewFrustum.right - a_camera->viewFrustum.left;
	const auto currentHeight = a_camera->viewFrustum.top - a_camera->viewFrustum.bottom;
	if (std::abs(currentHeight) <= .0001f)
		return;

	const auto aspect = (a_aspectOverride > 0.f) ?
		a_aspectOverride :
		std::abs(currentWidth) > .0001f ?
		std::abs(currentWidth / currentHeight) :
		kFallbackCameraAspect;
	const auto top = std::tanf((a_configFOV * M_PI / 180.f) * .15f);
	const auto right = top * aspect;

	a_camera->viewFrustum.left = -right;
	a_camera->viewFrustum.right = right;
	a_camera->viewFrustum.top = top;
	a_camera->viewFrustum.bottom = -top;
	a_camera->viewFrustum.ortho = false;
	if (a_camera->viewFrustum.near > 0.f)
		a_camera->maxFarNearRatio = a_camera->viewFrustum.far / a_camera->viewFrustum.near;
}

meRenderer::meRenderer()
{
}

meRenderer::~meRenderer()
{
}

float meRenderer::GetRenderWindowAspect() const noexcept
{
	const auto* rendererData = RE::BSGraphics::GetRendererData();
	const auto& window = rendererData->renderWindow[0];
	if (window.windowWidth > 0 && window.windowHeight > 0)
		return static_cast<float>(window.windowWidth) / static_cast<float>(window.windowHeight);
	return kFallbackCameraAspect;
}

bool meRenderer::Init() noexcept
{
	if (!renderer)
	{
		auto camera = RE::PlayerCamera::GetSingleton();
		if (!camera)
		{
			REX::ERROR("mePreviewModelSystem::Init return failed. Reason PlayerCamera no init.");
			return false;
		}		

		renderer = RE::Interface3D::Renderer::Create(RENDER_NAME, RE::UI_DEPTH_PRIORITY::kStandard3DModel, camera->worldFOV, false);
		if (!renderer)
		{
			REX::ERROR("mePreviewModelSystem::Init return failed when created own renderer");
			return false;
		}

		//REX::INFO("Interface3D renderer {}: renderer={:X}", !renderer ? "reused" : "created", &renderer);

		renderer->MainScreen_SetBackgroundMode(RE::Interface3D::BackgroundMode::kLive);
		renderer->useFullPremultAlpha = true;
		ApplyRendererFOV(camera->worldFOV);
		
	}

	return false;
}

void meRenderer::ApplyRendererFOV(float a_configFOV) noexcept
{
	if (!renderer)
		return;

	// IDA: Interface3D::Renderer::SetupCamera computes top = tan(DEG_TO_RAD * fov * 0.15).
	// Renderer::Create only applies this while constructing new cameras, so reused named renderers
	// need the frustum refreshed when config is reloaded.
	const auto nativeAspect = GetRenderWindowAspect();
	ApplyCameraFOV(renderer->pipboyAspect.get(), a_configFOV);
	ApplyCameraFOV(renderer->nativeAspect.get(), a_configFOV, nativeAspect);
	ApplyCameraFOV(renderer->nativeAspectLongRange.get(), a_configFOV, nativeAspect);
}







bool mePreviewModelSystem::LoadModel() noexcept
{
	Inventory3Dmgr->ClearModel();
	Inventory3Dmgr->LoadInventoryItem(currentForm.load(), nullptr, 0);

	/*if (loadedModel)
		loadedModel->DeleteThis();

	std::string path = "";
	auto cform = currentForm.load();

	switch (cform->formType.get())
	{
	case RE::ENUM_FORM_ID::kARMO:
	{
		auto form = reinterpret_cast<RE::TESObjectARMO*>(cform);
		auto pathModel = form->worldModel[0].GetModel();
		if (pathModel) path = pathModel;
	}
	break;
	case RE::ENUM_FORM_ID::kWEAP:
	{
		auto form = reinterpret_cast<RE::TESObjectWEAP*>(cform);
		auto pathModel = form->GetModel();
		if (pathModel) path = pathModel;
	}
	break;
	default:
		auto form = reinterpret_cast<RE::TESModel*>(RE::RTDynamicCast((void*)cform, 0,
			(void*)RE::RTTI::TESForm.address(), (void*)RE::RTTI::TESModel.address(), 0));
		if (form)
		{
			auto pathModel = form->GetModel();
			if (pathModel) path = pathModel;
		}
		break;
	}

	if (path.empty() || !path[0])
	{
		REX::ERROR("mePreviewModelSystem::LoadModel return failed, no path of model");
		return false;
	}

	REX::DEBUG("mePreviewModelSystem::LoadModel path model {}"sv, path);

	RE::BSModelDB::DBTraits::ArgsType args{};
	args.loadTextures = true;
	args.createFadeNode = true;

	auto result = RE::BSModelDB::Demand(path.c_str(), &loadedModel, args);
	REX::DEBUG("mePreviewModelSystem::LoadModel result {}"sv, std::to_underlying(result));

	if ((result != RE::BSResource::ErrorCode::kNone) || !loadedModel)
	{
		REX::ERROR("mePreviewModelSystem::LoadModel return failed, BSModelDB::Demand return error {}, path of model \"{}\"",
			std::to_underlying(result), path);
		return false;
	}

	*/
	return true;
}

bool mePreviewModelSystem::DrawModel() noexcept
{
	if (!renderer)
	{
		renderer = RE::Interface3D::Renderer::Create("ModExplorer_Preview3D"sv, RE::UI_DEPTH_PRIORITY::kStandard3DModel, .0f, 0);
		if (!renderer)
		{
			REX::ERROR("mePreviewModelSystem::DrawModel return failed when created own renderer");
			return false;
		}
	}

	renderer->customRenderTarget = 1;
	renderer->MainScreen_SetBackgroundMode(RE::Interface3D::BackgroundMode::kLive);
	renderer->Offscreen_SetRenderTargetSize(RE::Interface3D::OffscreenMenuSize::kFullFrame);
	renderer->MainScreen_EnableScreenAttached3DMasking("HUDShadow", nullptr);
	renderer->Offscreen_SetPostEffect(RE::Interface3D::PostEffect::kNone);
	renderer->clearColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	renderer->Enable(true);
	renderer->Offscreen_Clear3D();

	Inventory3Dmgr->EnableRendering("ModExplorer_Preview3D"sv);
	Inventory3Dmgr->Begin3D();

	


	//Inventory3Dmgr->End3D();
	//Inventory3Dmgr->DisableRendering("ModExplorer_Preview3D"sv);

	


	//
	//if (!camera)
	//	REX::ERROR("mePreviewModelSystem::Init failed get camera"sv);
	//else
	//{
	//	renderer = RE::Interface3D::Renderer::Create("ModExplorer_Preview", RE::UI_DEPTH_PRIORITY::kStandard3DModel, camera->worldFOV, true);
	//	if (!renderer)
	//		REX::ERROR("mePreviewModelSystem::Init failed create custom renderer"sv);
	//	else
	//	{
	//		renderer->clearColor = { .0f, 1.0f, .0f, 0.5f, };
	//	}
	//}

	/*auto clone = CloneObject(loadedModel);
	if (!clone)
	{
		REX::ERROR("mePreviewModelSystem::DrawModel return failed when created clone model");
		return false;
	}*/


//
////	auto node = clone->IsFadeNode();
////
//	
//	//renderer->MainScreen_SetWorldAttached3D(node);
//	//renderer->MainScreen_RegisterGeometryRequiringFullViewport(node);
//	renderer->Enable(true);
//
//	REX::INFO("RenderTarget: {}x{} id {}", renderer->Offscreen_GetRenderTargetWidth(), renderer->Offscreen_GetRenderTargetHeight(),
//		renderer->customRenderTarget);
//
//	//node->local.scale = 1.0f;
//	//auto& position = node->local.translate;
//	//position.x = 0.f;
//	//position.y = 375.f;
//	//position.z = 0.f;
//
//	//RE::NiUpdateData updateData{};
//	//auto result = SafeDrawModel(node, updateData);
//	//if (!result)
//	//	REX::ERROR("mePreviewModelSystem::DrawModel return failed when update model");
//
//	renderer->Release();

	return true;
}

bool mePreviewModelSystem::SafeDrawModel(RE::NiNode* a_node, RE::NiUpdateData& a_data)
{
	__try
	{
		a_node->Update(a_data);
		Inventory3Dmgr->Begin3D();





		Inventory3Dmgr->End3D();

		return true;
	}
	__except (1)
	{
		return false;
	}
}

mePreviewModelSystem::~mePreviewModelSystem()
{
	Shutdown();
}

bool mePreviewModelSystem::Init() noexcept
{
	/*Inventory3Dmgr = Inventory3DManager::Create();
	if (!Inventory3Dmgr)
	{
		REX::ERROR("mePreviewModelSystem::Init return failed when created Inventory3DManager");
		return false;
	}*/

	return IsInit();
}

void mePreviewModelSystem::Shutdown() noexcept
{
	while (busy.load())
	{}

	terminated.store(true);

	if (Inventory3Dmgr)
	{
		Inventory3DManager::Release(Inventory3Dmgr);
		Inventory3Dmgr = nullptr;
	}

	terminated.store(false);
}

bool mePreviewModelSystem::Renderer(RE::TESForm* a_form) noexcept
{
	if (terminated.load() || !IsInit() || !a_form || !a_form->IsBoundObject() || busy.load() || (currentForm.load() == a_form))
		return false;

	busy.store(true);
	currentForm.store(a_form);

	if (!LoadModel())
	{
		REX::ERROR("mePreviewModelSystem::Renderer return failed when loaded a model 0x{:X}", static_cast<std::uint32_t>(a_form->formID));
		return false;
	}

	if (!DrawModel())
	{
		REX::ERROR("mePreviewModelSystem::Renderer return failed when drew a model 0x{:X}", static_cast<std::uint32_t>(a_form->formID));
		return false;
	}

	return busy.exchange(false);
}