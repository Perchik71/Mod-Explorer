// Copyright © 2026 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/lgpl-3.0.html

#pragma once

#include <memory>

// Объект воспроизведения звуков без 3D пространства
// Подходит для воспроизведения звуков игрового меню
// Звук навсегда остаётся в памяти, не подгружается по мере необходимости
class meSoundFX
{
	// Память
	std::unique_ptr<std::uint8_t[]> _buffer;

	// Размер памяти
	std::size_t _bufsize{ 0 };

	meSoundFX(const meSoundFX&) = delete;
	meSoundFX& operator=(const meSoundFX&) = delete;
public:
	meSoundFX() = default;
	virtual ~meSoundFX() = default;

	// Загрузить звук из файла ANSI-строки
	[[nodiscard]] virtual bool LoadFromFile(const char* FileName) noexcept(true);

	// Загрузить звук из файла WIDE-строки
	[[nodiscard]] virtual bool LoadFromFile(const wchar_t* FileName) noexcept(true);

	// Загрузить звук из памяти
	[[nodiscard]] virtual bool LoadFromStream(const void* Buffer, std::uint32_t BufferSize) noexcept(true);

	// Загрузить звук из ресурса текущей DLL
	[[nodiscard]] virtual bool LoadFromResource(const char* Type, std::uint32_t ResourceID) noexcept(true);

	// Воспроизвести (Async - Звук воспроизводится асинхронно, и функция возвращается сразу после начала воспроизведения)
	virtual bool Play(bool Async = true) const noexcept(true);
};