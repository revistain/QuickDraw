// NewQuickDebug.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "NewQuickDebug.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NEWQUICKDEBUG, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NEWQUICKDEBUG));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NEWQUICKDEBUG));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_NEWQUICKDEBUG);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       300, 300, 500, 190, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   LONG style = GetWindowLong(hWnd, GWL_STYLE);
   style &= ~(WS_THICKFRAME | WS_SIZEBOX | WS_MAXIMIZEBOX); // Remove resizing and maximize buttons
   SetWindowLong(hWnd, GWL_STYLE, style);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
static HWND hLabel_map_title = NULL;
static HWND hOpenButton = NULL;
static HWND hStartButton = NULL;
static HWND hDebugButton = NULL;
static HWND hMessageLabel = NULL;
static HWND hSingleCheck = NULL;
static HFONT hFont = NULL;
static bool isSingleChecked = false;
static bool preventDuplicateInject = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
    {
        static HBRUSH hBrushGreen = NULL;
        hFont = CreateFont(
            16, // Height of font
            0,  // Width of font (0 means default)
            0,  // Angle of the font's base line (0 is horizontal)
            0,  // Base line orientation angle (0 is default)
            FW_NORMAL, // Font weight (normal)
            FALSE, // Italic
            FALSE, // Underline
            FALSE, // Strikeout
            ANSI_CHARSET, // Character set
            OUT_DEFAULT_PRECIS, // Output precision
            CLIP_DEFAULT_PRECIS, // Clipping precision
            DEFAULT_QUALITY, // Quality
            DEFAULT_PITCH | FF_SWISS, // Pitch and family
            L"Arial" // Font name
        );
        hMessageLabel = CreateWindowEx(0, L"STATIC",
            L"",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            124, 90, 330, 200,
            hWnd, (HMENU)600, GetModuleHandle(NULL), NULL);
        CreateWindowEx(
            0, L"BUTTON", L"Maps", // Group box label
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            120, 30, 345, 54, // Position and size of the group box
            hWnd, NULL, hInst, NULL
        );
        hOpenButton = CreateWindowW(
            L"button",
            L"Open",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 15, 80, 23,
            hWnd,
            (HMENU)100,
            hInst,
            NULL);
        RegisterHotKey(hWnd, 1, MOD_ALT, 0x4F); // ALT+O
        hStartButton = CreateWindowW(
            L"button",
            L"START",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 50, 80, 23,
            hWnd,
            (HMENU)101,
            hInst,
            NULL);
        hDebugButton = CreateWindowW(
            L"button",
            L"Attach",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 85, 80, 23,
            hWnd,
            (HMENU)102,
            hInst,
            NULL);
        hSingleCheck = CreateWindowW(
            L"BUTTON",
            L"Single",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
            400, 10, 100, 23,
            hWnd,
            (HMENU)103,
            hInst,
            nullptr);
        // EnableWindow(hDebugButton, FALSE);
        RegisterHotKey(hWnd, 2, MOD_ALT, 0x53); // ALT+S
        RegisterHotKey(hWnd, 3, 0x0, 0x77);     // F8
        RegisterHotKey(hWnd, 4, MOD_ALT, 0x41); // ALT+A
        hLabel_map_title = CreateWindowEx(
            0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            130, 57, 320, 20,
            hWnd, NULL, hInst, NULL
        );
        // SetTimer(hWnd, 1, 1000, NULL);
        // setting check box
        // static HWND hSingleCheckbox = GetDlgItem(hWnd, IDC_SINGLECHECK);
        // static HWND hAutoCheckbox = GetDlgItem(hWnd, IDC_AUTOCHECK);

        // wait edit box
        SendMessage(hLabel_map_title, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        std::cout << "command" << wmId<<"\n";
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {   
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        
        case 100:
            if (HIWORD(wParam) == BN_CLICKED) {
                std::cout << "100\n";
                LPWSTR map_dir = OpenFileDialog(hWnd);
                if (map_dir) {
                    auto prev_map_dir = getMapDir();
                    if (prev_map_dir) { delete[] prev_map_dir; }
                    setMapDir(map_dir);
                    SetWindowText(hLabel_map_title, PathFindFileNameW(map_dir));
                }
            }
            break;
        case 101:
            if (HIWORD(wParam) == BN_CLICKED) {
                HWND hCheckbox = GetDlgItem(hWnd, 103);
                int state = SendMessage(hCheckbox, BM_GETCHECK, 0, 0);
                isSingleChecked = (state == BST_CHECKED);
                int ret = startSamase(isSingleChecked);
                if (ret == -1) {
                    std::wstring toPrint = L"맵 파일을 골라주세요!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else if (ret == -2) {
                    std::wstring toPrint = L"여러 StarCraft를 실행할 수 없습니다!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
            }
            break;
        case 102:
            if (HIWORD(wParam) == BN_CLICKED) {
                if (preventDuplicateInject) {
                    std::wstring toPrint = L"debugger가 부착되고 있습니다!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                    break;
                }
                int ret = injectDebugger();
                if (ret == -1) {
                    std::wstring toPrint = L"스타크래프트를 실행하여 주세요!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else if (ret == -2) {
                    std::wstring toPrint = L"한개의 스타크래프트만 실행하여 주세요!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else if (ret == -3) {
                    std::wstring toPrint = L"debugger 부착에 실패했습니다!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else if (ret == -4) {
                    std::wstring toPrint = L"debugger가 이미 부착되어 있습니다!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else if (ret == -5) {
                    std::wstring toPrint = L"맵을 실행해주세요!!";
                    SetWindowText(hMessageLabel, toPrint.c_str());
                    setTextLabel(3, hWnd);
                }
                else {
                    preventDuplicateInject = true;
                    SetTimer(hWnd, 3, 3000, NULL);
                }
            }
            break;
        case 103:
        {
            HWND hCheckbox = GetDlgItem(hWnd, 103);
            int state = SendMessage(hCheckbox, BM_GETCHECK, 0, 0);
            isSingleChecked = (state == BST_CHECKED);

            if (state == BST_UNCHECKED) { SendMessage(hCheckbox, BM_SETCHECK, BST_CHECKED, 0); }
            else { SendMessage(hCheckbox, BM_SETCHECK, BST_UNCHECKED, 0); }
        }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            // if (!isAutoChecked) break;
        }
        else if (wParam == 2) {
            // Timer logic...
            if (decreaseTimer(hWnd)) {
                SetWindowText(hMessageLabel, L"");
                InvalidateRect(hWnd, NULL, TRUE); // Ensure WM_PAINT will be called
            }
        }
        else if (wParam == 3) {
            preventDuplicateInject = false;
            KillTimer(hWnd, 3);
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;
    case WM_USER + 1: // Handle the custom message
        SetWindowText(hMessageLabel, L"signature를 찾을 수 없습니다!");
        playWAVE(L"data\\buzz.wav");
        InvalidateRect(hWnd, NULL, TRUE); // Request redraw
        setTextLabel(5, hWnd);
        break;
    case WM_USER + 2: // Handle the custom message
        SetWindowText(hMessageLabel, L"signature와 debugger의 버전이 다릅니다!");
        playWAVE(L"data\\buzz.wav");
        InvalidateRect(hWnd, NULL, TRUE); // Request redraw
        setTextLabel(5, hWnd);
        break;
    case WM_CTLCOLORSTATIC: // STATIC 컨트롤의 배경색을 설정
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)); // 창 배경 색상 가져오기
            return (LRESULT)hBrush; // STATIC 배경에 색상 설정
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
        break;
    case WM_HOTKEY:
    {
        if (wParam == 1) SendMessage(hOpenButton, BM_CLICK, 0, 0);
        if (wParam == 2) SendMessage(hStartButton, BM_CLICK, 0, 0);
        if (wParam == 3) AttachConsole();
        if (wParam == 4) SendMessage(hDebugButton, BM_CLICK, 0, 0);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
