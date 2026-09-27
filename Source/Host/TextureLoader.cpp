#include "../mePlugin.h"
#include "TextureLoader.h"

#include <REX/REX.h>
#include <RE/B/BSGraphics.h>

#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"

#include <filesystem>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_SIZE

class DMUI_TextureLoaderHandler
{
	RE::BSGraphics::RendererData* rendererData{ nullptr };
public:
	constexpr DMUI_TextureLoaderHandler() noexcept = default;

	void ResolveRendererData() noexcept
	{
		if (!rendererData)
			rendererData =
			REL::Relocation<RE::BSGraphics::RendererData*>(REL::ID{ 235166, 2704527 }).get();
	}

	inline REX::W32::ID3D11Device* GetD3D11Device() const noexcept
	{
		return rendererData ? rendererData->device : nullptr;
	}

	inline REX::W32::ID3D11DeviceContext* GetD3D11DeviceContext() const noexcept
	{
		return rendererData ? rendererData->context : nullptr;
	}
};

static DMUI_TextureLoaderHandler g_DMUITextureLoaderHandler{};

dmui::Texture::~Texture()
{
	Release();
}

dmui::ImageHandle dmui::Texture::GetHandle() const noexcept
{
	return dmuiImage.has_value() ? dmuiImage.value().Handle() : dmui::ImageHandle();
}

void dmui::Texture::Release()
{
	if (handle)
		reinterpret_cast<ID3D11Resource*>(handle)->Release();
	if (resource)
		reinterpret_cast<ID3D11ShaderResourceView*>(resource)->Release();
}

void dmui::Texture::SendOnceDmuiRequest(Client* a_client) noexcept
{
	std::call_once(once, [&]() {
		if (!a_client) return;
		dmuiImage = a_client->ImportD3D11Image(resource, width, height);
		});
}

bool dmui::TextureLoader::IsValidWicFormat(const std::wstring& a_ext) noexcept
{
	return 
		!_wcsicmp(a_ext.c_str(), L".png") ||
		!_wcsicmp(a_ext.c_str(), L".jpg") || !_wcsicmp(a_ext.c_str(), L".jpeg") ||
		!_wcsicmp(a_ext.c_str(), L".bmp");
}

[[nodiscard]] bool dmui::TextureLoader::SupportedFormat(int32_t a_format) noexcept
{
	switch (static_cast<DXGI_FORMAT>(a_format))
	{
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return true;
	default:
		return false;
	}
}

