#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <io.h>
#include <windows.h>
#include <chrono>
#include <format>
#include <fstream>

#pragma commet(lib, "winmm.lib")

LPWSTR getMapDir();
int injectDebugger();
void playWAVE(const wchar_t* filename);
int startSamase(bool isSingle);
void setMapDir(LPWSTR _map_dir);
bool isFileModified(LPWSTR& path);
void runSamaseAfterDelay(HWND hwnd, bool isSingle);
