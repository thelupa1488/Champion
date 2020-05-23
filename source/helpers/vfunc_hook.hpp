#pragma once
#define NOMINMAX
#include <Windows.h>
#include <memory>
#ifndef _DEBUG
#define rifkhooks
#endif
#include "../helpers/nt.h"
#include <cstdint>
#include <stdexcept>
#include "../features/lazy_importer.hpp"

#ifndef rifkhooks
namespace detail
{
	class protect_guard
	{
	public:
		protect_guard(void* base, size_t len, std::uint32_t flags)
		{
			_base = base;
			_length = len;
			if (!VirtualProtect(base, len, flags, (PDWORD)& _old))
				throw std::runtime_error("Failed to protect region.");
		}
		~protect_guard()
		{
			VirtualProtect(_base, _length, _old, (PDWORD)& _old);
		}

	private:
		void* _base;
		size_t        _length;
		std::uint32_t _old;
	};
}

class vfunc_hook
{
public:
	vfunc_hook();
	vfunc_hook(void* base);
	~vfunc_hook();

	bool setup(void* class_base = nullptr);

	template<typename T>
	void hook_index(int index, T fun)
	{
		assert(index >= 0 && index <= (int)vftbl_len);
		new_vftbl.get()[index + 1] = reinterpret_cast<std::uintptr_t>(fun);
	}
	void unhook_index(int index)
	{
		new_vftbl.get()[index] = old_vftbl[index];
	}
	void unhook_all()
	{
		try {
			if (old_vftbl != nullptr) {
				auto guard = detail::protect_guard{ class_base, sizeof(std::uintptr_t), PAGE_READWRITE };
				*(std::uintptr_t**)class_base = old_vftbl;
				old_vftbl = nullptr;
			}
		}
		catch (...) {
		}
	}
	template<typename function, typename original_function>
	static function HookManual(uintptr_t* vftable, uint32_t index, original_function fnNew) {
		DWORD Dummy;
		function fnOld = (function)vftable[index];
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, PAGE_EXECUTE_READWRITE, &Dummy);
		vftable[index] = (uintptr_t)fnNew;
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, Dummy, &Dummy);
		return fnOld;
	}
	template<typename T>
	T get_original(int index)
	{
		return (T)old_vftbl[index];
	}

private:
	static inline std::size_t estimate_vftbl_length(std::uintptr_t* vftbl_start);

	void* class_base;
	std::size_t     vftbl_len;
	std::unique_ptr<uint32_t[]> new_vftbl;
	std::uintptr_t* old_vftbl;
};
#endif // !rifkhooks

template<class entity>
class c_hook
{
public:
	explicit c_hook(entity* ent)
	{
		base = reinterpret_cast<uintptr_t*>(ent);
		original = *base;

		const auto l = length() + 1;
		current = std::make_unique<uint32_t[]>(l);
		std::memcpy(current.get(), reinterpret_cast<void*>(original - sizeof(uint32_t)), l * sizeof(uint32_t));
		oldbase = base;
		patch_pointer(base);
	}