std::shared_ptr<dmui::Texture> dmui::TextureLoader::LoadFromStream(const void* a_buffer,
	size_t a_bufferSize, Format a_format) noexcept
{
	if (!a_buffer || !a_bufferSize)
		return nullptr;

	auto texture = std::make_shared<Texture>();
	g_DMUITextureLoaderHandler.ResolveRendererData();

	if (a_format == Format::kDDS)
	{
		if (FAILED(DirectX::CreateDDSTextureFromMemory(
			reinterpret_cast<ID3D11Device*>(g_DMUITextureLoaderHandler.GetD3D11Device()),
			reinterpret_cast<ID3D11DeviceContext*>(g_DMUITextureLoaderHandler.GetD3D11DeviceContext()),
			reinterpret_cast<const uint8_t*>(a_buffer),
			a_bufferSize,
			reinterpret_cast<ID3D11Resource**>(std::addressof(texture->handle)),
			reinterpret_cast<ID3D11ShaderResourceView**>(std::addressof(texture->resource)))))
			return nullptr;
	}
	else
	{
		if (FAILED(DirectX::CreateWICTextureFromMemory(
			reinterpret_cast<ID3D11Device*>(g_DMUITextureLoaderHandler.GetD3D11Device()),
			reinterpret_cast<ID3D11DeviceContext*>(g_DMUITextureLoaderHandler.GetD3D11DeviceContext()),
			reinterpret_cast<const uint8_t*>(a_buffer),
			a_bufferSize,
			reinterpret_cast<ID3D11Resource**>(std::addressof(texture->handle)),
			reinterpret_cast<ID3D11ShaderResourceView**>(std::addressof(texture->resource)))))
			return nullptr;
	}

	auto tex = reinterpret_cast<ID3D11Texture2D*>(texture->handle);
	auto res = reinterpret_cast<ID3D11ShaderResourceView*>(texture->resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDescription{};
	res->GetDesc(&resDescription);
	if (resDescription.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
		!SupportedFormat(resDescription.Format))
	{
		REX::ERROR("No supported format view resource");
		return nullptr;
	}

	D3D11_TEXTURE2D_DESC descTexture{};
	tex->GetDesc(std::addressof(descTexture));
	texture->width = descTexture.Width;
	texture->height = descTexture.Height;

	return texture;
}

std::shared_ptr<dmui::Texture> dmui::TextureLoader::LoadFromFile(const std::string& a_path) noexcept
{
	if (!std::filesystem::exists(a_path))
		return nullptr;

	std::filesystem::path path(a_path);
	std::wstring ext = path.extension();

	if (ext.empty() || !ext.length())
		return nullptr;

	auto format = !_wcsicmp(ext.c_str(), L".dds") ? Format::kDDS : Format::kWIC;
	if ((format == Format::kWIC) && !IsValidWicFormat(ext))
		return nullptr;

	auto texture = std::make_shared<Texture>();
	g_DMUITextureLoaderHandler.ResolveRendererData();

	if (format == Format::kDDS)
	{
		if (FAILED(DirectX::CreateDDSTextureFromFile(
			reinterpret_cast<ID3D11Device*>(g_DMUITextureLoaderHandler.GetD3D11Device()),
			reinterpret_cast<ID3D11DeviceContext*>(g_DMUITextureLoaderHandler.GetD3D11DeviceContext()),
			path.c_str(),
			reinterpret_cast<ID3D11Resource**>(std::addressof(texture->handle)),
			reinterpret_cast<ID3D11ShaderResourceView**>(std::addressof(texture->resource)))))
			return nullptr;
	}
	else
	{
		if (FAILED(DirectX::CreateWICTextureFromFile(
			reinterpret_cast<ID3D11Device*>(g_DMUITextureLoaderHandler.GetD3D11Device()),
			reinterpret_cast<ID3D11DeviceContext*>(g_DMUITextureLoaderHandler.GetD3D11DeviceContext()),
			path.c_str(),
			reinterpret_cast<ID3D11Resource**>(std::addressof(texture->handle)),
			reinterpret_cast<ID3D11ShaderResourceView**>(std::addressof(texture->resource)))))
			return nullptr;
	}

	auto tex = reinterpret_cast<ID3D11Texture2D*>(texture->handle);
	auto res = reinterpret_cast<ID3D11ShaderResourceView*>(texture->resource);

	D3D11_SHADER_RESOURCE_VIEW_DESC resDescription{};
	res->GetDesc(&resDescription);
	if (resDescription.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D ||
		!SupportedFormat(resDescription.Format))
	{
		REX::ERROR("No supported format view resource");
		return nullptr;
	}

	D3D11_TEXTURE2D_DESC descTexture{};
	tex->GetDesc(std::addressof(descTexture));
	texture->width = descTexture.Width;
	texture->height = descTexture.Height;

	return texture;
}

std::shared_ptr<dmui::Texture> dmui::TextureLoader::LoadFromResource(Format a_format,
	int32_t a_resourceId, const char* a_section) noexcept
{
	auto hmod = reinterpret_cast<HMODULE>(mePlugin::GetSingleton()->GetHandleCurrentDll());
	auto hResource = FindResourceA(hmod, MAKEINTRESOURCEA(a_resourceId), 
		(!a_section || !strlen(a_section)) ? MAKEINTRESOURCEA(10) /* RC_DATA */ : a_section);
	if (!hResource) return nullptr;

	REX::INFO("gfhfghf");

	auto hMemory = LoadResource(hmod, hResource);
	if (!hMemory) return nullptr;

	auto dataSize = SizeofResource(hmod, hResource);
	if (!dataSize) return nullptr;

	auto result = LoadFromStream(LockResource(hMemory), dataSize, a_format);
	FreeResource(hResource);

	REX::INFO("sdfs");

	return result;
}