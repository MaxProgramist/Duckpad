#include <windows.h>
#include <string>
#include <uxtheme.h>

#include "Button.h"

#pragma comment(lib, "uxtheme.lib")

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))


enum ProgramStates : unsigned char {
    STATE_NONE             = 0,              STATE_SAVED        = 1 << 0,
    STATE_FILE_BUTTONS     = 1 << 1,         STATE_EDIT_BUTTONS = 1 << 2,
    STATE_SETTINGS_BUTTONS = 1 << 3,         STATE_LIGHT_THEME  = 1 << 4
};

enum HotKeyState : unsigned char {
    NONE = 0, CTRL = 1 << 0, SHIFT = 1 << 1, ALT = 1 << 2
};

enum Encoding { ENCODING_ANSI, ENCODING_UTF8, ENCODING_UTF16LE, ENCODING_UTF16BE, ENCODING_UTF32LE, ENCODING_UTF32BE };

struct HotKey {
    unsigned char modifiers;
    WPARAM key;
    void (*action)( HWND );
};

const std::locale SYSTEM_LOCALE("");

unsigned char programState = STATE_SAVED;
short verticalOffsetForEditor = 0, editorOffset = 8;
short windowHeight = 0, windowWidth = 0;
std::wstring currentFilePath;
std::wstring settingsFilePath = L"settings.dat";
HWND edit, fontList;
HWND mainHWND;
WNDPROC oldEditProc;
HBRUSH defaultWindowColor = CreateSolidBrush( RGB(24, 24, 24) );
COLORREF defaultEditorColor = RGB(30, 30, 30);
COLORREF defaultColor = RGB(51, 51, 51);
COLORREF hoveredColor = RGB(69, 69, 69);
COLORREF pressedColor = RGB(0, 122, 204);
COLORREF buttonTextColor = RGB(255, 255, 255);
COLORREF editorTextColor = RGB(212, 212, 212);

LPCWSTR fontName = NULL;

