#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <commdlg.h>
#include <TlHelp32.h>
#include <shlwapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include "json.hpp"
#include <optional>
#include <fstream>

//void refreshScreenSize();
LPWSTR OpenFileDialog(HWND hwnd);
std::wstring LPWSTRToWString(LPWSTR& wideStr);
void AttachConsole();
std::vector<DWORD> GetProcessIDsByName(const std::wstring& processName);
bool IsProcessRunning(DWORD processID);

void SaveValueToJson(const std::string& key, int value);
std::optional<int> LoadValueFromJson(const std::string& key);

bool decreaseTimer(HWND hWnd);
void setTextLabel(int time, HWND hWnd);

template <class F, class... Args>
void runAfterDelay(std::chrono::milliseconds delay, F&& f, Args&&... args) {
    // Create a new thread to handle the delay and function execution
    std::cout << "delay: " << delay << "\n";
    std::thread([delay, f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
        std::this_thread::sleep_for(delay); // Sleep for the given delay
        std::apply(f, std::move(args)); // Call the function with the provided arguments
        }).detach(); // Detach the thread to run independently
}