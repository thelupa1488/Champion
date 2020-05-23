#pragma once

#define NOMINMAX
#include <Windows.h>

#include <string>
#include <initializer_list>
#include "../valve_sdk/sdk.hpp"

namespace Utils {
	std::vector<char> HexToBytes(const std::string& hex);
	std::string BytesToString(unsigned char* data, int len);
	std::vector<std::string> Split(const std::string& str, const char* delim);
	unsigned int FindInDataMap(datamap_t * pMap, const char * name);
    /*
     * @brief Create console
     *
     * Create and attach a console window to the current process
     */
	void AttachConsole();

    /*
     * @brief Detach console
     *
     * Detach and destroy the attached console
     */
    void DetachConsole();

    /*
     * @brief Print to console
     *
     * Replacement to printf that works with the newly created console
     */
    bool ConsolePrint(const char* fmt, ...);
    
    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char ConsoleReadKey();

    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules);

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    std::uint8_t* PatternScan(void* module, const char* signature);

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
    void SetClantag(const char* tag);

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    void SetName(const char* name);

    /*
     * @brief Reveal the ranks of all players on the server
     *
     */
    void RankRevealAll();
	void LoadNamedSkys(const char* sky_name);
	template<typename T>
	struct IntHash
	{
		size_t operator()(const T& t) const noexcept
		{
			return std::hash<int>()((int)t);
		}
	};
}

/**/
typedef PVOID(__cdecl* oKeyValuesSystem)();

class KeyValues
{
public:
	PVOID operator new(size_t iAllocSize)
	{
		static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandle(L"vstdlib.dll"), "KeyValuesSystem");
		auto KeyValuesSystem = KeyValuesSystemFn();

		typedef PVOID(__thiscall* oAllocKeyValuesMemory)(PVOID, int);
		return CallVFunction<oAllocKeyValuesMemory>(KeyValuesSystem, 1)(KeyValuesSystem, iAllocSize);
	}

	void operator delete(PVOID pMem)
	{
		static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandle(L"vstdlib.dll"), "KeyValuesSystem");
		auto KeyValuesSystem = KeyValuesSystemFn();

		typedef void(__thiscall* oFreeKeyValuesMemory)(PVOID, PVOID);
		CallVFunction<oFreeKeyValuesMemory>(KeyValuesSystem, 2)(KeyValuesSystem, pMem);
	}

	const char* GetName()
	{
		static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandle(L"vstdlib.dll"), "KeyValuesSystem");
		auto KeyValuesSystem = KeyValuesSystemFn();

		auto a2 = (DWORD)this;

		typedef const char* (__thiscall* oGetName)(PVOID, int);
		return CallVFunction<oGetName>(KeyValuesSystem, 4)(KeyValuesSystem, *(BYTE*)(a2 + 3) | (*(WORD*)(a2 + 18) << 8));
	}

	KeyValues* FindKey(const char* keyName, bool bCreate = false);
	void SetString(const char* keyName, const char* value);
	void InitKeyValues(const char* name);
	void SetUint64(const char* keyName, int value, int value2);
	const char* GetString(const char* keyName, const char* defaultValue);
	int GetInt(const char* keyName, int defaultValue);
	void SetInt(const char* keyName, int Value);
};