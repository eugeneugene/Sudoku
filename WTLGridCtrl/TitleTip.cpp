#include "stdafx.h"
#include "GridConfig.h"

////////////////////////////////////////////////////////////////////////////
// TitleTip.cpp : implementation file
//
// Based on code by Zafir Anjum
//
// Adapted by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with CGridCtrl v2.20+
//
// History
//         10 Apr 1999  Now accepts a LOGFONT pointer and 
//                      a tracking rect in Show(...)  (Chris Maunder)
//         18 Apr 1999  Resource leak in Show fixed by Daniel Gehriger
//          8 Mar 2000  Added double-click fix found on codeguru
//                      web site but forgot / can't find who contributed it
//         28 Mar 2000  Aqiruse (marked with //FNA)
//                      Titletips now use cell color
//         18 Jun 2000  Delayed window creation added
//
/////////////////////////////////////////////////////////////////////////////
 
#if !defined(GRIDCONTROL_NO_TITLETIPS)

#include "TitleTip.h"

/////////////////////////////////////////////////////////////////////////////
// CTitleTip

CTitleTip::CTitleTip()
{
	// Register the window class if it has not already been registered.
	WNDCLASS wndcls;
	HINSTANCE hInst = _Module.GetModuleInstance();
	if(!(::GetClassInfo(hInst, TITLETIP_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class
		wndcls.style			= CS_SAVEBITS;
		wndcls.lpfnWndProc		= ::DefWindowProc;
		wndcls.cbClsExtra		= wndcls.cbWndExtra = 0;
		wndcls.hInstance		= hInst;
		wndcls.hIcon			= nullptr;
		wndcls.hCursor			= LoadCursor(hInst, IDC_ARROW);
		wndcls.hbrBackground	= (HBRUSH)(COLOR_INFOBK +1);
		wndcls.lpszMenuName		= nullptr;
		wndcls.lpszClassName	= TITLETIP_CLASSNAME;

		if (!::RegisterClass(&wndcls))
			AtlThrowLastWin32();
	}

    m_dwLastLButtonDown = ULONG_MAX;
    m_dwDblClickMsecs   = GetDoubleClickTime();
    m_bCreated          = FALSE;
}

CTitleTip::~CTitleTip()
{
	if (m_hWnd)
		DestroyWindow();
	::UnregisterClass(TITLETIP_CLASSNAME, ModuleHelper::GetModuleInstance());
}

/////////////////////////////////////////////////////////////////////////////
// CTitleTip message handlers

BOOL CTitleTip::Create(HWND hWndParent)
{
    // Already created?
    if (m_bCreated)
        return TRUE;

	DWORD dwStyle = WS_BORDER | WS_POPUP; 
	DWORD dwExStyle = WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
	m_ParentWnd = hWndParent;

	HWND hWnd = ::CreateWindowEx(dwExStyle, TITLETIP_CLASSNAME, TITLETIP_CLASSNAME,
			dwStyle, rcDefault.left, rcDefault.top, rcDefault.right - rcDefault.left,
			rcDefault.bottom - rcDefault.top, hWndParent, nullptr,
			_AtlBaseModule.GetModuleInstance(), nullptr);
	if (nullptr == hWnd)
		AtlThrowLastWin32();
	if (!SubclassWindow(hWnd))
		AtlThrowLastWin32();

	return (m_bCreated = TRUE);
}

BOOL CTitleTip::DestroyWindow() 
{
    m_bCreated = FALSE;
	
	return baseClass::DestroyWindow();
}

// Show 		 - Show the titletip if needed
// rectTitle	 - The rectangle within which the original 
//				    title is constrained - in client coordinates
// lpszTitleText - The text to be displayed
// xoffset		 - Number of pixel that the text is offset from
//				   left border of the cell
void CTitleTip::Show(CRect rectTitle, LPCTSTR lpszTitleText, int xoffset /*=0*/,
                     LPRECT lpHoverRect /*=nullptr*/,
                     const LOGFONT* lpLogFont /*=nullptr*/,
                     COLORREF crTextClr /* CLR_DEFAULT */,
                     COLORREF crBackClr /* CLR_DEFAULT */)
{
    if (!IsWindow())
        Create(m_ParentWnd);

	ATLASSERT(IsWindow());

    if (rectTitle.IsRectEmpty())
        return;

	// If titletip is already displayed, don't do anything.
	if(IsWindowVisible()) 
		return;

    m_rectHover = (lpHoverRect != nullptr)? lpHoverRect : rectTitle;
    m_rectHover.right++; m_rectHover.bottom++;

	m_ParentWnd.ClientToScreen(m_rectHover);
    ScreenToClient(m_rectHover);
	//ATLTRACE(_T("rectHover: %d %d %d %d: \n"), m_rectHover.left, m_rectHover.top, m_rectHover.right, m_rectHover.bottom);

	// Do not display the titletip is app does not have focus
	if(GetFocus() == nullptr)
		return;

	// Define the rectangle outside which the titletip will be hidden.
	// We add a buffer of one pixel around the rectangle
	m_rectTitle.top    = -1;
	m_rectTitle.left   = -xoffset-1;
	m_rectTitle.right  = rectTitle.Width()-xoffset;
	m_rectTitle.bottom = rectTitle.Height()+1;

	// Determine the width of the text
	m_ParentWnd.ClientToScreen(rectTitle);

	CClientDC dc(m_hWnd);
	CString strTitle;
    strTitle += _T(" ");
    strTitle += lpszTitleText; 
    strTitle += _T(" ");

	CFont font;
	CFontHandle OldFont;
    if (lpLogFont)
    {
        font.CreateFontIndirect(lpLogFont);
	    OldFont = dc.SelectFont(font.m_hFont);
    }
    else
    {
        // use same font as ctrl
	    OldFont = dc.SelectFont(m_ParentWnd.GetFont());
    }

	CSize size;
	dc.GetTextExtent(strTitle, strTitle.GetLength(), &size);

    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    size.cx += tm.tmOverhang;

	CRect rectDisplay = rectTitle;
	rectDisplay.left += xoffset;
	rectDisplay.right = rectDisplay.left + size.cx + xoffset;
    
    // Do not display if the text fits within available space
    if (rectDisplay.right > rectTitle.right-xoffset)
    {
		// Show the titletip
        SetWindowPos(HWND_TOP, rectDisplay.left, rectDisplay.top,
			rectDisplay.Width(), rectDisplay.Height(),
			SWP_SHOWWINDOW|SWP_NOACTIVATE);
        
        // FNA - handle colors correctly
        if (crBackClr != CLR_DEFAULT)
        {
		    CBrush backBrush;
			backBrush.CreateSolidBrush(crBackClr);
		    CBrushHandle OldBrush(dc.SelectBrush(backBrush.m_hBrush));
		    CRect rect;
		    dc.GetClipBox(&rect);     // Erase the area needed 

		    dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(),  PATCOPY);
		    dc.SelectBrush(OldBrush);
	    }
        // Set color
        if (crTextClr != CLR_DEFAULT)//FNA
            dc.SetTextColor(crTextClr);//FA

        dc.SetBkMode(TRANSPARENT);
        dc.TextOut(0, (rectDisplay.Height() - size.cy)/2, strTitle);
        SetCapture();
    }
    
    dc.SelectFont(OldFont);
}

void CTitleTip::Hide()
{
  	if (!IsWindow())
        return;

    if (GetCapture() == m_hWnd)
        ReleaseCapture();

	ShowWindow(SW_HIDE);
	//ATLTRACE("Hide TitleTip\n");
}

LRESULT CTitleTip::OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT ret = 0;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		{
			int hittest;
			CWindow Wnd;
			POINTS pts = MAKEPOINTS(lParam);
			POINT  point;
			point.x = pts.x;
			point.y = pts.y;

			ClientToScreen(&point);
			Hide();

			Wnd = WindowFromPoint(point);
			if (Wnd.m_hWnd == nullptr)
				return FALSE;

			if(Wnd.m_hWnd == m_hWnd) 
				Wnd = m_ParentWnd;

			hittest = (int)Wnd.SendMessage(WM_NCHITTEST,0,MAKELONG(point.x,point.y));

			if (hittest == HTCLIENT) 
			{
				Wnd.ScreenToClient(&point);
				lParam = MAKELONG(point.x,point.y);
			}
			else 
			{
				switch (uMsg)
				{
				case WM_LBUTTONDOWN: 
					uMsg = WM_NCLBUTTONDOWN;
					break;
				case WM_LBUTTONDBLCLK: 
					uMsg = WM_NCLBUTTONDBLCLK;
					break;
				case WM_RBUTTONDOWN: 
					uMsg = WM_NCRBUTTONDOWN;
					break;
				case WM_MBUTTONDOWN: 
					uMsg = WM_NCMBUTTONDOWN;
					break;
				}
				wParam = hittest;
				lParam = MAKELONG(point.x,point.y);
			}

			// If this is the 2nd WM_LBUTTONDOWN in x milliseconds,
			// post a WM_LBUTTONDBLCLK message instead of a single click.
			return Wnd.PostMessage(uMsg, wParam, lParam);
		}
	case WM_MOUSEMOVE:
		{
			CPoint point(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			UINT nFlags = (UINT)wParam;
			ATLTRACE(_T("Point: %d %d\n"), point.x, point.y);
			if (!m_rectHover.PtInRect(point)) 
			{
				Hide();

				// Forward the message
				ClientToScreen(&point);
				CWindow Wnd(WindowFromPoint(point));
				if (Wnd.m_hWnd == m_hWnd)
					Wnd = m_ParentWnd;

				int hittest = (int)Wnd.SendMessage(WM_NCHITTEST,0,MAKELONG(point.x,point.y));

				if (hittest == HTCLIENT) {
					Wnd.ScreenToClient(&point);
					ret = Wnd.PostMessage(WM_MOUSEMOVE, nFlags, MAKELONG(point.x,point.y));
				} else {
					ret = Wnd.PostMessage(WM_NCMOUSEMOVE, hittest, MAKELONG(point.x,point.y));
				}
				return ret;
			}
		}
	}
	return ret;
}
#endif // GRIDCONTROL_NO_TITLETIPS
