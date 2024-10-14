#pragma once

#include <windows.h>
#include <psapi.h>
#include <winternl.h>
#include <vector>
#include <tlhelp32.h>
#include <string>
#include <iostream>
// Function prototypes
void setrecentPHandle(DWORD pid);
void getBaseHandle();
bool WriteMemory(SIZE_T offset, LPCVOID buffer, SIZE_T size);
bool WriteMemory(SIZE_T offset, DWORD dword);
bool WriteMemory(SIZE_T offset, BYTE byte);
DWORD ReadMemory(SIZE_T offset, SIZE_T size);
bool IsProcessResponsive(DWORD pid);
void refreshScreenSize(DWORD pid);
bool IsDllLoaded(DWORD processID, const std::wstring& dllName);


typedef NTSTATUS(NTAPI* NtQueryInformationProcess_t)(
    HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

