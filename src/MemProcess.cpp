#include "MemProcess.h"

uintptr_t GetModuleBaseAddress(DWORD procId, const char* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!strcmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

DWORD GetProcessIdByName(const char* process_name)
{
	PROCESSENTRY32 process_entry = { sizeof(PROCESSENTRY32) };
	HANDLE processes_snapshot;
	DWORD process_id = 0;

	//process name search
	processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(processes_snapshot, &process_entry)) {
		do {
			if (!strcmp(process_entry.szExeFile, process_name)) {
				process_id = process_entry.th32ProcessID;
				break;
			}
		} while (Process32Next(processes_snapshot, &process_entry));
	}

	CloseHandle(processes_snapshot);

	return process_id;
}

uintptr_t FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}
