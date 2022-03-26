/////////////////////////////////////////////////////////////////////////////
// Titletip.h : header file
//
// MFC Grid Control - cell titletips
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
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
// For use with CGridCtrl v2.10+
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "GridConfig.h"

#define TITLETIP_CLASSNAME _T("ZTitleTip")

/////////////////////////////////////////////////////////////////////////////
// CTitleTip window

#if !defined(GRIDCONTROL_NO_TITLETIPS)
class CTitleTip : public CWindowImpl<CTitleTip>
{
	typedef CWindowImpl<CTitleTip, ATL::CWindow> baseClass;
	typedef CTitleTip thisClass;
// Construction
public:
	DECLARE_WND_SUPERCLASS(GetWndClassName(), baseClass::GetWndClassName())

	CTitleTip();
	virtual ~CTitleTip();
	virtual BOOL Create(HWND ParentWnd);

// Attributes
public:
    void SetParentWnd(HWND ParentWnd)	{ m_ParentWnd.Attach(ParentWnd); }
    HWND GetParentWnd()					{ return m_ParentWnd;       }

// Operations
public:
	void Show(CRect rectTitle, LPCTSTR lpszTitleText, 
              int xoffset = 0, LPRECT lpHoverRect = nullptr, 
              const LOGFONT* lpLogFont = nullptr,
              COLORREF crTextClr = CLR_DEFAULT, COLORREF crBackClr = CLR_DEFAULT);
    void Hide();

// Overrides
public:
	virtual BOOL DestroyWindow();

// Implementation
protected:
	CWindow m_ParentWnd;
	CRect  m_rectTitle;
    CRect  m_rectHover;
    DWORD  m_dwLastLButtonDown;
    DWORD  m_dwDblClickMsecs;
    BOOL   m_bCreated;

protected:
	LRESULT OnMouseMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	BEGIN_MSG_MAP_EX(CTitleTip)
		MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessage)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
};
#endif
