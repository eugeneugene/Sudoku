//////////////////////////////////////////////////////////////////////
// InPlaceEdit.h : header file
//
// MFC Grid Control - inplace editing class
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
#include "GridCType.h"

class CInPlaceEdit : public CWindowImpl<CInPlaceEdit, CEdit>,
	public CEditCommands<CInPlaceEdit>
{
// Construction
public:
    CInPlaceEdit(CWindow Parent, CRect& rect, DWORD dwStyle, UINT nID,
                 int nRow, int nColumn, LPCTSTR sInitText, UINT nFirstChar, _gctype_t ctype);

// Operations
public:
     void EndEdit();
 
// Implementation
public:
	virtual ~CInPlaceEdit() { }
 
// Generated message map functions
protected:
	// As soon as this edit loses focus, kill it.
	void OnKillFocus(CWindow wndFocus)
	{
		//DefWindowProc();
		EndEdit();
	}
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) // Stupid win95 accelerator key problem workaround - Matt Weagle.
	{
		// SetMsgHandled(TRUE); - by default
		return;
	}
	UINT OnGetDlgCode(LPMSG /*lpMsg*/) const
	{
		return DLGC_WANTALLKEYS;
	}
	virtual void OnFinalMessage(_In_ HWND /*hWnd*/)
	{
		delete this;
	}
	BEGIN_MSG_MAP_EX(CMyEdit)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_CHAR(OnChar)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_SYSCHAR(OnSysChar)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		CHAIN_MSG_MAP_ALT(CEditCommands<CInPlaceEdit>, 1)
		DEFAULT_REFLECTION_HANDLER()
    END_MSG_MAP()

private:
    int       m_nRow;
    int       m_nColumn;
    CString   m_sInitText;
    UINT      m_nLastChar;
    BOOL      m_bExitOnArrows;
    CRect     m_Rect;
	_gctype_t m_ctype;
};
