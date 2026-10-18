#include "mePreviewModelSystem.h"

#include <RE/RTTI.h>
#include <RE/IDs_RTTI.h>
#include <RE/N/NiUpdateData.h>
#include <RE/B/BSGraphics.h>
#include <RE/H/hkRefPtr.h>
#include <RE/P/PlayerCamera.h>
#include <RE/T/TESModel.h>
#include <RE/B/BSModelDB.h>
#include <RE/S/ShadowSceneNode.h>
#include <RE/T/TESObjectARMO.h>
#include <RE/T/TESObjectWEAP.h>
#include <RE/U/UI_DEPTH_PRIORITY.h>

using namespace std::literals;

mePreviewModelSystem::~mePreviewModelSystem()
{
	Shutdown();
}

void mePreviewModelSystem::Init() noexcept
{
	//auto camera = RE::PlayerCamera::GetSingleton();
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
}

void mePreviewModelSystem::Shutdown() noexcept
{
	/*if (renderer)
	{
		renderer->Release();
		renderer = nullptr;
	}*/
}

bool mePreviewModelSystem::LoadModel(RE::TESForm* a_form) noexcept
{
	if (!a_form || !a_form->IsBoundObject())
		return false;

	if (currentForm == a_form)
		return true;

	done.store(false);

	if (loadedModel)
		loadedModel->DeleteThis();

	std::string path = "";

	switch (a_form->formType.get())
	{
	case RE::ENUM_FORM_ID::kARMO:
		{
		auto form = reinterpret_cast<RE::TESObjectARMO*>(a_form);
		auto pathModel = form->worldModel[0].GetModel();
		if (pathModel) path = pathModel;
		}
		break;
	case RE::ENUM_FORM_ID::kWEAP:
		{
		auto form = reinterpret_cast<RE::TESObjectWEAP*>(a_form);
		auto pathModel = form->GetModel();
		if (pathModel) path = pathModel;
		}
		break;
	default:
		auto form = reinterpret_cast<RE::TESModel*>(RE::RTDynamicCast((void*)a_form, 0,
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
		done.store(true);
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
		done.store(true);
		return false;
	}
	
	// Update local transform and bounding spheres
	RE::NiUpdateData updateData;
	//updateData.time = 0.0f;
	//updateData.flags = 0;

	// Apply orientation adjustments so the model faces the camera
	//loadedModel->local.scale = 1.0f;
	//loadedModel->local.translate = RE::NiPoint3(0.0f, 100.0f, 0.0f); // Move forward 

//	loadedModel->UpdateDownwardPass(updateData, 0);

	auto renderer = RE::Interface3D::Renderer::GetByName("PipboyRenderer");

//	renderer->Offscreen_Clear3D();
	renderer->Offscreen_Enable3D(true);
	renderer->Offscreen_Set3D(loadedModel.get());

	//auto renderTargetManager = RE::BSGraphics::RenderTargetManager::GetSingleton();
	//renderTargetManager->GetRenderTarget(0).rtView
	

	//renderer->Disable();
	//renderer->Offscreen_Clear3D();
	//renderer->Offscreen_Set3D(loadedModel.get());
	//renderer->RenderPrepassesAndMenus();

	//
	//if (renderTargetManager)
	//{
	//	//renderTargetManager->GetRenderTarget(0);
	//	//renderTargetManager->
	//}

	//renderer->Enable();

	done.store(true);
	return true;
}