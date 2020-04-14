 #include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <vector>

const char* PROCESS_NAME = "CivilizationV_DX11.exe";
const unsigned int GOLD_VALUE = 10000000;
const unsigned int BASE_GOLD_OFFSET = 0x00900A28;
const unsigned int FAITH_VALUE = 100000;
const unsigned int BASE_FAITH_OFFSET = 0x00900A28;
const unsigned int CULTURE_INCREASE_VALUE = 1000;
const unsigned int BASE_CULTURE_OFFSET = 0x00900A28;

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
	for (unsigned int i = 0; i < offsets.size(); ++i){
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

int main()
{
    std::cout << "Searching for game..." << std::endl;
	DWORD process_id = NULL;
	while (!process_id) {
		process_id = GetProcessIdByName(PROCESS_NAME);
		Sleep(100);
	}

	//open game handle
	HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (process_handle) {
		std::cout << "Game handle opened" << std::endl;
	}
	else {
		std::cout << "Game handle not opened" << std::endl;
		exit(1);
	}
	Sleep(1000);


	//get addresses from their respective base pointers
	uintptr_t moduleBase = 0, baseGoldPtr = 0, goldPtr = 0, baseFaithPtr = 0, faithPtr = 0;
	
	moduleBase = GetModuleBaseAddress(process_id, PROCESS_NAME);
	
	std::vector<unsigned int> goldOffsets = { 0x84, 0x8, 0x0, 0x8, 0x94, 0x14, 0x68 };
	baseGoldPtr = moduleBase + 0x00900A28;
	goldPtr = FindDMAAddy(process_handle, baseGoldPtr, goldOffsets);

	std::vector<unsigned int> faithOffsets = { 0x40, 0x90, 0x8, 0x8, 0x6C, 0x8, 0xD8 };
	baseFaithPtr = moduleBase + 0x00900A28;
	faithPtr = FindDMAAddy(process_handle, baseFaithPtr, faithOffsets);
	
	std::vector<unsigned int> cultureOffsets = { 0x40, 0x90, 0x8, 0x18, 0x24, 0xC, 0x58 };
	baseCulturePtr = moduleBase + BASE_CULTURE_OFFSET;
	culturePtr = FindDMAAddy(process_handle, baseCulturePtr, cultureOffsets);

	
	bool lock_gold = false, lock_faith = false;
	bool update_screen = true;
	//main loop
	while (1) {
		if (update_screen) {
			system("cls");
			//std::cout << "Gold address = " << "0x" << std::hex << goldPtr << std::endl;//DEBUG
			//std::cout << "Faith address = " << "0x" << std::hex << faithPtr << std::endl;//DEBUG
			std::cout << "[NUMPAD 0] - EXIT" << std::endl;
			std::cout << "[NUMPAD 1] - <" << (lock_gold ? "on" : "off") << "> Lock Gold at 100000" << std::endl;
			std::cout << "[NUMPAD 2] - <" << (lock_faith ? "on" : "off") << "> Lock Faith at 100000" << std::endl;
			std::cout << "[NUMPAD 3] - +1000 Culture" << std::endl;
			update_screen = false;
		}

		if (GetAsyncKeyState(VK_NUMPAD0)) {
			std::cout << "Exiting..." << std::endl;
			Sleep(1700);
			break;
		}
		if (GetAsyncKeyState(VK_NUMPAD1)) {
			lock_gold = !lock_gold;
			update_screen = true;
		}
		if (GetAsyncKeyState(VK_NUMPAD2)) {
			lock_faith = !lock_faith;
			update_screen = true;
		}
		if (GetAsyncKeyState(VK_NUMPAD3)) {
			unsigned int read_value = 0;
			ReadProcessMemory(process_handle, (LPVOID)culturePtr, &read_value, sizeof(read_value), NULL);
			unsigned int write_value = read_value + CULTURE_INCREASE_VALUE;
			WriteProcessMemory(process_handle, (LPVOID)culturePtr, &write_value, sizeof(write_value), NULL);
			update_screen = true;
		}


		if (lock_gold) {
			unsigned int write_value = GOLD_VALUE;
			WriteProcessMemory(process_handle, (LPVOID)goldPtr, &write_value, sizeof(write_value), NULL);
		}
		if (lock_faith) {
			unsigned int write_value = FAITH_VALUE;
			WriteProcessMemory(process_handle, (LPVOID)faithPtr, &write_value, sizeof(write_value), NULL);
		}
		Sleep(100);
	}


	CloseHandle(process_handle);
}
