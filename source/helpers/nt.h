#pragma once

#include <cstdint>
#include <Windows.h>
#include <winternl.h>

using _NtAllocateVirtualMemory = NTSTATUS(NTAPI*)(HANDLE process_handle, PVOID* base_address,
	std::uint32_t* bits, size_t* size, std::uint32_t type, std::uint32_t access);

using _NtReadVirtualMemory = NTSTATUS(NTAPI*)(HANDLE process_handle, PVOID base_address, PVOID buffer,
	ULONG number_of_bytes_to_read, PULONG number_of_bytes_readed);

using _NtWriteVirtualMemory = NTSTATUS(NTAPI*)(HANDLE process_handle, PVOID base_address, PVOID buffer,
	ULONG number_of_bytes_to_write, PULONG number_of_bytes_written);

using NtFreeVirtualMemory = NTSTATUS(NTAPI*)(HANDLE process_handle, PVOID* base_address, PSIZE_T region_size, ULONG free_type);

using _NtProtectVirtualMemory = NTSTATUS(NTAPI*)(HANDLE process_handle, PVOID* base_address, PULONG number_of_bytes_to_protect,
	ULONG new_access_protection, PULONG old_access_protection);

using _NtYieldExecution = NTSTATUS(NTAPI*)();

using _NtCreateThreadEx = NTSTATUS(NTAPI*)(PHANDLE thread_handle, ACCESS_MASK desired_access, POBJECT_ATTRIBUTES object_attributes,
	HANDLE process_handle, PVOID start_routine, PVOID argument, ULONG create_flags,
	ULONG_PTR zero_bits, SIZE_T stack_size, SIZE_T maximum_stack_size, PVOID attribute_list);

using _NtResumeThread = NTSTATUS(NTAPI*)(HANDLE thread_handle, PULONG suspend_count);

using _NtGetContextThread = NTSTATUS(NTAPI*)(HANDLE thread_handle, PCONTEXT context);

using _NtSetContextThread = NTSTATUS(NTAPI*)(HANDLE thread_handle, PCONTEXT context);

static const auto current_process = reinterpret_cast<HANDLE>(-1);