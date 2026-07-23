#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <richedit.h>
#include <stdio.h>
#include <shlwapi.h>

#define ID_EDIT 101

// Menu Command IDs
#define ID_FILE_NEW 9001
#define ID_FILE_OPEN 9002
#define ID_FILE_SAVE 9003
#define ID_FILE_SAVEAS 9004
#define ID_FORMAT_BOLDFONT 9005

HWND hEdit;
HFONT hFont;
int fontSize = 20;
int fontWidgetWeight = FW_NORMAL;
char currentFilePath[MAX_PATH] = "";

// Function to update font styling/sizing dynamically with crisp subpixel anti-aliasing
void UpdateFont(HWND hwnd) {
    if (hFont) DeleteObject(hFont);
    
    hFont = CreateFont(
        fontSize, 0, 0, 0, fontWidgetWeight, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Consolas")
    );
    
    SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// Function to load file contents into the editor
void LoadFileToEditor(HWND hwnd, const char *path) {
    FILE *file = fopen(path, "r");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        long len = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *text = (char*)GlobalAlloc(GPTR, len + 1);
        fread(text, 1, len, file);
        fclose(file);
        SetWindowText(hEdit, text);
        GlobalFree(text);
        strcpy(currentFilePath, path);
        SetWindowText(hwnd, currentFilePath);
    }
}

// Function to handle Save As dialogue
BOOL SaveFileAs(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0Python (*.py)\0*.py\0C/C++ (*.c;*.cpp)\0*.c;*.cpp\0Java (*.java)\0*.java\0HTML (*.html)\0*.html\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        FILE *file = fopen(ofn.lpstrFile, "w");
        if (file != NULL) {
            int len = GetWindowTextLength(hEdit);
            char *text = (char*)GlobalAlloc(GPTR, len + 1);
            GetWindowText(hEdit, text, len + 1);
            fprintf(file, "%s", text);
            fclose(file);
            GlobalFree(text);
            strcpy(currentFilePath, ofn.lpstrFile);
            SetWindowText(hwnd, currentFilePath);
            return TRUE;
        }
    }
    return FALSE;
}

// Function to Save File
void SaveFile(HWND hwnd) {
    if (strlen(currentFilePath) == 0) {
        SaveFileAs(hwnd);
    } else {
        FILE *file = fopen(currentFilePath, "w");
        if (file != NULL) {
            int len = GetWindowTextLength(hEdit);
            char *text = (char*)GlobalAlloc(GPTR, len + 1);
            GetWindowText(hEdit, text, len + 1);
            fprintf(file, "%s", text);
            fclose(file);
            GlobalFree(text);
        }
    }
}

// Function to Open File Dialog
void OpenFileDlg(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Supported\0*.txt;*.html;*.py;*.c;*.cpp;*.java\0Text Files (*.txt)\0*.txt\0Code Files\0*.html;*.py;*.c;*.cpp;*.java\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        LoadFileToEditor(hwnd, ofn.lpstrFile);
    }
}

