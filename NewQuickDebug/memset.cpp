#include "memset.h"

static HANDLE recentPHandle = NULL;
static HMODULE baseAddr = NULL;
static DWORD targetPID = NULL;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowPID;
    GetWindowThreadProcessId(hwnd, &windowPID);

    if (targetPID || windowPID == targetPID) {
        // Store the window handle in lParam
        *(HWND*)lParam = hwnd;
        return FALSE; // Stop enumeration
    }

    return TRUE; // Continue enumeration
}

// Function to find the window handle based on the PID
HWND FindWindowByPID(DWORD pid) {
    HWND hwnd = NULL;
    targetPID = pid;
    EnumWindows(EnumWindowsProc, (LPARAM)&hwnd);
    return hwnd;
}



bool IsDllLoaded(DWORD processID, const std::wstring& dllName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot." << std::endl;
        return false;
    }

    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &me)) {
        do {
            // DLL 이름을 비교
            if (_wcsicmp(me.szModule, dllName.c_str()) == 0) {
                CloseHandle(hSnapshot);
                return true; // DLL이 로드되어 있음
            }
        } while (Module32Next(hSnapshot, &me));
    }

    CloseHandle(hSnapshot);
    return false; // DLL이 로드되지 않음
}

void refreshScreenSize(DWORD pid) {
    //HWND hwnd = FindWindowByPID(pid);
    //std::cout << "hwnd:: " << hwnd << "\n";
    //if (hwnd == NULL) {
    //    std::cerr << "Window with PID " << pid << " not found.\n";
    //    return;
    //}

    //std::cout << "pid:: " << pid << "\n";
    //RECT rect;
    //if (GetWindowRect(hwnd, &rect)) {
    //    std::cout << "Current Size - Left: " << rect.left << ", Top: " << rect.top << ", Width: " << (rect.right - rect.left) << ", Height: " << (rect.bottom - rect.top) << "\n";

    //    // Resize the window by increasing width and height
    //    int newWidth = (rect.right - rect.left) + 100;
    //    int newHeight = (rect.bottom - rect.top) + 200;

    //    if (!SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, newWidth, newHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW)) {
    //        std::cerr << "Failed to resize window.\n";
    //    }
    //}
    //else {
    //    std::cerr << "Failed to get window rectangle.\n";
    //}
}

void setrecentPHandle(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (hProcess == NULL) std::cout << "Failed to open process. Error: " << GetLastError() << std::endl;
	recentPHandle = hProcess;
    //WaitForSingleObject(recentPHandle, INFINITE);
}

bool IsProcess64Bit(HANDLE hProcess) {
    BOOL isWow64 = FALSE;

    // Check if the process is running under WoW64
    if (IsWow64Process(hProcess, &isWow64)) {
        if (isWow64) {
            // The process is 32-bit running on a 64-bit system
            return false;
        }
        else {
            // The process is 64-bit or running on a 32-bit system
            return true;
        }
    }
    else {
        std::cerr << "Failed to query process bitness: " << GetLastError() << std::endl;
        return false;
    }

}
void getBaseHandle() {
    HMODULE hMods[1024];
    DWORD cbNeeded;

    // Get a list of all the modules in the process.
    if (EnumProcessModules(recentPHandle, hMods, sizeof(hMods), &cbNeeded)) {
        // hMods[0] is the base address of the main module (executable)
        std::cout << std::hex << "base: 0x" << (DWORD)hMods[0] << std::endl;
        baseAddr = (HMODULE)hMods[0];
    }
}
bool WriteMemory(SIZE_T offset, LPCVOID buffer, SIZE_T size) {
    if (recentPHandle == NULL || baseAddr == NULL || buffer == NULL || size == 0) return false;

    LPVOID targetAddress = (LPVOID)((DWORD_PTR)baseAddr + offset);
    std::cout << std::hex << "targetAddr:: 0x" << targetAddress << " base: 0x" << baseAddr << std::endl;

    SIZE_T bytesWritten;
    bool writeSuccess = WriteProcessMemory(recentPHandle, targetAddress, buffer, size, &bytesWritten) && (bytesWritten == size);
    return writeSuccess;
}

bool WriteMemory(SIZE_T offset, DWORD dword) {
    if (recentPHandle == NULL || baseAddr == NULL) return false;

    LPVOID targetAddress = (LPVOID)((DWORD_PTR)baseAddr + offset);
    std::cout << std::hex << "targetAddr:: 0x" << targetAddress << " base: 0x" << baseAddr << std::endl;

    SIZE_T bytesWritten;
    bool writeSuccess = WriteProcessMemory(recentPHandle, targetAddress, &dword, 4, &bytesWritten) && (bytesWritten == 4);
    return writeSuccess;
}

bool WriteMemory(SIZE_T offset, BYTE byte) {
    if (recentPHandle == NULL || baseAddr == NULL) return false;

    LPVOID targetAddress = (LPVOID)((DWORD_PTR)baseAddr + offset);
    std::cout << std::hex << "targetAddr:: 0x" << targetAddress << " base: 0x" << baseAddr << std::endl;

    SIZE_T bytesWritten;
    bool writeSuccess = WriteProcessMemory(recentPHandle, targetAddress, &byte, 1, &bytesWritten) && (bytesWritten == 4);
    return writeSuccess;
}

DWORD ReadMemory(SIZE_T offset, SIZE_T size) {
    if (recentPHandle == NULL || baseAddr == NULL || size == 0) return false;

    LPVOID targetAddress = (LPVOID)((DWORD_PTR)baseAddr + offset);
    //std::cout << std::hex << "targetAddr:: 0x" << targetAddress << " base: 0x" << baseAddr << std::endl;

    SIZE_T bytesRead;
    DWORD ret;
    bool readSuccess = ReadProcessMemory(recentPHandle, targetAddress, (LPVOID)&ret, size, &bytesRead) && (bytesRead == size);

    if (!readSuccess) {
        std::cerr << "Failed to read memory. Error: " << GetLastError() << std::endl;
    }

    return ret;
}

bool IsProcessResponsive(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        return false;
    }

    DWORD exitCode;
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        if (exitCode == STILL_ACTIVE) {
            CloseHandle(hProcess);
            return true;
        }
    }

    CloseHandle(hProcess);
    return false;
}

#include <iostream>
#include <filesystem>
#include <chrono>

bool checkFileModified(LPWSTR file_dir) {

}