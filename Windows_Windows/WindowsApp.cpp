//***********************************************
// WindowsWindows
// Copyright 2024 Accurate Tool Company, Inc.
//===============================================
// WindowsApp.cpp
// ----------------------------------------------
// // 08/01/2024 MS-24.01.02.07 Added stack windows button (broke scrolling again)
// 07/25/2024 MS-24.01.02.06 Fixed scrolling, cleaned up window destruction for better memory management, updated child window size for sub control windows
// 07/25/2024 MS-24.01.02.05 Added global oss to store active windows ostreamstring and added function to get all active windows to the create event
// 07/25/2024 MS-24.01.02.2 Updated to be compatible with template window
// 07/23/2024 MS-24.01.01.0 created
//-----------------------------------------------
// Main window source code

#include "WindowsApp.h"
#include <string>
#include <sstream>
#include <iostream>

#define STACK 1
#define WM_STACK_BUTTON (WM_USER + 1)


WindowsApp::WindowsApp() {}


std::wostringstream WindowsApp::oss;
HWND WindowsApp::WindowHandle;

void WindowsApp::RunMessageLoop() {
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

PCWSTR WindowsApp::ClassName() const { return L"Windows Window Extension"; }

HRESULT WindowsApp::Initialize()
{
    HRESULT hr;
    hr = this->Create(L"Windows Window Extension Window",
        WS_OVERLAPPEDWINDOW | WS_VSCROLL,
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        1000
    );
    return hr;
}

LRESULT WindowsApp::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 0;

    switch (uMsg) {
    case WM_CREATE:
        HandleCreate();
        break;
    case WM_PAINT:
        HandlePaint();
        break;
    case WM_SIZE:
        HandleResize();
        break;
    case WM_VSCROLL:
        HandleScroll(wParam);
        break;
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            int id = LOWORD(wParam);
            switch (id) {
            case STACK:
                StackWindows();
            }
        }
        break;
    case WM_CLOSE:
        if (m_hwnd != NULL) {
            DestroyWindow(m_hwnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

void WindowsApp::PrintActiveWindows() {
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
    
}

void WindowsApp::StackWindows()
{   
    int stackPos = 10;
    for (WindowControl* ctrl : WindowsVector) {
        SetWindowPos(ctrl->GetInstanceHandle(), HWND_TOPMOST, stackPos, stackPos, 500, 500, NULL);
        stackPos += 50;
    }
}

HRESULT WindowsApp::HandleCreate() {
    HRESULT hr = S_OK;
    if (m_hwnd && m_hwnd != 0) {
        ShowWindow(m_hwnd, SW_SHOWNORMAL);
        m_hControlWindow = CreateWindowEx(
            0, TEXT("STATIC"), NULL,
            WS_CHILD | WS_VISIBLE | SS_LEFT ,
            0, 0, 500, 9000, m_hwnd, NULL, (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE), NULL);

        UpdateWindow(m_hwnd);
       
    }
    CreateControlOpts();
    PrintActiveWindows();
    int ControlY = ((WindowsVector.size() + 1) * 100) + 75;
    SetWindowPos(m_hControlWindow, NULL, 0, 0, 220, ControlY, SW_SHOWNORMAL);
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = ControlY;
    si.nPage = 1000;
    if (m_hwnd != 0) {
        SetScrollInfo(m_hwnd, SB_VERT, &si, TRUE);
    }

    return hr;
}

void WindowsApp::CreateControlOpts() {
   // m_hControlOptions = CreateWindowEx(
   //     0, TEXT("STATIC"), NULL,
   //     WS_CHILD | WS_VISIBLE | SS_LEFT,
   //     0, 0, 500, 100, m_hControlWindow, NULL, (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE), NULL);

    m_hStackButton = CreateWindowEx(
        0,
        L"BUTTON",
        L"STACK",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        20, 20, 100, 30,
        m_hwnd,
        (HMENU)STACK,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL);

}

void WindowsApp::HandlePaint() {
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    GetClientRect(m_hwnd, &rect);
    hdc = BeginPaint(m_hwnd, &ps);
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    EndPaint(m_hwnd, &ps);

}

void WindowsApp::HandleResize(){
    RECT resizeRect;
    if (m_hwnd != NULL) {
        GetWindowRect(m_hwnd, &resizeRect);
        SetWindowPos(m_hControlWindow, NULL, 0, 0, resizeRect.right, resizeRect.bottom, NULL);
        UpdateWindow(m_hwnd);
    }
}

void WindowsApp::HandleScroll(WPARAM wParam) {
    int scrollCode = LOWORD(wParam);
    int nPos;
    int nOldPos;
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
    GetScrollInfo(m_hwnd, SB_VERT, &si);

    nOldPos = si.nPos;

    switch (scrollCode) {
        case SB_TOP:            
            nPos = si.nMin; 
            break;
        case SB_BOTTOM:         
            nPos = si.nMax; 
            break;
        case SB_LINEUP:
            nPos = si.nPos - 1;
            break;
        case SB_LINEDOWN:
            nPos = si.nPos + 1;
            break;
        case SB_THUMBPOSITION:
            nPos = si.nTrackPos;
            break;
        default:
        case SB_THUMBTRACK:
            nPos = si.nPos;
            break;
    }
    SetScrollPos(m_hwnd, SB_VERT, nPos, TRUE);

    nPos = GetScrollPos(m_hwnd, SB_VERT);

    ScrollWindowEx(m_hwnd, 0, (int)(nOldPos - nPos) * 1,
        NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);



    UpdateWindow(m_hwnd);

}