// Subclass procedure for the Edit control to intercept keystrokes & auto-pairs
LRESULT CALLBACK EditSubProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            BOOL ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000);
            BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000);

            if (ctrlPressed && wParam == 'N') {
                SetWindowText(hEdit, "");
                strcpy(currentFilePath, "");
                SetWindowText(GetParent(hwnd), "LetterPad - Untitled");
                return 0;
            }
            if (ctrlPressed && wParam == 'O') {
                OpenFileDlg(GetParent(hwnd));
                return 0;
            }
            if (ctrlPressed && shiftPressed && wParam == 'S') {
                SaveFileAs(GetParent(hwnd));
                return 0;
            }
            else if (ctrlPressed && wParam == 'S') {
                SaveFile(GetParent(hwnd));
                return 0;
            }
            if (ctrlPressed && wParam == 'A') {
                SendMessage(hEdit, EM_SETSEL, 0, -1);
                return 0;
            }
            if (ctrlPressed && wParam == 'L') {
                int lineIdx = SendMessage(hEdit, EM_LINEFROMCHAR, -1, 0);
                int lineStart = SendMessage(hEdit, EM_LINEINDEX, lineIdx, 0);
                int lineLen = SendMessage(hEdit, EM_LINELENGTH, lineStart, 0);
                SendMessage(hEdit, EM_SETSEL, lineStart, lineStart + lineLen);
                return 0;
            }
            if (ctrlPressed && wParam == '7') {
                fontWidgetWeight = (fontWidgetWeight == FW_NORMAL) ? FW_BOLD : FW_NORMAL;
                UpdateFont(GetParent(hwnd));
                return 0;
            }
            break;
        }
        case WM_CHAR: {
            char openChar = 0;
            char closeChar = 0;
            BOOL shiftPressed = (GetKeyState(VK_SHIFT) & 0x8000);

            if (wParam == '(') { openChar = '('; closeChar = ')'; }
            else if (wParam == '{') { openChar = '{'; closeChar = '}'; }
            else if (wParam == '[') { openChar = '['; closeChar = ']'; }
            else if (wParam == '\'' && shiftPressed) { openChar = '\''; closeChar = '\''; }
            else if (wParam == '"') { openChar = '"'; closeChar = '"'; }

            if (openChar != 0) {
                DWORD start, end;
                SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
                DefSubclassProc(hwnd, uMsg, wParam, lParam);
                char closeStr[2] = {closeChar, '\0'};
                SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)closeStr);
                SendMessage(hEdit, EM_SETSEL, start + 1, start + 1);
                return 0;
            }
            break;
        }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// Window Procedure to handle menu inputs and events
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            LoadLibrary(TEXT("Msftedit.dll"));

            HMENU hMenuBar = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            HMENU hFormatMenu = CreatePopupMenu();

            AppendMenu(hFileMenu, MF_STRING, ID_FILE_NEW, TEXT("New\tCtrl+N"));
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, TEXT("Open...\tCtrl+O"));
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, TEXT("Save\tCtrl+S"));
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVEAS, TEXT("Save As...\tCtrl+Shift+S"));

            AppendMenu(hFormatMenu, MF_STRING, ID_FORMAT_BOLDFONT, TEXT("Toggle Bold Font Weight\tCtrl+7"));

            AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, TEXT("File"));
            AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFormatMenu, TEXT("Format"));
            SetMenu(hwnd, hMenuBar);

            hEdit = CreateWindowEx(
                0, TEXT("RICHEDIT50W"), TEXT(""),
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                0, 0, 800, 600, hwnd, (HMENU)ID_EDIT, GetModuleHandle(NULL), NULL
            );
            
            SendMessage(hEdit, EM_SETEVENTMASK, 0, ENM_CHANGE);
            UpdateFont(hwnd);
            SetWindowSubclass(hEdit, EditSubProc, 1, 0);
            break;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_NEW:
                    SetWindowText(hEdit, "");
                    strcpy(currentFilePath, "");
                    SetWindowText(hwnd, "LetterPad - Untitled");
                    break;
                case ID_FILE_OPEN:
                    OpenFileDlg(hwnd);
                    break;
                case ID_FILE_SAVE:
                    SaveFile(hwnd);
                    break;
                case ID_FILE_SAVEAS:
                    SaveFileAs(hwnd);
                    break;
                case ID_FORMAT_BOLDFONT:
                    fontWidgetWeight = (fontWidgetWeight == FW_NORMAL) ? FW_BOLD : FW_NORMAL;
                    UpdateFont(hwnd);
                    break;
            }
            break;

        case WM_SIZE:
            MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break;

        case WM_MOUSEWHEEL:
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
                if (zDelta > 0) {
                    if (fontSize < 56) {
                        fontSize += 2;
                        UpdateFont(hwnd);
                    }
                } else {
                    if (fontSize > 8) {
                        fontSize -= 2;
                        if (fontSize < 8) fontSize = 8;
                        UpdateFont(hwnd);
                    }
                }
                return 0;
            }
            break;

        case WM_DESTROY:
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HMODULE hUser32 = LoadLibrary(TEXT("user32.dll"));
    if (hUser32) {
        typedef BOOL(WINAPI *SetProcessDPIAwareProc)(void);
        SetProcessDPIAwareProc pSetProcessDPIAware = (SetProcessDPIAwareProc)GetProcAddress(hUser32, "SetProcessDPIAware");
        if (pSetProcessDPIAware) {
            pSetProcessDPIAware();
        }
        FreeLibrary(hUser32);
    }

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("LetterPadClass");

    if (!RegisterClassEx(&wc)) return 1;

    HWND hwnd = CreateWindowEx(
        0, TEXT("LetterPadClass"), TEXT("LetterPad - Untitled"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}