Button fileButton       ( {0, 0, 100, 25},   L"File",         L"",             {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button editButton       ( {100, 0, 200, 25}, L"Edit",         L"",             {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button settingsButton   ( {200, 0, 300, 25}, L"Settings",     L"",             {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );

Button saveFileButton   ( {0, 50, 100, 75},  L"Save",         L"CTRL+S",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button openFileButton   ( {0, 50, 100, 75},  L"Open",         L"CTRL+O",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button saveAsFileButton ( {0, 50, 100, 75},  L"Save As",      L"CTRL+SHIFT+S", {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );

Button undoButton       ( {0, 50, 100, 75},  L"Undo",         L"CTRL+Z",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button copyButton       ( {0, 50, 100, 75},  L"Copy",         L"CTRL+C",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button cutButton        ( {0, 50, 100, 75},  L"Cut",          L"CTRL+X",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button pasteButton      ( {0, 50, 100, 75},  L"Paste",        L"CTRL+V",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button selectAllButton  ( {0, 50, 100, 75},  L"Select All",   L"CTRL+A",       {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );

Button themeButton      ( {0, 50, 100, 75},  L"Theme: Dark", L"",              {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );
Button fontButton       ( {0, 50, 100, 75},  L"Font",        L"",              {140, 30}, {16, 12}, &defaultColor, &hoveredColor, &pressedColor, &buttonTextColor, &fontName );

Button* mainButtons[]     = {&fileButton, &editButton, &settingsButton};
Button* fileButtons[]     = {&saveFileButton, &saveAsFileButton, &openFileButton};
Button* editButtons[]     = {&undoButton, &copyButton, &cutButton, &pasteButton, &selectAllButton};
Button* settingsButtons[] = {&themeButton, &fontButton};
HotKey hotKeys[] = 
{
    { CTRL         , 'S', nullptr }, { CTRL | SHIFT , 'S', nullptr },
    { CTRL         , 'O', nullptr }, { CTRL         , 'Z', nullptr},
    { CTRL         , 'C', nullptr }, { CTRL         , 'X', nullptr }, 
    { CTRL         , 'V', nullptr }, { CTRL         , 'A', nullptr }
};


void SetUpFunctionForButtons ();
void UndoEditor              ();
void CopyEditor              ();
void CutEditor               ();
void PasteEditor             ();
void SelectAllEditor         ();
void ShowFontList            ();
void ChangeColor             ();
void FillFontList            ( HWND listBox );
void ChangeTheme             ( HWND hwnd );
bool HandleHotKeys           ( WPARAM key );
void ChangeWindowName        ( HWND hwnd );
void OpenFile                ( HWND hwnd );
void OpenFileWithoutWindow   ( HWND hwnd );
void SaveFile                ( HWND hwnd );
void SaveFileAs              ( HWND hwnd );
void ShowFileButtons         ( HWND hwnd );
void ShowEditButtons         ( HWND hwnd );
void ShowSettingsButtons     ( HWND hwnd );
void FieldOfButtons_Draw     ( HDC hdc );
void FieldOfButtons_Resize   ( HWND hwnd );
void FieldOfButtons_MoveMouse( POINT mousePoint, HWND hwnd );
void FieldOfButtons_MouseDown( POINT mousePoint, HWND hwnd );
void FieldOfButtons_MouseUp  ( POINT mousePoint, HWND hwnd );
void SaveSettingsWinAPI      ( bool isLightTheme, LPCWSTR fontName );
bool LoadSettingsWinAPI      ( bool& isLightTheme, std::wstring& fontNameOut );


template <typename T> int SizeOfArray( T &array );

LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMessages, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK EditProc  ( HWND hwnd, UINT uMessages, WPARAM wParam, LPARAM lParam );


int CALLBACK EnumFontCallback( const LOGFONTW* logFont, const TEXTMETRICW*, DWORD, LPARAM lParam ) {
    HWND listBox = (HWND)lParam;

    SendMessageW(
        listBox,
        LB_ADDSTRING,
        0,
        (LPARAM)logFont->lfFaceName
    );

    return 1;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow ) {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    std::wstring exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        exeDir = exeDir.substr(0, pos);

    settingsFilePath = exeDir + L"\\settings.dat";
    
    std::wstring loadedFont;
    bool lightTheme;
    if (LoadSettingsWinAPI(lightTheme, loadedFont)) {
        if (lightTheme) programState |= STATE_LIGHT_THEME;
        else programState &= ~STATE_LIGHT_THEME;

        ChangeColor();

        fontName = loadedFont.c_str();
    }

    const wchar_t CLASS_NAME[]  = L"Duckpad Window Class";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor( nullptr, IDC_ARROW );

    RegisterClass( &wc );

    HWND hwnd = CreateWindowEx( 
        0, CLASS_NAME, L"Duckpad", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL
    );

    HICON hIcon = (HICON)LoadImageW(nullptr, L"icon.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    HICON hIconSmall = (HICON)LoadImageW(nullptr, L"icon.ico", IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);


    windowHeight = CW_USEDEFAULT, windowWidth = CW_USEDEFAULT;

    edit = CreateWindowExW(
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | 
        ES_AUTOVSCROLL | ES_AUTOHSCROLL |
        WS_VSCROLL | WS_HSCROLL | ES_LEFT,
        0, 0, 100, 100,
        hwnd, (HMENU)1, hInstance, nullptr
    );
    oldEditProc = (WNDPROC)SetWindowLongPtrW( edit, GWLP_WNDPROC, (LONG_PTR)EditProc );
    SendMessageW( edit, EM_SETLIMITTEXT, 500000, 0 );

    fontList = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"LISTBOX", L"Fonts",
        WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        25, 150, 250, 300,
        hwnd, nullptr, GetModuleHandle(nullptr), nullptr
    );

    FillFontList( fontList );

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW( GetCommandLineW(), &argc );

    if ( argv && argc > 1 ) {
        currentFilePath = argv[1];
        OpenFileWithoutWindow( hwnd );
        ChangeWindowName( hwnd );
    }

    if (argv) LocalFree(argv);


    ShowWindow( hwnd, nCmdShow );
    SetUpFunctionForButtons();

    MSG messages = { };
    while ( GetMessage( &messages, NULL, 0, 0 ) ) {
        TranslateMessage( &messages );
        DispatchMessage( &messages );
    }

    return 0;
}

LRESULT CALLBACK EditProc( HWND hwnd, UINT uMessages, WPARAM wParam, LPARAM lParam ) {
    switch ( uMessages ) {
        case WM_KEYDOWN: {
            if (HandleHotKeys(wParam)) {
                return 0;
            }
        }

        case WM_CHAR: {
            if ( GetKeyState( VK_CONTROL ) & 0x8000 ) {
                return 0;
            }
            break;
        }
    }

    return CallWindowProcW( oldEditProc, hwnd, uMessages, wParam, lParam );
}

LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMessages, WPARAM wParam, LPARAM lParam ) {
    mainHWND = hwnd;

    switch ( uMessages ) {
        case WM_KEYDOWN: {
            if (HandleHotKeys(wParam)) {
                return 0;
            }
        }
        case WM_COMMAND: {
            HWND sender = (HWND)lParam;
            int notifyCode = HIWORD(wParam);
        
            if ( sender == edit && notifyCode == EN_CHANGE )
            {
                programState &= ~STATE_SAVED;
                ChangeWindowName( hwnd );
            }
            else if ( sender == fontList && notifyCode == LBN_DBLCLK )
            {
                int index = SendMessageW( fontList, LB_GETCURSEL, 0, 0 );
                if ( index != LB_ERR )
                {
                    static wchar_t selectedFont[LF_FACESIZE];
                    SendMessageW( fontList, LB_GETTEXT, index, (LPARAM)selectedFont );
                
                    fontName = selectedFont;
                
                    static HFONT currentEditFont = nullptr;
                    if ( currentEditFont )
                        DeleteObject( currentEditFont );
                
                    currentEditFont = CreateUiFont(-16, fontName);
                
                    SendMessageW( edit, WM_SETFONT, (WPARAM)currentEditFont, TRUE );
                
                    ShowWindow( fontList, SW_HIDE );
                    SaveSettingsWinAPI((programState & STATE_LIGHT_THEME) != 0, fontName);
                }
            }
        }
        break;

        case WM_SYSKEYDOWN: {
            if (HandleHotKeys(wParam)) {
                return 0;
            }
        }


        case WM_MOUSEMOVE: {
            POINT mousePoint = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };

            for ( Button* currentButton : mainButtons )
                currentButton->OnMouseMove( mousePoint, hwnd );

            FieldOfButtons_MoveMouse( mousePoint, hwnd );

            break;
        }
        case WM_LBUTTONDOWN: {
            POINT mousePoint = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };

            for ( Button* currentButton : mainButtons )
                currentButton->OnMouseDown( mousePoint, hwnd );

            FieldOfButtons_MouseDown( mousePoint, hwnd );

            break;
        }
        case WM_LBUTTONUP: {
            POINT mousePoint = { GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) };

            for ( Button* currentButton : mainButtons )
                currentButton->OnMouseUp( mousePoint, hwnd );

            FieldOfButtons_MouseUp( mousePoint, hwnd );

            break;
        }
        case WM_SIZE: {
            if ( wParam == SIZE_MINIMIZED )
                return 0;

            windowWidth = LOWORD( lParam );
            windowHeight = HIWORD( lParam );
            verticalOffsetForEditor = 0;

            int xOffsetForButton = 0,
                xOffsetForEditor = ( ( programState & ( STATE_FILE_BUTTONS | STATE_EDIT_BUTTONS | STATE_SETTINGS_BUTTONS) ) ? saveFileButton.GetRightPoint() : 0 );

            for ( Button* currentButton : mainButtons ) {
                currentButton->Scale( xOffsetForButton, 0, windowWidth, windowHeight, hwnd );
                xOffsetForButton = currentButton->GetRightPoint();
                verticalOffsetForEditor = max( currentButton->GetBottomPoint(), verticalOffsetForEditor );
            }

            FieldOfButtons_Resize( hwnd );

            int editorWidth = windowWidth - xOffsetForEditor - editorOffset*2, 
                editorHeight = windowHeight - verticalOffsetForEditor - editorOffset*2;

            MoveWindow( edit, xOffsetForEditor + editorOffset, verticalOffsetForEditor + editorOffset, editorWidth, editorHeight, TRUE );

            return 0;
        }


        case WM_PAINT: {
            PAINTSTRUCT paintStruct;
            HDC hdc = BeginPaint( hwnd, &paintStruct );

            FillRect( hdc, &paintStruct.rcPaint, defaultWindowColor );
            int xOffsetForButton = 0;
            for ( Button* currentButton : mainButtons ) {
                currentButton->Draw( xOffsetForButton, 0, hdc );
                xOffsetForButton = max( xOffsetForButton, currentButton->GetRightPoint() );
            }
            FieldOfButtons_Draw( hdc );
            EndPaint( hwnd, &paintStruct );

            return 0;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = (HDC)wParam;

            if ( programState & STATE_LIGHT_THEME) 
                 SetWindowTheme( edit, L"Explorer", nullptr );
            else SetWindowTheme( edit, L"DarkMode_Explorer", nullptr );

            SetBkColor( hdcEdit, defaultEditorColor );
            SetTextColor( hdcEdit, editorTextColor );

            return (LRESULT)CreateSolidBrush( defaultEditorColor );
        }



        case WM_GETMINMAXINFO: {
            MINMAXINFO* info = (MINMAXINFO*)lParam;

            Pair minWindowSize = {850, 480};

            info->ptMinTrackSize.x = minWindowSize.first;
            info->ptMinTrackSize.y = minWindowSize.second;

            return 0;
        }


        case WM_CLOSE: {
            if ( (programState & STATE_SAVED) == 0 ) {
                int msgboxID = MessageBoxW( hwnd, L"You have no saved file! Save it before exiting?", L"Duckpad warning", MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON3 );


                if ( msgboxID == IDCANCEL ) return 0;
                if ( msgboxID == IDYES ) SaveFile( hwnd );
            }
            DestroyWindow( hwnd );
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage( 0 );
            return 0;
    }

    return DefWindowProc( hwnd, uMessages, wParam, lParam );
}

void SetUpFunctionForButtons() {
    fileButton.OnClickFunction     = ShowFileButtons;
    editButton.OnClickFunction     = ShowEditButtons;
    settingsButton.OnClickFunction = ShowSettingsButtons;

    openFileButton.OnClickFunction   = OpenFile;
    saveFileButton.OnClickFunction   = SaveFile;
    saveAsFileButton.OnClickFunction = SaveFileAs;

    undoButton.OnClickFunction      = [](HWND) { UndoEditor(); };
    copyButton.OnClickFunction      = [](HWND) { CopyEditor(); };
    cutButton.OnClickFunction       = [](HWND) { CutEditor(); };
    pasteButton.OnClickFunction     = [](HWND) { PasteEditor(); };
    selectAllButton.OnClickFunction = [](HWND) { SelectAllEditor(); };

    themeButton.OnClickFunction = ChangeTheme;
    fontButton.OnClickFunction = [](HWND) { ShowFontList(); };

    hotKeys[0].action = SaveFile;
    hotKeys[1].action = SaveFileAs;
    hotKeys[2].action = OpenFile;

    hotKeys[3].action = [](HWND) { UndoEditor(); };
    hotKeys[4].action = [](HWND) { CopyEditor(); };
    hotKeys[5].action = [](HWND) { CutEditor(); };
    hotKeys[6].action = [](HWND) { PasteEditor(); };
    hotKeys[7].action = [](HWND) { SelectAllEditor(); };
}

void FieldOfButtons_Draw( HDC hdc ) {
    int yOffsetForButtons = fileButton.GetBottomPoint() + 5;
    Button** currentButtonArray = nullptr;
    int currentButtonCount = 0;

    if ( programState & STATE_FILE_BUTTONS ) {
        currentButtonArray = fileButtons;
        currentButtonCount = SizeOfArray( fileButtons );
    } else if ( programState & STATE_EDIT_BUTTONS ) {
        currentButtonArray = editButtons;
        currentButtonCount = SizeOfArray( editButtons );
    } else if ( programState & STATE_SETTINGS_BUTTONS ) {
        currentButtonArray = settingsButtons;
        currentButtonCount = SizeOfArray( settingsButtons );
    }

    for ( int i = 0; i < currentButtonCount; i++ ) {
        currentButtonArray[i]->Draw( 0, yOffsetForButtons, hdc );
        yOffsetForButtons = currentButtonArray[i]->GetBottomPoint();
    }
}

void FieldOfButtons_Resize( HWND hwnd ) {
    int yOffsetForButtons = fileButton.GetBottomPoint() + 5;
    for ( Button* currentButton : fileButtons ) {
        currentButton->Scale( 0, yOffsetForButtons, windowWidth, windowHeight, hwnd );
        yOffsetForButtons = max( yOffsetForButtons,currentButton->GetBottomPoint() );
    }

    yOffsetForButtons = fileButton.GetBottomPoint() + 5;
    for ( Button* currentButton : editButtons ) {
        currentButton->Scale( 0, yOffsetForButtons, windowWidth, windowHeight, hwnd );
        yOffsetForButtons = max( yOffsetForButtons,currentButton->GetBottomPoint() );
    }

    yOffsetForButtons = fileButton.GetBottomPoint() + 5;
    for ( Button* currentButton : settingsButtons ) {
        currentButton->Scale( 0, yOffsetForButtons, windowWidth, windowHeight, hwnd );
        yOffsetForButtons = max( yOffsetForButtons,currentButton->GetBottomPoint() );
    }
}

void FieldOfButtons_MoveMouse( POINT mousePoint, HWND hwnd ) {
    if ( programState & STATE_FILE_BUTTONS ) 
        for ( Button* currentButton : fileButtons )
            currentButton->OnMouseMove( mousePoint, hwnd );
    else if ( programState & STATE_EDIT_BUTTONS ) 
        for ( Button* currentButton : editButtons )
            currentButton->OnMouseMove( mousePoint, hwnd );
    else if ( programState & STATE_SETTINGS_BUTTONS ) 
        for ( Button* currentButton : settingsButtons )
            currentButton->OnMouseMove( mousePoint, hwnd );
}

void FieldOfButtons_MouseDown( POINT mousePoint, HWND hwnd ) {
    if ( programState & STATE_FILE_BUTTONS ) 
        for ( Button* currentButton : fileButtons )
            currentButton->OnMouseDown( mousePoint, hwnd );
    else if ( programState & STATE_EDIT_BUTTONS ) 
        for ( Button* currentButton : editButtons )
            currentButton->OnMouseDown( mousePoint, hwnd );
    else if ( programState & STATE_SETTINGS_BUTTONS ) 
        for ( Button* currentButton : settingsButtons )
            currentButton->OnMouseDown( mousePoint, hwnd );
}

void FieldOfButtons_MouseUp( POINT mousePoint, HWND hwnd ) {
    if ( programState & STATE_FILE_BUTTONS ) 
        for ( Button* currentButton : fileButtons )
            currentButton->OnMouseUp( mousePoint, hwnd );
    else if ( programState & STATE_EDIT_BUTTONS ) 
        for ( Button* currentButton : editButtons )
            currentButton->OnMouseUp( mousePoint, hwnd );
    else if ( programState & STATE_SETTINGS_BUTTONS ) 
        for ( Button* currentButton : settingsButtons )
            currentButton->OnMouseUp( mousePoint, hwnd );
}

void ResizeEditor(HWND hwnd) {
    RECT rect; GetWindowRect(hwnd, &rect);

    int xOffsetForEditor = ( ( programState & ( STATE_FILE_BUTTONS | STATE_EDIT_BUTTONS | STATE_SETTINGS_BUTTONS ) ) ? saveFileButton.GetRightPoint() : 0 );

    int editorWidth = windowWidth - xOffsetForEditor - editorOffset*2, 
        editorHeight = windowHeight - verticalOffsetForEditor - editorOffset*2;

    MoveWindow( edit, xOffsetForEditor + editorOffset, verticalOffsetForEditor + editorOffset, 
        editorWidth, editorHeight, TRUE );
}

void ShowFileButtons( HWND hwnd ) {
    programState &= ~STATE_EDIT_BUTTONS;
    programState &= ~STATE_SETTINGS_BUTTONS;
    programState ^= STATE_FILE_BUTTONS;
    InvalidateRect( hwnd, nullptr, TRUE );

    ResizeEditor( hwnd );
}

void ShowEditButtons( HWND hwnd ) {
    programState &= ~STATE_FILE_BUTTONS;
    programState &= ~STATE_SETTINGS_BUTTONS;
    programState ^= STATE_EDIT_BUTTONS;
    InvalidateRect( hwnd, nullptr, TRUE );

    ResizeEditor( hwnd );
}

void ShowSettingsButtons( HWND hwnd ) {
    programState &= ~STATE_FILE_BUTTONS;
    programState &= ~STATE_EDIT_BUTTONS;
    programState ^= STATE_SETTINGS_BUTTONS;
    InvalidateRect( hwnd, nullptr, TRUE );

    ResizeEditor( hwnd );
}

Encoding DetectEncoding(const BYTE* data, DWORD size) {
    if (size >= 3 && data[0]==0xEF && data[1]==0xBB && data[2]==0xBF) return ENCODING_UTF8;
    if (size >= 2 && data[0]==0xFF && data[1]==0xFE) return ENCODING_UTF16LE;
    if (size >= 2 && data[0]==0xFE && data[1]==0xFF) return ENCODING_UTF16BE;
    if (size >= 4 && data[0]==0xFF && data[1]==0xFE && data[2]==0x00 && data[3]==0x00) return ENCODING_UTF32LE;
    if (size >= 4 && data[0]==0x00 && data[1]==0x00 && data[2]==0xFE && data[3]==0xFF) return ENCODING_UTF32BE;
    return ENCODING_UTF8;
}

std::wstring ConvertToWstring(const BYTE* data, DWORD size, Encoding enc) {
    if (!data || size == 0) return L"";

    int skip = 0;
    int charsNeeded = 0;
    std::wstring result;

    switch (enc) {
        case ENCODING_UTF8:
            if (size >= 3 && data[0]==0xEF && data[1]==0xBB && data[2]==0xBF) skip = 3;
            charsNeeded = MultiByteToWideChar(CP_UTF8, 0, (const char*)data + skip, size - skip, nullptr, 0);
            result.resize(charsNeeded);
            MultiByteToWideChar(CP_UTF8, 0, (const char*)data + skip, size - skip, &result[0], charsNeeded);
            break;

        case ENCODING_UTF16LE:
            if (size >= 2 && data[0]==0xFF && data[1]==0xFE) skip = 2;
            result.resize((size - skip)/2);
            memcpy(&result[0], data + skip, (size - skip));
            break;

        case ENCODING_UTF16BE:
            if (size >= 2 && data[0]==0xFE && data[1]==0xFF) skip = 2;
            result.resize((size - skip)/2);
            for (size_t i = 0; i < result.size(); i++) {
                result[i] = data[skip + i*2] << 8 | data[skip + i*2 + 1];
            }
            break;

        case ENCODING_UTF32LE:
            if (size >= 4 && data[0]==0xFF && data[1]==0xFE && data[2]==0x00 && data[3]==0x00) skip = 4;
            result.resize((size - skip)/4);
            for (size_t i = 0; i < result.size(); i++) {
                DWORD val = *(DWORD*)(data + skip + i*4);
                result[i] = (wchar_t)val;
            }
            break;

        case ENCODING_UTF32BE:
            if (size >= 4 && data[0]==0x00 && data[1]==0x00 && data[2]==0xFE && data[3]==0xFF) skip = 4;
            result.resize((size - skip)/4);
            for (size_t i = 0; i < result.size(); i++) {
                DWORD val = (data[skip + i*4] << 24) | (data[skip + i*4 + 1] << 16) | (data[skip + i*4 + 2] << 8) | data[skip + i*4 + 3];
                result[i] = (wchar_t)val;
            }
            break;

        case ENCODING_ANSI:
            charsNeeded = MultiByteToWideChar(CP_ACP, 0, (const char*)data, size, nullptr, 0);
            result.resize(charsNeeded);
            MultiByteToWideChar(CP_ACP, 0, (const char*)data, size, &result[0], charsNeeded);
            break;
    }

    return result;
}

void SaveFileAs( HWND hwnd ) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"All files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;

    if ( !GetSaveFileNameW( &ofn ) ) return;

    currentFilePath = fileName;
    ChangeWindowName( hwnd );
    SaveFile( hwnd );
}

void SaveFile(HWND hwnd) {
    if (currentFilePath.empty()) {
        SaveFileAs(hwnd);
        return;
    }

    int textLength = GetWindowTextLengthW(edit);
    if (textLength <= 0) return;

    std::wstring buffer;
    buffer.resize(textLength + 1);
    GetWindowTextW(edit, &buffer[0], textLength + 1);

    HANDLE file = CreateFileW(
        currentFilePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE) return;

    // записуємо BOM для UTF-16LE
    WORD bom = 0xFEFF;
    DWORD bytesWritten = 0;
    WriteFile(file, &bom, sizeof(bom), &bytesWritten, nullptr);

    // записуємо сам текст
    WriteFile(file, buffer.c_str(), textLength * sizeof(wchar_t), &bytesWritten, nullptr);

    CloseHandle(file);

    programState |= STATE_SAVED;
    ChangeWindowName(hwnd);
}

bool OpenFileDialog( HWND hwnd, std::wstring& filePath ) {
    wchar_t fileName[MAX_PATH] = L"";

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"All files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT;

    if ( GetOpenFileNameW( &ofn ) ) {
        filePath = fileName;
        return true;
    }
    return false;
}

void OpenFile( HWND hwnd ) {
    if ( !OpenFileDialog( hwnd, currentFilePath ) ) return;
    if ( currentFilePath.empty() ) return;

    HANDLE file = CreateFileW(
        currentFilePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if ( file == INVALID_HANDLE_VALUE ) return;

    DWORD fileSize = GetFileSize( file, nullptr );
    if ( fileSize == INVALID_FILE_SIZE || fileSize == 0 ) {
        CloseHandle( file );
        SetWindowTextW( edit, L"" );
        return;
    }

    BYTE* buffer = new BYTE[fileSize];
    DWORD bytesRead = 0;
    ReadFile( file, buffer, fileSize, &bytesRead, nullptr );
    CloseHandle( file );

    Encoding enc = DetectEncoding( buffer, bytesRead );
    std::wstring text = ConvertToWstring( buffer, bytesRead, enc );

    SetWindowTextW( edit, text.c_str() );
    ChangeWindowName( hwnd );

    delete[] buffer;
}

void OpenFileWithoutWindow(HWND hwnd) {
    if (currentFilePath.empty()) return;

    HANDLE file = CreateFileW(
        currentFilePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE) return;

    DWORD fileSize = GetFileSize(file, nullptr);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(file);
        SetWindowTextW(edit, L"");
        return;
    }

    BYTE* buffer = new BYTE[fileSize];
    DWORD bytesRead = 0;
    if (!ReadFile(file, buffer, fileSize, &bytesRead, nullptr)) {
        CloseHandle(file);
        delete[] buffer;
        return;
    }
    CloseHandle(file);

    Encoding enc = DetectEncoding(buffer, bytesRead);
    std::wstring text;

    int skip = 0;

    switch (enc) {
        case ENCODING_UTF8:
            if (bytesRead >= 3 && buffer[0]==0xEF && buffer[1]==0xBB && buffer[2]==0xBF) skip = 3;
            {
                int len = MultiByteToWideChar(CP_UTF8, 0, (char*)buffer + skip, bytesRead - skip, nullptr, 0);
                text.resize(len);
                MultiByteToWideChar(CP_UTF8, 0, (char*)buffer + skip, bytesRead - skip, &text[0], len);
            }
            break;

        case ENCODING_UTF16LE:
            if (bytesRead >= 2 && buffer[0]==0xFF && buffer[1]==0xFE) skip = 2;
            {
                size_t count = (bytesRead - skip) / 2;
                text.resize(count);
                for (size_t i = 0; i < count; i++) {
                    text[i] = *((wchar_t*)(buffer + skip + i*2));
                }
            }
            break;
        
        case ENCODING_UTF16BE:
            if (bytesRead >= 2 && buffer[0]==0xFE && buffer[1]==0xFF) skip = 2;
            {
                size_t count = (bytesRead - skip) / 2;
                text.resize(count);
                for (size_t i = 0; i < count; i++) {
                    text[i] = (buffer[skip + i*2] << 8) | buffer[skip + i*2 + 1];
                }
            }
            break;


        case ENCODING_ANSI:
            {
                int len = MultiByteToWideChar(CP_ACP, 0, (char*)buffer + skip, bytesRead - skip, nullptr, 0);
                text.resize(len);
                MultiByteToWideChar(CP_ACP, 0, (char*)buffer + skip, bytesRead - skip, &text[0], len);
            }
            break;

        default:
            text = L""; // для UTF32BE/LE поки можна відкласти
            break;
    }

    SetWindowTextW(edit, text.c_str());
    ChangeWindowName(hwnd);

    delete[] buffer;
}

void ChangeWindowName( HWND hwnd ) {
    std::wstring windowTitle = L"Duckpad — " + currentFilePath;
    if ( ( programState & STATE_SAVED) == 0 ) windowTitle += L" *";

    SetWindowTextW( hwnd, windowTitle.c_str() );
}

void UndoEditor() {
    if ( SendMessageW( edit, EM_CANUNDO, 0, 0 ) )
        SendMessageW( edit, EM_UNDO, 0, 0 );
}

void CopyEditor() {
    SendMessageW( edit, WM_COPY, 0, 0 );
}

void CutEditor() {
    SendMessageW( edit, WM_CUT, 0, 0 );
}

void PasteEditor() {
    SendMessageW( edit, WM_PASTE, 0, 0 );
}

void SelectAllEditor() {
    SendMessageW( edit, EM_SETSEL, 0, -1 );
}

void ChangeTheme( HWND hwnd ) {
    programState ^= STATE_LIGHT_THEME;

    ChangeColor();

    InvalidateRect( hwnd, nullptr, TRUE );
    ResizeEditor( hwnd );
    SaveSettingsWinAPI((programState & STATE_LIGHT_THEME) != 0, fontName);
}

void ShowFontList() {
    ShowWindow( fontList, SW_SHOW );
    SetForegroundWindow( fontList );
}

bool IsKeyDown( int key ) {
    return ( GetKeyState( key ) & 0x8000 ) != 0;
}

bool HandleHotKeys( WPARAM key ) {
    for (int i = 0; i < SizeOfArray(hotKeys); i++) {
        HotKey& hotKey = hotKeys[i];

        if (hotKey.key != key) continue;
        if ( ( hotKey.modifiers & SHIFT ) >> 1 != IsKeyDown( VK_SHIFT ) )  continue;
        if ( ( hotKey.modifiers & ALT )  >> 2  != IsKeyDown( VK_MENU ))    continue;
        if ( ( hotKey.modifiers & CTRL )  != IsKeyDown( VK_CONTROL )) continue;

        hotKey.action(mainHWND);
        return true;
    }
    return false;
}

template <typename T> int SizeOfArray( T &array ) {

    return sizeof(array) / sizeof(array[0]);
}

void FillFontList( HWND listBox ) {
    HDC hdc = GetDC( nullptr );

    LOGFONTW logFont = {};
    logFont.lfCharSet = DEFAULT_CHARSET;

    EnumFontFamiliesExW(
        hdc,
        &logFont,
        EnumFontCallback,
        (LPARAM)listBox,
        0
    );

    ReleaseDC( nullptr, hdc );
}

void SaveSettingsWinAPI( bool isLightTheme, LPCWSTR fontName ) {
    HANDLE file = CreateFileW( settingsFilePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
    if ( file == INVALID_HANDLE_VALUE ) return;

    DWORD bytesWritten = 0;

    WriteFile( file, &isLightTheme, sizeof(isLightTheme), &bytesWritten, nullptr );

    uint32_t length = fontName ? (uint32_t)wcslen( fontName ) : 0;
    WriteFile( file, &length, sizeof(length), &bytesWritten, nullptr );

    if ( length > 0 ) {
        WriteFile( file, fontName, length * sizeof(wchar_t), &bytesWritten, nullptr );
    }

    CloseHandle( file );
}

bool LoadSettingsWinAPI( bool& isLightTheme, std::wstring& fontNameOut ) {
    HANDLE file = CreateFileW( settingsFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
    if ( file == INVALID_HANDLE_VALUE ) return false;

    DWORD bytesRead = 0;

    ReadFile( file, &isLightTheme, sizeof(isLightTheme), &bytesRead, nullptr );

    uint32_t length = 0;
    ReadFile( file, &length, sizeof(length), &bytesRead, nullptr );

    fontNameOut.clear();
    if ( length > 0 ) {
        fontNameOut.resize( length );
        ReadFile( file, &fontNameOut[0], length * sizeof(wchar_t), &bytesRead, nullptr );
    }

    CloseHandle( file );
    return true;
}

void ChangeColor() {
    if (programState & STATE_LIGHT_THEME) {
        defaultWindowColor = CreateSolidBrush( RGB(243, 243, 243) );
        defaultEditorColor = RGB(255, 255, 255);
        defaultColor       = RGB(0, 122, 204);
        hoveredColor       = RGB(28, 134, 238);
        pressedColor       = RGB(0, 90, 158);
        buttonTextColor    = RGB(255, 255, 255);
        editorTextColor    = RGB(37, 37, 38);
        themeButton.mainText = L"Theme: Light";
    } else {
        defaultWindowColor = CreateSolidBrush( RGB(24, 24, 24) );
        defaultEditorColor = RGB(30, 30, 30);
        defaultColor       = RGB(51, 51, 51);
        hoveredColor       = RGB(69, 69, 69);
        pressedColor       = RGB(0, 122, 204);
        buttonTextColor    = RGB(255, 255, 255);
        editorTextColor    = RGB(212, 212, 212);
        themeButton.mainText = L"Theme: Dark";
    }
}