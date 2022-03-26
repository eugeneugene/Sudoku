#include "stdafx.h"
#include "InPlaceEdit.h"
#include "GridCtrl.h"
#include "gridctype.h"

// InPlaceEdit.cpp : implementation file
//
// Adapted by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// The code contained in this file is based on the original
// CInPlaceEdit from http://www.codeguru.com/listview/edit_subitems.shtml
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
// History:
//         10 May 1998  Uses GVN_ notifications instead of LVN_,
//                      Sends notification messages to the parent, 
//                      instead of the parent's parent.
//         15 May 1998  There was a problem when editing with the in-place editor, 
//                      there arises a general protection fault in user.exe, with a 
//                      few qualifications:
//                         (1) This only happens with owner-drawn buttons;
//                         (2) This only happens in Win95
//                         (3) This only happens if the handler for the button does not 
//                             create a new window (even an AfxMessageBox will avoid the 
//                             crash)
//                         (4) This will not happen if Spy++ is running.
//                      PreTranslateMessage was added to route messages correctly.
//                      (Matt Weagle found and fixed this problem)
//         26 Jul 1998  Removed the ES_MULTILINE style - that fixed a few probs!
//          6 Aug 1998  Added nID to the constructor param list
//          6 Sep 1998  Space no longer clears selection when starting edit (Franco Bez)
//         10 Apr 1999  Enter, Tab and Esc key prob fixed (Koay Kah Hoe)
//                      Workaround for bizzare "shrinking window" problem in CE
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit

CInPlaceEdit::CInPlaceEdit(CWindow Parent, CRect& rect, DWORD dwStyle, UINT nID,
                           int nRow, int nColumn, LPCTSTR sInitText, 
                           UINT nFirstChar, _gctype_t ctype)
{
    m_sInitText     = sInitText;
    m_nRow          = nRow;
    m_nColumn       = nColumn;
    m_nLastChar     = 0; 
    m_bExitOnArrows = (nFirstChar != VK_LBUTTON);    // If mouse click brought us here,
                                                     // then no exit on arrows

    m_Rect = rect;  // For bizarre CE bug.
    
    DWORD dwEditStyle = dwStyle | WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL; //|ES_MULTILINE
    if (!Create(Parent, rect, _T("InPlaceEdit"), dwEditStyle, 0, nID)) return;
    
    SetFont(Parent.GetFont());
    
    SetWindowText(sInitText);
    SetFocus();
    
	m_ctype = ctype;
	
	switch (nFirstChar) {
        case VK_LBUTTON: 
        case VK_RETURN:   SetSel((int)_tcslen(sInitText), -1); return;
        case VK_BACK:     SetSel((int)_tcslen(sInitText), -1); break;
        case VK_TAB:
        case VK_DOWN: 
        case VK_UP:   
        case VK_RIGHT:
        case VK_LEFT:  
        case VK_NEXT:  
        case VK_PRIOR: 
        case VK_HOME:
        case VK_SPACE:
        case VK_END:      SetSel(0,-1); return;
        default:          SetSel(0,-1);
    }

    // Added by KiteFly. When entering DBCS chars into cells the first char was being lost
    // SenMessage changed to PostMessage (John Lagerquist)
    if(nFirstChar < 0x80)
        PostMessage(WM_CHAR, nFirstChar);   
    else
        PostMessage(WM_IME_CHAR, nFirstChar);
}

////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit message handlers

// If an arrow key (or associated) is pressed, then exit if
//  a) The Ctrl key was down, or
//  b) m_bExitOnArrows == TRUE
void CInPlaceEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ((nChar == VK_PRIOR || nChar == VK_NEXT ||
        nChar == VK_DOWN  || nChar == VK_UP   ||
        nChar == VK_RIGHT || nChar == VK_LEFT) &&
        (m_bExitOnArrows || GetKeyState(VK_CONTROL) < 0))
    {
        m_nLastChar = nChar;
        GetParent().SetFocus();
        return;
    }
	DefWindowProc();
}

void CInPlaceEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!_IsCType(nChar, m_ctype | GCT_CONTROL, GridLocale))
		return;
	if (nChar == VK_TAB || nChar == VK_RETURN)
    {
        m_nLastChar = nChar;
        GetParent().SetFocus();    // This will destroy this window
        return;
    }
    if (nChar == VK_ESCAPE) 
    {
        SetWindowText(m_sInitText);    // restore previous text
        m_nLastChar = nChar;
        GetParent().SetFocus();
        return;
    }
	DefWindowProc();
    
    // Resize edit control if needed
    
    // Get text extent
    CString str;
	int nLen = GetWindowTextLength();
	int nRetLen = -1;
	LPTSTR lpstr = str.GetBufferSetLength(nLen);
	if (lpstr != nullptr)
	{
		nRetLen = GetWindowText(lpstr, nLen + 1);
		str.ReleaseBuffer();
	}
	ATLASSERT(nRetLen <= nLen);

    // add some extra buffer
    str += _T("MM");	// "M" is just not enough - EP
    
    CWindowDC dc(m_hWnd);
    CFontHandle FontDC = dc.SelectFont(GetFont());
    CSize size;
	BOOL bRes = dc.GetTextExtent(str, (int)_tcslen(str), &size);
	ATLASSERT(bRes);
    dc.SelectFont(FontDC);
       
    // Get client rect
    CRect ParentRect;
    GetParent().GetClientRect(&ParentRect);
    
    // Check whether control needs to be resized
    // and whether there is space to grow
    if (size.cx > m_Rect.Width())
    {
        if(size.cx + m_Rect.left < ParentRect.right)
            m_Rect.right = m_Rect.left + size.cx;
        else
            m_Rect.right = ParentRect.right;
        MoveWindow(&m_Rect);
    }
}

////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit implementation

void CInPlaceEdit::EndEdit()
{
    CString str;

    // EFW - BUG FIX - Clicking on a grid scroll bar in a derived class
    // that validates input can cause this to get called multiple times
    // causing assertions because the edit control goes away the first time.
    static BOOL bAlreadyEnding = FALSE;

    if(bAlreadyEnding)
        return;

    bAlreadyEnding = TRUE;
	int nLen = GetWindowTextLength();
	int nRetLen = -1;
	LPTSTR lpstr = str.GetBufferSetLength(nLen);
	if (lpstr != nullptr)
	{
		nRetLen = GetWindowText(lpstr, nLen + 1);
		str.ReleaseBuffer();
	}
	ATLASSERT(nRetLen <= nLen);

    // Send Notification to parent
    GV_DISPINFO dispinfo;

    dispinfo.hdr.hwndFrom = m_hWnd;
    dispinfo.hdr.idFrom   = GetDlgCtrlID();
    dispinfo.hdr.code     = GVN_ENDLABELEDIT;

    dispinfo.item.mask    = LVIF_TEXT|LVIF_PARAM;
    dispinfo.item.row     = m_nRow;
    dispinfo.item.col     = m_nColumn;
    dispinfo.item.strText  = str;
    dispinfo.item.lParam  = (LPARAM) m_nLastChar;

	GetOwner(m_hWnd).SendMessage(WM_NOTIFY, GetDlgCtrlID(), (LPARAM)&dispinfo);

    // Close this window (OnDestroy will delete this)
    if (IsWindow())
        SendMessage(WM_CLOSE, 0, 0);
    bAlreadyEnding = FALSE;
}