	template<typename function, typename original_function>
	static function HookManual(uintptr_t* vftable, uint32_t index, original_function fnNew) {
		DWORD Dummy;
		function fnOld = (function)vftable[index];
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, PAGE_EXECUTE_READWRITE, &Dummy);
		vftable[index] = (uintptr_t)fnNew;
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, Dummy, &Dummy);
		return fnOld;
	}

	template<typename function, typename original_function>
	function apply(const uint32_t index, original_function func)
	{
		auto old = reinterpret_cast<uintptr_t*>(original)[index];
		current.get()[index + 1] = reinterpret_cast<uintptr_t>(func);
		return reinterpret_cast<function>(old);
	}

	void unapply(int index) {
		current.get()[index + 1] = reinterpret_cast<uintptr_t*>(original)[index];
		DWORD old;
		VirtualProtect(&base, sizeof(uintptr_t), PAGE_READWRITE, &old);
		*base = *oldbase;
		VirtualProtect(&base, sizeof(uintptr_t), old, &old);
	}

	void patch_pointer(uintptr_t* location) const
	{
		if (!location)
			return;

		_NtProtectVirtualMemory NtProtectVirtualMemory = (_NtProtectVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenProcess");
		DWORD old;
		PVOID address = location;
		ULONG size = sizeof(uintptr_t);
		NtProtectVirtualMemory(current_process, &address, &size, PAGE_READWRITE, &old);
		address = location;
		size = sizeof(uintptr_t);
		*location = reinterpret_cast<uint32_t>(current.get()) + sizeof(uint32_t);
		NtProtectVirtualMemory(current_process, &address, &size, old, &old);
	}

private:
	uint32_t length() const
	{
		uint32_t index;
		const auto vmt = reinterpret_cast<uint32_t*>(original);

		for (index = 0; vmt[index]; index++)
			if (IS_INTRESOURCE(vmt[index]))
				break;

		return index;
	}

	std::uintptr_t* base;
	std::uintptr_t original;
	std::uintptr_t* oldbase;
	std::unique_ptr<uint32_t[]> current;
};

#ifdef rifkhooks
class vfunc_hook
{
public:

	void setup(void* ent)
	{
		base = reinterpret_cast<uintptr_t*>(ent);
		original = *base;

		const auto l = length() + 1;
		current = std::make_unique<uint32_t[]>(l);
		std::memcpy(current.get(), reinterpret_cast<void*>(original - sizeof(uint32_t)), l * sizeof(uint32_t));
		oldbase = base;
		patch_pointer(base);
	}

	template<typename original_function>
	void hook_index(const uint32_t index, original_function func)
	{
		current.get()[index + 1] = reinterpret_cast<uintptr_t>(func);
	}

	template<class Type>
	static Type HookManual(uintptr_t* vftable, uint32_t index, Type fnNew) {
		DWORD Dummy;
		Type fnOld = (Type)vftable[index];
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, PAGE_EXECUTE_READWRITE, &Dummy);
		vftable[index] = (uintptr_t)fnNew;
		VirtualProtect((void*)(vftable + index * 0x4), 0x4, Dummy, &Dummy);
		return fnOld;
	}

	template<typename T>
	T get_original(int index)
	{
		auto old = reinterpret_cast<uintptr_t*>(original)[index];
		return reinterpret_cast<T>(old);
	}

	void unapply(int index) {
		current.get()[index + 1] = reinterpret_cast<uintptr_t*>(original)[index];
		DWORD old;
		VirtualProtect(&base, sizeof(uintptr_t), PAGE_READWRITE, &old);
		*base = *oldbase;
		VirtualProtect(&base, sizeof(uintptr_t), old, &old);
	}

	void patch_pointer(uintptr_t* location) const
	{
		if (!location)
			return;

		_NtProtectVirtualMemory NtProtectVirtualMemory = (_NtProtectVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtOpenProcess");
		DWORD old;
		PVOID address = location; 
		ULONG size = sizeof(uintptr_t);
		NtProtectVirtualMemory(current_process, &address, &size, PAGE_READWRITE, &old);
		address = location;
		size = sizeof(uintptr_t);
		*location = reinterpret_cast<uint32_t>(current.get()) + sizeof(uint32_t);
		NtProtectVirtualMemory(current_process, &address, &size, old, &old);
	}

private:
	uint32_t length() const
	{
		uint32_t index;
		const auto vmt = reinterpret_cast<uint32_t*>(original);

		for (index = 0; vmt[index]; index++)
			if (IS_INTRESOURCE(vmt[index]))
				break;

		return index;
	}

	std::uintptr_t* base;
	std::uintptr_t original;
	std::uintptr_t* oldbase;
	std::unique_ptr<uint32_t[]> current;

};
#endif