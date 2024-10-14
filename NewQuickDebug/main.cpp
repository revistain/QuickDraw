#include "main.h"
#include "memset.h"
#include "utils.h"
#include "Resource.h"
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <iostream>

#pragma comment(lib,"winmm.lib")
LPWSTR map_dir = NULL;
std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

void setMapDir(LPWSTR _map_dir) {
    map_dir = _map_dir;
}

LPWSTR getMapDir() {
    return map_dir;
}

std::wstring getExecutableDirectory() {
    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH)) {
        std::wstring exePath(path);
        size_t pos = exePath.find_last_of(L"\\/");
        std::wstring exeDir = exePath.substr(0, pos);
        return exeDir;
    }
    else {
        std::wcerr << L"Error getting executable path" << std::endl;
        return L"";
    }
}

void playWAVE(const wchar_t* filename) {
    PlaySound(filename, NULL, SND_FILENAME | SND_ASYNC);
}

int injectDebugger() {
    std::wstring current_directory = getExecutableDirectory();
    std::wstring command = L"\"" + current_directory + L"\\data\\Injector.exe\" --process-name StarCraft.exe --inject \".\\eudplib_debug.dll" + L"\"";

    DWORD targetPID = 0;
    std::vector<DWORD> foundPID;
    foundPID = GetProcessIDsByName(L"StarCraft.exe");
    if (foundPID.size() == 0) {
        playWAVE(L"data\\buzz.wav");
        return -1;
    }
    else if (foundPID.size() > 1) {
        playWAVE(L"data\\buzz.wav");
        return -2;
    }
    targetPID = foundPID[0];
    std::wcout << L"StarCraft Process ID: " << targetPID << std::endl;
    std::wcout << L"command: " << command << "\n";
    if (IsDllLoaded(targetPID, L"eudplib_debug.dll")) {
        playWAVE(L"data\\buzz.wav");
        return -4;
    }

    setrecentPHandle(targetPID);
    getBaseHandle();
    SIZE_T UMS_TIMER = 0xBD069C;
    if ((ReadMemory(UMS_TIMER, 2) & 0xFFFF) > 0x1) {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        if(CreateProcessW(NULL, &command[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            std::cout << "runned\n";
            playWAVE(L"data\\bnetclick.wav");
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        else {
            std::wcout << L"Failed to run the injector. Error code: " << GetLastError() << std::endl;
            playWAVE(L"data\\buzz.wav");
            return -3;
        }
    }
    else {
        playWAVE(L"data\\buzz.wav");
        return -5;
    }

    return 0;
}

#include <chrono>
std::vector<DWORD> previousPID;
int startSamase(bool isSingle) {
    std::cout << "=====================================\n";
    if (map_dir == NULL) {
        std::cout << "Map is not loaded" << std::endl;
        playWAVE(L"data\\buzz.wav");
        return -1;
    }

    std::vector<DWORD> _foundPID = GetProcessIDsByName(L"StarCraft.exe");
    if (_foundPID.size() > 0) {
        std::cout << "only one starcraft can be runned!" << std::endl;
        playWAVE(L"data\\buzz.wav");
        return -2;
    }
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    std::wstring current_directory = getExecutableDirectory();
    std::wcout << L"Running At : " << current_directory << "\\Samase\n";
    std::wcout << L"Map Title : " << map_dir << "\n";

    std::wstring command = L"\"" + current_directory + L"\\data\\samase\" " +
                           L"\"" + current_directory + L"\\dummy\" --map \"" +
                           map_dir + L"\"";
 
    if (std::filesystem::exists(map_dir) && CreateProcessW(NULL, &command[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        std::cout << "Samase Process ID: " << pi.dwProcessId << std::endl;
        for (auto& p : previousPID) {
            std::cout << std::dec << "pids: " << p << std::endl;
        }

        DWORD targetPID = 0;
        std::vector<DWORD> foundPID;
        while (foundPID.empty()) {
            foundPID = GetProcessIDsByName(L"StarCraft.exe");
            if (foundPID.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(100));
            else {
                if (previousPID.empty()) {
                    targetPID = foundPID[0];
                    previousPID.push_back(targetPID);
                    break;
                }
                for (auto& f : foundPID) {
                    bool flag = true;
                    for (auto& p : previousPID) {
                        if (f == p) flag = false;
                    }
                    if (flag) {
                        targetPID = f;
                        previousPID = foundPID;
                        break;
                    }
                }
            }
        }
        std::wcout << L"StarCraft Process ID: " << targetPID << std::endl;
        playWAVE(L"data\\bnetclick.wav");
        setrecentPHandle(targetPID);
        getBaseHandle();
        refreshScreenSize(targetPID);

        SIZE_T MULTIPLAYER  = 0xDCC7CC;
        SIZE_T TOOL_TIP     = 0xDF93F5;
        SIZE_T UMS_TIMER    = 0xBD069C;
        // SIZE_T NICKNAME     = 0xDCC808;

        static int timer = 0;
        if (!isSingle) {
            while (true) {
                auto start = std::chrono::steady_clock::now(); // 시작 시간 기록

                BYTE prevOffsetVal = (ReadMemory(TOOL_TIP, 1) & 0xFF);
                if ((prevOffsetVal & 0x1) == 0x1) {
                    prevOffsetVal -= 1;
                    WriteMemory(TOOL_TIP, prevOffsetVal);
                }
                if ((ReadMemory(UMS_TIMER, 2) & 0xFFFF) > 0x1) {
                    if (WriteMemory(MULTIPLAYER, (BYTE)255)) {
                        std::cout << "Memory written successfully." << std::endl;
                    }
                    else {
                        std::cout << "Failed to write memory: " << GetLastError() << std::endl;
                    }
                    break;
                }
                if (!IsProcessRunning(targetPID)) {
                    for (auto& p : previousPID) {
                        auto it = std::find(previousPID.begin(), previousPID.end(), targetPID);
                        if (it != previousPID.end()) previousPID.erase(it);
                    }
                    std::cout << "process terminated " << targetPID << "\n";
                    break;
                }

                // 5초 경과 확인
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > 5) {
                    std::cout << "Exiting loop after 5 seconds." << std::endl;
                    break;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // CPU 사용량을 줄이기 위해 잠시 대기
            }
        }
        // Clean up
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else {
        playWAVE(L"data\\buzz.wav");
        std::wcout << L"Error CreatProcessW command: " << command << std::endl;
        std::wcout << L"Error code : " << GetLastError() << std::endl;
        std::cerr << "error creating process";
    }
    return 0;
}

DWORD time_between(
    const std::filesystem::file_time_type& time1,
    const std::filesystem::file_time_type& time2) {
    using namespace std::chrono;
    auto duration = time1 - time2;
    auto duration_in_seconds = duration_cast<seconds>(duration).count();
    return (DWORD)duration_in_seconds;
}

bool isFileInUse(LPCWSTR filePath) {
    HANDLE hFile = CreateFile(
        filePath,                // File path
        GENERIC_READ,            // Desired access
        FILE_SHARE_READ,         // Share mode (allow read access)
        NULL,                    // Security attributes
        OPEN_EXISTING,           // Create or open the file
        FILE_ATTRIBUTE_NORMAL,   // File attributes
        NULL                     // Template file
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // If the file is being used by another process, error code will be ERROR_SHARING_VIOLATION
        DWORD error = GetLastError();
        if (error == ERROR_SHARING_VIOLATION) {
            return true; // File is in use
        }
        // Handle other errors if needed
        std::wcerr << L"Error opening file: " << error << std::endl;
        return false; // Error other than file in use
    }

    // Successfully opened the file, so it's not in use
    CloseHandle(hFile);
    return false;
}

#undef min
std::filesystem::file_time_type previous_ftime = std::filesystem::file_time_type::min();
bool isFileModified(LPWSTR& path) {
    if (previous_ftime == std::filesystem::file_time_type::min()) {
        previous_ftime = std::filesystem::last_write_time(path);
    }
    else if (previous_ftime < std::filesystem::last_write_time(path) && time_between(std::filesystem::last_write_time(path), previous_ftime) > 5) {
        if (isFileInUse(path)) {
            std::cout << "file is in use\n";
        }
        else {
            previous_ftime = std::filesystem::last_write_time(path);
            std::cout << "Detected file change " << previous_ftime << "\n";
            return true;
        }
    }
    return false;
}

void runSamaseAfterDelay(HWND hwnd, bool isSingle) {
    SetWindowText(hwnd, L"");
    startSamase(isSingle);
}