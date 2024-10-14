#include "utils.h"

LPWSTR OpenFileDialog(HWND hwnd) {
    OPENFILENAME ofn;       // File Open dialog structure
    WCHAR szFile[260] = { 0 }; // Buffer to store the file path

    // Initialize the OPENFILENAME structure
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR); // Size in WCHARs
    ofn.lpstrFilter = L"Map files\0*.scx;*.scm\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileName(&ofn) == TRUE) {
        size_t length = wcslen(ofn.lpstrFile) + 1;
        LPWSTR path = new WCHAR[length];
        wcscpy_s(path, length, ofn.lpstrFile);
        return path;
    }
    return NULL;
}

std::wstring LPWSTRToWString(LPWSTR& wideStr) {
    std::wstring test(wideStr);
    return test;
}


static uint32_t label_timer = 0;
bool isTime() {

}
bool decreaseTimer(HWND hWnd) {
    uint32_t flag = label_timer;
    if (label_timer > 0) label_timer--;
    if (flag != 0 && label_timer == 0) {
        KillTimer(hWnd, 2);
        return true;
    }
    return false;
}
void setTextLabel(int time, HWND hWnd) {
    SetTimer(hWnd, 2, 1000, NULL);
    label_timer = time;
}

void AttachConsole() {
    if (AllocConsole()) {
        SetConsoleOutputCP(CP_UTF8); // Set the console to UTF-8
        freopen("CONOUT$", "w", stdout); // Redirect stdout to the console
        freopen("CONIN$", "r", stdin);  // Redirect stdin from the console
        std::wcout.imbue(std::locale("en_US.UTF-8")); // Use UTF-8 locale for wide output
        std::wcout << L"Console attached" << std::endl;
    }
    else {
        std::cerr << "Failed to allocate console" << std::endl;
    }
}

std::vector<DWORD> GetProcessIDsByName(const std::wstring& processName) {
    std::vector<DWORD> pids;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take a snapshot of the processes." << std::endl;
        return pids;
    }

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Failed to retrieve information about the first process." << std::endl;
        CloseHandle(hSnapshot);
        return pids;
    }

    do {
        if (processName == pe32.szExeFile) {
            pids.push_back(pe32.th32ProcessID);
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return pids;
}

bool IsProcessRunning(DWORD processID) {
    // Open a handle to the process with PROCESS_QUERY_INFORMATION access right
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);

    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }

    DWORD exitCode;
    // Get the exit code of the process
    if (GetExitCodeProcess(hProcess, &exitCode)) {
        // If the exit code is STILL_ACTIVE, the process is still running
        bool isRunning = (exitCode == STILL_ACTIVE);
        CloseHandle(hProcess);  // Close the handle to the process
        return isRunning;
    }
    else {
        std::cerr << "Failed to get exit code. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }
}

const std::string json_name = "settings.json";
const std::string getJsonName() { return json_name; }
void SaveValueToJson(const std::string& key, int value) {
    nlohmann::json jsonData;

    // Try to load existing data
    std::ifstream inputFile(json_name);
    if (inputFile.is_open()) {
        inputFile >> jsonData;
        inputFile.close();
    }

    // Update or add the key-value pair
    jsonData[key] = value;

    // Save updated data to the file
    std::ofstream outputFile(json_name);
    if (outputFile.is_open()) {
        outputFile << jsonData.dump(4); // Write JSON with indentation for readability
        outputFile.close();
    }
}

std::optional<int> LoadValueFromJson(const std::string& key) {
    nlohmann::json jsonData;

    // Attempt to open and read JSON data from the file
    std::ifstream inputFile(json_name);
    if (inputFile.is_open()) {
        try {
            inputFile >> jsonData; // Try to parse JSON data
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return std::nullopt; // Return std::nullopt if parsing fails
        }
        inputFile.close();
    }
    else {
        std::cerr << "Could not open file: " << json_name << std::endl;
        return std::nullopt; // Return std::nullopt if the file cannot be opened
    }

    // Check if the key exists and return the value
    if (jsonData.contains(key)) {
        return jsonData[key].get<int>();
    }

    return std::nullopt; // Return std::nullopt if the key is not found
}

