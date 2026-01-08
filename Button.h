#pragma once
#include <windows.h>
#include <string>
#include <fstream>

#define min(  a,b ) (  (  a<b )?a:b )
#define max(  a,b ) (  (  a>b )?a:b )

struct Pair { int first, second; };

void DrawRoundedRect( HDC hdc, RECT rect, int radius, HBRUSH brush, HPEN pen );
HFONT CreateUiFont( int height, LPCWSTR fontName, bool bold = false, bool italic = false );

enum ButtonState : unsigned char {
    STATE_DEFAULT = 0,              STATE_HOVER    = 1 << 0,
    STATE_PRESSED = 1 << 1,         STATE_DISABLED = 1 << 2
};

class Button {
    RECT buttonRect;
    std::wstring hotkeyText;
    Pair maxSize;
    Pair percentageOfScreen;

    COLORREF* defaultColor = nullptr; 
    COLORREF* hoverColor = nullptr;
    COLORREF* pressedColor = nullptr; 
    COLORREF* textColor = nullptr; 

    LPCWSTR* fontName = nullptr;

    unsigned char buttonState = STATE_DEFAULT;

public:
    std::wstring mainText;
    void ( *OnClickFunction )( HWND ) = nullptr; 

    Button( RECT rect, const std::wstring& main_text, const std::wstring& hotkey_text, Pair maxS, Pair percentage, COLORREF* defaultColor, COLORREF* hoverColor, COLORREF* pressColor, COLORREF* textColor, LPCWSTR* textFont )
        : buttonRect( rect ), mainText( main_text ), hotkeyText( hotkey_text ), maxSize( maxS ),  percentageOfScreen( percentage ), defaultColor( defaultColor ), hoverColor( hoverColor ), pressedColor( pressColor ), textColor( textColor ), fontName( textFont ) {}

    void Draw( int startX, int startY, HDC hdc ) {
        COLORREF currentColor;
        buttonRect.left = startX, buttonRect.top = startY;
        RECT textRect = {buttonRect.left + 5, buttonRect.top + 5, buttonRect.right - 5, buttonRect.bottom - 5};

        if ( buttonState & STATE_PRESSED ) currentColor = *pressedColor;
        else if ( buttonState & STATE_HOVER ) currentColor = *hoverColor;
        else currentColor = *defaultColor;

        HBRUSH background = CreateSolidBrush( currentColor );
        HFONT fontMain    = CreateUiFont( -15, *fontName );
        HFONT fontHotkey  = CreateUiFont( -8, *fontName, false, true );
        HFONT oldFont     = (HFONT)SelectObject( hdc, fontMain );


        int radius = 10;
        HPEN pen = CreatePen( PS_SOLID, 1, RGB(0,0,0) );
        HPEN oldPen = (HPEN)SelectObject( hdc, pen );
        HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, background );

        RoundRect( hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom, radius, radius );

        SelectObject( hdc, oldPen );
        SelectObject( hdc, oldBrush );
        DeleteObject( pen );
        DeleteObject( background );

        SetBkMode( hdc, TRANSPARENT );
        SetTextColor( hdc, *textColor );
        DrawTextW( hdc, mainText.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE );
        SelectObject( hdc, fontHotkey );
        DrawTextW( hdc, hotkeyText.c_str(), -1, &textRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE );
        SelectObject( hdc, oldFont );

        DeleteObject( fontMain );
        DeleteObject( fontHotkey );
    }

    void Scale( int xPosition, int yPosition, int windowWidth, int windowHeight, HWND hwnd ) {
        Pair screenScaled = {windowWidth * percentageOfScreen.first / 100, windowHeight * percentageOfScreen.second / 100};

        Pair buttonNewSize = {min( screenScaled.first, maxSize.first ), min( screenScaled.second, maxSize.second )};

        buttonRect = {xPosition, yPosition, buttonNewSize.first + xPosition, buttonNewSize.second + yPosition};

        InvalidateRect( hwnd, nullptr, TRUE );
    }

    void OnMouseMove( POINT mousePoint, HWND hwnd ) {
        bool hoverNow = PtInRect( &buttonRect, mousePoint );

        if ( hoverNow == ( buttonState & STATE_HOVER ) ) return;

        if ( hoverNow ) buttonState |= STATE_HOVER;
        else buttonState &= ~STATE_HOVER;

        InvalidateRect( hwnd, &buttonRect, TRUE );
    }

    void OnMouseDown( POINT mousePoint, HWND hwnd ) {
        if ( !PtInRect( &buttonRect, mousePoint ) ) return;

        buttonState |= STATE_PRESSED;

        InvalidateRect( hwnd, &buttonRect, TRUE );
    }

    void OnMouseUp( POINT mousePoint, HWND hwnd ) {
        buttonState &= ~STATE_PRESSED;

        if ( ( *OnClickFunction ) != nullptr && PtInRect( &buttonRect, mousePoint ) ) OnClickFunction( hwnd );

        InvalidateRect( hwnd, &buttonRect, TRUE );
    }

    int GetRightPoint() {
        return buttonRect.right;
    }

    int GetBottomPoint() {
        return buttonRect.bottom;
    }
};


HFONT CreateUiFont( int height, LPCWSTR fontName, bool bold, bool italic ) {
    return CreateFontW(
        height, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        italic, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
        CLEARTYPE_QUALITY, VARIABLE_PITCH, fontName
    );
}

void DrawRoundedRect( HDC hdc, RECT rect, int radius, HBRUSH brush, HPEN pen ) {
    HBRUSH oldBrush = (HBRUSH)SelectObject( hdc, brush );
    HPEN oldPen = (HPEN)SelectObject( hdc, pen );

    RoundRect( hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius );

    SelectObject( hdc, oldBrush );
    SelectObject( hdc, oldPen );
}