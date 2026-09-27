#pragma once

#include <DearModdingUI/Client.h>

#include <memory>
#include <mutex>

namespace dmui
{
	class Texture;
	class TextureLoader;

	class Texture
	{
	public:
		std::optional<ImageResource> dmuiImage;
		std::once_flag once{};
		void* handle{ nullptr };
		void* resource{ nullptr };
		uint32_t width{ 0 };
		uint32_t height{ 0 };
	public:
		friend class TextureLoader;

		constexpr Texture() noexcept = default;
		~Texture();

		void Release();
		void SendOnceDmuiRequest(Client* a_client) noexcept;

		[[nodiscard]] ImageHandle GetHandle() const noexcept;
		[[nodiscard]] inline uint32_t GetWidth() const noexcept { return width; }
		[[nodiscard]] inline uint32_t GetHeight() const noexcept { return height; }
	};

	class TextureLoader
	{
		[[nodiscard]] static bool IsValidWicFormat(const std::wstring& a_ext) noexcept;
		[[nodiscard]] static bool SupportedFormat(int32_t a_format) noexcept;
	public:
		enum class Format
		{
			kDDS = 0,
			kWIC,
		};

		constexpr TextureLoader() = default;

		[[nodiscard]] static std::shared_ptr<Texture> LoadFromStream(const void* a_buffer,
			size_t a_bufferSize, Format a_format) noexcept;
		[[nodiscard]] static std::shared_ptr<Texture> LoadFromFile(const std::string& a_path) noexcept;
		[[nodiscard]] static std::shared_ptr<Texture> LoadFromResource(Format a_format,
			int32_t a_resourceId, const char* a_section = nullptr) noexcept;
	};
}