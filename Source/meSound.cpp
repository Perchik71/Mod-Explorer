// Copyright © 2026 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/lgpl-3.0.html

#include <Windows.h>
#include <mmsystem.h>

#undef ERROR
#undef MEM_RELEASE
#undef MAX_PATH

#include "meSound.h"
#include "mePlugin.h"

#include <REX/REX.h>

/////////////////////////////////////////
// meSoundFX

bool meSoundFX::LoadFromFile(const char* FileName) noexcept(true)
{
	if (!FileName)
		return false;

	// Чтение звука из файла (блокируем файл для записи)
	auto handle = _fsopen(FileName, "rb", _SH_DENYWR);
	if (!handle)
		return false;
	// Смещаем каретку в самый конец
	if (!fseek(handle, 0, SEEK_END))
	{
		fclose(handle);
		return false;
	}
	// Получаем размер файла и выделяем под это память
	_bufsize = ftell(handle);
	_buffer = std::make_unique<std::uint8_t[]>(_bufsize);
	if (!_buffer)
	{
		fclose(handle);
		return false;
	}
	// Возвращаем каретку в начало файла
	if (!fseek(handle, 0, SEEK_SET))
	{
		fclose(handle);
		_bufsize = 0;
		_buffer = nullptr;

		return false;
	}
	// Читаем файл одним вызовом fread (так как файлы небольшие)
	if (fread(_buffer.get(), _bufsize, 1, handle) != _bufsize)
	{
		fclose(handle);
		_bufsize = 0;
		_buffer = nullptr;

		return false;
	}

	return true;
}

bool meSoundFX::LoadFromFile(const wchar_t* FileName) noexcept(true)
{
	if (!FileName)
		return false;

	// Чтение звука из файла (блокируем файл для записи)
	auto handle = _wfsopen(FileName, L"rb", _SH_DENYWR);
	if (!handle)
		return false;
	// Смещаем каретку в самый конец
	if (!fseek(handle, 0, SEEK_END))
	{
		fclose(handle);
		return false;
	}
	// Получаем размер файла и выделяем под это память
	_bufsize = ftell(handle);
	_buffer = std::make_unique<std::uint8_t[]>(_bufsize);
	if (!_buffer)
	{
		fclose(handle);
		return false;
	}
	// Возвращаем каретку в начало файла
	if (!fseek(handle, 0, SEEK_SET))
	{
		fclose(handle);
		_bufsize = 0;
		_buffer = nullptr;

		return false;
	}
	// Читаем файл одним вызовом fread (так как файлы небольшие)
	if (fread(_buffer.get(), _bufsize, 1, handle) != _bufsize)
	{
		fclose(handle);
		_bufsize = 0;
		_buffer = nullptr;

		return false;
	}

	return true;
}

bool meSoundFX::LoadFromStream(const void* Buffer, std::uint32_t BufferSize) noexcept(true)
{
	if (!Buffer || !BufferSize)
		return false;

	// Выделяем память
	_bufsize = BufferSize;
	_buffer = std::make_unique<std::uint8_t[]>(BufferSize);
	if (!_buffer)
		return false;
	// Копируем память
	memcpy(_buffer.get(), Buffer, BufferSize);

	return true;
}

bool meSoundFX::LoadFromResource(const char* Type, std::uint32_t ResourceID) noexcept(true)
{
	if (!Type)
		return false;

	// Получаем структуру самого плагина
	auto plugin = mePlugin::GetSingleton();
	if (!plugin)
		return false;

	// Получаем hmod нашего плагина
	auto hModule = (HMODULE)plugin->GetHandleCurrentDll();
	if (!hModule)
		return false;
	// Достаём ресурс
	HRSRC hResource = FindResourceA(hModule, MAKEINTRESOURCEA(ResourceID), Type);
	if (!hResource)
	{
		REX::WARN("Resource no found");
		return false;
	}
	// Определяем размер ресурса
	std::uint32_t Size = (std::uint32_t)SizeofResource(hModule, hResource);
	if (Size >= static_cast<std::uint32_t>(std::numeric_limits<int32_t>::max()))
	{
		REX::WARN("Size of resource exceeds 2 gigs");
		return false;
	}
	// Загружаем ресурс
	auto hResourceMemory = LoadResource(hModule, hResource);
	if (!hResourceMemory)
	{
		REX::ERROR("hResourceMemory == nullptr");
		return false;
	}
	// Копируем его из памяти
	bool Succeed = LoadFromStream(LockResource(hResourceMemory), Size);
	// Освобождаем ресурс
	FreeResource(hResourceMemory);

	return Succeed;
}

bool meSoundFX::Play(bool Async) const noexcept(true)
{
	if (!_buffer)
		return false;
	// Проиграть из памяти (не тормозим процесс)
	return PlaySoundA((LPCSTR)_buffer.get(), NULL, (Async ? SND_ASYNC : SND_SYNC) | SND_MEMORY);
}