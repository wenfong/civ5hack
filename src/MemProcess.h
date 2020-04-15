#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName);

DWORD GetProcessIdByName(const char* process_name);

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets);