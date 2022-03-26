#include "stdafx.h"
#include "GridCtrl.h"
#include "InPlaceEdit.h"
#include "gridctype.h"

// GridCtrl.cpp : implementation file
//
// MFC Grid Control v2.26
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// The code contained in this file was based on the original
// WorldCom Grid control written by Joe Willcoxson,
//        mailto:chinajoe@aol.com
//        http://users.aol.com/chinajoe
// (These addresses may be out of date) The code has gone through 
// so many modifications that I'm not sure if there is even a single 
// original line of code. In any case Joe's code was a great 
// framework on which to build.
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
// Expect bugs!
// 
// Please use and enjoy, and let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into 
// this file. 
//
//  History:
//  --------
//  This control is constantly evolving, sometimes due to new features that I
//  feel are necessary, and sometimes due to existing bugs. Where possible I
//  have credited the changes to those who contributed code corrections or
//  enhancements (names in brackets) or code suggestions (suggested by...)
//
//          1.0 - 1.13   20 Feb 1998 - 6 May 1999
//                          First release version. Progressed from being a basic
//                          grid based on the original WorldCom Grid control
//                          written by Joe Willcoxson (mailto:chinajoe@aol.com,
//                          http://users.aol.com/chinajoe) to something a little
//                          more feature rich. Rewritten so many times I doubt
//                          there is a single line of Joe's code left. Many, many,
//                          MANY people sent in bug reports and fixes. Thank you
//                          all.
//
//          2.0         1 Feb 2000
//                          Rewritten to make the grid more object oriented, in
//                          that the CGridCell class now takes care of cell-specific
//                          tasks. This makes the code more robust, but more
//                          importantly it allows the simple insertion of other
//                          types of cells.
//                          
//          2.10       11 Mar 2000 - Ken Bertelson and Chris Maunder
//                          - Additions for virtual CGridCell support of embedded tree 
//                            & cell buttons implementation
//                          - Optional WYSIWYG printing
//                          - Awareness of hidden (0 width/height) rows and columns for 
//                            key movements,  cut, copy, paste, and autosizing
//                          - CGridCell can make title tips display any text rather than 
//                            cell text only
//                          - Minor vis bug fixes
//                          - CGridCtrl now works with CGridCellBase instead of CGridCell
//                            This is a taste of things to come.
//
//          2.20       30 Jul 2000 - Chris Maunder
//                          - Font storage optimised (suggested by Martin Richter)
//                          - AutoSizeColumn works on either column header, data or both
//                          - EnsureVisible. The saga continues... (Ken)
//                          - Rewrote exception handling
//                          - Added TrackFocusCell and FrameFocusCell properties, as well as
//                            ExpandLastColumn (suggested by Bruce E. Stemplewski).
//                          - InsertColumn now allows you to insert columns at the end of the
//                            column range (David Weibel)
//                          - Shift-cell-selection more intuitive
//                          - API change: Set/GetGridColor now Set/GetGridLineColor
//                          - API change: Set/GetBkColor now Set/GetGridBkColor
//                          - API change: Set/GetTextColor, Set/GetTextBkColor depricated 
//                          - API change: Set/GetFixedTextColor, Set/GetFixedBkColor depricated 
//                          - Stupid DDX_GridControl workaround removed.
//                          - Added "virtual mode" via Set/GetVirtualMode
//                          - Added SetCallbackFunc to allow callback functions in virtual mode
//                          - Added Set/GetAutoSizeStyle
//                          - AutoSize() bug fixed
//                          - added GVIS_FIXEDROW, GVIS_FIXEDCOL states
//                          - added Get/SetFixed[Row|Column]Selection
//                          - cell "Get" methods now const'd. Sorry folks...
//                          - GetMouseScrollLines now uses win98/W2K friendly code
//                          - WS_EX_CLIENTEDGE style now implicit
//
//                          [ Only the latest version and major version changes will be shown ]
////
//          2.25       13 Mar 2004 - Chris Maunder
//                                 - Minor changes so it will compile in VS.NET (inc. Whidbey)
//                                 - Fixed minor bug in EnsureVisible - Junlin Xu
//                                 - Changed AfxGetInstanceHandle for AfxGetResourceHandle in RegisterWindowClass
//					               - Various changes thanks to Yogurt
//
//          2.26       13 Dec 2005 - Pierre Couderc
//                                 - Added sort in Virtual mode
//                                 - Change row/column order programatically or via drag and drop
//                                 - Added save/restore layer (for undoing row/column order changes)
//
// TODO:   1) Implement sparse grids (super easy now)
//         2) Fix it so that as you drag select, the speed of selection increases
//            with time.
//         3) Scrolling is still a little dodgy (too much grey area). I know there
//            is a simple fix but it's been a low priority
//
// ISSUES: 1) WindowFromPoint seems to do weird things in W2K. Causing problems for
//            the rigt-click-on-titletip code.
//
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// Global Variable
///////////////////////////////////////////////////////////////////////////////////

_locale_t GridLocale;

// Get the number of lines to scroll with each mouse wheel notch
// Why doesn't windows give us this function???
UINT GetMouseScrollLines()
{
    int nScrollLines = 3;            // reasonable default

#ifndef _WIN32_WCE
    // Do things the hard way in win95
    OSVERSIONINFO VersionInfo;
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (IsWindowsVersionOrGreater(5, 0 ,0))
    {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER,  _T("Control Panel\\Desktop"),
            0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
            TCHAR szData[128];
            DWORD dwKeyDataType;
            DWORD dwDataBufSize = sizeof(szData);
            
            if (RegQueryValueEx(hKey, _T("WheelScrollLines"), nullptr, &dwKeyDataType,
                (LPBYTE) &szData, &dwDataBufSize) == ERROR_SUCCESS)
            {
                nScrollLines = _tcstoul(szData, nullptr, 10);
            }
            RegCloseKey(hKey);
        }
    }
    // win98 or greater
    else
		SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
#endif

    return nScrollLines;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl

CGridCtrl::CGridCtrl(int nRows, int nCols, int nFixedRows, int nFixedCols)
{
#ifndef GRIDCONTROL_NO_CLIPBOARD
	HRESULT hRes = ::OleInitialize(nullptr);
	if(!SUCCEEDED(hRes))
		AtlThrowLastWin32();
#endif

	RegisterWindowClass();

    // Store the system colours in case they change. The gridctrl uses
    // these colours, and in OnSysColorChange we can check to see if
    // the gridctrl colours have been changed from the system colours.
    // If they have, then leave them, otherwise change them to reflect
    // the new system colours.
    m_crWindowText        = ::GetSysColor(COLOR_WINDOWTEXT);
    m_crWindowColour      = ::GetSysColor(COLOR_WINDOW);
    m_cr3DFace            = ::GetSysColor(COLOR_3DFACE);
    m_crShadow            = ::GetSysColor(COLOR_3DSHADOW);
    m_crGridLineColour    = RGB(192,192,192);

    m_nRows               = 0;
    m_nCols               = 0;
    m_nFixedRows          = 0;
    m_nFixedCols          = 0;
	m_InDestructor        = false;

    m_bVirtualMode        = FALSE;
    m_pfnCallback         = nullptr;

    m_nVScrollMax         = 0;          // Scroll position
    m_nHScrollMax         = 0;
    m_nRowsPerWheelNotch  = GetMouseScrollLines(); // Get the number of lines
                                                   // per mouse wheel notch to scroll
    m_nBarState           = GVL_NONE;
    m_MouseMode           = MOUSE_NOTHING;
    m_nGridLines          = GVL_BOTH;
    m_bEditable           = TRUE;
    m_bListMode           = FALSE;
    m_bSingleRowSelection = FALSE;
    m_bSingleColSelection = FALSE;
    m_bLMouseButtonDown   = FALSE;
    m_bRMouseButtonDown   = FALSE;
    m_bAllowDraw          = TRUE;       // allow draw updates
    m_bEnableSelection    = TRUE;
    m_bFixedColumnSelection = TRUE;
    m_bFixedRowSelection  = TRUE;
    m_bAllowRowResize     = TRUE;
    m_bAllowColumnResize  = TRUE;
    m_bSortOnClick        = FALSE;      // Sort on header row click
    m_bHandleTabKey       = TRUE;
#ifdef _WIN32_WCE
    m_bDoubleBuffer       = FALSE;      // Use double buffering to avoid flicker?
#else
    m_bDoubleBuffer       = TRUE;       // Use double buffering to avoid flicker?
#endif
    m_bTitleTips          = TRUE;       // show cell title tips

    m_bHiddenColUnhide    = TRUE;       // 0-width columns can be expanded via mouse
    m_bHiddenRowUnhide    = TRUE;       // 0-Height rows can be expanded via mouse

    m_bAllowColHide       = TRUE;       // Columns can be contracted to 0-width via mouse
    m_bAllowRowHide       = TRUE;       // Rows can be contracted to 0-height via mouse

    m_bAscending          = TRUE;       // sorting stuff
    m_nSortColumn         = -1;
	m_pfnCompare		  = nullptr;
	m_pfnVirtualCompare   = nullptr;
    m_nAutoSizeColumnStyle = GVS_BOTH;  // Autosize grid using header and data info

    m_nTimerID            = 0;          // For drag-selection
    m_nTimerInterval      = 25;         // (in milliseconds)
    m_nResizeCaptureRange = 3;          // When resizing columns/row, the cursor has to be
                                        // within +/-3 pixels of the dividing line for
                                        // resizing to be possible
    m_pImageList          = nullptr;       // Images in the grid
    m_bAllowDragAndDrop   = FALSE;      // for drag and drop - EFW - off by default
    m_bTrackFocusCell     = TRUE;       // Track Focus cell?
    m_bFrameFocus         = TRUE;       // Frame the selected cell?
	m_AllowReorderColumn  = false;
	m_QuitFocusOnTab      = false;
	m_AllowSelectRowInFixedCol = false;
	m_bDragRowMode        = false; // allow to drop a line over another one to change row order
    m_pRtcDefault = new CGridCell;

    SetupDefaultCells();
    SetGridBkColor(m_crShadow);

    // Set up the initial grid size
    SetRowCount(nRows);
    SetColumnCount(nCols);
    SetFixedRowCount(nFixedRows);
    SetFixedColumnCount(nFixedCols);

    SetTitleTipTextClr(CLR_DEFAULT);  //FNA
    SetTitleTipBackClr(CLR_DEFAULT); 

    // set initial selection range (ie. none)
    m_SelectedCellMap.RemoveAll();
    m_PrevSelectedCellMap.RemoveAll();

#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	m_bWysiwygPrinting    = FALSE;	// use size-to-width printing
    // EFW - Added to support shaded/unshaded printout and
    // user-definable margins.
    m_bShadedPrintOut = TRUE;
    SetPrintMarginInfo(2, 2, 4, 4, 1, 1, 1);
#endif
#ifndef GRIDCONTROL_NO_DRAGDROP
	m_DragEnterCell = CCellID(-1, -1);
	m_LastDragOverCell = CCellID(-1, -1);
#endif

	GridLocale = _get_current_locale();
}

CGridCtrl::~CGridCtrl()
{
	m_InDestructor = true;
    DeleteAllItems();

	if (m_pRtcDefault)
		delete m_pRtcDefault;

#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	if (!m_PrinterFont.IsNull())
		m_PrinterFont.DeleteObject();
#endif

#ifndef GRIDCONTROL_NO_CLIPBOARD
	//::OleFlushClipboard();
	::OleUninitialize();
#endif
	::UnregisterClass(GRIDCTRL_CLASSNAME, ModuleHelper::GetModuleInstance());
}

// Register the window class if it has not already been registered.
BOOL CGridCtrl::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = ModuleHelper::GetModuleInstance();

    if (!(::GetClassInfo(hInst, GRIDCTRL_CLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = nullptr;
#ifndef _WIN32_WCE_NO_CURSOR
        wndcls.hCursor          = AtlLoadSysCursor(IDC_ARROW);
#else
        wndcls.hCursor          = 0;
#endif
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = nullptr;
        wndcls.lpszClassName    = GRIDCTRL_CLASSNAME;

        if (!::RegisterClass(&wndcls))
        {
            AtlTrace(_T("Class registration failed"));
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CGridCtrl::Initialise()
{
    // Stop re-entry problems
    static BOOL bInProcedure = FALSE;
    if (bInProcedure)
        return FALSE;
    bInProcedure = TRUE;

#ifndef GRIDCONTROL_NO_DRAGDROP
	FORMATETC fmtetc = {GRID_CF, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	m_pDropTarget = new CGridDropTarget(m_hWnd, m_hWnd);
	m_pDropTarget->AddRef();
	m_pDropTarget->AddSuportedFormat(fmtetc);
#endif

#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.SetParentWnd(m_hWnd);
#endif

    if (IsWindow())
        ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	// Kludge: Make sure the client edge shows
	// This is so horrible it makes my eyes water.
	CRect rect;
	GetWindowRect(rect);
	CWindow Parent = GetParent();
	if (Parent.m_hWnd)
		Parent.ScreenToClient(rect);
	rect.InflateRect(1,1);	MoveWindow(rect);
	rect.DeflateRect(1,1);  MoveWindow(rect);
 
	SetScrollPos32(SB_BOTH, 0, FALSE);

    bInProcedure = FALSE;
    return TRUE;
}

void CGridCtrl::OnDestroy()
{
#ifndef GRIDCONTROL_NO_DRAGDROP
	m_pDropTarget->Release();
#endif
	::RevokeDragDrop(m_hWnd);
}

// creates the control - use like any other window create control
BOOL CGridCtrl::Create(const RECT& rect, CWindow ParentWnd, UINT nID, DWORD dwStyle)
{
    ATLASSERT(ParentWnd.m_hWnd);

	CRect r(rect);
    if (!CWindow::Create(GRIDCTRL_CLASSNAME, ParentWnd.m_hWnd, r, nullptr, dwStyle, NULL, nID))
        return FALSE;

    //Initialise(); - called in PreSubclassWnd

    // The number of rows and columns will only be non-zero if the constructor
    // was called with non-zero initialising parameters. If this window was created
    // using a dialog template then the number of rows and columns will be 0 (which
    // means that the code below will not be needed - which is lucky 'cause it ain't
    // gonna get called in a dialog-template-type-situation.

	m_arRowHeights.SetCount(m_nRows);    // initialize row heights
	m_arColWidths.SetCount(m_nCols);     // initialize column widths

    int i;
    for (i = 0; i < m_nRows; i++)
        m_arRowHeights[i] = m_cellDefault.GetHeight();
    for (i = 0; i < m_nCols; i++)
        m_arColWidths[i] = m_cellDefault.GetWidth();

    return TRUE;
}

void CGridCtrl::SetupDefaultCells()
{
    m_cellDefault.SetGrid(this);            // Normal editable cell
    m_cellFixedColDef.SetGrid(this);        // Cell for fixed columns
    m_cellFixedRowDef.SetGrid(this);        // Cell for fixed rows
    m_cellFixedRowColDef.SetGrid(this);     // Cell for area overlapped by fixed columns/rows

    m_cellDefault.SetTextClr(m_crWindowText);   
    m_cellDefault.SetBackClr(m_crWindowColour); 
    m_cellFixedColDef.SetTextClr(m_crWindowText);
    m_cellFixedColDef.SetBackClr(m_cr3DFace);
    m_cellFixedRowDef.SetTextClr(m_crWindowText);
    m_cellFixedRowDef.SetBackClr(m_cr3DFace);
    m_cellFixedRowColDef.SetTextClr(m_crWindowText);
    m_cellFixedRowColDef.SetBackClr(m_cr3DFace);
}

// Sends a message to the parent in the form of a WM_NOTIFY message with
// a NM_GRIDVIEW structure attached
LRESULT CGridCtrl::SendMessageToParent(int nRow, int nCol, int nMessage) const
{
    if (!IsWindow())
        return 0;

    NM_GRIDVIEW nmgv;
    nmgv.iRow         = nRow;
    nmgv.iColumn      = nCol;
    nmgv.hdr.hwndFrom = m_hWnd;
    nmgv.hdr.idFrom   = GetDlgCtrlID();
    nmgv.hdr.code     = nMessage;

    CWindow Owner = GetOwner(m_hWnd);
    if (Owner.IsWindow())
        return Owner.SendMessage(WM_NOTIFY, nmgv.hdr.idFrom, (LPARAM)&nmgv);
    else
        return 0;
}

// Send a request to the parent to return information on a given cell
LRESULT CGridCtrl::SendDisplayRequestToParent(GV_DISPINFO* pDisplayInfo) const
{
    if (!IsWindow())
        return 0;

    // Fix up the message headers
    pDisplayInfo->hdr.hwndFrom = m_hWnd;
    pDisplayInfo->hdr.idFrom   = GetDlgCtrlID();
    pDisplayInfo->hdr.code     = GVN_GETDISPINFO;

    // Send the message
    CWindow Owner = GetOwner(m_hWnd);
    if (Owner.IsWindow())
        return Owner.SendMessage(WM_NOTIFY, pDisplayInfo->hdr.idFrom, (LPARAM)pDisplayInfo);
    else
        return 0;
}

// Send a hint to the parent about caching information
LRESULT CGridCtrl::SendCacheHintToParent(const CCellRange& range) const
{
    if (!IsWindow())
        return 0;

    GV_CACHEHINT CacheHint;

    // Fix up the message headers
    CacheHint.hdr.hwndFrom = m_hWnd;
    CacheHint.hdr.idFrom   = GetDlgCtrlID();
    CacheHint.hdr.code     = GVN_ODCACHEHINT;

    CacheHint.range = range;

    // Send the message
    CWindow Owner = GetOwner(m_hWnd);
    if (Owner.IsWindow())
        return Owner.SendMessage(WM_NOTIFY, CacheHint.hdr.idFrom, (LPARAM)&CacheHint);
    else
        return 0;
}

#define LAYER_SIGNATURE (0x5FD4E64)
int CGridCtrl::GetLayer(int** pLayer) // used to save and restore order of columns
{ //  gives back the size of the area (do not forget to delete pLayer)
	int Length = 2+GetColumnCount()*2;
	int *Layer = new int[Length];	// the caller is supposed to delete it
	Layer[0]= LAYER_SIGNATURE;
	Layer[1]= GetColumnCount();
	memcpy(&Layer[2], &m_arColOrder[0], GetColumnCount()*sizeof(int));
	memcpy(&Layer[2+GetColumnCount()], &m_arColWidths[0], GetColumnCount()*sizeof(int));
	*pLayer = Layer;
	return Length;
}
void CGridCtrl::SetLayer(int* pLayer)
{ // coming from a previous GetLayer (ignored if not same number of column, or the same revision number)
	if(pLayer[0] != LAYER_SIGNATURE) return;
	if(pLayer[1] != GetColumnCount()) return;
/*	TRACE("  %d == %d \n",m_arColOrder[0],pLayer[2]);
	TRACE("  %d == %d \n",m_arColOrder[1],pLayer[3]);
	TRACE("  %d == %d \n",m_arColOrder[2],pLayer[4]);
	TRACE("  %d == %d \n",m_arColWidths[0],pLayer[2+3]);
	TRACE("  %d == %d \n",m_arColWidths[1],pLayer[3+3]);
	TRACE("  %d == %d \n",m_arColWidths[2],pLayer[4+3]);
	TRACE("  %d == %d \n",GetColumnCount(),3);

	
	ASSERT(m_arColOrder[0]==pLayer[2]);
	ASSERT(m_arColOrder[1]==pLayer[3]);
	ASSERT(m_arColOrder[2]==pLayer[4]);
	ASSERT(m_arColWidths[0]==pLayer[2+3]);
	ASSERT(m_arColWidths[1]==pLayer[3+3]);
	ASSERT(m_arColWidths[2]==pLayer[4+3]);
	ASSERT(GetColumnCount()==3); */
	memcpy(&m_arColOrder[0], &pLayer[2], GetColumnCount()*sizeof(int));
	memcpy(&m_arColWidths[0], &pLayer[2+GetColumnCount()], GetColumnCount()*sizeof(int));
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl message handlers

void CGridCtrl::OnPaint(CDCHandle /*dc*/)
{
	static BOOL m_bInitialized = FALSE;
	if (!m_bInitialized)
	{
		m_bInitialized = TRUE;
		Initialise();
	}
	CPaintDC dc(m_hWnd);
    if (m_bDoubleBuffer)    // Use a memory DC to remove flicker
    {
		CRect rect;
		dc.GetClipBox(&rect);
        CMemoryDC MemDC(dc, rect);
        OnDraw(MemDC.m_hDC);
    }
    else                    // Draw raw - this helps in debugging vis problems.
        OnDraw(dc.m_hDC);
}

BOOL CGridCtrl::OnEraseBkgnd(CDCHandle /*dc*/)
{
    return TRUE;    // Don't erase the background.
}

// Custom background erasure. This gets called from within the OnDraw function,
// since we will (most likely) be using a memory DC to stop flicker. If we just
// erase the background normally through OnEraseBkgnd, and didn't fill the memDC's
// selected bitmap with colour, then all sorts of vis problems would occur
void CGridCtrl::EraseBkgnd(CDCHandle DC)
{
    CRect  VisRect, ClipRect, rect;
    CBrush FixedRowColBack,
           FixedRowBack,
           FixedColBack,
           TextBack;
	FixedRowColBack.CreateSolidBrush(GetDefaultCell(TRUE, TRUE)->GetBackClr());
	FixedRowBack.CreateSolidBrush(GetDefaultCell(TRUE, FALSE)->GetBackClr());
	FixedColBack.CreateSolidBrush(GetDefaultCell(FALSE, TRUE)->GetBackClr());
	TextBack.CreateSolidBrush(GetDefaultCell(FALSE, FALSE)->GetBackClr());

    CBrush Back;
	Back.CreateSolidBrush(GetGridBkColor()); 
    //CBrush Back(GetTextBkColor());

    if (DC.GetClipBox(ClipRect) == ERROR)
        return;
    GetVisibleNonFixedCellRange(VisRect);

    int nFixedColumnWidth = GetFixedColumnWidth();
    int nFixedRowHeight = GetFixedRowHeight();

    // Draw Fixed row/column background
    if (ClipRect.left < nFixedColumnWidth && ClipRect.top < nFixedRowHeight)
        DC.FillRect(CRect(ClipRect.left, ClipRect.top, 
                      nFixedColumnWidth, nFixedRowHeight),
                      FixedRowColBack);

    // Draw Fixed columns background
    if (ClipRect.left < nFixedColumnWidth && ClipRect.top < VisRect.bottom)
        DC.FillRect(CRect(ClipRect.left, ClipRect.top, 
                      nFixedColumnWidth, VisRect.bottom),
                      FixedColBack);
        
    // Draw Fixed rows background
    if (ClipRect.top < nFixedRowHeight && 
        ClipRect.right > nFixedColumnWidth && ClipRect.left < VisRect.right)
        DC.FillRect(CRect(nFixedColumnWidth-1, ClipRect.top,
                      VisRect.right, nFixedRowHeight),
                      FixedRowBack);

    // Draw non-fixed cell background
    if (rect.IntersectRect(VisRect, ClipRect)) 
    {
        CRect CellRect(__max((LONG)nFixedColumnWidth, rect.left),
			__max((LONG)nFixedRowHeight, rect.top),
                       rect.right, rect.bottom);
        DC.FillRect(CellRect, TextBack);
    }

    // Draw right hand side of window outside grid
    if (VisRect.right < ClipRect.right) 
        DC.FillRect(CRect(VisRect.right, ClipRect.top, 
                      ClipRect.right, ClipRect.bottom),
                      Back);

    // Draw bottom of window below grid
    if (VisRect.bottom < ClipRect.bottom && ClipRect.left < VisRect.right) 
        DC.FillRect(CRect(ClipRect.left, VisRect.bottom,
                      VisRect.right, ClipRect.bottom),
                      Back);
}

void CGridCtrl::OnSize(UINT /*nType*/, CSize /*size*/)
{  
    static BOOL bAlreadyInsideThisProcedure = FALSE;
    if (bAlreadyInsideThisProcedure)
        return;

    if (!IsWindow())
        return;

    // Start re-entry blocking
    bAlreadyInsideThisProcedure = TRUE;

    EndEditing();        // destroy any InPlaceEdit's
    DefWindowProc();
    ResetScrollBars();

    // End re-entry blocking
    bAlreadyInsideThisProcedure = FALSE;
}

UINT CGridCtrl::OnGetDlgCode(LPMSG /*lpMsg*/)
{
    UINT nCode = DLGC_WANTARROWS | DLGC_WANTCHARS; // DLGC_WANTALLKEYS; //

    if (m_bHandleTabKey && !IsCTRLpressed())
        nCode |= DLGC_WANTTAB;

    return nCode;
}

#ifndef _WIN32_WCE
// If system colours change, then redo colours
void CGridCtrl::OnSysColorChange()
{
    // forward this message to all other child windows
	if (!(GetStyle() & WS_CHILD))
		SendMessageToDescendants(WM_SYSCOLORCHANGE, 0, 0L, TRUE);
	DefWindowProc();

    if (GetDefaultCell(FALSE, FALSE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(FALSE, FALSE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(FALSE, FALSE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(FALSE, FALSE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(TRUE, FALSE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(TRUE, FALSE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(TRUE, FALSE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(TRUE, FALSE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(FALSE, TRUE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(FALSE, TRUE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(FALSE, TRUE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(FALSE, TRUE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(TRUE, TRUE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(TRUE, TRUE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(TRUE, TRUE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(TRUE, TRUE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetGridBkColor() == m_crShadow)
        SetGridBkColor(::GetSysColor(COLOR_3DSHADOW));

    m_crWindowText   = ::GetSysColor(COLOR_WINDOWTEXT);
    m_crWindowColour = ::GetSysColor(COLOR_WINDOW);
    m_cr3DFace       = ::GetSysColor(COLOR_3DFACE);
    m_crShadow       = ::GetSysColor(COLOR_3DSHADOW);
}
#endif

#ifndef _WIN32_WCE_NO_CURSOR
// If we are drag-selecting cells, or drag and dropping, stop now
void CGridCtrl::OnCaptureChanged(CWindow wnd)
{
    if (wnd.m_hWnd == m_hWnd)
        return;

    // kill timer if active
    if (m_nTimerID != 0)
    {
        KillTimer(m_nTimerID);
        m_nTimerID = 0;
    }

#ifndef GRIDCONTROL_NO_DRAGDROP
    // Kill drag and drop if active
    if (m_MouseMode == MOUSE_DRAGGING)
        m_MouseMode = MOUSE_NOTHING;
#endif
}
#endif

// If system settings change, then redo colours
void CGridCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    DefWindowProc();

    if (GetDefaultCell(FALSE, FALSE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(FALSE, FALSE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(FALSE, FALSE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(FALSE, FALSE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(TRUE, FALSE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(TRUE, FALSE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(TRUE, FALSE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(TRUE, FALSE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(FALSE, TRUE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(FALSE, TRUE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(FALSE, TRUE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(FALSE, TRUE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetDefaultCell(TRUE, TRUE)->GetTextClr() == m_crWindowText)                   // Still using system colours
        GetDefaultCell(TRUE, TRUE)->SetTextClr(::GetSysColor(COLOR_WINDOWTEXT));      // set to new system colour
    if (GetDefaultCell(TRUE, TRUE)->GetBackClr() == m_crWindowColour)
        GetDefaultCell(TRUE, TRUE)->SetBackClr(::GetSysColor(COLOR_WINDOW));

    if (GetGridBkColor() == m_crShadow)
        SetGridBkColor(::GetSysColor(COLOR_3DSHADOW));

    m_crWindowText   = ::GetSysColor(COLOR_WINDOWTEXT);
    m_crWindowColour = ::GetSysColor(COLOR_WINDOW);
    m_cr3DFace       = ::GetSysColor(COLOR_3DFACE);
    m_crShadow       = ::GetSysColor(COLOR_3DSHADOW);

    m_nRowsPerWheelNotch = GetMouseScrollLines(); // Get the number of lines
}

// For drag-selection. Scrolls hidden cells into view
// TODO: decrease timer interval over time to speed up selection over time
void CGridCtrl::OnTimer(UINT_PTR nIDEvent)
{
    ATLASSERT(nIDEvent == WM_LBUTTONDOWN);
    if (nIDEvent != WM_LBUTTONDOWN)
        return;

    CPoint pt, origPt;

#ifdef _WIN32_WCE
    if (m_MouseMode == MOUSE_NOTHING)
        return;
    origPt = GetMessagePos();
#else
    if (!GetCursorPos(&origPt))
        return;
#endif

    ScreenToClient(&origPt);

    CRect rect;
    GetClientRect(rect);

    int nFixedRowHeight = GetFixedRowHeight();
    int nFixedColWidth = GetFixedColumnWidth();

    pt = origPt;
    if (pt.y > rect.bottom)
    {
        //SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
        SendMessage(WM_KEYDOWN, VK_DOWN, 0);

        if (pt.x < rect.left)
            pt.x = rect.left;
        if (pt.x > rect.right)
            pt.x = rect.right;
        pt.y = rect.bottom;
        OnSelecting(GetCellFromPt(pt));
    }
    else if (pt.y < nFixedRowHeight)
    {
        //SendMessage(WM_VSCROLL, SB_LINEUP, 0);
        SendMessage(WM_KEYDOWN, VK_UP, 0);

        if (pt.x < rect.left)
            pt.x = rect.left;
        if (pt.x > rect.right)
            pt.x = rect.right;
        pt.y = nFixedRowHeight + 1;
        OnSelecting(GetCellFromPt(pt));
    }

    pt = origPt;
    if (pt.x > rect.right)
    {
        // SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
        SendMessage(WM_KEYDOWN, VK_RIGHT, 0);

        if (pt.y < rect.top)
            pt.y = rect.top;
        if (pt.y > rect.bottom)
            pt.y = rect.bottom;
        pt.x = rect.right;
        OnSelecting(GetCellFromPt(pt));
    }
    else if (pt.x < nFixedColWidth)
    {
        //SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
        SendMessage(WM_KEYDOWN, VK_LEFT, 0);

        if (pt.y < rect.top)
            pt.y = rect.top;
        if (pt.y > rect.bottom)
            pt.y = rect.bottom;
        pt.x = nFixedColWidth + 1;
        OnSelecting(GetCellFromPt(pt));
    }
}

// move about with keyboard
void CGridCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (!IsValid(m_idCurrentCell))
    {
		DefWindowProc();
        return;
    }
    CCellID next = m_idCurrentCell;
    BOOL bChangeLine = FALSE;
	BOOL bHorzScrollAction = FALSE;
	BOOL bVertScrollAction = FALSE;

    if (IsCTRLpressed())
    {
        switch (nChar)
        {
        case 'A':
            EditSelectAll();
            break;
        case 'k':		// This is ctrl+ on french keyboard, may need to be better processed for other locales
            AutoSizeColumns();
			Invalidate();
            break;
#ifndef GRIDCONTROL_NO_CLIPBOARD
        case 'X':
            EditCut();
            break;
        case VK_INSERT:
        case 'C':
            EditCopy();
            break;
        case 'V':
            EditPaste();
            break;
#endif
        }
    }

	BOOL bFoundVisible;
	int iOrig;

#ifndef GRIDCONTROL_NO_CLIPBOARD
    if (IsSHIFTpressed() &&(nChar == VK_INSERT))
        EditPaste();
    if (nChar == VK_DELETE)
    {
		CutSelectedText();
    }
    else
#endif
	if (nChar == VK_DOWN)
    {
        // don't let user go to a hidden row
        bFoundVisible = FALSE;
        iOrig = next.row;
        next.row++;
        while(next.row < GetRowCount())
        {
            if(GetRowHeight(next.row) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            next.row++;
        }
        if(!bFoundVisible)
            next.row = iOrig;
    }
    else if (nChar == VK_UP)
    {
        // don't let user go to a hidden row
        bFoundVisible = FALSE;
        iOrig = next.row;
        next.row--;
        while(next.row >= m_nFixedRows)
        {
            if(GetRowHeight(next.row) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            next.row--;
        }
        if(!bFoundVisible)
            next.row = iOrig;
    }
    else if (nChar == VK_RIGHT || (nChar == VK_TAB && !IsSHIFTpressed()))
    {
		if((nChar == VK_TAB) &&  m_QuitFocusOnTab)
		{
			CWindow w = GetParent();
			if (w.IsWindow()) w.NextDlgCtrl();
			return;

		}
        // don't let user go to a hidden column
        bFoundVisible = FALSE;
        iOrig = next.col;
        next.col++;

        if (nChar == VK_TAB)
        {
			// If we're at the end of a row, go down a row till we find a non-hidden row
            if (next.col == (GetColumnCount()) && next.row < (GetRowCount() - 1))
            {
				next.row++;
				while(next.row < GetRowCount())
				{
					if(GetRowHeight(next.row) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					next.row++; 
				}

                next.col = m_nFixedCols;	// Place focus on first non-fixed column
                bChangeLine = TRUE;
            }
        }

		// We're on a non-hidden row, so look across for the next non-hidden column
        while(next.col < GetColumnCount())
        {
            if(GetColumnWidth(next.col) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            next.col++;
        }

		// If nothing worked then don't bother
        if(!bFoundVisible)
            next.col = iOrig;
    }
    else if (nChar == VK_LEFT || (nChar == VK_TAB && IsSHIFTpressed()))
    {
        // don't let user go to a hidden column
        bFoundVisible = FALSE;
        iOrig = next.col;
        next.col--;

        if (nChar == VK_TAB)
        {
            if (next.col == (GetFixedColumnCount()-1) && next.row > GetFixedRowCount())
            {
                next.row--;
				while(next.row > GetFixedRowCount())
				{
					if(GetRowHeight(next.row) > 0)
					{
						bFoundVisible = TRUE;
						break;
					}
					next.row--; 
				}

                next.col = GetColumnCount() - 1; 
                bChangeLine = TRUE;
            }
        }

        while(next.col >= m_nFixedCols)
        {
            if(GetColumnWidth(next.col) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            next.col--;
        }
        if(!bFoundVisible)
            next.col = iOrig;
    }
    else if (nChar == VK_NEXT)
    {
        CCellID idOldTopLeft = GetTopleftNonFixedCell();
        SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
		bVertScrollAction = TRUE;
        CCellID idNewTopLeft = GetTopleftNonFixedCell();

        int increment = idNewTopLeft.row - idOldTopLeft.row;
        if (increment)
        {
            next.row += increment;
            if (next.row >(GetRowCount() - 1))
                next.row = GetRowCount() - 1;
        }
        else
            next.row = GetRowCount() - 1;
    }
    else if (nChar == VK_PRIOR)
    {
        CCellID idOldTopLeft = GetTopleftNonFixedCell();
        SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
		bVertScrollAction = TRUE;
        CCellID idNewTopLeft = GetTopleftNonFixedCell();
            
        int increment = idNewTopLeft.row - idOldTopLeft.row;
        if (increment) 
        {
            next.row += increment;
            if (next.row < m_nFixedRows) 
                next.row = m_nFixedRows;
        }
        else
            next.row = m_nFixedRows;
    }
    else if (nChar == VK_HOME)
    {
        // Home and Ctrl-Home work more like Excel
        //  and don't let user go to a hidden cell
        if (IsCTRLpressed())
        {
            SendMessage(WM_VSCROLL, SB_TOP, 0);
            SendMessage(WM_HSCROLL, SB_LEFT, 0);
			bVertScrollAction = TRUE;
			bHorzScrollAction = TRUE;
            next.row = m_nFixedRows;
            next.col = m_nFixedCols;
        }
        else
        {
            SendMessage(WM_HSCROLL, SB_LEFT, 0);
 			bHorzScrollAction = TRUE;
            next.col = m_nFixedCols;
        }
        // adjust column to avoid hidden columns and rows
        while(next.col < GetColumnCount() - 1)
        {
            if(GetColumnWidth(next.col) > 0)
                break;
            next.col++;
        }
        while(next.row < GetRowCount() - 1)
        {
            if(GetRowHeight(next.row) > 0)
                break;
            next.row++;
        }
    }
    else if (nChar == VK_END)
    {
        // End and Ctrl-End work more like Excel
        //  and don't let user go to a hidden cell
        if (IsCTRLpressed())
        {
            SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
            SendMessage(WM_HSCROLL, SB_RIGHT, 0);
			bHorzScrollAction = TRUE;
			bVertScrollAction = TRUE;
            next.row = GetRowCount() - 1;
            next.col = GetColumnCount() - 1;
        }
        else
        {
            SendMessage(WM_HSCROLL, SB_RIGHT, 0);
			bHorzScrollAction = TRUE;
            next.col = GetColumnCount() - 1;
        }
        // adjust column to avoid hidden columns and rows
        while(next.col > m_nFixedCols + 1)
        {
            if(GetColumnWidth(next.col) > 0)
                break;
            next.col--;
        }
        while(next.row > m_nFixedRows + 1)
        {
            if(GetRowHeight(next.row) > 0)
                break;
            next.row--;
        }
    }
    else if (nChar == VK_F2)
    {
        OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), VK_LBUTTON);
    }
    else
	{
		DefWindowProc();
		return;
	}

    if (next != m_idCurrentCell)
    {
        // While moving with the Cursorkeys the current ROW/CELL will get selected
        // OR Selection will get expanded when SHIFT is pressed
        // Cut n paste from OnLButtonDown - Franco Bez
        // Added check for NULL mouse mode - Chris Maunder.
        if (m_MouseMode == MOUSE_NOTHING)
        {
            m_PrevSelectedCellMap.RemoveAll();
            m_MouseMode = m_bListMode? MOUSE_SELECT_ROW : MOUSE_SELECT_CELLS;
            if (!IsSHIFTpressed() || nChar == VK_TAB)
                m_SelectionStartCell = next;

			// Notify parent that selection is changing - Arthur Westerman/Scot Brennecke 
            SendMessageToParent(next.row, next.col, GVN_SELCHANGING);
			OnSelecting(next);
			SendMessageToParent(next.row, next.col, GVN_SELCHANGED);

            m_MouseMode = MOUSE_NOTHING;
        }

        SetFocusCell(next);

        if (!IsCellVisible(next))
        {

            switch (nChar)
            {
            case VK_RIGHT:  
                SendMessage(WM_HSCROLL, SB_LINERIGHT, 0); 
				bHorzScrollAction = TRUE;
                break;
                
            case VK_LEFT:   
                SendMessage(WM_HSCROLL, SB_LINELEFT, 0);  
				bHorzScrollAction = TRUE;
                break;
                
            case VK_DOWN:   
                SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);  
				bVertScrollAction = TRUE;
                break;
                
            case VK_UP:     
                SendMessage(WM_VSCROLL, SB_LINEUP, 0);    
				bVertScrollAction = TRUE;
                break;                
                
            case VK_TAB:    
                if (IsSHIFTpressed())
                {
                    if (bChangeLine) 
                    {
                        SendMessage(WM_VSCROLL, SB_LINEUP, 0);
						bVertScrollAction = TRUE;
                        SetScrollPos32(SB_HORZ, m_nHScrollMax);
                        break;
                    }
                    else 
					{
                        SendMessage(WM_HSCROLL, SB_LINELEFT, 0);
						bHorzScrollAction = TRUE;
					}
                }
                else
                {
                    if (bChangeLine) 
                    {
                        SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
						bVertScrollAction = TRUE;
                        SetScrollPos32(SB_HORZ, 0);
                        break;
                    }
                    else 
					{
						SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);
						bHorzScrollAction = TRUE;
					}
                }
                break;
            }
            EnsureVisible(next); // Make sure cell is visible
            Invalidate();
        }
        EnsureVisible(next); // Make sure cell is visible

		if (bHorzScrollAction)
			SendMessage(WM_HSCROLL, SB_ENDSCROLL, 0);
		if (bVertScrollAction)
			SendMessage(WM_VSCROLL, SB_ENDSCROLL, 0);
    }
}

void CGridCtrl::OnKeyUp(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
    DefWindowProc();
}

void CGridCtrl::OnSysKeyDown(UINT /*nChar*/, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.Hide();  // hide any titletips
#endif
    DefWindowProc();
}

// Instant editing of cells when keys are pressed
void CGridCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CGridCell *cell = static_cast<CGridCell *>(GetCell(m_idCurrentCell.row, m_idCurrentCell.col));
	if (!cell || !_IsCType(nChar, cell->GetCType(), GridLocale))
		return;

	if (!IsCTRLpressed() && m_MouseMode == MOUSE_NOTHING && nChar != VK_ESCAPE && (!m_bHandleTabKey || (m_bHandleTabKey && nChar != VK_TAB)))
		OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), nChar);

	DefWindowProc();
}

// Added by KiteFly
void CGridCtrl::OnImeChar(WPARAM wCharCode, LPARAM /*nCode*/)
{
	CGridCell *cell = static_cast<CGridCell *>(GetCell(m_idCurrentCell.row, m_idCurrentCell.col));
	if (!cell || !_IsCType((wint_t)wCharCode, cell->GetCType(), GridLocale))
		return;
	
	if (!IsCTRLpressed() && m_MouseMode == MOUSE_NOTHING && wCharCode != VK_ESCAPE)
        OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), (UINT)wCharCode);
}

// Callback from any CInPlaceEdits that ended. This just calls OnEndEditCell,
// refreshes the edited cell and moves onto next cell if the return character
// from the edit says we should.
LRESULT CGridCtrl::OnEndInPlaceEdit(LPNMHDR pnmh)
{
    GV_DISPINFO *pgvDispInfo = (GV_DISPINFO *)pnmh;
    GV_ITEM     *pgvItem = &pgvDispInfo->item;

    // In case OnEndInPlaceEdit called as window is being destroyed
    if (!IsWindow())
        return 0;

    OnEndEditCell(pgvItem->row, pgvItem->col, pgvItem->strText);
    //InvalidateCellRect(CCellID(pgvItem->row, pgvItem->col));

    switch (pgvItem->lParam)
    {
    case VK_TAB:
    case VK_DOWN:
    case VK_UP:
    case VK_RIGHT:
    case VK_LEFT:
    case VK_NEXT:
    case VK_PRIOR:
    case VK_HOME:
    case VK_END:
        OnKeyDown((UINT)pgvItem->lParam, 0, 0);
    //    OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, CPoint(-1, -1), (UINT)pgvItem->lParam);	// EP: More like MSExcel behavior
    }

    return 0;
}

// Handle horz scrollbar notifications
void CGridCtrl::OnHScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar /*pScrollBar*/)
{
    EndEditing();

#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.Hide();  // hide any titletips
#endif

    int scrollPos = GetScrollPos32(SB_HORZ);

    CCellID idTopLeft = GetTopleftNonFixedCell();

    CRect rect;
    GetClientRect(rect);

    switch (nSBCode)
    {
    case SB_LINERIGHT:
        if (scrollPos < m_nHScrollMax)
        {
            // may have contiguous hidden columns.  Blow by them
            while (idTopLeft.col < (GetColumnCount()-1)
                    && GetColumnWidth(idTopLeft.col) < 1)
            {
                idTopLeft.col++;
            }
            int xScroll = GetColumnWidth(idTopLeft.col);
            SetScrollPos32(SB_HORZ, scrollPos + xScroll);
            if (GetScrollPos32(SB_HORZ) == scrollPos)
                break;          // didn't work

            rect.left = GetFixedColumnWidth();
            //rect.left = GetFixedColumnWidth() + xScroll;
            //ScrollWindow(-xScroll, 0, rect);
            //rect.left = rect.right - xScroll;
            InvalidateRect(rect);
        }
        break;

    case SB_LINELEFT:
        if (scrollPos > 0 && idTopLeft.col > GetFixedColumnCount())
        {
            int iColToUse = idTopLeft.col-1;
            // may have contiguous hidden columns.  Blow by them
            while(iColToUse > GetFixedColumnCount()
                    && GetColumnWidth(iColToUse) < 1)
            {
                iColToUse--;
            }

            int xScroll = GetColumnWidth(iColToUse);
            SetScrollPos32(SB_HORZ, __max(0, scrollPos - xScroll));
            rect.left = GetFixedColumnWidth();
            //ScrollWindow(xScroll, 0, rect);
            //rect.right = rect.left + xScroll;
            InvalidateRect(rect);
        }
        break;

    case SB_PAGERIGHT:
        if (scrollPos < m_nHScrollMax)
        {
            rect.left = GetFixedColumnWidth();
            int offset = rect.Width();
            int pos = __min(m_nHScrollMax, scrollPos + offset);
            SetScrollPos32(SB_HORZ, pos);
            rect.left = GetFixedColumnWidth();
            InvalidateRect(rect);
        }
        break;
        
    case SB_PAGELEFT:
        if (scrollPos > 0)
        {
            rect.left = GetFixedColumnWidth();
            int offset = -rect.Width();
            int pos = __max(0, scrollPos + offset);
            SetScrollPos32(SB_HORZ, pos);
            rect.left = GetFixedColumnWidth();
            InvalidateRect(rect);
        }
        break;
        
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        {
            SetScrollPos32(SB_HORZ, GetScrollPos32(SB_HORZ, TRUE));
            m_idTopLeftCell.row = -1;
            CCellID idNewTopLeft = GetTopleftNonFixedCell();
            if (idNewTopLeft != idTopLeft)
            {
                rect.left = GetFixedColumnWidth();
                InvalidateRect(rect);
            }
        }
        break;
        
    case SB_LEFT:
        if (scrollPos > 0)
        {
            SetScrollPos32(SB_HORZ, 0);
            Invalidate();
        }
        break;
        
    case SB_RIGHT:
        if (scrollPos < m_nHScrollMax)
        {
            SetScrollPos32(SB_HORZ, m_nHScrollMax);
            Invalidate();
        }
        break;
        
        
    default: 
        break;
    }
}

// Handle vert scrollbar notifications
void CGridCtrl::OnVScroll(UINT nSBCode, UINT /*nPos*/, CScrollBar /*pScrollBar*/)
{
	EndEditing();

#ifndef GRIDCONTROL_NO_TITLETIPS
	m_TitleTip.Hide();  // hide any titletips
#endif

	// Get the scroll position ourselves to ensure we get a 32 bit value
	int scrollPos = GetScrollPos32(SB_VERT);

	CCellID idTopLeft = GetTopleftNonFixedCell();

	CRect rect;
	GetClientRect(rect);

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (scrollPos < m_nVScrollMax)
		{
			// may have contiguous hidden rows.  Blow by them
			while(idTopLeft.row < (GetRowCount()-1) && GetRowHeight(idTopLeft.row) < 1)
				idTopLeft.row++;

			SetScrollPos32(SB_VERT, scrollPos + GetRowHeight(idTopLeft.row));
			if (GetScrollPos32(SB_VERT) == scrollPos)
				break;          // didn't work

			rect.top = GetFixedRowHeight();
			InvalidateRect(rect);
		}
		break;

	case SB_LINEUP:
		if (scrollPos > 0 && idTopLeft.row > GetFixedRowCount())
		{
			int iRowToUse = idTopLeft.row-1;
			// may have contiguous hidden rows.  Blow by them
			while(iRowToUse > GetFixedRowCount() && GetRowHeight(iRowToUse) < 1)
				iRowToUse--;

			SetScrollPos32(SB_VERT, __max(0, scrollPos - GetRowHeight(iRowToUse)));
			rect.top = GetFixedRowHeight();
			InvalidateRect(rect);
		}
		break;

	case SB_PAGEDOWN:
		if (scrollPos < m_nVScrollMax)
		{
			rect.top = GetFixedRowHeight();
			SetScrollPos32(SB_VERT, __min(m_nVScrollMax, scrollPos + rect.Height()));
			InvalidateRect(rect);
		}
		break;

	case SB_PAGEUP:
		if (scrollPos > 0)
		{
			rect.top = GetFixedRowHeight();
			SetScrollPos32(SB_VERT, __max(0, scrollPos - rect.Height()));
			InvalidateRect(rect);
		}
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			SetScrollPos32(SB_VERT, GetScrollPos32(SB_VERT, TRUE));
			CCellID idNewTopLeft = GetTopleftNonFixedCell();
			if (idNewTopLeft != idTopLeft)
			{
				rect.top = GetFixedRowHeight();
				InvalidateRect(rect);
			}
		}
		break;

	case SB_TOP:
		if (scrollPos > 0)
		{
			SetScrollPos32(SB_VERT, 0);
			Invalidate();
		}
		break;

	case SB_BOTTOM:
		if (scrollPos < m_nVScrollMax)
		{
			SetScrollPos32(SB_VERT, m_nVScrollMax);
			Invalidate();
		}

	default: 
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl implementation functions

void CGridCtrl::OnDraw(CDCHandle DC)
{
    if (!m_bAllowDraw)
        return;

    CRect clipRect;
    if (DC.GetClipBox(&clipRect) == ERROR)
        return;

    EraseBkgnd(DC);            // OnEraseBkgnd does nothing, so erase bkgnd here.
    // This necessary since we may be using a Memory DC.

#ifdef _DEBUG
	LARGE_INTEGER iStartCount;
	QueryPerformanceCounter(&iStartCount);
#endif

    CRect rect;
    int row, col;
    CGridCellBase* pCell;

    int nFixedRowHeight = GetFixedRowHeight();
    int nFixedColWidth  = GetFixedColumnWidth();

    CCellID idTopLeft = GetTopleftNonFixedCell();
    int minVisibleRow = idTopLeft.row,
        minVisibleCol = idTopLeft.col;

    CRect VisRect;
    CCellRange VisCellRange = GetVisibleNonFixedCellRange(VisRect);
    int maxVisibleRow = VisCellRange.GetMaxRow(),
        maxVisibleCol = VisCellRange.GetMaxCol();

    if (GetVirtualMode())
        SendCacheHintToParent(VisCellRange);

    // draw top-left cells 0..m_nFixedRows-1, 0..m_nFixedCols-1
    rect.bottom = -1;
    for (row = 0; row < m_nFixedRows; row++)
    {
        if (GetRowHeight(row) <= 0) continue;

        rect.top = rect.bottom+1;
        rect.bottom = rect.top + GetRowHeight(row)-1;
        rect.right = -1;

        for (col = 0; col < m_nFixedCols; col++)
        {
            if (GetColumnWidth(col) <= 0) continue;

            rect.left = rect.right+1;
            rect.right = rect.left + GetColumnWidth(col)-1;

            pCell = GetCell(row, col);
            if (pCell)
			{
				pCell->SetCoords(row,col);
                pCell->Draw(DC, row, col, rect, FALSE);
			}
        }
    }

    // draw fixed column cells:  m_nFixedRows..n, 0..m_nFixedCols-1
    rect.bottom = nFixedRowHeight-1;
    for (row = minVisibleRow; row <= maxVisibleRow; row++)
    {
        if (GetRowHeight(row) <= 0) continue;

        rect.top = rect.bottom+1;
        rect.bottom = rect.top + GetRowHeight(row)-1;

        // rect.bottom = bottom pixel of previous row
        if (rect.top > clipRect.bottom)
            break;                // Gone past cliprect
        if (rect.bottom < clipRect.top)
            continue;             // Reached cliprect yet?

        rect.right = -1;
        for (col = 0; col < m_nFixedCols; col++)
        {
            if (GetColumnWidth(col) <= 0) continue;

            rect.left = rect.right+1;
            rect.right = rect.left + GetColumnWidth(col)-1;

            if (rect.left > clipRect.right)
                break;            // gone past cliprect
            if (rect.right < clipRect.left)
                continue;         // Reached cliprect yet?

            pCell = GetCell(row, col);
            if (pCell)
			{
				pCell->SetCoords(row,col);
                pCell->Draw(DC, row, col, rect, FALSE);
			}
        }
    }

    // draw fixed row cells  0..m_nFixedRows, m_nFixedCols..n
    rect.bottom = -1;
    for (row = 0; row < m_nFixedRows; row++)
    {
        if (GetRowHeight(row) <= 0) continue;

        rect.top = rect.bottom+1;
        rect.bottom = rect.top + GetRowHeight(row)-1;

        // rect.bottom = bottom pixel of previous row
        if (rect.top > clipRect.bottom)
            break;                // Gone past cliprect
        if (rect.bottom < clipRect.top)
            continue;             // Reached cliprect yet?

        rect.right = nFixedColWidth-1;
        for (col = minVisibleCol; col <= maxVisibleCol; col++)
        {
            if (GetColumnWidth(col) <= 0) continue;

            rect.left = rect.right+1;
            rect.right = rect.left + GetColumnWidth(col)-1;

            if (rect.left > clipRect.right)
                break;        // gone past cliprect
            if (rect.right < clipRect.left)
                continue;     // Reached cliprect yet?

            pCell = GetCell(row, col);
            if (pCell)
			{
				pCell->SetCoords(row,col);
                pCell->Draw(DC, row, col, rect, FALSE);
			}
        }
    }

    // draw rest of non-fixed cells
    rect.bottom = nFixedRowHeight-1;
    for (row = minVisibleRow; row <= maxVisibleRow; row++)
    {
        if (GetRowHeight(row) <= 0) continue;

        rect.top = rect.bottom+1;
        rect.bottom = rect.top + GetRowHeight(row)-1;

        // rect.bottom = bottom pixel of previous row
        if (rect.top > clipRect.bottom)
            break;                // Gone past cliprect
        if (rect.bottom < clipRect.top)
            continue;             // Reached cliprect yet?

        rect.right = nFixedColWidth-1;
        for (col = minVisibleCol; col <= maxVisibleCol; col++)
        {
            if (GetColumnWidth(col) <= 0) continue;

            rect.left = rect.right+1;
            rect.right = rect.left + GetColumnWidth(col)-1;

            if (rect.left > clipRect.right)
                break;        // gone past cliprect
            if (rect.right < clipRect.left)
                continue;     // Reached cliprect yet?

            pCell = GetCell(row, col);
            // TRACE(_T("Cell %d,%d type: %s\n"), row, col, pCell->GetRuntimeClass()->m_lpszClassName);
            if (pCell)
			{
				pCell->SetCoords(row,col);
                pCell->Draw(DC, row, col, rect, FALSE);
			}
        }
    }


    CPen pen;
    pen.CreatePen(PS_SOLID, 0, m_crGridLineColour);
    DC.SelectPen(pen);

    // draw vertical lines (drawn at ends of cells)
    if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
    {
        int x = nFixedColWidth;
        for (col = minVisibleCol; col <= maxVisibleCol; col++)
        {
            if (GetColumnWidth(col) <= 0) continue;

            x += GetColumnWidth(col);
            DC.MoveTo(x-1, nFixedRowHeight);
            DC.LineTo(x-1, VisRect.bottom);
        }
    }

    // draw horizontal lines (drawn at bottom of each cell)
    if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
    {
        int y = nFixedRowHeight;
        for (row = minVisibleRow; row <= maxVisibleRow; row++)
        {
            if (GetRowHeight(row) <= 0) continue;

            y += GetRowHeight(row);
            DC.MoveTo(nFixedColWidth, y-1);
            DC.LineTo(VisRect.right,  y-1);
        }
    }

    DC.SelectStockPen(NULL_PEN);

    // Let parent know it can discard it's data if it needs to.
    if (GetVirtualMode())
       SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

#ifdef _DEBUG
	LARGE_INTEGER iEndCount;
	QueryPerformanceCounter(&iEndCount);
	ATLTRACE("Draw counter ticks: %d\n", iEndCount.LowPart-iStartCount.LowPart);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
// CGridCtrl Cell selection stuff

BOOL CGridCtrl::IsValid(int nRow, int nCol) const
{
	return (nRow >= 0 && nRow < m_nRows && nCol >= 0 && nCol < m_nCols);
}
BOOL CGridCtrl::IsValid(const CCellID& cell) const
{
	return IsValid(cell.row, cell.col);
}
BOOL CGridCtrl::IsValid(const CCellRange& range) const
{
	return (range.GetMinRow() >= 0 && range.GetMinCol() >= 0 &&
		range.GetMaxRow() >= 0 && range.GetMaxCol() >= 0 &&
		range.GetMaxRow() < m_nRows && range.GetMaxCol() < m_nCols &&
		range.GetMinRow() <= range.GetMaxRow() && range.GetMinCol() <= range.GetMaxCol());
}

// Enables/Disables redraw for certain operations like columns auto-sizing etc,
// but not for user caused things such as selection changes.
void CGridCtrl::SetRedraw(BOOL bAllowDraw, BOOL bResetScrollBars /* = FALSE */)
{
//    TRACE(_T("%s: Setting redraw to %s\n"),
//             GetRuntimeClass()->m_lpszClassName, bAllowDraw? _T("TRUE") : _T("FALSE"));

    if (bAllowDraw && !m_bAllowDraw)
    {
        m_bAllowDraw = TRUE;
        Refresh();
    }

    m_bAllowDraw = bAllowDraw;
    if (bResetScrollBars)
        ResetScrollBars();
}

// Forces a redraw of a cell immediately (using a direct DC construction,
// or the supplied dc)
BOOL CGridCtrl::RedrawCell(const CCellID& cell, CDCHandle DC /* = nullptr */)
{
    return RedrawCell(cell.row, cell.col, DC);
}

BOOL CGridCtrl::RedrawCell(int nRow, int nCol, CDCHandle DC /* = nullptr */)
{
    BOOL bResult = TRUE;
    BOOL bMustReleaseDC = FALSE;

    if (!m_bAllowDraw || !IsCellVisible(nRow, nCol))
        return FALSE;

    CRect rect;
    if (!GetCellRect(nRow, nCol, rect))
        return FALSE;

    if (DC.IsNull())
    {
        DC = GetDC();
        if (!DC.IsNull())
            bMustReleaseDC = TRUE;
    }

    if (!DC.IsNull())
    {
        // Redraw cells directly
        if (nRow < m_nFixedRows || nCol < m_nFixedCols)
        {
            CGridCellBase* pCell = GetCell(nRow, nCol);
            if (pCell)
                bResult = pCell->Draw(DC, nRow, nCol, rect, TRUE);
        }
        else
        {
            CGridCellBase* pCell = GetCell(nRow, nCol);
            if (pCell)
                bResult = pCell->Draw(DC, nRow, nCol, rect, TRUE);

            // Since we have erased the background, we will need to redraw the gridlines
            CPen pen;
            pen.CreatePen(PS_SOLID, 0, m_crGridLineColour);

            CPenHandle OldPen = DC.SelectPen(pen);
            if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
            {
                DC.MoveTo(rect.left,    rect.bottom);
                DC.LineTo(rect.right + 1, rect.bottom);
            }
            if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
            {
                DC.MoveTo(rect.right, rect.top);
                DC.LineTo(rect.right, rect.bottom + 1);
            }
            DC.SelectPen(OldPen);
        }
    } else
        InvalidateRect(rect, TRUE);     // Could not get a DC - invalidate it anyway
    // and hope that OnPaint manages to get one

    if (bMustReleaseDC)
        ReleaseDC(DC);

    return bResult;
}

// redraw a complete row
BOOL CGridCtrl::RedrawRow(int row)
{
    BOOL bResult = TRUE;

    CDCHandle DC = GetDC();
    for (int col = 0; col < GetColumnCount(); col++)
        bResult = RedrawCell(row, col, DC) && bResult;
    if (!DC.IsNull())
        ReleaseDC(DC);

    return bResult;
}

// redraw a complete column
BOOL CGridCtrl::RedrawColumn(int col)
{
    BOOL bResult = TRUE;

    CDCHandle DC = GetDC();
    for (int row = 0; row < GetRowCount(); row++)
        bResult = RedrawCell(row, col, DC) && bResult;
    if (DC)
        ReleaseDC(DC);

    return bResult;
}


// Sets the currently selected cell, returning the previous current cell
CCellID CGridCtrl::SetFocusCell(int nRow, int nCol)
{
    return SetFocusCell(CCellID(nRow, nCol));
}

CCellID CGridCtrl::SetFocusCell(CCellID cell)
{
    if (cell == m_idCurrentCell)
        return m_idCurrentCell;

    CCellID idPrev = m_idCurrentCell;

    // EFW - Bug Fix - Force focus to be in a non-fixed cell
    if (cell.row != -1 && cell.row < GetFixedRowCount())
        cell.row = GetFixedRowCount();
    if (cell.col != -1 && cell.col < GetFixedColumnCount())
        cell.col = GetFixedColumnCount();

    m_idCurrentCell = cell;

    if (IsValid(idPrev))
    {
        SetItemState(idPrev.row, idPrev.col,
            GetItemState(idPrev.row, idPrev.col) & ~GVIS_FOCUSED);
        RedrawCell(idPrev); // comment to reduce flicker

        if (GetTrackFocusCell() && idPrev.col != m_idCurrentCell.col)
            for (int row = 0; row < m_nFixedRows; row++)
                RedrawCell(row, idPrev.col);
        if (GetTrackFocusCell() && idPrev.row != m_idCurrentCell.row)
            for (int col = 0; col < m_nFixedCols; col++)
                RedrawCell(idPrev.row, col);
    }

    if (IsValid(m_idCurrentCell))
    {
        SetItemState(m_idCurrentCell.row, m_idCurrentCell.col,
            GetItemState(m_idCurrentCell.row, m_idCurrentCell.col) | GVIS_FOCUSED);

        RedrawCell(m_idCurrentCell); // comment to reduce flicker

        if (GetTrackFocusCell() && idPrev.col != m_idCurrentCell.col)
            for (int row = 0; row < m_nFixedRows; row++)
                RedrawCell(row, m_idCurrentCell.col);
        if (GetTrackFocusCell() && idPrev.row != m_idCurrentCell.row)
            for (int col = 0; col < m_nFixedCols; col++)
                RedrawCell(m_idCurrentCell.row, col);

        // EFW - New addition.  If in list mode, make sure the selected
        // row highlight follows the cursor.
        // Removed by C Maunder 27 May
        //if (m_bListMode)
        //{
        //    m_PrevSelectedCellMap.RemoveAll();
        //    m_MouseMode = MOUSE_SELECT_ROW;
        //    OnSelecting(m_idCurrentCell);

            // Leave this off so that you can still drag the highlight around
            // without selecting rows.
            // m_MouseMode = MOUSE_NOTHING;
        //}

	}

    return idPrev;
}

// Sets the range of currently selected cells
void CGridCtrl::SetSelectedRange(const CCellRange& Range,
                                 BOOL bForceRepaint /* = FALSE */, BOOL bSelectCells/*=TRUE*/)
{
    SetSelectedRange(Range.GetMinRow(), Range.GetMinCol(),
                     Range.GetMaxRow(), Range.GetMaxCol(),
                     bForceRepaint, bSelectCells);
}

void CGridCtrl::SetSelectedRange(int nMinRow, int nMinCol, int nMaxRow, int nMaxCol,
                                 BOOL bForceRepaint /* = FALSE */, BOOL bSelectCells/*=TRUE*/)
{
    if (!m_bEnableSelection)
        return;

	//CWaitCursor wait; // Thomas Haase 

    CDCHandle DC(nullptr);
    if (bForceRepaint)
        DC = GetDC();

	// Only redraw visible cells
    CCellRange VisCellRange, FixedVisCellRange;
	if (IsWindow())
	{
		VisCellRange = GetVisibleNonFixedCellRange();
		FixedVisCellRange = GetVisibleFixedCellRange();
	}
   
    // EFW - Bug fix - Don't allow selection of fixed rows
	int Left= (m_AllowSelectRowInFixedCol ? 0 : GetFixedColumnCount());

    if(nMinRow >= 0 && nMinRow < GetFixedRowCount())
        nMinRow = GetFixedRowCount();
    if(nMaxRow >= 0 && nMaxRow < GetFixedRowCount())
        nMaxRow = GetFixedRowCount();
    if(nMinCol >= 0 && nMinCol < Left)
        nMinCol = GetFixedColumnCount();
    if(nMaxCol >= 0 && nMaxCol < Left)
        nMaxCol = GetFixedColumnCount();

    // If we are selecting cells, then first clear out the list of currently selected cells, then
    if (bSelectCells)
    {
        POSITION pos;

        // Unselect all previously selected cells
        for (pos = m_SelectedCellMap.GetStartPosition(); pos != NULL;)
        {
            DWORD key;
            CCellID cell;
            m_SelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);

            // Reset the selection flag on the cell
            if (IsValid(cell))
            {
                // This will remove the cell from the m_SelectedCellMap map
                SetItemState(cell.row, cell.col, GetItemState(cell.row, cell.col) & ~GVIS_SELECTED);

                // If this is to be reselected, continue on past the redraw
                if (nMinRow <= cell.row && cell.row <= nMaxRow &&
                    nMinCol <= cell.col && cell.col <= nMaxCol)
                    continue;

				if ((VisCellRange.IsValid() && VisCellRange.InRange(cell)) || FixedVisCellRange.InRange(cell))
				{
					if (bForceRepaint && !DC.IsNull())			// Redraw NOW
						RedrawCell(cell.row, cell.col, DC);
					else
						InvalidateCellRect(cell);                // Redraw at leisure
				}
            }
            else
            {
                m_SelectedCellMap.RemoveKey(key);  // if it's not valid, get rid of it!
            }
        }

        // if we are selecting cells, and there are previous selected cells to be retained 
        // (eg Ctrl is being held down) then copy them to the newly created list, and mark 
        // all these cells as selected
        // Note that if we are list mode, single row selection, then we won't be adding 
        // the previous cells. Only the current row of cells will be added (see below)
        if (!GetSingleRowSelection() &&
            nMinRow >= 0 && nMinCol >= 0 && nMaxRow >= 0 && nMaxCol >= 0)
        {
            for (pos = m_PrevSelectedCellMap.GetStartPosition(); pos != NULL; /* nothing */)
            {
                DWORD key;
                CCellID cell;
                m_PrevSelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);

                if (!IsValid(cell))
                    continue;

                int nState = GetItemState(cell.row, cell.col);

                // Set state as Selected. This will add the cell to m_SelectedCellMap
                SetItemState(cell.row, cell.col, nState | GVIS_SELECTED);

				if ((VisCellRange.IsValid() && VisCellRange.InRange(cell))  || FixedVisCellRange.InRange(cell))
				{
					// Redraw (immediately or at leisure)
					if (bForceRepaint && !DC.IsNull())
					    RedrawCell(cell.row, cell.col, DC);
					else
					    InvalidateCellRect(cell);
				}
            }
        }
    }

    // Now select/deselect all cells in the cell range specified. If selecting, and the cell 
    // has already been marked as selected (above) then ignore it. If we are deselecting and
    // the cell isn't selected, then ignore
    if (nMinRow >= 0 && nMinCol >= 0 && nMaxRow >= 0 && nMaxCol >= 0 &&
        nMaxRow < m_nRows && nMaxCol < m_nCols &&
        nMinRow <= nMaxRow && nMinCol <= nMaxCol)
    {
        for (int row = nMinRow; row <= nMaxRow; row++)
            for (int col = nMinCol; col <= nMaxCol; col++)
            {
                BOOL bCellSelected = IsCellSelected(row, col);
                if (bSelectCells == bCellSelected)
                    continue;    // Already selected or deselected - ignore

                // Set the selected state. This will add/remove the cell to m_SelectedCellMap
                if (bSelectCells)
                    SetItemState(row, col, GetItemState(row, col) | GVIS_SELECTED);
                else
                    SetItemState(row, col, GetItemState(row, col) & ~GVIS_SELECTED);

				if ((VisCellRange.IsValid() && VisCellRange.InRange(row, col))  || FixedVisCellRange.InRange(row, col))
				{
	                // Redraw (immediately or at leisure)
	                if (bForceRepaint && !DC.IsNull())
	                    RedrawCell(row, col, DC);
	                else
	                    InvalidateCellRect(row, col);
				}
            }
    }
    ATLTRACE(_T("%d cells selected.\n"), m_SelectedCellMap.GetCount());

    if (!DC.IsNull())
        ReleaseDC(DC);
}

// selects all cells
void CGridCtrl::SelectAllCells()
{
    if (!m_bEnableSelection)
        return;

    SetSelectedRange(m_nFixedRows, m_nFixedCols, GetRowCount()-1, GetColumnCount()-1);
}

// selects columns
void CGridCtrl::SelectColumns(CCellID currentCell, 
                              BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
    if (!m_bEnableSelection)
        return;

    //if (currentCell.col == m_idCurrentCell.col) return;
    if (currentCell.col < m_nFixedCols)
        return;
    if (!IsValid(currentCell))
        return;

    if (GetSingleColSelection())
        SetSelectedRange(GetFixedRowCount(), currentCell.col,
			GetRowCount()-1, currentCell.col,
			bForceRedraw, bSelectCells);
    else
        SetSelectedRange(GetFixedRowCount(),
			__min(m_SelectionStartCell.col, currentCell.col),
			GetRowCount()-1,
			__max(m_SelectionStartCell.col, currentCell.col),
			bForceRedraw, bSelectCells);
}

// selects rows
void CGridCtrl::SelectRows(CCellID currentCell, 
                           BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
    if (!m_bEnableSelection)
        return;

    //if (currentCell.row; == m_idCurrentCell.row) return;
    if (currentCell.row < m_nFixedRows)
        return;
    if (!IsValid(currentCell))
        return;
	int Left = (m_AllowSelectRowInFixedCol ? 0 : GetFixedColumnCount());
    if (GetSingleRowSelection())
        SetSelectedRange(currentCell.row, Left ,
                         currentCell.row, GetColumnCount()-1, 
                         bForceRedraw, bSelectCells);
    else
        SetSelectedRange(__min(m_SelectionStartCell.row, currentCell.row),
			Left,
			__max(m_SelectionStartCell.row, currentCell.row),
			GetColumnCount()-1,
			bForceRedraw, bSelectCells);
}

// selects cells
void CGridCtrl::SelectCells(CCellID currentCell, 
                            BOOL bForceRedraw /*=FALSE*/, BOOL bSelectCells /*=TRUE*/)
{
    if (!m_bEnableSelection)
        return;

    int row = currentCell.row;
    int col = currentCell.col;
    if (row < m_nFixedRows || col < m_nFixedCols)
        return;
    if (!IsValid(currentCell))
        return;

    // Prevent unnecessary redraws
    //if (currentCell == m_LeftClickDownCell)  return;
    //else if (currentCell == m_idCurrentCell) return;

    SetSelectedRange(__min(m_SelectionStartCell.row, row),
		__min(m_SelectionStartCell.col, col),
		__max(m_SelectionStartCell.row, row),
		__max(m_SelectionStartCell.col, col),
		bForceRedraw, bSelectCells);
}

// Called when mouse/keyboard selection is a-happening.
void CGridCtrl::OnSelecting(const CCellID& currentCell)
{
    if (!m_bEnableSelection)
        return;

    switch (m_MouseMode)
    {
    case MOUSE_SELECT_ALL:
        SelectAllCells();
        break;
    case MOUSE_SELECT_COL:
        SelectColumns(currentCell, FALSE);
        break;
    case MOUSE_SELECT_ROW:
        SelectRows(currentCell, FALSE);
        break;
    case MOUSE_SELECT_CELLS:
        SelectCells(currentCell, FALSE);
        break;
    }

    // EFW - Bug fix [REMOVED CJM: this will cause infinite loop in list mode]
    // SetFocusCell(max(currentCell.row, m_nFixedRows), max(currentCell.col, m_nFixedCols));
}

void CGridCtrl::ValidateAndModifyCellContents(int nRow, int nCol, LPCTSTR strText)
{
    if (!IsCellEditable(nRow, nCol))
        return;

    if (SendMessageToParent(nRow, nCol, GVN_BEGINLABELEDIT) >= 0)
    {
        CString strCurrentText = GetItemText(nRow, nCol);
        if (strCurrentText != strText)
        {
            SetItemText(nRow, nCol, strText);
            if (ValidateEdit(nRow, nCol, strText) && 
                SendMessageToParent(nRow, nCol, GVN_ENDLABELEDIT) >= 0)
            {
                SetModified(TRUE, nRow, nCol);
                RedrawCell(nRow, nCol);
            }
            else
                SetItemText(nRow, nCol, strCurrentText);
        }
    }
}

void CGridCtrl::ClearCells(CCellRange Selection)
{
    for (int row = Selection.GetMinRow(); row <= Selection.GetMaxRow(); row++)
    {
        for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
        {
            // don't clear hidden cells
            if (m_arRowHeights[row] > 0 && m_arColWidths[col] > 0)
                ValidateAndModifyCellContents(row, col, _T(""));
		}
	}
    Refresh();
}

#ifndef GRIDCONTROL_NO_CLIPBOARD

////////////////////////////////////////////////////////////////////////////////////////
// Clipboard functions

// Deletes the contents from the selected cells
void CGridCtrl::CutSelectedText()
{
    if (!IsEditable())
        return;

    for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != NULL;)
    {
		DWORD key;
        CCellID cell;
        m_SelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);
        ValidateAndModifyCellContents(cell.row, cell.col, _T(""));
    }
}

// Copies text from the selected cells to the clipboard
CIDataObject *CGridCtrl::CopyTextFromGrid()
{
	CCellRange Selection = GetSelectedCellRange();
	if (!IsValid(Selection))
		return nullptr;

	if (GetVirtualMode())
		SendCacheHintToParent(Selection);

	CIDataObject *pdobj = new CIDataObject(nullptr);
	if (!pdobj)
		AtlThrowLastWin32();

	{	// Copy text
		// Get a tab delimited string to copy to cache
		CString str;
		CGridCellBase *pCell;
		for (int row = Selection.GetMinRow(); row <= Selection.GetMaxRow(); row++)
		{
			// don't copy hidden cells
			if(m_arRowHeights[row] <= 0)
				continue;

			for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
			{
				// don't copy hidden cells
				if(m_arColWidths[col] <= 0)
					continue;

				pCell = GetCell(row, col);
				if (pCell &&(pCell->GetState() & GVIS_SELECTED))
					str += pCell->GetText();
				if (col != Selection.GetMaxCol()) 
					str += _T("\t");
			}

			if (row != Selection.GetMaxRow()) 
				str += _T("\r\n");

		}

		unsigned nSize = (str.GetLength() + 1) * sizeof(TCHAR); // Include the last '\0'
		HGLOBAL hGl = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (!hGl)
			AtlThrowLastWin32();
		LPVOID ptr = ::GlobalLock(hGl);
		if (!ptr)
			AtlThrowLastWin32();
		::CopyMemory(ptr, str, nSize);
		if (!::GlobalUnlock(hGl))
			if (::GetLastError() != NO_ERROR) AtlThrowLastWin32();

		if (GetVirtualMode())
			SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

		// Init the supported format
		FORMATETC fmtetc = { GRID_CF, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		// Init the medium used
		STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };
		medium.hGlobal = hGl;

		// Add it to DataObject
		if (!SUCCEEDED(pdobj->SetData(&fmtetc, &medium, FALSE)))
			AtlThrowLastWin32();
		::GlobalFree(hGl);
	}

#ifndef GRIDCONTROL_NO_RTF
	// TODO: Insert table formatting
	{	// Copy RTF
		// Get a tab delimited string to copy to cache
		CString str;
		CGridCellBase *pCell;

		// Get Clipboard format id for RTF.
		UINT cf = RegisterClipboardFormat(_T("Rich Text Format"));
		if (0 == cf)
			AtlThrowLastWin32();

		str += _T("{\\rtf1\\ansi\\deff0\\deflang1049{\\fonttbl{\\f0 Times New Roman;}}\r\n{\\colortbl;\\red160\\green160\\blue0;\\red160\\green0\\blue0;}\r\n{\\pard\r\n");
		for (int row = Selection.GetMinRow(); row <= Selection.GetMaxRow(); row++)
		{
			// don't copy hidden cells
			if(m_arRowHeights[row] <= 0)
				continue;

			str += _T("\\trowd\\trgaph108\\trql\\trrh280\\trleft36\r\n");
			unsigned i = 0;
			for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
			{
				CString tmp;
				tmp.Format(_T("\\clbrdrt\\brdrs\\clbrdrl\\brdrs\\clbrdrb\\brdrdb\\clbrdrr\\brdrdb\\cellx%d\r\n"), i+=1500);
				str += tmp;
			}

			for (int col = Selection.GetMinCol(); col <= Selection.GetMaxCol(); col++)
			{
				// don't copy hidden cells
				if(m_arColWidths[col] <= 0)
					continue;

				pCell = GetCell(row, col);
				if (pCell &&(pCell->GetState() & GVIS_SELECTED))
				{
					str += _T("\\pard\\intbl ");
					str += EncodeRTF(pCell->GetText());
					str += _T(" \\cell\r\n");
				}
			}
			str += _T("\\row\\pard\r\n");
		}
		str += _T("\\par}\r\n}");

		unsigned nSize = (str.GetLength() + 1) * sizeof(TCHAR); // Include the last '\0'
		HGLOBAL hGl = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (!hGl)
			AtlThrowLastWin32();
		LPVOID ptr = ::GlobalLock(hGl);
		if (!ptr)
			AtlThrowLastWin32();
		::CopyMemory(ptr, str, nSize);
		if (!::GlobalUnlock(hGl))
			if (::GetLastError() != NO_ERROR) AtlThrowLastWin32();

		if (GetVirtualMode())
			SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

		// Init the supported format
		FORMATETC fmtetc = { cf, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		// Init the medium used
		STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };
		medium.hGlobal = hGl;

		// Add it to DataObject
		if (!SUCCEEDED(pdobj->SetData(&fmtetc, &medium, FALSE)))
			AtlThrowLastWin32();
		::GlobalFree(hGl);
	}
#endif	// GRIDCONTROL_NO_RTF
	return pdobj;
}

// Pastes text from the clipboard to the selected cells
BOOL CGridCtrl::PasteTextToGrid(CCellID cell, FORMATETC* pFmtEtc, STGMEDIUM& medium, BOOL bSelectPastedCells /*=TRUE*/)
{
    if (!IsValid(cell) || !IsCellEditable(cell))
        return FALSE;

	HGLOBAL hGl = medium.hGlobal;
	SIZE_T nSize = ::GlobalSize(hGl);
	if (!nSize)
		AtlThrowLastWin32();
	CString strText;
	LPTSTR szBuffer = strText.GetBufferSetLength(int(nSize + sizeof(TCHAR)));
	if (!szBuffer)
		AtlThrowLastWin32();
	LPVOID ptr = ::GlobalLock(hGl);
	if (!ptr)
		AtlThrowLastWin32();
	::CopyMemory(szBuffer, ptr, nSize);
	if (!::GlobalUnlock(hGl))
		if (::GetLastError() != NO_ERROR) AtlThrowLastWin32();
	strText.ReleaseBuffer();

    // Parse text data and set in cells...
    strText.LockBuffer();
    CString strLine = strText;
    int nLine = 0;

    // Find the end of the first line
	CCellRange PasteRange(cell.row, cell.col,-1,-1);
    int nIndex;
    do
    {
        int nColumn = 0;
        nIndex = strLine.Find(_T("\n"));

        // Store the remaining chars after the newline
        CString strNext = (nIndex < 0)? _T("")  : strLine.Mid(nIndex + 1);

        // Remove all chars after the newline
        if (nIndex >= 0)
            strLine = strLine.Left(nIndex);

        int nLineIndex = strLine.FindOneOf(_T("\t,"));
        CString strCellText = (nLineIndex >= 0)? strLine.Left(nLineIndex) : strLine;

        // skip hidden rows
        int iRowVis = cell.row + nLine;
        while(iRowVis < GetRowCount())
        {
            if(GetRowHeight(iRowVis) > 0)
                break;
            nLine++;
            iRowVis++;
        }

        while (!strLine.IsEmpty())
        {
            // skip hidden columns
            int iColVis = cell.col + nColumn;
            while(iColVis < GetColumnCount())
            {
                if(GetColumnWidth(iColVis) > 0)
                    break;
                nColumn++;
                iColVis++;
            }

            CCellID TargetCell(iRowVis, iColVis);
            if (IsValid(TargetCell))
            {
                strCellText.TrimLeft();
                strCellText.TrimRight();

                ValidateAndModifyCellContents(TargetCell.row, TargetCell.col, strCellText);

                // Make sure cell is not selected to avoid data loss
                SetItemState(TargetCell.row, TargetCell.col,
                    GetItemState(TargetCell.row, TargetCell.col) & ~GVIS_SELECTED);

				if (iRowVis > PasteRange.GetMaxRow()) PasteRange.SetMaxRow(iRowVis);
				if (iColVis > PasteRange.GetMaxCol()) PasteRange.SetMaxCol(iColVis);
            }

            strLine = (nLineIndex >= 0)? strLine.Mid(nLineIndex + 1) : _T("");
            nLineIndex = strLine.FindOneOf(_T("\t,"));
            strCellText = (nLineIndex >= 0)? strLine.Left(nLineIndex) : strLine;

            nColumn++;
        }

        strLine = strNext;
        nLine++;
    } while (nIndex >= 0);

    strText.UnlockBuffer();

	if (bSelectPastedCells)
		SetSelectedRange(PasteRange, TRUE);
	else
	{
		ResetSelectedRange();
		Refresh();
	}

    return TRUE;
}

#ifndef GRIDCONTROL_NO_RTF
CString CGridCtrl::EncodeRTF(const CString &strRTF)
{
	CString strOutput;
	for (int i = 0; i < strRTF.GetLength(); i++)
	{
		char ch = strRTF[i];
		switch (ch)
		{
		case ' ':
			strOutput += "\\ ";
			break;
		case '{':
			strOutput += "\\{";
			break;
		case '}':
			strOutput += "\\}";
			break;
		case '\\':
			strOutput += "\\\\";
			break;
		default:
			strOutput += ch;
		}
	}
	return strOutput;
}
#endif	// GRIDCONTROL_NO_RTF
#endif	//GRIDCONTROL_NO_CLIPBOARD

#ifndef GRIDCONTROL_NO_DRAGDROP

// Start drag n drop
void CGridCtrl::OnBeginDrag()
{
	CIDataObject *pdobj = nullptr;
    CIDropSource *pSource = nullptr;
	DWORD dropEffect = DROPEFFECT_MOVE;

	if (!m_bAllowDragAndDrop)
        return;
	
	pdobj = CopyTextFromGrid();
	if (!pdobj)
		return;	// Selection is invalid;

	pSource = new CIDropSource();
	pSource->AddRef();

	SendMessageToParent(GetSelectedCellRange().GetTopLeft().row, GetSelectedCellRange().GetTopLeft().col, GVN_BEGINDRAG);

	m_MouseMode = MOUSE_DRAGGING;
	m_bLMouseButtonDown = FALSE;

	CAtlMap<DWORD, CCellID> m_DragSelectedCellMap;
	for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != NULL;)
	{
		DWORD key;
		CCellID cell;
		m_SelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);
		m_DragSelectedCellMap.SetAt(key, cell);
	}

	if (FAILED(::DoDragDrop(pdobj, pSource, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dropEffect)))
		AtlThrowLastWin32();

	m_DragEnterCell = CCellID(-1, -1);
	if (dropEffect & DROPEFFECT_MOVE)
	{
		if (IsEditable())
		{
			for (POSITION pos = m_DragSelectedCellMap.GetStartPosition(); pos != NULL;)
			{
				DWORD key;
				CCellID cell;
				m_DragSelectedCellMap.GetNextAssoc(pos, key, cell);
				ValidateAndModifyCellContents(cell.row, cell.col, _T(""));
			}
		}
	}

	pSource->Release();
}

// Handle drag over grid
HRESULT CGridCtrl::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	CPoint point(pt.x, pt.y);
	ScreenToClient(&point);
	// Find which cell we are over and drop-highlight it
	CCellID cell = GetCellFromPt(point, FALSE);
	AtlTrace(_T("DragOver: %d %d\n"), cell.col, cell.row);
	bool Valid;
	// Any text data available for us?
	if (m_CurCol==-1)
	{
		if (m_bDragRowMode)
			Valid = cell.col>=GetFixedColumnCount() && cell.row>=GetFixedRowCount();
		else
		{
			if (!m_bAllowDragAndDrop || !IsEditable())
			{
				*pdwEffect = DROPEFFECT_NONE;
				return S_OK;
			}
			Valid = IsValid(cell)!=0;
		}
	}
	else
		Valid = cell.col>=GetFixedColumnCount() && cell.row<GetFixedRowCount();

	// If not valid, set the previously drop-highlighted cell as no longer drop-highlighted
    if (cell == m_DragEnterCell || !Valid)
    {
        DragLeave();
        m_LastDragOverCell = CCellID(-1, -1);
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
    }
	if (m_CurCol==-1)
	{
		if (!m_bDragRowMode && !IsCellEditable(cell))
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
	}

    // Have we moved over a different cell than last time?
    if (cell != m_LastDragOverCell)
    {
        // Set the previously drop-highlighted cell as no longer drop-highlighted
        if (IsValid(m_LastDragOverCell))
        {
            UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
            SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col, nState & ~GVIS_DROPHILITED);
            RedrawCell(m_LastDragOverCell);
        }

        m_LastDragOverCell = cell;

        // Set the new cell as drop-highlighted
        if (IsValid(m_LastDragOverCell))
        {
            UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
            SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col, nState | GVIS_DROPHILITED);
            RedrawCell(m_LastDragOverCell);
        }
    }

	return S_OK;
}

// Something has just been dragged onto the grid
HRESULT CGridCtrl::DragEnter(IDataObject __RPC_FAR *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	CPoint point(pt.x, pt.y);
	ScreenToClient(&point);
    // Any text data available for us?
    m_DragEnterCell = m_LastDragOverCell = GetCellFromPt(point, m_CurCol>=0);
	AtlTrace(_T("DragEnter: %d %d\n"), m_DragEnterCell.col, m_DragEnterCell.row);
	bool Valid;
	if(m_CurCol==-1)
	{
		if (!m_bAllowDragAndDrop)
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		// Find which cell we are over and drop-highlight it
		if (!IsValid(m_LastDragOverCell))
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}

		if (!IsCellEditable(m_LastDragOverCell))
		{
			*pdwEffect = DROPEFFECT_NONE;
			return S_OK;
		}
		Valid = IsValid(m_LastDragOverCell)!=0;
	}
	else
		Valid = m_LastDragOverCell.row>=0 && m_LastDragOverCell.row<GetFixedRowCount() && m_LastDragOverCell.col>=GetFixedColumnCount();

	if (Valid)
	{
		UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
		SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
			nState | GVIS_DROPHILITED);
		RedrawCell(m_LastDragOverCell);
	}

	return S_OK;
}

// Something has just been dragged away from the grid
HRESULT CGridCtrl::DragLeave(void)
{
    // Set the previously drop-highlighted cell as no longer drop-highlighted
    if (IsValid(m_LastDragOverCell))
    {
        UINT nState = GetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col);
        SetItemState(m_LastDragOverCell.row, m_LastDragOverCell.col,
            nState & ~GVIS_DROPHILITED);
        RedrawCell(m_LastDragOverCell);
    }
	return S_OK;
}

// Something has just been dropped onto the grid
bool CGridCtrl::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD *pdwEffect)
{
	m_MouseMode = MOUSE_NOTHING;
	AtlTrace(_T("DragDrop: %d %d\n"), m_LastDragOverCell.col, m_LastDragOverCell.row);
	if(m_CurCol ==-1)
	{
		if (m_LastDragOverCell == m_DragEnterCell || !m_bAllowDragAndDrop || (!IsCellEditable(m_LastDragOverCell) && !m_bDragRowMode))
			return true;
	}

	DragLeave();
	if (m_CurCol>=0)
	{
		if(m_LastDragOverCell.col == m_CurCol || m_LastDragOverCell.row >= GetFixedRowCount()) 
			return true;
		else
		{
			int New = m_arColOrder[m_CurCol];
			m_arColOrder.erase(m_arColOrder.begin()+m_CurCol);
			m_arColOrder.insert(m_arColOrder.begin()+m_LastDragOverCell.col, New);
			m_CurCol=-1;
			Invalidate();
		}
	}
	else
	{
		if(m_bDragRowMode)
		{
			Reorder(m_CurRow, m_LastDragOverCell.row);
			Invalidate();
		}
		else
			PasteTextToGrid(m_LastDragOverCell, pFmtEtc, medium, FALSE);
	}
	m_DragEnterCell = CCellID(-1,-1);
	return true;
}
#endif

#ifndef GRIDCONTROL_NO_CLIPBOARD
void CGridCtrl::EditCut()
{
    if (!IsEditable())
        return;

	EditCopy();
    CutSelectedText();
}

void CGridCtrl::EditCopy()
{
	CIDataObject *pdobj = CopyTextFromGrid();

	// Add data to clipboard
	if (!SUCCEEDED(OleSetClipboard(pdobj)))
		AtlThrowLastWin32();
}

void CGridCtrl::EditPaste()
{
    if (!IsEditable())
        return;

	CCellRange cellRange = GetSelectedCellRange();

	// Get the top-left selected cell, or the Focus cell, or the topleft (non-fixed) cell
	CCellID cell;
	if (cellRange.IsValid())
	{
		cell.row = cellRange.GetMinRow();
		cell.col = cellRange.GetMinCol();
	}
	else
	{
		cell = GetFocusCell();
		if (!IsValid(cell))
			cell = GetTopleftNonFixedCell();
		if (!IsValid(cell))
			return;
	}

/*
	// If a cell is being edited, then call it's edit window paste function.
	if (IsItemEditing(cell.row, cell.col))
    {
        CGridCellBase* pCell = GetCell(cell.row, cell.col);
        ATLASSERT(pCell);
        if (!pCell) return;

		CWnd* pEditWnd = pCell->GetEditWnd();
		if (pEditWnd && pEditWnd->IsKindOf(RUNTIME_CLASS(CEdit)))
		{
			((CEdit*)pEditWnd)->Paste();
			return;
		}
    }
*/

	if (!IsClipboardFormatAvailable(GRID_CF))
		return;
	IDataObject *pdobj;
	if (!SUCCEEDED(OleGetClipboard(&pdobj)))
		return;
	FORMATETC fmtetc = {GRID_CF, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM medium;
	if (!SUCCEEDED(pdobj->QueryGetData(&fmtetc)))
		return;
	if (!SUCCEEDED(pdobj->GetData(&fmtetc, &medium)))
	{
		pdobj->Release();
		return;
	}
	pdobj->Release();
	PasteTextToGrid(cell, &fmtetc, medium);
	ReleaseStgMedium(&medium);
}
#endif

void CGridCtrl::EditSelectAll()
{
    SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);
    SelectAllCells();
    SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);
}

////////////////////////////////////////////////////////////////////////////////////////
// hittest-like functions

// TRUE if the mouse is over a row resize area
BOOL CGridCtrl::MouseOverRowResizeArea(CPoint& point)
{
    if (point.x >= GetFixedColumnWidth())
        return FALSE;

    CCellID idCurrentCell = GetCellFromPt(point);
    CPoint start;
    if (!GetCellOrigin(idCurrentCell, &start))
        return FALSE;

    int endy = start.y + GetRowHeight(idCurrentCell.row);

    if ((point.y - start.y < m_nResizeCaptureRange && idCurrentCell.row != 0) ||
        endy - point.y < m_nResizeCaptureRange)
        return TRUE;
    else
        return FALSE;
}

// TRUE if the mouse is over a column resize area. point is in Client coords
BOOL CGridCtrl::MouseOverColumnResizeArea(CPoint& point)
{
    if (point.y >= GetFixedRowHeight())
        return FALSE;

    CCellID idCurrentCell = GetCellFromPt(point);
    CPoint start;
    if (!GetCellOrigin(idCurrentCell, &start))
        return FALSE;

    int endx = start.x + GetColumnWidth(idCurrentCell.col);

    if ((point.x - start.x < m_nResizeCaptureRange && idCurrentCell.col != 0) ||
        endx - point.x < m_nResizeCaptureRange)
        return TRUE;
    else
        return FALSE;
}

// Get cell from point.
// point - client coordinates
// bAllowFixedCellCheck - if TRUE then fixed cells are checked
CCellID CGridCtrl::GetCellFromPt(CPoint point, BOOL bAllowFixedCellCheck /*=TRUE*/)
{
    CCellID cellID; // return value

    CCellID idTopLeft = GetTopleftNonFixedCell();
	if (!bAllowFixedCellCheck && !IsValid(idTopLeft))
		return cellID;

    // calculate column index
    int fixedColWidth = GetFixedColumnWidth();

    if (point.x < 0 || (!bAllowFixedCellCheck && point.x < fixedColWidth)) // not in window
        cellID.col = -1;
    else if (point.x < fixedColWidth) // in fixed col
    {
        int xpos = 0;
        int col = 0;
        while (col < m_nFixedCols)
        {
            xpos += GetColumnWidth(col);
            if (xpos > point.x)
                break;
			col++;
        }

        cellID.col = col;
    }
    else    // in non-fixed col
    {
        int xpos = fixedColWidth;
		int col = idTopLeft.col; //m_nFixedCols;
        while (col < GetColumnCount())
        {
            xpos += GetColumnWidth(col);
            if (xpos > point.x)
                break;
			col++;
        }

        if (col >= GetColumnCount())
            cellID.col = -1;
        else
            cellID.col = col;
    }

    // calculate row index
    int fixedRowHeight = GetFixedRowHeight();
    if (point.y < 0 || (!bAllowFixedCellCheck && point.y < fixedRowHeight)) // not in window
        cellID.row = -1;
    else if (point.y < fixedRowHeight) // in fixed col
    {
        int ypos = 0;
        int row = 0;
        while (row < m_nFixedRows) 
        {
            ypos += GetRowHeight(row);
            if (ypos > point.y)
                break;
			row++;
        }
        cellID.row = row;
    }
    else
    {
        int ypos = fixedRowHeight;
		int row = idTopLeft.row; //m_nFixedRows;
        while (row < GetRowCount())
        {
            ypos += GetRowHeight(row);
            if (ypos > point.y)
                break;
			row++;
        }

        if (row >= GetRowCount())
            cellID.row = -1;
        else
            cellID.row = row;
    }
    return cellID;
}

////////////////////////////////////////////////////////////////////////////////
// CGridCtrl cellrange functions

// Gets the first non-fixed cell ID
CCellID CGridCtrl::GetTopleftNonFixedCell(BOOL bForceRecalculation /*=FALSE*/)
{
    // Used cached value if possible
    if (m_idTopLeftCell.IsValid() && !bForceRecalculation)
        return m_idTopLeftCell;

    int nVertScroll = GetScrollPos(SB_VERT), 
        nHorzScroll = GetScrollPos(SB_HORZ);

    m_idTopLeftCell.col = m_nFixedCols;
    int nRight = 0;
    while (nRight < nHorzScroll && m_idTopLeftCell.col < (GetColumnCount()-1))
        nRight += GetColumnWidth(m_idTopLeftCell.col++);

    m_idTopLeftCell.row = m_nFixedRows;
    int nTop = 0;
    while (nTop < nVertScroll && m_idTopLeftCell.row < (GetRowCount()-1))
        nTop += GetRowHeight(m_idTopLeftCell.row++);

    //TRACE2("TopLeft cell is row %d, col %d\n",m_idTopLeftCell.row, m_idTopLeftCell.col);
    return m_idTopLeftCell;
}

// This gets even partially visible cells
CCellRange CGridCtrl::GetVisibleNonFixedCellRange(LPRECT pRect /*=nullptr*/, 
                                                  BOOL bForceRecalculation /*=FALSE*/)
{
    int i;
    CRect rect;
    GetClientRect(rect);

    CCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

    // calc bottom
    int bottom = GetFixedRowHeight();
    for (i = idTopLeft.row; i < GetRowCount(); i++)
    {
        bottom += GetRowHeight(i);
        if (bottom >= rect.bottom)
        {
            bottom = rect.bottom;
            break;
        }
    }
    int maxVisibleRow = __min(i, GetRowCount() - 1);

    // calc right
    int right = GetFixedColumnWidth();
    for (i = idTopLeft.col; i < GetColumnCount(); i++)
    {
        right += GetColumnWidth(i);
        if (right >= rect.right)
        {
            right = rect.right;
            break;
        }
    }
    int maxVisibleCol = __min(i, GetColumnCount() - 1);
    if (pRect)
    {
        pRect->left = pRect->top = 0;
        pRect->right = right;
        pRect->bottom = bottom;
    }

    return CCellRange(idTopLeft.row, idTopLeft.col, maxVisibleRow, maxVisibleCol);
}

CCellRange CGridCtrl::GetVisibleFixedCellRange(LPRECT pRect /*=nullptr*/, 
                                                  BOOL bForceRecalculation /*=FALSE*/)
{
    int i;
    CRect rect;
    GetClientRect(rect);

    CCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

    // calc bottom
    int bottom = GetFixedRowHeight();
    for (i = idTopLeft.row; i < GetRowCount(); i++)
    {
        bottom += GetRowHeight(i);
        if (bottom >= rect.bottom)
        {
            bottom = rect.bottom;
            break;
        }
    }
    int maxVisibleRow = __min(i, GetRowCount() - 1);

    // calc right
    int right = 0;
    for (i = 0; i < GetFixedColumnCount(); i++)
    {
        right += GetColumnWidth(i);
        if (right >= rect.right)
        {
            right = rect.right;
            break;
        }
    }
    int maxVisibleCol = __min(i, GetColumnCount() - 1);
    if (pRect)
    {
        pRect->left = pRect->top = 0;
        pRect->right = right;
        pRect->bottom = bottom;
    }

    return CCellRange(idTopLeft.row, 0, maxVisibleRow, maxVisibleCol);
}

// used by ResetScrollBars() - This gets only fully visible cells
CCellRange CGridCtrl::GetUnobstructedNonFixedCellRange(BOOL bForceRecalculation /*=FALSE*/)
{
    CRect rect;
    GetClientRect(rect);

    CCellID idTopLeft = GetTopleftNonFixedCell(bForceRecalculation);

    // calc bottom
    int i;
    int bottom = GetFixedRowHeight();
    for (i = idTopLeft.row; i < GetRowCount(); i++)
    {
        bottom += GetRowHeight(i);
        if (bottom >= rect.bottom)
            break;
    }
    int maxVisibleRow = __min(i, GetRowCount() - 1);
    if (maxVisibleRow > 0 && bottom > rect.bottom)
        maxVisibleRow--;

    // calc right
    int right = GetFixedColumnWidth();
    for (i = idTopLeft.col; i < GetColumnCount(); i++)
    {
        right += GetColumnWidth(i);
        if (right >= rect.right)
            break;
    }
    int maxVisibleCol = __min(i, GetColumnCount() - 1);
    if (maxVisibleCol > 0 && right > rect.right)
        maxVisibleCol--;


    return CCellRange(idTopLeft.row, idTopLeft.col, maxVisibleRow, maxVisibleCol);
}

// Returns the minimum bounding range of the current selection
// If no selection, then the returned CCellRange will be invalid
CCellRange CGridCtrl::GetSelectedCellRange() const
{
    CCellRange Selection(GetRowCount(), GetColumnCount(), -1,-1);

    for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != NULL;)
    {
        DWORD key;
        CCellID cell;
        m_SelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);

        Selection.SetMinRow(__min(Selection.GetMinRow(), cell.row));
        Selection.SetMinCol(__min(Selection.GetMinCol(), cell.col));
        Selection.SetMaxRow(__max(Selection.GetMaxRow(), cell.row));
        Selection.SetMaxCol(__max(Selection.GetMaxCol(), cell.col));
    }

    return Selection;
}

// Returns ALL the cells in the grid
CCellRange CGridCtrl::GetCellRange() const
{
    return CCellRange(0, 0, GetRowCount() - 1, GetColumnCount() - 1);
}

// Resets the selected cell range to the empty set.
void CGridCtrl::ResetSelectedRange()
{
    m_PrevSelectedCellMap.RemoveAll();
    SetSelectedRange(-1,-1,-1,-1);
    SetFocusCell(-1,-1);
}

// Get/Set scroll position using 32 bit functions
int CGridCtrl::GetScrollPos32(int nBar, BOOL bGetTrackPos /* = FALSE */)
{
    SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(SCROLLINFO);

    if (bGetTrackPos)
    {
		si.fMask = SIF_TRACKPOS;
        if (GetScrollInfo(nBar, &si))
            return si.nTrackPos;
    }
    else
    {
		si.fMask = SIF_POS;
        if (GetScrollInfo(nBar, &si))
            return si.nPos;
    }

    return 0;
}

BOOL CGridCtrl::SetScrollPos32(int nBar, int nPos, BOOL bRedraw /* = TRUE */)
{
    m_idTopLeftCell.row = -1;

    SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask  = SIF_POS;
    si.nPos   = nPos;
    return SetScrollInfo(nBar, &si, bRedraw);
}

void CGridCtrl::ShowScrollBars(int nBar, BOOL bEnable /*=TRUE*/)
{
    if (bEnable)
    {
        if (!IsVisibleHScroll() && (nBar == SB_HORZ || nBar == SB_BOTH))
        {
            m_nBarState |= GVL_HORZ;
            ShowScrollBar(SB_HORZ, TRUE);
        }
        
        if (!IsVisibleVScroll() && (nBar == SB_VERT || nBar == SB_BOTH))
        {
            m_nBarState |= GVL_VERT;
            ShowScrollBar(SB_VERT, TRUE);
        }
    }
    else
    {
        if (IsVisibleHScroll() && (nBar == SB_HORZ || nBar == SB_BOTH))
        {
            m_nBarState &= ~GVL_HORZ; 
            ShowScrollBar(SB_HORZ, FALSE);
        }
        
        if (IsVisibleVScroll() && (nBar == SB_VERT || nBar == SB_BOTH))
        {
            m_nBarState &= ~GVL_VERT;
            ShowScrollBar(SB_VERT, FALSE);
        }
    }
}

// If resizing or cell counts/sizes change, call this - it'll fix up the scroll bars
void CGridCtrl::ResetScrollBars()
{
    // Force a refresh. 
    m_idTopLeftCell.row = -1;

    if (!m_bAllowDraw || !IsWindow()) 
        return;
    
    CRect rect;
    
    // This would have caused OnSize event - Brian 
    //EnableScrollBars(SB_BOTH, FALSE); 
    
    GetClientRect(rect);
    
    if (rect.left == rect.right || rect.top == rect.bottom)
        return;
    
    if (IsVisibleVScroll())
        rect.right += GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXBORDER);
    
    if (IsVisibleHScroll())
        rect.bottom += GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYBORDER);
    
    rect.left += GetFixedColumnWidth();
    rect.top += GetFixedRowHeight();
    
    
    if (rect.left >= rect.right || rect.top >= rect.bottom)
    {
        ShowScrollBar(SB_BOTH, FALSE);
        return;
    }
    
    CRect VisibleRect(GetFixedColumnWidth(), GetFixedRowHeight(), 
		              rect.right, rect.bottom);
    CRect VirtualRect(GetFixedColumnWidth(), GetFixedRowHeight(),
		              GetVirtualWidth(), GetVirtualHeight());
    
    // Removed to fix single row scrollbar problem (Pontus Goffe)
    // CCellRange visibleCells = GetUnobstructedNonFixedCellRange();
    // if (!IsValid(visibleCells)) return;
        
    ATLTRACE(_T("Visible: %d x %d, Virtual %d x %d.  H %d, V %d\n"), 
          VisibleRect.Width(), VisibleRect.Height(),
          VirtualRect.Width(), VirtualRect.Height(),
          IsVisibleHScroll(), IsVisibleVScroll());

    // If vertical scroll bar, horizontal space is reduced
    if (VisibleRect.Height() < VirtualRect.Height())
        VisibleRect.right -= ::GetSystemMetrics(SM_CXVSCROLL);
    // If horz scroll bar, vert space is reduced
    if (VisibleRect.Width() < VirtualRect.Width())
        VisibleRect.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);
    
    // Recheck vertical scroll bar
    //if (VisibleRect.Height() < VirtualRect.Height())
    // VisibleRect.right -= ::GetSystemMetrics(SM_CXVSCROLL);
    
    if (VisibleRect.Height() < VirtualRect.Height())
    {
        ShowScrollBars(SB_VERT, TRUE); 
        m_nVScrollMax = VirtualRect.Height() - 1;
    }
    else
    {
        ShowScrollBars(SB_VERT, FALSE); 
        m_nVScrollMax = 0;
    }

    if (VisibleRect.Width() < VirtualRect.Width())
    {
        ShowScrollBars(SB_HORZ, TRUE); 
        m_nHScrollMax = VirtualRect.Width() - 1;
    }
    else
    {
        ShowScrollBars(SB_HORZ, FALSE); 
        m_nHScrollMax = 0;
    }

    /* Old code - CJM
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE;
    si.nPage = (m_nHScrollMax>0)? VisibleRect.Width() : 0;
    SetScrollInfo(SB_HORZ, &si, FALSE); 
    si.nPage = (m_nVScrollMax>0)? VisibleRect.Height() : 0;
    SetScrollInfo(SB_VERT, &si, FALSE);

    SetScrollRange(SB_VERT, 0, m_nVScrollMax, TRUE);
    SetScrollRange(SB_HORZ, 0, m_nHScrollMax, TRUE);
    */

    // New code - Paul Runstedler 
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = (m_nHScrollMax>0)? VisibleRect.Width() : 0;
    si.nMin = 0;
    si.nMax = m_nHScrollMax;
    SetScrollInfo(SB_HORZ, &si, TRUE);

    //si.fMask |= SIF_DISABLENOSCROLL;
    si.nPage = (m_nVScrollMax>0)? VisibleRect.Height() : 0;
    si.nMin = 0;
    si.nMax = m_nVScrollMax;
    SetScrollInfo(SB_VERT, &si, TRUE);
}

////////////////////////////////////////////////////////////////////////////////////
// Row/Column position functions

// returns the top left point of the cell. Returns FALSE if cell not visible.
BOOL CGridCtrl::GetCellOrigin(int nRow, int nCol, LPPOINT p)
{
	int i;

	if (!IsValid(nRow, nCol))
		return FALSE;

	CCellID idTopLeft;
	if (nCol >= m_nFixedCols || nRow >= m_nFixedRows)
		idTopLeft = GetTopleftNonFixedCell();

	if ((nRow >= m_nFixedRows && nRow < idTopLeft.row) ||
		(nCol>= m_nFixedCols && nCol < idTopLeft.col))
		return FALSE;

	p->x = 0;
	if (nCol < m_nFixedCols)                      // is a fixed column
	{
		for (i = 0; i < nCol; i++)
			p->x += GetColumnWidth(i);
	}
	else 
	{                                        // is a scrollable data column
		for (i = 0; i < m_nFixedCols; i++)
			p->x += GetColumnWidth(i);
		for (i = idTopLeft.col; i < nCol; i++)
			p->x += GetColumnWidth(i);
	}

	p->y = 0;
	if (nRow < m_nFixedRows)                      // is a fixed row
	{
		for (i = 0; i < nRow; i++)
			p->y += GetRowHeight(i);
	}
	else 
	{                                        // is a scrollable data row
		for (i = 0; i < m_nFixedRows; i++)
			p->y += GetRowHeight(i);
		for (i = idTopLeft.row; i < nRow; i++)
			p->y += GetRowHeight(i);
	}

	return TRUE;
}

BOOL CGridCtrl::GetCellRect(int nRow, int nCol, LPRECT pRect)
{
    CPoint CellOrigin;
    if (!GetCellOrigin(nRow, nCol, &CellOrigin))
        return FALSE;

    pRect->left   = CellOrigin.x;
    pRect->top    = CellOrigin.y;
    pRect->right  = CellOrigin.x + GetColumnWidth(nCol)-1;
    pRect->bottom = CellOrigin.y + GetRowHeight(nRow)-1;

    //TRACE("Row %d, col %d: L %d, T %d, W %d, H %d:  %d,%d - %d,%d\n",
    //      nRow,nCol, CellOrigin.x, CellOrigin.y, GetColumnWidth(nCol), GetRowHeight(nRow),
    //      pRect->left, pRect->top, pRect->right, pRect->bottom);

    return TRUE;
}

BOOL CGridCtrl::GetTextRect(int nRow, int nCol, LPRECT pRect)
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    if(pCell == nullptr)
        return FALSE;
    
    if(!GetCellRect(nRow, nCol, pRect))
        return FALSE;

    return pCell->GetTextRect(pRect);
}

// Returns the bounding box of a range of cells
BOOL CGridCtrl::GetCellRangeRect(const CCellRange& cellRange, LPRECT lpRect)
{
    CPoint MinOrigin,MaxOrigin;

    if (!GetCellOrigin(cellRange.GetMinRow(), cellRange.GetMinCol(), &MinOrigin))
        return FALSE;
    if (!GetCellOrigin(cellRange.GetMaxRow(), cellRange.GetMaxCol(), &MaxOrigin))
        return FALSE;

    lpRect->left   = MinOrigin.x;
    lpRect->top    = MinOrigin.y;
    lpRect->right  = MaxOrigin.x + GetColumnWidth(cellRange.GetMaxCol()) - 1;
    lpRect->bottom = MaxOrigin.y + GetRowHeight(cellRange.GetMaxRow()) - 1;

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////
// Grid attribute functions

void CGridCtrl::OnSetFont(CFont font, BOOL bRedraw)
{
    // Get the logical font
    LOGFONT lf;
    if (!GetObject(font, sizeof(LOGFONT), &lf))
        AtlThrowLastWin32();

    m_cellDefault.SetFont(&lf);
    m_cellFixedColDef.SetFont(&lf);
    m_cellFixedRowDef.SetFont(&lf);
    m_cellFixedRowColDef.SetFont(&lf);

    Refresh();
}

#ifndef _WIN32_WCE_NO_CURSOR
BOOL CGridCtrl::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
    if (nHitTest == HTCLIENT)
    {
        switch (m_MouseMode)
        {
        case MOUSE_OVER_COL_DIVIDE:
            SetCursor(AtlLoadSysCursor(IDC_SIZEWE));
            break;
        case MOUSE_OVER_ROW_DIVIDE:
            SetCursor(AtlLoadSysCursor(IDC_SIZENS));
            break;
#ifndef GRIDCONTROL_NO_DRAGDROP
        case MOUSE_DRAGGING:
            break;
#endif
        default:
            if (!GetVirtualMode())
            {
                CPoint pt(GetMessagePos());
                ScreenToClient(&pt);
                CCellID cell = GetCellFromPt(pt);
                if (IsValid(cell))
                {
                    CGridCellBase* pCell = GetCell(cell.row, cell.col);
                    if (pCell)
                        return pCell->OnSetCursor();
                }
            }

            SetCursor(AtlLoadSysCursor(IDC_ARROW));
        }
        return TRUE;
    }

    return FALSE;
}
#endif

////////////////////////////////////////////////////////////////////////////////////
// Row/Column count functions

BOOL CGridCtrl::SetFixedRowCount(int nFixedRows)
{
    if (m_nFixedRows == nFixedRows)
        return TRUE;

    ATLASSERT(nFixedRows >= 0);

    ResetSelectedRange();

    // Force recalculation
    m_idTopLeftCell.col = -1;

    if (nFixedRows > GetRowCount())
        if (!SetRowCount(nFixedRows))
            return FALSE;
        
        if (m_idCurrentCell.row < nFixedRows)
            SetFocusCell(-1, - 1);
        
        if (!GetVirtualMode())
        {
            if (nFixedRows > m_nFixedRows)
            {
                for (int i = m_nFixedRows; i < nFixedRows; i++)
                    for (int j = 0; j < GetColumnCount(); j++)
                    {
                        SetItemState(i, j, GetItemState(i, j) | GVIS_FIXED | GVIS_FIXEDROW);
                        SetItemBkColour(i, j, CLR_DEFAULT);
                        SetItemFgColour(i, j, CLR_DEFAULT);
                    }
            }
            else
            {
                for (int i = nFixedRows; i < m_nFixedRows; i++)
                {
                    int j;
                    for (j = 0; j < GetFixedColumnCount(); j++)
                        SetItemState(i, j, GetItemState(i, j) & ~GVIS_FIXEDROW);

                    for (j = GetFixedColumnCount(); j < GetColumnCount(); j++)
                    {
                        SetItemState(i, j, GetItemState(i, j) & ~(GVIS_FIXED | GVIS_FIXEDROW));
                        SetItemBkColour(i, j, CLR_DEFAULT);
                        SetItemFgColour(i, j, CLR_DEFAULT);
                    }
                }
            }
        }

        m_nFixedRows = nFixedRows;
        
        Refresh();
        
        return TRUE;
}

BOOL CGridCtrl::SetFixedColumnCount(int nFixedCols)
{
    if (m_nFixedCols == nFixedCols)
        return TRUE;

    ATLASSERT(nFixedCols >= 0);

    if (nFixedCols > GetColumnCount())
        if (!SetColumnCount(nFixedCols))
            return FALSE;

    if (m_idCurrentCell.col < nFixedCols)
        SetFocusCell(-1, - 1);

    ResetSelectedRange();

    // Force recalculation
    m_idTopLeftCell.col = -1;

    if (!GetVirtualMode())
    {
        if (nFixedCols > m_nFixedCols)
        {
            for (int i = 0; i < GetRowCount(); i++)
                for (int j = m_nFixedCols; j < nFixedCols; j++)
                {
                    SetItemState(i, j, GetItemState(i, j) | GVIS_FIXED | GVIS_FIXEDCOL);
                    SetItemBkColour(i, j, CLR_DEFAULT);
                    SetItemFgColour(i, j, CLR_DEFAULT);
                }
        }
        else
        {
			{ // Scope limit i,j
	            for (int i = 0; i < GetFixedRowCount(); i++)
		            for (int j = nFixedCols; j < m_nFixedCols; j++)
			            SetItemState(i, j, GetItemState(i, j) & ~GVIS_FIXEDCOL);
			}
			{// Scope limit i,j
	            for (int i = GetFixedRowCount(); i < GetRowCount(); i++)
		            for (int j = nFixedCols; j < m_nFixedCols; j++)
			        {
				        SetItemState(i, j, GetItemState(i, j) & ~(GVIS_FIXED | GVIS_FIXEDCOL));
					    SetItemBkColour(i, j, CLR_DEFAULT);
						SetItemFgColour(i, j, CLR_DEFAULT);
	                }
			}
        }
    }
        
    m_nFixedCols = nFixedCols;
        
    Refresh();
        
    return TRUE;
}

BOOL CGridCtrl::SetRowCount(int nRows)
{
    BOOL bResult = TRUE;

    ATLASSERT(nRows >= 0);
    if (nRows == GetRowCount())
        return bResult;

    // Force recalculation
    m_idTopLeftCell.col = -1;

    if (nRows < m_nFixedRows)
        m_nFixedRows = nRows;

    if (m_idCurrentCell.row >= nRows)
        SetFocusCell(-1, - 1);

    int addedRows = nRows - GetRowCount();

    // If we are about to lose rows, then we need to delete the GridCell objects
    // in each column within each row
    if (addedRows < 0)
    {
        if (!GetVirtualMode())
        {
            for (int row = nRows; row < m_nRows; row++)
            {
                // Delete cells
                for (int col = 0; col < m_nCols; col++)
                    DestroyCell(row, col);
            
                // Delete rows
                GRID_ROW* pRow = m_RowData[row];
                if (pRow)
                    delete pRow;
            }
        }
        m_nRows = nRows;
    }
    
	m_arRowHeights.SetCount(nRows);

	if (GetVirtualMode())
	{
		m_nRows = nRows;
		if (addedRows > 0)
		{
			int startRow = nRows - addedRows;
			for (int row = startRow; row < nRows; row++)
				m_arRowHeights[row] = m_cellDefault.GetHeight();
		}
		ResetVirtualOrder();
	}
	else
	{
		// Change the number of rows.
		m_RowData.SetCount(nRows);

		// If we have just added rows, we need to construct new elements for each cell
		// and set the default row height
		if (addedRows > 0)
		{
			// initialize row heights and data
			int startRow = nRows - addedRows;
			for (int row = startRow; row < nRows; row++)
			{
				m_arRowHeights[row] = m_cellDefault.GetHeight();

				m_RowData[row] = new GRID_ROW;
				m_RowData[row]->SetCount(m_nCols);
				for (int col = 0; col < m_nCols; col++)
				{
					GRID_ROW* pRow = m_RowData[row];
					if (pRow && !GetVirtualMode())
						pRow->SetAt(col, CreateCell(row, col));
				}
				m_nRows++;
			}
		}
	}

    SetModified();
    ResetScrollBars();
    Refresh();

    return bResult;
}

BOOL CGridCtrl::SetColumnCount(int nCols)
{
    BOOL bResult = TRUE;

    ATLASSERT(nCols >= 0);

    if (nCols == GetColumnCount())
        return bResult;

    // Force recalculation
    m_idTopLeftCell.col = -1;

    if (nCols < m_nFixedCols)
        m_nFixedCols = nCols;

    if (m_idCurrentCell.col >= nCols)
        SetFocusCell(-1, - 1);

    int addedCols = nCols - GetColumnCount();

    // If we are about to lose columns, then we need to delete the GridCell objects
    // within each column
    if (addedCols < 0 && !GetVirtualMode())
    {
        for (int row = 0; row < m_nRows; row++)
            for (int col = nCols; col < GetColumnCount(); col++)
                DestroyCell(row, col);
    }

	// Change the number of columns.
	m_arColWidths.SetCount(nCols);

	// Change the number of columns in each row.
	if (!GetVirtualMode())
		for (int i = 0; i < m_nRows; i++)
			if (m_RowData[i])
				m_RowData[i]->SetCount(nCols);

	// If we have just added columns, we need to construct new elements for each cell
	// and set the default column width
	if (addedCols > 0)
	{
		int row, col;

		// initialized column widths
		int startCol = nCols - addedCols;
		for (col = startCol; col < nCols; col++)
			m_arColWidths[col] = m_cellFixedColDef.GetWidth();

		// initialize column data
		if (!GetVirtualMode())
		{
			for (row = 0; row < m_nRows; row++)
				for (col = startCol; col < nCols; col++)
				{
					GRID_ROW* pRow = m_RowData[row];
					if (pRow)
						pRow->SetAt(col, CreateCell(row, col));
				}
		}
	}
	// else    // check for selected cell ranges
	//    ResetSelectedRange();
	m_arColOrder.resize(nCols);  // Reset Column Order
    for (int i = 0; i < nCols; i++)
	{
		m_arColOrder[i] = i;	
	}

    m_nCols = nCols;

    SetModified();
    ResetScrollBars();
    Refresh();

    return bResult;
}

// Insert a column at a given position, or add to end of columns (if nColumn = -1)
int CGridCtrl::InsertColumn(LPCTSTR strHeading,
                            UINT nFormat /* = DT_CENTER|DT_VCENTER|DT_SINGLELINE */,
                            int nColumn  /* = -1 */)
{
	ATLASSERT(!m_AllowReorderColumn); // function not implemented in case of m_AllowReorderColumn option
    if (nColumn >= 0 && nColumn < m_nFixedCols)
    {

        // TODO: Fix it so column insertion works for in the fixed column area
        ATLASSERT(FALSE);
        return -1;
    }

    // If the insertion is for a specific column, check it's within range.
    if (nColumn >= 0 && nColumn > GetColumnCount())
        return -1;

    // Force recalculation
    m_idTopLeftCell.col = -1;

    ResetSelectedRange();

    // Gotta be able to at least _see_ some of the column.
    if (m_nRows < 1)
        SetRowCount(1);

    // Allow the user to insert after the last of the columns, but process it as a
    // "-1" column, meaning it gets flagged as being the last column, and not a regular
    // "insert" routine.
    if (nColumn == GetColumnCount())
        nColumn = -1;

	if (nColumn < 0)
	{
		nColumn = m_nCols;
		m_arColWidths.Add(0);
		m_arColOrder.insert(m_arColOrder.end(), nColumn);
		if (!GetVirtualMode())
		{
			for (int row = 0; row < m_nRows; row++)
			{
				GRID_ROW* pRow = m_RowData[row];
				if (!pRow)
					return -1;
				pRow->Add(CreateCell(row, nColumn));
			}
		}
	} 
	else
	{
		m_arColWidths.InsertAt(nColumn, (UINT)0);
		m_arColOrder.insert(m_arColOrder.begin() + nColumn, nColumn);
		if (!GetVirtualMode())
		{
			for (int row = 0; row < m_nRows; row++) 
			{
				GRID_ROW* pRow = m_RowData[row];
				if (!pRow)
					return -1;
				pRow->InsertAt(nColumn, CreateCell(row, nColumn));
			}
		}
	}
    m_nCols++;
    
    // Initialise column data
    SetItemText(0, nColumn, strHeading);
    for (int row = 0; row < m_nRows; row++) 
        SetItemFormat(row, nColumn, nFormat);
    
    // initialized column width
    m_arColWidths[nColumn] = GetTextExtent(0, nColumn, strHeading).cx;
    
    if (m_idCurrentCell.col != -1 && nColumn < m_idCurrentCell.col)
        m_idCurrentCell.col++;
    
    ResetScrollBars();

    SetModified();
    
    return nColumn;
}

// Insert a row at a given position, or add to end of rows (if nRow = -1)
int CGridCtrl::InsertRow(LPCTSTR strHeading, int nRow /* = -1 */)
{
    if (nRow >= 0 && nRow < m_nFixedRows)
    {
        // TODO: Fix it so column insertion works for in the fixed row area
        ATLASSERT(FALSE);
        return -1;
    }

    // If the insertion is for a specific row, check it's within range.
    if (nRow >= 0 && nRow >= GetRowCount())
        return -1;

    // Force recalculation
    m_idTopLeftCell.col = -1;

    ResetSelectedRange();

    // Gotta be able to at least _see_ some of the row.
    if (m_nCols < 1)
        SetColumnCount(1);

	// Adding a row to the bottom
	if (nRow < 0)
	{
		nRow = m_nRows;
		m_arRowHeights.Add(0);
		if (!GetVirtualMode())
			m_RowData.Add(new GRID_ROW);
		else
			m_arRowOrder.push_back(m_nRows);

	}
	else
	{
		m_arRowHeights.InsertAt(nRow, (UINT)0);
		if (!GetVirtualMode())
			m_RowData.InsertAt(nRow, new GRID_ROW);
		else
		{
			ResetVirtualOrder();
		}
	}

	if (!GetVirtualMode())
		m_RowData[nRow]->SetCount(m_nCols);
    m_nRows++;

    // Initialise cell data
    if (!GetVirtualMode())
    {
        for (int col = 0; col < m_nCols; col++)
        {
            GRID_ROW* pRow = m_RowData[nRow];
            if (!pRow)
                return -1;
            pRow->SetAt(col, CreateCell(nRow, col));
        }
    }

    // Set row title
    SetItemText(nRow, 0, strHeading);

    // initialized row height
    if (strHeading && strHeading[0])
        m_arRowHeights[nRow] = GetTextExtent(nRow, 0, strHeading).cy;
    else
        m_arRowHeights[nRow] = m_cellFixedRowDef.GetHeight();

    if (m_idCurrentCell.row != -1 && nRow < m_idCurrentCell.row)
        m_idCurrentCell.row++;

    ResetScrollBars();

    SetModified();

    return nRow;
}

///////////////////////////////////////////////////////////////////////////////
// Cell creation stuff

BOOL CGridCtrl::SetCellType(int nRow, int nCol, CGridCellBase* pRuntimeClass)
{
    if (GetVirtualMode())
        return FALSE;

    ATLASSERT(IsValid(nRow, nCol));
    if (!IsValid(nRow, nCol))
        return FALSE;

    CGridCellBase* pNewCell = pRuntimeClass->CreateObject();

    CGridCellBase* pCurrCell = GetCell(nRow, nCol);
    if (pCurrCell)
        *pNewCell = *pCurrCell;

    SetCell(nRow, nCol, pNewCell);
    delete pCurrCell;

    return TRUE;
}

BOOL CGridCtrl::SetDefaultCellType(CGridCellBase* pRuntimeClass)
{
    ATLASSERT(pRuntimeClass != nullptr);
    m_pRtcDefault = pRuntimeClass;
    return TRUE;
}

// Creates a new grid cell and performs any necessary initialization
CGridCellBase* CGridCtrl::CreateCell(int nRow, int nCol)
{
	ATLASSERT(!GetVirtualMode());

    CGridCellBase* pCell = m_pRtcDefault->CreateObject();
    if (!pCell)
        return nullptr;

    pCell->SetGrid(this);
    pCell->SetCoords(nRow, nCol); 

    if (nCol < m_nFixedCols)
        pCell->SetState(pCell->GetState() | GVIS_FIXED | GVIS_FIXEDCOL);
    if (nRow < m_nFixedRows)
        pCell->SetState(pCell->GetState() | GVIS_FIXED | GVIS_FIXEDROW);
    
    pCell->SetFormat(pCell->GetDefaultCell()->GetFormat());

    return pCell;
}

// Performs any cell cleanup necessary to maintain grid integrity
/*virtual*/ void CGridCtrl::DestroyCell(int nRow, int nCol)
{
    // Should NEVER get here in virtual mode.
    ATLASSERT(!GetVirtualMode());

    // Set the cells state to 0. If the cell is selected, this
    // will remove the cell from the selected list.
    SetItemState(nRow, nCol, 0);

    delete GetCell(nRow, nCol);
}

BOOL CGridCtrl::DeleteColumn(int nColumn)
{
    if (nColumn < 0 || nColumn >= GetColumnCount())
        return FALSE;

    ResetSelectedRange();

    if (!GetVirtualMode())
    {
        for (int row = 0; row < GetRowCount(); row++)
        {
            GRID_ROW* pRow = m_RowData[row];
            if (!pRow)
                return FALSE;

            DestroyCell(row, nColumn);
        
            //pRow->RemoveAt(nColumn);
			pRow->RemoveAt(m_arColOrder[nColumn]);
        }
    }
    //m_arColWidths.RemoveAt(nColumn);
	m_arColWidths.RemoveAt(m_arColOrder[nColumn]);
	int DEL = m_arColOrder[nColumn];
	m_arColOrder.erase(m_arColOrder.begin() + nColumn);
    m_nCols--;
	for(int i = 0; i < m_nCols; i++)
	{
		if(m_arColOrder[i] > DEL)
			m_arColOrder[i] = m_arColOrder[i]-1;
	}
    if (nColumn < m_nFixedCols)
        m_nFixedCols--;
    
    if (nColumn == m_idCurrentCell.col)
        m_idCurrentCell.row = m_idCurrentCell.col = -1;
    else if (nColumn < m_idCurrentCell.col)
        m_idCurrentCell.col--;
    
    ResetScrollBars();

    SetModified();

    return TRUE;
}


void CGridCtrl::AddSubVirtualRow(int Num, int Nb)
{
	ATLASSERT(Nb == -1 || Nb ==1); // only these values are implemented now
	if(!GetVirtualMode()) return;
	for(int ind = 0; ind<m_nRows ; ind++)
		if(m_arRowOrder[ind]>Num) m_arRowOrder[ind]+=Nb;
	if(Nb>0)
		m_arRowOrder.insert(m_arRowOrder.begin()+Num,Num);
	else
		m_arRowOrder.erase(m_arRowOrder.begin()+Num);
}


BOOL CGridCtrl::DeleteRow(int nRow)
{
    if (nRow < 0 || nRow >= GetRowCount())
        return FALSE;

    ResetSelectedRange();

    if (!GetVirtualMode())
    {
        GRID_ROW* pRow = m_RowData[nRow];
        if (!pRow)
            return FALSE;

        for (int col = 0; col < GetColumnCount(); col++)
            DestroyCell(nRow, col);

        delete pRow;
        m_RowData.RemoveAt(nRow);
    }
    else
    	AddSubVirtualRow(nRow,-1);

    m_arRowHeights.RemoveAt(nRow);

    m_nRows--;
    if (nRow < m_nFixedRows)
        m_nFixedRows--;
    
    if (nRow == m_idCurrentCell.row)
        m_idCurrentCell.row = m_idCurrentCell.col = -1;
    else if (nRow < m_idCurrentCell.row)
        m_idCurrentCell.row--;
    
    ResetScrollBars();

    SetModified();
    
    return TRUE;
}

// Handy function that removes all non-fixed rows
BOOL CGridCtrl::DeleteNonFixedRows()
{
    ResetSelectedRange();
    int nFixed = GetFixedRowCount();
    int nCount = GetRowCount();
    if (GetVirtualMode())
	{
		if(nCount != nFixed)
		{
			SetRowCount(nFixed);
			m_arRowOrder.resize(nFixed);
			m_arRowHeights.SetCount(nFixed);
			m_idCurrentCell.row = m_idCurrentCell.col = -1;
			ResetScrollBars();
			SetModified();
		}
	}
	else
	{
    // Delete all data rows
    for (int nRow = nCount; nRow >= nFixed; nRow--)
        DeleteRow(nRow);
	}
    return TRUE;
}

// Removes all rows, columns and data from the grid.
BOOL CGridCtrl::DeleteAllItems()
{
    ResetSelectedRange();

    m_arColWidths.RemoveAll();
    m_arRowHeights.RemoveAll();

    // Delete all cells in the grid
    if (!GetVirtualMode())
    {
        for (int row = 0; row < m_nRows; row++)
        {
            for (int col = 0; col < m_nCols; col++)
                DestroyCell(row, col);

            GRID_ROW* pRow = m_RowData[row];
            delete pRow;
        }

        // Remove all rows
        m_RowData.RemoveAll();
    }
	else
		m_arRowOrder.clear();


    m_idCurrentCell.row = m_idCurrentCell.col = -1;
    m_nRows = m_nFixedRows = m_nCols = m_nFixedCols = 0;

    ResetScrollBars();

    SetModified();

    return TRUE;
}

void CGridCtrl::AutoFill()
{
    if (!IsWindow())
        return;

    CRect rect;
    GetClientRect(rect);

    SetColumnCount(rect.Width() / m_cellDefault.GetWidth() + 1);
    SetRowCount(rect.Height() / m_cellDefault.GetHeight() + 1);
    SetFixedRowCount(1);
    SetFixedColumnCount(1);
    ExpandToFit();
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl data functions

// Set CListCtrl::GetNextItem for details
CCellID CGridCtrl::GetNextItem(CCellID& cell, int nFlags) const
{
    if ((nFlags & GVNI_ALL) == GVNI_ALL)
    {    // GVNI_ALL Search whole Grid beginning from cell
        //          First row (cell.row) -- ONLY Columns to the right of cell
        //          following rows       -- ALL  Columns
        int row = cell.row , col = cell.col + 1;
        if (row <= 0)
            row = GetFixedRowCount();
        for (; row < GetRowCount(); row++)
        {
            if (col <= 0)
                col = GetFixedColumnCount();
            for (; col < GetColumnCount(); col++)
            {
                int nState = GetItemState(row, col);
                if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                    (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                    (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                    (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                    (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                    (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                    return CCellID(row, col);
            }
            // go to First Column
            col = GetFixedColumnCount();
        }
    }
    else if ((nFlags & GVNI_BELOW) == GVNI_BELOW && 
             (nFlags & GVNI_TORIGHT) == GVNI_TORIGHT)
    {   // GVNI_AREA Search Grid beginning from cell to Lower-Right of Grid
        //           Only rows starting with  cell.row and below
        //           All rows   -- ONLY Columns to the right of cell
        int row = cell.row;
        if (row <= 0)
            row = GetFixedRowCount();
        for (; row < GetRowCount(); row++)
        {
            int col = cell.col + 1;
            if (col <= 0)
                col = GetFixedColumnCount();
            for (; col < GetColumnCount(); col++) 
            {
                int nState = GetItemState(row, col);
                if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                    (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                    (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                    (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                    (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                    (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                    return CCellID(row, col);
            }
        }
    }
    else if ((nFlags & GVNI_ABOVE) == GVNI_ABOVE) 
    {
        for (int row = cell.row - 1; row >= GetFixedRowCount(); row--) 
        {
            int nState = GetItemState(row, cell.col);
            if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                return CCellID(row, cell.col);
        }
    }
    else if ((nFlags & GVNI_BELOW) == GVNI_BELOW)
    {
        for (int row = cell.row + 1; row < GetRowCount(); row++) 
        {
            int nState = GetItemState(row, cell.col);
            if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                return CCellID(row, cell.col);
        }
    } 
    else if ((nFlags & GVNI_TOLEFT) == GVNI_TOLEFT)
    {
        for (int col = cell.col - 1; col >= GetFixedColumnCount(); col--) 
        {
            int nState = GetItemState(cell.row, col);
            if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                return CCellID(cell.row, col);
        }
    }
    else if ((nFlags & GVNI_TORIGHT) == GVNI_TORIGHT)
    {
        for (int col = cell.col + 1; col < GetColumnCount(); col++) 
        {
            int nState = GetItemState(cell.row, col);
            if ((nFlags & GVNI_DROPHILITED && nState & GVIS_DROPHILITED) || 
                (nFlags & GVNI_FOCUSED     && nState & GVIS_FOCUSED)     ||
                (nFlags & GVNI_SELECTED    && nState & GVIS_SELECTED)    ||
                (nFlags & GVNI_READONLY    && nState & GVIS_READONLY)    ||
                (nFlags & GVNI_FIXED       && nState & GVIS_FIXED)       ||
                (nFlags & GVNI_MODIFIED    && nState & GVIS_MODIFIED))
                return CCellID(cell.row, col);
        }
    }
    
    return CCellID(-1, -1);
}

// Sorts on a given column using the cell text
BOOL CGridCtrl::SortTextItems(int nCol, BOOL bAscending, LPARAM data /* = 0 */)
{
    return CGridCtrl::SortItems(pfnCellTextCompare, nCol, bAscending, data);
}

void CGridCtrl::SetCompareFunction(PFNLVCOMPARE pfnCompare)
{
	m_pfnCompare = pfnCompare;
}

// Sorts on a given column using the cell text and using the specified comparison
// function
BOOL CGridCtrl::SortItems(int nCol, BOOL bAscending, LPARAM data /* = 0 */)
{
    SetSortColumn(nCol);
    SetSortAscending(bAscending);
    ResetSelectedRange();
    SetFocusCell(-1, - 1);

	if (m_pfnCompare == nullptr)
		return CGridCtrl::SortItems(pfnCellTextCompare, nCol, bAscending, data);
	else
	    return CGridCtrl::SortItems(m_pfnCompare, nCol, bAscending, data);
}

// Sorts on a given column using the supplied compare function (see CListCtrl::SortItems)
BOOL CGridCtrl::SortItems(PFNLVCOMPARE pfnCompare, int nCol, BOOL bAscending,
                          LPARAM data /* = 0 */)
{
    SetSortColumn(nCol);
    SetSortAscending(bAscending);
    ResetSelectedRange();
    SetFocusCell(-1, -1);
    return SortItems(pfnCompare, nCol, bAscending, data, GetFixedRowCount(), -1);
}

int CALLBACK CGridCtrl::pfnCellTextCompare(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

	return _tcscmp(pCell1->GetText(), pCell2->GetText());
}

int CALLBACK CGridCtrl::pfnCellNumericCompare(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	CGridCellBase* pCell1 = (CGridCellBase*) lParam1;
	CGridCellBase* pCell2 = (CGridCellBase*) lParam2;
	if (!pCell1 || !pCell2) return 0;

	int nValue1 = _ttol(pCell1->GetText());
	int nValue2 = _ttol(pCell2->GetText());

	if (nValue1 < nValue2)
		return -1;
	else if (nValue1 == nValue2)
		return 0;
	else
		return 1;
}

CGridCtrl *CGridCtrl::m_This;
// private recursive sort implementation
bool CGridCtrl::NotVirtualCompare(int c1, int c2)
{
	return ! CGridCtrl::m_This->m_pfnVirtualCompare(c1, c2);
}
BOOL CGridCtrl::SortItems(PFNLVCOMPARE pfnCompare, int nCol, BOOL bAscending, LPARAM data,
                          int low, int high)
{
    if (nCol >= GetColumnCount())
        return FALSE;

    if (high == -1)
        high = GetRowCount() - 1;

    int lo = low;
    int hi = high;
    
    if (hi <= lo)
        return FALSE;

    if (GetVirtualMode())
	{
		ATLASSERT(m_pfnVirtualCompare);
		m_CurCol = m_arColOrder[nCol];
		m_This = this;
		std::stable_sort(m_arRowOrder.begin() + m_nFixedRows, m_arRowOrder.end(), 
                         bAscending ? m_pfnVirtualCompare : NotVirtualCompare);
		return TRUE;
	}
   
    
    //LPARAM midItem = GetItemData((lo + hi)/2, nCol);
	LPARAM pMidCell = (LPARAM) GetCell((lo + hi)/2, nCol);
    
    // loop through the list until indices cross
    while (lo <= hi)
    {
        // Find the first element that is greater than or equal to the partition 
        // element starting from the left Index.
        if (bAscending)
            while (lo < high  && pfnCompare((LPARAM)GetCell(lo, nCol), (LPARAM) pMidCell, data) < 0)
                ++lo;
        else
            while (lo < high && pfnCompare((LPARAM)GetCell(lo, nCol), pMidCell, data) > 0)
				++lo;
                
		// Find an element that is smaller than or equal to  the partition 
		// element starting from the right Index.
		if (bAscending)
			while (hi > low && pfnCompare((LPARAM)GetCell(hi, nCol), pMidCell, data) > 0)
				--hi;
		else
			while (hi > low && pfnCompare((LPARAM)GetCell(hi, nCol), pMidCell, data) < 0)
				--hi;
                        
        // If the indexes have not crossed, swap if the items are not equal
        if (lo <= hi)
        {
			// swap only if the items are not equal
            if (pfnCompare((LPARAM)GetCell(lo, nCol), (LPARAM)GetCell(hi, nCol), data) != 0)
            {
				for (int col = 0; col < GetColumnCount(); col++)
                {
					CGridCellBase *pCell = GetCell(lo, col);
                    SetCell(lo, col, GetCell(hi, col));
                    SetCell(hi, col, pCell);
                }
                UINT nRowHeight = m_arRowHeights[lo];
                m_arRowHeights[lo] = m_arRowHeights[hi];
                m_arRowHeights[hi] = nRowHeight;
            }
                            
            ++lo;
            --hi;
         }
    }
    
    // If the right index has not reached the left side of array
    // must now sort the left partition.
    if (low < hi)
        SortItems(pfnCompare, nCol, bAscending, data, low, hi);
    
    // If the left index has not reached the right side of array
    // must now sort the right partition.
    if (lo < high)
        SortItems(pfnCompare, nCol, bAscending, data, lo, high);
    
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl data functions

BOOL CGridCtrl::SetItem(const GV_ITEM* pItem)
{
    if (!pItem || GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(pItem->row, pItem->col);
    if (!pCell)
        return FALSE;

    SetModified(TRUE, pItem->row, pItem->col);

    if (pItem->mask & GVIF_TEXT)
        pCell->SetText(pItem->strText);
    if (pItem->mask & GVIF_PARAM)
        pCell->SetData(pItem->lParam);
    if (pItem->mask & GVIF_IMAGE)
        pCell->SetImage(pItem->iImage);
    if (pItem->mask & GVIF_STATE)
        SetItemState(pItem->row, pItem->col, pItem->nState);
    if (pItem->mask & GVIF_FORMAT)
        pCell->SetFormat(pItem->nFormat);
    if (pItem->mask & GVIF_BKCLR)
        pCell->SetBackClr(pItem->crBkClr);
    if (pItem->mask & GVIF_FGCLR)
        pCell->SetTextClr(pItem->crFgClr);
    if (pItem->mask & GVIF_FONT)
        pCell->SetFont(&(pItem->lfFont));
    if (pItem->mask & GVIF_MARGIN)
        pCell->SetMargin(pItem->nMargin);
	if (pItem->mask & GVIF_CTYPE)
		pCell->SetCType(pItem->ctype);
    
    return TRUE;
}

BOOL CGridCtrl::GetItem(GV_ITEM* pItem)
{
    if (!pItem)
        return FALSE;
    CGridCellBase* pCell = GetCell(pItem->row, pItem->col);
    if (!pCell)
        return FALSE;

    if (pItem->mask & GVIF_TEXT)
        pItem->strText = GetItemText(pItem->row, pItem->col);
    if (pItem->mask & GVIF_PARAM)
        pItem->lParam  = pCell->GetData();;
    if (pItem->mask & GVIF_IMAGE)
        pItem->iImage  = pCell->GetImage();
    if (pItem->mask & GVIF_STATE)
        pItem->nState  = pCell->GetState();
    if (pItem->mask & GVIF_FORMAT)
        pItem->nFormat = pCell->GetFormat();
    if (pItem->mask & GVIF_BKCLR)
        pItem->crBkClr = pCell->GetBackClr();
    if (pItem->mask & GVIF_FGCLR)
        pItem->crFgClr = pCell->GetTextClr();
    if (pItem->mask & GVIF_FONT)
        memcpy(&(pItem->lfFont), pCell->GetFont(), sizeof(LOGFONT));
    if(pItem->mask & GVIF_MARGIN)
        pItem->nMargin = pCell->GetMargin();
	if (pItem->mask & GVIF_CTYPE)
		pItem->ctype   = pCell->GetCType();

    return TRUE;
}

BOOL CGridCtrl::SetItemText(int nRow, int nCol, LPCTSTR str)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    pCell->SetText(str);

    SetModified(TRUE, nRow, nCol);
    return TRUE;
}

#if !defined(_WIN32_WCE) || (_WIN32_WCE >= 210)
// EFW - 06/13/99 - Added to support printf-style formatting codes
BOOL CGridCtrl::SetItemTextFmt(int nRow, int nCol, LPCTSTR szFmt, ...)
{
    if (GetVirtualMode())
        return FALSE;

    CString strText;

    va_list argptr;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    // Format the message text
    va_start(argptr, szFmt);
    strText.FormatV(szFmt, argptr);
    va_end(argptr);

    pCell->SetText(strText);

    SetModified(TRUE, nRow, nCol);
    return TRUE;
}

// EFW - 06/13/99 - Added to support string resource ID.  Supports
// a variable argument list too.
BOOL CGridCtrl::SetItemTextFmtID(int nRow, int nCol, UINT nID, ...)
{
    if (GetVirtualMode())
        return FALSE;

    CString strFmt, strText;
    va_list argptr;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    // Format the message text
    va_start(argptr, nID);
    ATLVERIFY(strFmt.LoadString(nID));
    strText.FormatV(strFmt, argptr);
    va_end(argptr);

    pCell->SetText(strText);

    SetModified(TRUE, nRow, nCol);
    return TRUE;
}
#endif

BOOL CGridCtrl::SetItemData(int nRow, int nCol, LPARAM lParam)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    pCell->SetData(lParam);
    SetModified(TRUE, nRow, nCol);
    return TRUE;
}

LPARAM CGridCtrl::GetItemData(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return (LPARAM) 0;

    return pCell->GetData();
}

BOOL CGridCtrl::SetItemImage(int nRow, int nCol, int iImage)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return FALSE;

    pCell->SetImage(iImage);
    SetModified(TRUE, nRow, nCol);
    return TRUE;
}

int CGridCtrl::GetItemImage(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return -1;

    return pCell->GetImage();
}

BOOL CGridCtrl::SetItemState(int nRow, int nCol, UINT state)
{
    BOOL bSelected = IsCellSelected(nRow, nCol);

    // If the cell is being unselected, remove it from the selected list
    if (bSelected && !(state & GVIS_SELECTED))
    {
        CCellID cell;
        DWORD key = MAKELONG(nRow, nCol);

        if (m_SelectedCellMap.Lookup(key, (CCellID&)cell))
            m_SelectedCellMap.RemoveKey(key);
    }

    // If cell is being selected, add it to the list of selected cells
    else if (!bSelected && (state & GVIS_SELECTED))
    {
        CCellID cell(nRow, nCol);
        m_SelectedCellMap.SetAt(MAKELONG(nRow, nCol), cell);
    }

    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;

    // Set the cell's state
    pCell->SetState(state);

    return TRUE;
}

UINT CGridCtrl::GetItemState(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return 0;

    return pCell->GetState();
}

BOOL CGridCtrl::SetItemFormat(int nRow, int nCol, UINT nFormat)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;

    pCell->SetFormat(nFormat);
    return TRUE;
}

UINT CGridCtrl::GetItemFormat(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return 0;

    return pCell->GetFormat();
}

BOOL CGridCtrl::SetItemBkColour(int nRow, int nCol, COLORREF cr /* = CLR_DEFAULT */)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;

    pCell->SetBackClr(cr);
    return TRUE;
}

COLORREF CGridCtrl::GetItemBkColour(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return 0;

    return pCell->GetBackClr();
}

BOOL CGridCtrl::SetItemFgColour(int nRow, int nCol, COLORREF cr /* = CLR_DEFAULT */)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;
    
    pCell->SetTextClr(cr);
    return TRUE;
}

COLORREF CGridCtrl::GetItemFgColour(int nRow, int nCol) const
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return 0;
    
    return pCell->GetTextClr();
}

BOOL CGridCtrl::SetItemFont(int nRow, int nCol, const LOGFONT* plf)
{
    if (GetVirtualMode())
        return FALSE;

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;
    
    pCell->SetFont(plf);
    
    return TRUE;
}

const LOGFONT* CGridCtrl::GetItemFont(int nRow, int nCol)
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell) 
        return GetDefaultCell(nRow < GetFixedRowCount(), nCol < GetFixedColumnCount())->GetFont();
    
    return pCell->GetFont();
}

BOOL CGridCtrl::IsItemEditing(int nRow, int nCol)
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;

    return pCell->IsEditing();
}

////////////////////////////////////////////////////////////////////////////////////
// Row/Column size functions

long CGridCtrl::GetVirtualWidth() const
{
    long lVirtualWidth = 0;
    int iColCount = GetColumnCount();
    for (int i = 0; i < iColCount; i++)
        lVirtualWidth += m_arColWidths[i];

    return lVirtualWidth;
}

long CGridCtrl::GetVirtualHeight() const
{
    long lVirtualHeight = 0;
    int iRowCount = GetRowCount();
    for (int i = 0; i < iRowCount; i++)
        lVirtualHeight += m_arRowHeights[i];

    return lVirtualHeight;
}

int CGridCtrl::GetRowHeight(int nRow) const
{
    ATLASSERT(nRow >= 0 && nRow < m_nRows);
    if (nRow < 0 || nRow >= m_nRows)
        return -1;

    return m_arRowHeights[nRow];
}

int CGridCtrl::GetColumnWidth(int nCol) const
{
    ATLASSERT(nCol >= 0 && nCol < m_nCols);
    if (nCol < 0 || nCol >= m_nCols)
        return -1;

    return m_arColWidths[m_arColOrder[nCol]];
}

BOOL CGridCtrl::SetRowHeight(int nRow, int height)
{
    ATLASSERT(nRow >= 0 && nRow < m_nRows && height >= 0);
    if (nRow < 0 || nRow >= m_nRows || height < 0)
        return FALSE;

    m_arRowHeights[nRow] = height;
    ResetScrollBars();

    return TRUE;
}

BOOL CGridCtrl::SetColumnWidth(int nCol, int width)
{
    ATLASSERT(nCol >= 0 && nCol < m_nCols && width >= 0);
    if (nCol < 0 || nCol >= m_nCols || width < 0)
        return FALSE;

    m_arColWidths[m_arColOrder[nCol]] = width;
    ResetScrollBars();

    return TRUE;
}

int CGridCtrl::GetFixedRowHeight() const
{
    int nHeight = 0;
    for (int i = 0; i < m_nFixedRows; i++)
        nHeight += GetRowHeight(i);

    return nHeight;
}

int CGridCtrl::GetFixedColumnWidth() const
{
    int nWidth = 0;
    for (int i = 0; i < m_nFixedCols; i++)
        nWidth += GetColumnWidth(i);

    return nWidth;
}

BOOL CGridCtrl::AutoSizeColumn(int nCol, UINT nAutoSizeStyle /*=GVS_DEFAULT*/, 
                               BOOL bResetScroll /*=TRUE*/)
{
    ATLASSERT(nCol >= 0 && nCol < m_nCols);
    if (nCol < 0 || nCol >= m_nCols)
        return FALSE;

    //  Skip hidden columns when autosizing
    if(GetColumnWidth(nCol) <=0)
        return FALSE;

    CSize size;
    CDCHandle DC = GetDC();
    if (DC.IsNull())
        return FALSE;

    int nWidth = 0;

    ATLASSERT(GVS_DEFAULT <= nAutoSizeStyle && nAutoSizeStyle <= GVS_BOTH);
    if (nAutoSizeStyle == GVS_DEFAULT)
        nAutoSizeStyle = GetAutoSizeStyle();

    int nStartRow = (nAutoSizeStyle & GVS_HEADER)? 0 : GetFixedRowCount();
    int nEndRow   = (nAutoSizeStyle & GVS_DATA)? GetRowCount()-1 : GetFixedRowCount()-1;

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(nStartRow, nCol, nEndRow, nCol));

    for (int nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        CGridCellBase* pCell = GetCell(nRow, nCol);
        if (pCell)
            size = pCell->GetCellExtent(DC);
        if (size.cx > nWidth)
            nWidth = size.cx;
    }

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

    m_arColWidths[nCol] = nWidth;

    ReleaseDC(DC);
    if (bResetScroll)
        ResetScrollBars();

    return TRUE;
}

BOOL CGridCtrl::AutoSizeRow(int nRow, BOOL bResetScroll /*=TRUE*/)
{
    ATLASSERT(nRow >= 0 && nRow < m_nRows);
    if (nRow < 0 || nRow >= m_nRows)
        return FALSE;

    //  Skip hidden rows when autosizing
    if(GetRowHeight(nRow) <=0)
        return FALSE;

    CSize size;
    CDCHandle DC = GetDC();
    if (DC.IsNull())
        return FALSE;

    int nHeight = 0;
    int nNumColumns = GetColumnCount();

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(nRow, 0, nRow, nNumColumns));

    for (int nCol = 0; nCol < nNumColumns; nCol++)
    {
        CGridCellBase* pCell = GetCell(nRow, nCol);
        if (pCell)
            size = pCell->GetCellExtent(DC);
        if (size.cy > nHeight)
            nHeight = size.cy;
    }
    m_arRowHeights[nRow] = nHeight;

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

    ReleaseDC(DC);
    if (bResetScroll)
        ResetScrollBars();

    return TRUE;
}

void CGridCtrl::AutoSizeColumns(UINT nAutoSizeStyle /*=GVS_DEFAULT*/)
{
    int nNumColumns = GetColumnCount();
    for (int nCol = 0; nCol < nNumColumns; nCol++)
    {
        //  Skip hidden columns when autosizing
        if(GetColumnWidth(nCol) > 0)
            AutoSizeColumn(nCol, nAutoSizeStyle, FALSE);
    }
    ResetScrollBars();
}

void CGridCtrl::AutoSizeRows()
{
    int nNumRows = GetRowCount();
    for (int nRow = 0; nRow < nNumRows; nRow++)
    {
        //  Skip hidden rows when autosizing
        if(GetRowHeight(nRow) > 0)
            AutoSizeRow(nRow, FALSE);
    }
    ResetScrollBars();
}

// sizes all rows and columns
// faster than calling both AutoSizeColumns() and AutoSizeRows()
void CGridCtrl::AutoSize(UINT nAutoSizeStyle /*=GVS_DEFAULT*/)
{
    CDCHandle DC = GetDC();
    if (DC.IsNull())
        return;

    int nNumColumns = GetColumnCount();

    int nCol, nRow;

    ATLASSERT(GVS_DEFAULT <= nAutoSizeStyle && nAutoSizeStyle <= GVS_BOTH);
    if (nAutoSizeStyle == GVS_DEFAULT)
        nAutoSizeStyle = GetAutoSizeStyle();

    int nStartRow = (nAutoSizeStyle & GVS_HEADER)? 0 : GetFixedRowCount();
    int nEndRow   = (nAutoSizeStyle & GVS_DATA)? GetRowCount()-1 : GetFixedRowCount()-1;

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(nStartRow, 0, nEndRow, nNumColumns));

    // Row initialisation - only work on rows whose height is > 0
    for (nRow = nStartRow; nRow <= nEndRow; nRow++)
    {
        if(GetRowHeight(nRow) > 0)
            m_arRowHeights[nRow] = 1;
    }

    CSize size;
    for (nCol = 0; nCol < nNumColumns; nCol++)
    {
        //  Don't size hidden columns or rows
        if(GetColumnWidth(nCol) > 0)
        {
            // Skip columns that are hidden, but now initialize
            m_arColWidths[nCol] = 0;
            for (nRow = nStartRow; nRow <= nEndRow; nRow++)
            {
                if(GetRowHeight(nRow) > 0)
                {
                    CGridCellBase* pCell = GetCell(nRow, nCol);
                    if (pCell)
                        size = pCell->GetCellExtent(DC);
                    if (size.cx >(int) m_arColWidths[nCol])
                        m_arColWidths[nCol] = size.cx;
                    if (size.cy >(int) m_arRowHeights[nRow])
                        m_arRowHeights[nRow] = size.cy;
                }
            }
        }
    }

    if (GetVirtualMode())
        SendCacheHintToParent(CCellRange(-1,-1,-1,-1));

    ReleaseDC(DC);

    ResetScrollBars();
    Refresh();
}

// Expands the columns to fit the screen space. If bExpandFixed is FALSE then fixed 
// columns will not be affected
void CGridCtrl::ExpandColumnsToFit(BOOL bExpandFixed /*=TRUE*/)
{
    if (bExpandFixed)
	{
		if (GetColumnCount() <= 0) return;
	} 
	else
	{
		if (GetColumnCount() <= GetFixedColumnCount()) return;
	}

    ShowScrollBars(SB_HORZ, FALSE);

    int col;
    CRect rect;
    GetClientRect(rect);

    int nFirstColumn = (bExpandFixed)? 0 : GetFixedColumnCount();

    int nNumColumnsAffected = 0;
    for (col = nFirstColumn; col < GetColumnCount(); col++)
    {
        if (m_arColWidths[col] > 0)
            nNumColumnsAffected++;
    }

    if (nNumColumnsAffected <= 0)
        return;

    long virtualWidth = GetVirtualWidth();
    int nDifference = rect.Width() -(int) virtualWidth;
    int nColumnAdjustment = nDifference / nNumColumnsAffected;

    for (col = nFirstColumn; col < GetColumnCount(); col++)
    {
        if (m_arColWidths[col] > 0)
            m_arColWidths[col] += nColumnAdjustment;    
    }

    if (nDifference > 0)
    {
        int leftOver = nDifference % nNumColumnsAffected;
        for (int nCount = 0, col = nFirstColumn; 
             (col < GetColumnCount()) && (nCount < leftOver); col++, nCount++)
        {
            if (m_arColWidths[col] > 0)
                m_arColWidths[col] += 1;
        }
    }
    else 
    {
        int leftOver = (-nDifference) % nNumColumnsAffected;
        for (int nCount = 0, col = nFirstColumn; 
             (col < GetColumnCount()) && (nCount < leftOver); col++, nCount++)
        {
            if (m_arColWidths[col] > 0)
                m_arColWidths[col] -= 1;
        }
    }

    Refresh();

    ResetScrollBars();
}

void CGridCtrl::ExpandLastColumn()
{
    if (GetColumnCount() <= 0)
        return;

    // Search for last non-hidden column
    int nLastColumn = GetColumnCount()-1;
    while (m_arColWidths[nLastColumn] <= 0)
        nLastColumn--;

    if (nLastColumn <= 0)
        return;

    ShowScrollBars(SB_HORZ, FALSE);

    CRect rect;
    GetClientRect(rect);

    long virtualWidth = GetVirtualWidth();
    int nDifference = rect.Width() -(int) virtualWidth;

    if (nDifference > 0)
    {
        //if (GetVirtualHeight() > rect.Height())
        //    nDifference -= GetSystemMetrics(SM_CXVSCROLL);

        m_arColWidths[ nLastColumn ] += nDifference;
        Refresh();
    }

    ResetScrollBars();
}

// Expands the rows to fit the screen space. If bExpandFixed is FALSE then fixed 
// rows will not be affected
void CGridCtrl::ExpandRowsToFit(BOOL bExpandFixed /*=TRUE*/)
{
    if (bExpandFixed)
	{
		if (GetRowCount() <= 0) return;
	} 
	else
	{
		if (GetRowCount() <= GetFixedRowCount()) return;
	}

    ShowScrollBars(SB_VERT, FALSE); 

    int row;
    CRect rect;
    GetClientRect(rect);
    
    int nFirstRow = (bExpandFixed)? 0 : GetFixedRowCount();

    int nNumRowsAffected = 0;
    for (row = nFirstRow; row < GetRowCount(); row++)
    {
        if (m_arRowHeights[row] > 0)
            nNumRowsAffected++;
    }

    if (nNumRowsAffected <= 0)
        return;

    long virtualHeight = GetVirtualHeight();
    int nDifference = rect.Height() -(int) virtualHeight;
    int nRowAdjustment = nDifference / nNumRowsAffected;
    
    for (row = nFirstRow; row < GetRowCount(); row++)
    {
        if (m_arRowHeights[row] > 0)
            m_arRowHeights[row] += nRowAdjustment;    
    }
    
    if (nDifference > 0)
    {
        int leftOver = nDifference % nNumRowsAffected;
        for (int nCount = 0, row = nFirstRow; 
             (row < GetRowCount()) && (nCount < leftOver); row++, nCount++)
        {
            if (m_arRowHeights[row] > 0)
                m_arRowHeights[row] += 1;
        }
    } 
    else 
    {
        int leftOver = (-nDifference) % nNumRowsAffected;
        for (int nCount = 0, row = nFirstRow; 
             (row < GetRowCount()) && (nCount < leftOver); row++, nCount++)
        {
            if (m_arRowHeights[row] > 0)
                m_arRowHeights[row] -= 1;
        }
    }

    Refresh();

    ResetScrollBars();
}

// Expands the cells to fit the screen space. If bExpandFixed is FALSE then fixed 
// cells  will not be affected
void CGridCtrl::ExpandToFit(BOOL bExpandFixed /*=TRUE*/)
{
    ExpandColumnsToFit(bExpandFixed);   // This will remove any existing horz scrollbar
    ExpandRowsToFit(bExpandFixed);      // This will remove any existing vert scrollbar
    ExpandColumnsToFit(bExpandFixed);   // Just in case the first adjustment was with a vert
                                        // scrollbar in place
    Refresh();
}

/////////////////////////////////////////////////////////////////////////////////////
// Attributes

void CGridCtrl::SetVirtualMode(BOOL bVirtual)
{
    DeleteAllItems();
    m_bVirtualMode = bVirtual;

    // Force some defaults here.
    if (m_bVirtualMode)
    {
        SetEditable(FALSE);
        SetAutoSizeStyle(GVS_HEADER);
        SetFixedColumnSelection(FALSE);
        SetFixedRowSelection(FALSE);
		ResetVirtualOrder();
    }
}

void CGridCtrl::SetGridLines(int nWhichLines /*=GVL_BOTH*/) 
{
    m_nGridLines = nWhichLines;
    Refresh();
}

void CGridCtrl::SetListMode(BOOL bEnableListMode /*=TRUE*/)
{
    ResetSelectedRange();
    SetSortColumn(-1);
    m_bListMode = bEnableListMode;
    SetFixedRowSelection(FALSE);
    Refresh();
}

void CGridCtrl::SetSortColumn(int nCol)
{
    if (m_nSortColumn >= 0)
        InvalidateCellRect(0, m_nSortColumn);
    m_nSortColumn = nCol;
    if (nCol >= 0)
        InvalidateCellRect(0, nCol);
}

BOOL CGridCtrl::IsCellFixed(int nRow, int nCol)
{
    return (nRow < GetFixedRowCount() || nCol < GetFixedColumnCount());
}

void CGridCtrl::SetModified(BOOL bModified /*=TRUE*/, int nRow /*=-1*/, int nCol /*=-1*/)
{
    // Cannot guarantee sorting anymore...
    if (nCol < 0 || nCol == GetSortColumn())
        SetSortColumn(-1);

    if (nRow >= 0 && nCol >= 0)
    {
        if (bModified)
        {
            SetItemState(nRow, nCol, GetItemState(nRow, nCol) | GVIS_MODIFIED);
            m_bModified = TRUE;
        }
        else
            SetItemState(nRow, nCol, GetItemState(nRow, nCol) & ~GVIS_MODIFIED);
    }
    else
        m_bModified = bModified;

    if (!m_bModified)
    {
        for (int row = 0; row < GetRowCount(); row++)
            for (int col = 0; col < GetColumnCount(); col++)
                SetItemState(row, col, GetItemState(row, col) & ~GVIS_MODIFIED);
    }
}

BOOL CGridCtrl::GetModified(int nRow /*=-1*/, int nCol /*=-1*/)
{
    if (nRow >= 0 && nCol >= 0)
        return ((GetItemState(nRow, nCol) & GVIS_MODIFIED) == GVIS_MODIFIED);
    else
        return m_bModified;
}

/////////////////////////////////////////////////////////////////////////////////////
// GridCtrl cell visibility tests and invalidation/redraw functions

void CGridCtrl::Refresh()
{
    if (IsWindow() && m_bAllowDraw)
        Invalidate();
}

void CGridCtrl::EnsureVisible(int nRow, int nCol)
{
    if (!m_bAllowDraw)
        return;

    EndEditing();

#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.Hide();  // hide any titletips
#endif

	CRect rectWindow;
	GetClientRect(rectWindow);

 	int nHscrollPos = GetScrollPos32(SB_HORZ);
	int nVscrollPos = GetScrollPos32(SB_VERT);
	
	CWindow FocusWnd = GetFocus();

    CCellRange VisibleCells = GetVisibleNonFixedCellRange();

    int right = nCol - VisibleCells.GetMaxCol();
    int left  = VisibleCells.GetMinCol() - nCol;
    int down  = nRow - VisibleCells.GetMaxRow();
    int up    = VisibleCells.GetMinRow() - nRow;

	bool bInvalidate = false;

    if (right > 0)
	{
		bool bi = false;
		int iColumnStart = VisibleCells.GetMaxCol() + 1;
		while(right > 0)
		{
			int nColWidth = GetColumnWidth(iColumnStart);
			if(nColWidth > 0)
			{
				nHscrollPos += nColWidth;
				bi = true;
			}
			right--;
			iColumnStart++;
		}
		if (bi)
		{
			bInvalidate = true;
			rectWindow.left = GetFixedColumnWidth();
		}
	}

	if (left > 0)
	{
		bool bi = false;
		int iColumnStart = VisibleCells.GetMinCol() - 1;
		while(left > 0)
		{
			int nColWidth = GetColumnWidth(iColumnStart);
			if(nColWidth > 0)
			{
				nHscrollPos -= nColWidth;
				bi = true;
			}
			left--;
			iColumnStart--;
		}
		if (bi)
		{
			bInvalidate = true;
			rectWindow.left = GetFixedColumnWidth();
		}
    }

	if (down > 0)
	{
		bool bi = false;
		int iRowStart = VisibleCells.GetMaxRow() + 1;
		while(down > 0)
		{
			int nRowHeight = GetRowHeight(iRowStart);
			if(nRowHeight > 0)
			{
				nVscrollPos += nRowHeight;
				bi = true;
			}
			down--;
			iRowStart++;
		}
		if (bi)
		{
			bInvalidate = true;
			rectWindow.top = GetFixedRowHeight();
		}
    }

	if (up > 0)
	{
		bool bi = false;
		int iRowStart = VisibleCells.GetMinRow() - 1;
		while(up > 0)
		{
 			int nRowHeight = GetRowHeight(iRowStart);
			if(nRowHeight > 0)
			{
				nVscrollPos -= nRowHeight;
				bi = true;
			}
			up--;
			iRowStart--;
		}
		if (bi)
		{
			bInvalidate = true;
			rectWindow.top = GetFixedRowHeight();
		}
	}

	if (bInvalidate)
	{
		SetScrollPos32(SB_HORZ, __max(0, nHscrollPos));
		SetScrollPos32(SB_VERT, __max(0, nVscrollPos));
		InvalidateRect(rectWindow);
	}

    // Move one more if we only see a snall bit of the cell
    CRect rectCell;
    if (!GetCellRect(nRow, nCol, rectCell))
    {
		if (FocusWnd.IsWindow())
			FocusWnd.SetFocus(); 
        return;
    }

    GetClientRect(rectWindow);
	if (rectCell.right > rectWindow.right && rectCell.left > GetFixedColumnWidth() && IsVisibleHScroll()) // Junlin Xu: added to prevent infinite loop
        SendMessage(WM_HSCROLL, SB_LINERIGHT, 0);

	if (rectCell.bottom > rectWindow.bottom && rectCell.top > GetFixedRowHeight() && IsVisibleVScroll()) // Junlin Xu: added to prevent infinite loop
        SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);

    // restore focus to whoever owned it
    FocusWnd.SetFocus(); 
}

BOOL CGridCtrl::IsCellEditable(CCellID &cell) const
{
    return IsCellEditable(cell.row, cell.col);
}

BOOL CGridCtrl::IsCellEditable(int nRow, int nCol) const
{
    return IsEditable() && ((GetItemState(nRow, nCol) & GVIS_READONLY) != GVIS_READONLY);
}

BOOL CGridCtrl::IsCellSelected(CCellID &cell) const
{
    return IsCellSelected(cell.row, cell.col);
}

BOOL CGridCtrl::IsCellSelected(int nRow, int nCol) const
{
    if (GetVirtualMode())
    {   
        if (!IsSelectable())
            return FALSE;

        CCellID cell;
        DWORD key = MAKELONG(nRow, nCol);

        return (m_SelectedCellMap.Lookup(key, (CCellID&)cell));       
    }
    else
        return IsSelectable() && ((GetItemState(nRow, nCol) & GVIS_SELECTED) == GVIS_SELECTED);
}

BOOL CGridCtrl::IsCellVisible(CCellID cell) 
{
    return IsCellVisible(cell.row, cell.col);
}

BOOL CGridCtrl::IsCellVisible(int nRow, int nCol)
{
    if (!IsWindow())
        return FALSE;

    int x, y;

    CCellID TopLeft;
    if (nCol >= GetFixedColumnCount() || nRow >= GetFixedRowCount())
    {
        TopLeft = GetTopleftNonFixedCell();
        if (nCol >= GetFixedColumnCount() && nCol < TopLeft.col)
            return FALSE;
        if (nRow >= GetFixedRowCount() && nRow < TopLeft.row)
            return FALSE;
    }
    
    CRect rect;
    GetClientRect(rect);
    if (nCol < GetFixedColumnCount())
    {
        x = 0;
        for (int i = 0; i <= nCol; i++) 
        {
            if (x >= rect.right)
                return FALSE;
            x += GetColumnWidth(i);    
        }
    } 
    else 
    {
        x = GetFixedColumnWidth();
        for (int i = TopLeft.col; i <= nCol; i++) 
        {
            if (x >= rect.right)
                return FALSE;
            x += GetColumnWidth(i);    
        }
    }
    
    if (nRow < GetFixedRowCount())
    {
        y = 0;
        for (int i = 0; i <= nRow; i++) 
        {
            if (y >= rect.bottom)
                return FALSE;
            y += GetRowHeight(i);    
        }
    } 
    else 
    {
        if (nRow < TopLeft.row)
            return FALSE;
        y = GetFixedRowHeight();
        for (int i = TopLeft.row; i <= nRow; i++) 
        {
            if (y >= rect.bottom)
                return FALSE;
            y += GetRowHeight(i);    
        }
    }
    
    return TRUE;
}

BOOL CGridCtrl::InvalidateCellRect(const CCellID& cell)
{
    return InvalidateCellRect(cell.row, cell.col);
}

BOOL CGridCtrl::InvalidateCellRect(const int row, const int col)
{
    if (!IsWindow() || !m_bAllowDraw)
        return FALSE;

    if (!IsValid(row, col))
        return FALSE;

    if (!IsCellVisible(row, col))
        return FALSE;

    CRect rect;
    if (!GetCellRect(row, col, rect))
        return FALSE;
    rect.right++;
    rect.bottom++;
    InvalidateRect(rect, TRUE);

    return TRUE;
}

BOOL CGridCtrl::InvalidateCellRect(const CCellRange& cellRange)
{
    ATLASSERT(IsValid(cellRange));
    if (!IsWindow() || !m_bAllowDraw)
        return FALSE;

    CCellRange visibleCellRange = GetVisibleNonFixedCellRange().Intersect(cellRange);

    CRect rect;
    if (!GetCellRangeRect(visibleCellRange, rect))
        return FALSE;

    rect.right++;
    rect.bottom++;
    InvalidateRect(rect, TRUE);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl Mouse stuff

// Handles mouse wheel notifications
// Note - if this doesn't work for win95 then use OnRegisteredMouseWheel instead
#if !defined(_WIN32_WCE)
BOOL CGridCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL bRet = TRUE;
    // A m_nRowsPerWheelNotch value less than 0 indicates that the mouse
    // wheel scrolls whole pages, not just lines.
    if (m_nRowsPerWheelNotch == -1)
    {
        int nPagesScrolled = zDelta / 120;

        if (nPagesScrolled > 0)
		{
            for (int i = 0; i < nPagesScrolled; i++)
			{
                bRet = PostMessage(WM_VSCROLL, SB_PAGEUP, 0) & bRet;
			}
		}
        else
		{
            for (int i = 0; i > nPagesScrolled; i--)
			{
                bRet = PostMessage(WM_VSCROLL, SB_PAGEDOWN, 0) & bRet;
			}
		}
    }
    else
    {
        int nRowsScrolled = m_nRowsPerWheelNotch * zDelta / 120;

        if (nRowsScrolled > 0)
		{
            for (int i = 0; i < nRowsScrolled; i++)
			{
                bRet = PostMessage(WM_VSCROLL, SB_LINEUP, 0) & bRet;
			}
		}
        else
		{
            for (int i = 0; i > nRowsScrolled; i--)
			{
                bRet = PostMessage(WM_VSCROLL, SB_LINEDOWN, 0) & bRet;
			}
		}
    }

    return bRet;
}
#endif // !defined(_WIN32_WCE) && (_MFC_VER >= 0x0421)

void CGridCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
    CRect rect;
    GetClientRect(rect);

#ifndef GRIDCONTROL_NO_DRAGDROP
    // If outside client area, return (unless we are drag n dropping)
    if (m_MouseMode != MOUSE_DRAGGING && !rect.PtInRect(point))
        return;
#endif

    // Sometimes a MOUSEMOVE message can come after the left buttons
    // has been let go, but before the BUTTONUP message has been processed.
    // We'll keep track of mouse buttons manually to avoid this.
    // All bMouseButtonDown's have been replaced with the member m_bLMouseButtonDown
    // BOOL bMouseButtonDown = ((nFlags & MK_LBUTTON) == MK_LBUTTON);

    // If the left mouse button is up, then test to see if row/column sizing is imminent
    if (!m_bLMouseButtonDown ||
        (m_bLMouseButtonDown && m_MouseMode == MOUSE_NOTHING))
    {
        if (m_bAllowColumnResize && MouseOverColumnResizeArea(point))
        {
            if (m_MouseMode != MOUSE_OVER_COL_DIVIDE)
            {
#ifndef _WIN32_WCE_NO_CURSOR
                SetCursor(AtlLoadSysCursor(IDC_SIZEWE));
#endif
                m_MouseMode = MOUSE_OVER_COL_DIVIDE;
            }
        }
        else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
        {
            if (m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
            {
#ifndef _WIN32_WCE_NO_CURSOR
                SetCursor(AtlLoadSysCursor(IDC_SIZENS));
#endif
                m_MouseMode = MOUSE_OVER_ROW_DIVIDE;
            }
        }
        else if (m_MouseMode != MOUSE_NOTHING)
        {
#ifndef _WIN32_WCE_NO_CURSOR
            SetCursor(AtlLoadSysCursor(IDC_ARROW));
#endif
            m_MouseMode = MOUSE_NOTHING;
        }

        if (m_MouseMode == MOUSE_NOTHING)
        {
            CGridCellBase* pCell = nullptr;
            CCellID idCurrentCell;
            if (!GetVirtualMode() || m_bTitleTips)
            {
                // Let the cell know that a big fat cursor is currently hovering
                // over it.
                idCurrentCell = GetCellFromPt(point);
                pCell = GetCell(idCurrentCell.row, idCurrentCell.col);
                if (pCell)
                    pCell->OnMouseOver();
            }

#ifndef GRIDCONTROL_NO_TITLETIPS
            // Titletips anyone? anyone?
            if (m_bTitleTips)
            {
                CRect TextRect, CellRect;
                if (pCell)
                {
                    LPCTSTR szTipText = pCell->GetTipText();
                    if (!m_bRMouseButtonDown
						&& szTipText && szTipText[0]
                        && !pCell->IsEditing()
                        && GetCellRect(idCurrentCell.row, idCurrentCell.col, &TextRect)
                        && pCell->GetTipTextRect(&TextRect)
                        && GetCellRect(idCurrentCell.row, idCurrentCell.col, CellRect))
                    {
						//ATLTRACE("Showing TitleTip\n");
						m_TitleTip.Show(TextRect, pCell->GetTipText(),  0, CellRect, pCell->GetFont(),  GetTitleTipTextClr(), GetTitleTipBackClr());
                    }
                }
            }
#endif
        }

        m_LastMousePoint = point;
        return;
    }

    if (!IsValid(m_LeftClickDownCell))
    {
        m_LastMousePoint = point;
        return;
    }

    // If the left mouse button is down, then process appropriately
    if (m_bLMouseButtonDown)
    {
        switch (m_MouseMode)
        {
        case MOUSE_SELECT_ALL:
            break;

        case MOUSE_SELECT_COL:
        case MOUSE_SELECT_ROW:
        case MOUSE_SELECT_CELLS:
            {
                CCellID idCurrentCell = GetCellFromPt(point);
                if (!IsValid(idCurrentCell))
                    return;

                if (idCurrentCell != GetFocusCell())
                {
                    OnSelecting(idCurrentCell);

                    // EFW - BUG FIX - Keep the appropriate cell row and/or
                    // column focused.  A fix in SetFocusCell() will place
                    // the cursor in a non-fixed cell as needed.
                    if((idCurrentCell.row >= m_nFixedRows &&
                      idCurrentCell.col >= m_nFixedCols) ||
                      m_MouseMode==MOUSE_SELECT_COL ||
                      m_MouseMode==MOUSE_SELECT_ROW)
                    {
                        SetFocusCell(idCurrentCell);
                    }
                }
                break;
            }

        case MOUSE_SIZING_COL:
            {
                CDCHandle DC = GetDC();
                if (DC.IsNull())
                    break;

                CRect oldInvertedRect(m_LastMousePoint.x, rect.top,
                    m_LastMousePoint.x + 2, rect.bottom);
                DC.InvertRect(&oldInvertedRect);
                CRect newInvertedRect(point.x, rect.top, 
                    point.x + 2, rect.bottom);
                DC.InvertRect(&newInvertedRect);
                ReleaseDC(DC);
            }
            break;
            
        case MOUSE_SIZING_ROW:        
            {
				CDCHandle DC = GetDC();
				if (DC.IsNull())
                    break;
                
                CRect oldInvertedRect(rect.left, m_LastMousePoint.y,
                    rect.right, m_LastMousePoint.y + 2);
                DC.InvertRect(&oldInvertedRect);
                CRect newInvertedRect(rect.left, point.y, 
                    rect.right, point.y + 2);
                DC.InvertRect(&newInvertedRect);
                ReleaseDC(DC);
            }
            break;
            
#ifndef GRIDCONTROL_NO_DRAGDROP
        case MOUSE_PREPARE_EDIT:
        case MOUSE_PREPARE_DRAG:
            m_MouseMode = MOUSE_PREPARE_DRAG;
            OnBeginDrag();    
            break;
#endif
        }    
    }

    m_LastMousePoint = point;
}

// Returns the point inside the cell that was clicked (coords relative to cell top left)
CPoint CGridCtrl::GetPointClicked(int nRow, int nCol, const CPoint& point)
{
    CPoint PointCellOrigin;
    if(!GetCellOrigin(nRow, nCol, &PointCellOrigin))
        return CPoint(0, 0);

    CPoint PointClickedCellRelative(point);
    PointClickedCellRelative -= PointCellOrigin;
    return PointClickedCellRelative;
}

void CGridCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    ATLTRACE("CGridCtrl::OnLButtonDblClk\n");

    CCellID cell = GetCellFromPt(point);
    if(!IsValid(cell))
    {
        //ASSERT(FALSE);
        return;
    }

#ifdef _WIN32_WCE
    if (MouseOverColumnResizeArea(point))
#else
    if (m_MouseMode == MOUSE_OVER_COL_DIVIDE)
#endif
    {
        CPoint start;
        if (!GetCellOrigin(0, cell.col, &start))
            return;

        if (point.x - start.x < m_nResizeCaptureRange)     // Clicked right of border
            cell.col--;

        //  ignore columns that are hidden and look left towards first visible column
        BOOL bFoundVisible = FALSE;
        while(cell.col >= 0)
        {
            if(GetColumnWidth(cell.col) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            cell.col--;
        }
        if(!bFoundVisible)
            return;

        AutoSizeColumn(cell.col, GetAutoSizeStyle());
        Invalidate();
    }
#ifdef _WIN32_WCE
    else if (MouseOverRowResizeArea(point))
#else
    else if (m_MouseMode == MOUSE_OVER_ROW_DIVIDE)
#endif
    {
        CPoint start;
        if (!GetCellOrigin(0, cell.col, &start))
            return;

        if (point.y - start.y < m_nResizeCaptureRange)     // Clicked below border
            cell.row--;

        //  ignore rows that are hidden and look up towards first visible row
        BOOL bFoundVisible = FALSE;
        while(cell.row >= 0)
        {
            if(GetRowHeight(cell.row) > 0)
            {
                bFoundVisible = TRUE;
                break;
            }
            cell.row--;
        }
        if(!bFoundVisible)
            return;

        AutoSizeRow(cell.row);
        Invalidate();
    }
    else if (m_MouseMode == MOUSE_NOTHING)
    {
        CPoint pointClickedRel;
        pointClickedRel = GetPointClicked(cell.row, cell.col, point);

        CGridCellBase* pCell = nullptr;
        if (IsValid(cell))
            pCell = GetCell(cell.row, cell.col);

        // Clicked in the text area? Only then will cell selection work
        BOOL bInTextArea = FALSE;
        if (pCell)
        {
            CRect rectCell;
            if (GetCellRect(cell.row, cell.col, rectCell) && pCell->GetTextRect(rectCell))
                bInTextArea = rectCell.PtInRect(point);
        }

        if (cell.row >= m_nFixedRows && IsValid(m_LeftClickDownCell) && 
            cell.col >= m_nFixedCols && bInTextArea)
        {
            OnEditCell(cell.row, cell.col, pointClickedRel, VK_LBUTTON);
        }
        else if (m_bListMode)
        {
            if (!IsValid(cell))
                return;
            if (cell.row >= m_nFixedRows && cell.col >= m_nFixedCols && bInTextArea)
                OnEditCell(cell.row, cell.col, pointClickedRel, VK_LBUTTON);
        }

        if (IsValid(cell))
        {
            CGridCellBase* pCell = GetCell(cell.row, cell.col);
            if (pCell)
                pCell->OnDblClick(pointClickedRel);
            SendMessageToParent(cell.row, cell.col, NM_DBLCLK);
        }
    }

    DefWindowProc();
}

void CGridCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
#ifndef GRIDCONTROL_NO_TITLETIPS
    // EFW - Bug Fix
    m_TitleTip.Hide();  // hide any titletips
#endif

    // TRACE0("CGridCtrl::OnLButtonDown\n");
    // CWnd::OnLButtonDown(nFlags, point);

    SetFocus();
	m_CurCol = -1;
    m_bLMouseButtonDown   = TRUE;
    m_LeftClickDownPoint = point;
    m_LeftClickDownCell  = GetCellFromPt(point);
    if (!IsValid(m_LeftClickDownCell))
        return;
	m_CurRow = m_LeftClickDownCell.row;

    // If the SHIFT key is not down, then the start of the selection area should be the 
    // cell just clicked. Otherwise, keep the previous selection-start-cell so the user
    // can add to their previous cell selections in an intuitive way. If no selection-
    // start-cell has been specified, then set it's value here and now.
    if ((nFlags & MK_SHIFT) != MK_SHIFT)
        m_SelectionStartCell = m_LeftClickDownCell;
    else
    {
        if (!IsValid(m_SelectionStartCell))
            m_SelectionStartCell = m_idCurrentCell;
    }

    EndEditing();

    // tell the cell about it 
    CGridCellBase* pCell = GetCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
    if (pCell)
        pCell->OnClickDown(GetPointClicked(m_LeftClickDownCell.row, m_LeftClickDownCell.col, point));

    // Clicked in the text area? Only then will cell selection work
    BOOL bInTextArea = FALSE;
    if (pCell)
    {
        CRect rectCell;
        if (GetCellRect(m_LeftClickDownCell.row, m_LeftClickDownCell.col, rectCell) && pCell->GetTextRect(rectCell))
            bInTextArea = rectCell.PtInRect(point);
    }

    // If the user clicks on the current cell, then prepare to edit it.
    // (If the user moves the mouse, then dragging occurs)
    if (m_LeftClickDownCell == m_idCurrentCell && !(nFlags & MK_CONTROL) && bInTextArea && IsCellEditable(m_LeftClickDownCell))
    {
        m_MouseMode = MOUSE_PREPARE_EDIT;
        return;
    }
    // If the user clicks on a selected cell, then prepare to drag it.
    // (If the user moves the mouse, then dragging occurs)
    else if (IsCellSelected(m_LeftClickDownCell))
    {
        SetFocusCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);

        // If control is pressed then unselect the cell or row (depending on the list mode)
        if (nFlags & MK_CONTROL)
        {
            SetFocusCell(m_LeftClickDownCell);
            if (GetListMode())
                SelectRows(m_LeftClickDownCell, TRUE, FALSE);
            else
                SelectCells(m_LeftClickDownCell, TRUE, FALSE);
            return;
        }
#ifndef GRIDCONTROL_NO_DRAGDROP
        else if (m_bAllowDragAndDrop)
            m_MouseMode = MOUSE_PREPARE_DRAG;
#endif
    }
    else if (m_MouseMode != MOUSE_OVER_COL_DIVIDE &&
             m_MouseMode != MOUSE_OVER_ROW_DIVIDE)        
    {
		if (m_LeftClickDownCell.row >= GetFixedRowCount() && 
			m_LeftClickDownCell.col >= GetFixedColumnCount())
		{
            SetFocusCell(m_LeftClickDownCell.row, m_LeftClickDownCell.col);
		}
		else
			SetFocusCell(-1, -1);

		//Don't set focus on any cell if the user clicked on a fixed cell - David Pritchard
        //if (GetRowCount() > GetFixedRowCount() && 
        //    GetColumnCount() > GetFixedColumnCount())
        //    SetFocusCell(max(m_LeftClickDownCell.row, m_nFixedRows),
        //                 max(m_LeftClickDownCell.col, m_nFixedCols));
    }
    
    SetCapture();
    
    if (m_MouseMode == MOUSE_NOTHING)
    {
        if (m_bAllowColumnResize && MouseOverColumnResizeArea(point))
        {
            if (m_MouseMode != MOUSE_OVER_COL_DIVIDE)
            {
#ifndef _WIN32_WCE_NO_CURSOR
                SetCursor(AtlLoadSysCursor(IDC_SIZEWE));
#endif
                m_MouseMode = MOUSE_OVER_COL_DIVIDE;
            }
        }
        else if (m_bAllowRowResize && MouseOverRowResizeArea(point))
        {
            if (m_MouseMode != MOUSE_OVER_ROW_DIVIDE)
            {
#ifndef _WIN32_WCE_NO_CURSOR
                SetCursor(AtlLoadSysCursor(IDC_SIZENS));
#endif
                m_MouseMode = MOUSE_OVER_ROW_DIVIDE;
            }
        }
        // else if (m_MouseMode != MOUSE_NOTHING)
        //{
        //    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
        //    m_MouseMode = MOUSE_NOTHING;
        //}
    }
    
    if (m_MouseMode == MOUSE_OVER_COL_DIVIDE) // sizing column
    {
        m_MouseMode = MOUSE_SIZING_COL;

        // Kludge for if we are over the last column...
        if (GetColumnWidth(GetColumnCount()-1) < m_nResizeCaptureRange)
        {
            CRect VisRect;
            GetVisibleNonFixedCellRange(VisRect);
            if (abs(point.x - VisRect.right) < m_nResizeCaptureRange)
                m_LeftClickDownCell.col = GetColumnCount()-1;
        }

        CPoint start;
        if (!GetCellOrigin(0, m_LeftClickDownCell.col, &start))
            return;

        if(!m_bHiddenColUnhide)
        {
            //  ignore columns that are hidden and look left towards first visible column
            BOOL bLookForVisible = TRUE;
            BOOL bIsCellRightBorder = point.x - start.x >= m_nResizeCaptureRange;

            if(bIsCellRightBorder && m_LeftClickDownCell.col + 1 >= GetColumnCount())
            {
                // clicked on last column's right border

                // if last column is visible, don't do anything
                if(m_LeftClickDownCell.col >= 0)
                    bLookForVisible = FALSE;
            }

            if(bLookForVisible)
            {
                // clicked on column divider other than last right border
                BOOL bFoundVisible = FALSE;
                int iOffset = 1;

                if(bIsCellRightBorder)
                    iOffset = 0;

                while(m_LeftClickDownCell.col - iOffset >= 0)
                {
                    if(GetColumnWidth(m_LeftClickDownCell.col - iOffset) > 0)
                    {
                        bFoundVisible = TRUE;
                        break;
                    }
                    m_LeftClickDownCell.col--;
                }
                if(!bFoundVisible)
                    return;
            }
        }


        CRect rect;
        GetClientRect(rect);
        CRect invertedRect(point.x, rect.top, point.x + 2, rect.bottom);

        CDCHandle DC = GetDC();
        if (!DC.IsNull())
        {
            DC.InvertRect(&invertedRect);
            ReleaseDC(DC);
        }

        // If we clicked to the right of the colimn divide, then reset the click-down cell
        // as the cell to the left of the column divide - UNLESS we clicked on the last column
        // and the last column is teensy (kludge fix)
        if (point.x - start.x < m_nResizeCaptureRange)
        {
            if (m_LeftClickDownCell.col < GetColumnCount()-1 || 
                GetColumnWidth(GetColumnCount()-1) >= m_nResizeCaptureRange)
            {
                if (!GetCellOrigin(0, --m_LeftClickDownCell.col, &start))
                    return;
            }
        }

		// Allow a cell resize width no greater than that which can be viewed within
		// the grid itself
		int nMaxCellWidth = rect.Width()-GetFixedColumnWidth();
        rect.left  = start.x + 1;
		rect.right = rect.left + nMaxCellWidth;

        ClientToScreen(rect);
#ifndef _WIN32_WCE_NO_CURSOR
        ClipCursor(rect);
#endif
    }
    else if (m_MouseMode == MOUSE_OVER_ROW_DIVIDE) // sizing row
    {
        m_MouseMode = MOUSE_SIZING_ROW;

        // Kludge for if we are over the last column...
        if (GetRowHeight(GetRowCount()-1) < m_nResizeCaptureRange)
        {
            CRect VisRect;
            GetVisibleNonFixedCellRange(VisRect);
            if (abs(point.y - VisRect.bottom) < m_nResizeCaptureRange)
                m_LeftClickDownCell.row = GetRowCount()-1;
        }

        CPoint start;
        if (!GetCellOrigin(m_LeftClickDownCell, &start))
            return;

        if(!m_bHiddenRowUnhide)
        {
            //  ignore rows that are hidden and look up towards first visible row
            BOOL bLookForVisible = TRUE;
            BOOL bIsCellBottomBorder = point.y - start.y >= m_nResizeCaptureRange;

            if(bIsCellBottomBorder
                && m_LeftClickDownCell.row + 1 >= GetRowCount())
            {
                // clicked on last row's bottom border

                // if last row is visible, don't do anything
                if(m_LeftClickDownCell.row >= 0)
                    bLookForVisible = FALSE;
            }

            if(bLookForVisible)
            {
                // clicked on row divider other than last bottom border
                BOOL bFoundVisible = FALSE;
                int iOffset = 1;

                if(bIsCellBottomBorder)
                    iOffset = 0;

                while(m_LeftClickDownCell.row - iOffset >= 0)
                {
                    if(GetRowHeight(m_LeftClickDownCell.row - iOffset) > 0)
                    {
                        bFoundVisible = TRUE;
                        break;
                    }
                    m_LeftClickDownCell.row--;
                }
                if(!bFoundVisible)
                    return;
            }
        }

        CRect rect;
        GetClientRect(rect);
        CRect invertedRect(rect.left, point.y, rect.right, point.y + 2);

        CDCHandle DC = GetDC();
        if (!DC.IsNull())
        {
            DC.InvertRect(&invertedRect);
            ReleaseDC(DC);
        }

        // If we clicked below the row divide, then reset the click-down cell
        // as the cell above the row divide - UNLESS we clicked on the last row
        // and the last row is teensy (kludge fix)
        if (point.y - start.y < m_nResizeCaptureRange)            // clicked below border
        {
            if (m_LeftClickDownCell.row < GetRowCount()-1 || 
                GetRowHeight(GetRowCount()-1) >= m_nResizeCaptureRange)
            {
                if (!GetCellOrigin(--m_LeftClickDownCell.row, 0, &start))
                    return;
            }
        }

		int nMaxCellHeight = rect.Height()-GetFixedRowHeight();
        rect.top = start.y + 1;
		rect.bottom = rect.top + nMaxCellHeight;

        ClientToScreen(rect);

#ifndef _WIN32_WCE_NO_CURSOR
        ClipCursor(rect);
#endif
    }
    else
#ifndef GRIDCONTROL_NO_DRAGDROP
    if (m_MouseMode != MOUSE_PREPARE_DRAG) // not sizing or editing -- selecting
#endif
    {
	    SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);

        // If Ctrl pressed, save the current cell selection. This will get added
        // to the new cell selection at the end of the cell selection process
        m_PrevSelectedCellMap.RemoveAll();
        if (nFlags & MK_CONTROL)
        {
            for (POSITION pos = m_SelectedCellMap.GetStartPosition(); pos != NULL;)
            {
                DWORD key;
                CCellID cell;
                m_SelectedCellMap.GetNextAssoc(pos, key, (CCellID&)cell);
                m_PrevSelectedCellMap.SetAt(key, cell);
            }
        }
        
        if (m_LeftClickDownCell.row < GetFixedRowCount())
		{
            OnFixedRowClick(m_LeftClickDownCell);
            if(m_AllowReorderColumn && m_LeftClickDownCell.col >=  GetFixedColumnCount())
			{
				ResetSelectedRange(); // TODO : This is not the better solution, as we do not see why clicking in column header should reset selection
											//but the state of selection is instable after drag (at least until someone debugs it), so better clear it allways.
				m_MouseMode = MOUSE_PREPARE_DRAG;
				m_CurCol = m_LeftClickDownCell.col;
			}
		}
        else if (m_LeftClickDownCell.col < GetFixedColumnCount())
            OnFixedColumnClick(m_LeftClickDownCell);
        else
        {
            m_MouseMode = m_bListMode? MOUSE_SELECT_ROW : MOUSE_SELECT_CELLS;
            OnSelecting(m_LeftClickDownCell);

            m_nTimerID = SetTimer(WM_LBUTTONDOWN, m_nTimerInterval, 0);
        }
    }   
    m_LastMousePoint = point;
}

void CGridCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TRACE0("CGridCtrl::OnLButtonUp\n");

    DefWindowProc();

    m_bLMouseButtonDown = FALSE;

#ifndef _WIN32_WCE_NO_CURSOR
    ClipCursor(nullptr);
#endif

    if (GetCapture() == m_hWnd)
    {
        ReleaseCapture();
        KillTimer(m_nTimerID);
        m_nTimerID = 0;
    }

    CPoint pointClickedRel;
    pointClickedRel = GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point);

    // m_MouseMode == MOUSE_PREPARE_EDIT only if user clicked down on current cell
    // and then didn't move mouse before clicking up (releasing button)
    if (m_MouseMode == MOUSE_PREPARE_EDIT)
    {
        OnEditCell(m_idCurrentCell.row, m_idCurrentCell.col, pointClickedRel, VK_LBUTTON);
    }
#ifndef GRIDCONTROL_NO_DRAGDROP
    // m_MouseMode == MOUSE_PREPARE_DRAG only if user clicked down on a selected cell
    // and then didn't move mouse before clicking up (releasing button)
    else if (m_MouseMode == MOUSE_PREPARE_DRAG) 
    {
        CGridCellBase* pCell = GetCell(m_idCurrentCell.row, m_idCurrentCell.col);
        if (pCell)
            pCell->OnClick(GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point));
        SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, NM_CLICK);
	    SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, GVN_SELCHANGING);
        ResetSelectedRange();
	    SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);
    }
#endif
    else if (m_MouseMode == MOUSE_SIZING_COL)
    {
        CRect rect;
        GetClientRect(rect);
        CRect invertedRect(m_LastMousePoint.x, rect.top, m_LastMousePoint.x + 2, rect.bottom);
        
        CDCHandle DC = GetDC();
        if (!DC.IsNull())
        {
            DC.InvertRect(&invertedRect);
            ReleaseDC(DC);
        }
        
        if (m_LeftClickDownPoint != point && (point.x != 0 || point.y != 0)) // 0 pt fix by email1@bierling.net
        {   
            CPoint start;
            if (!GetCellOrigin(m_LeftClickDownCell, &start))
                return;

            int nColumnWidth = __max(point.x - start.x, m_bAllowColHide? 0L : 1L);

            SetColumnWidth(m_LeftClickDownCell.col, nColumnWidth);
            ResetScrollBars();
            Invalidate();
        }
    }
    else if (m_MouseMode == MOUSE_SIZING_ROW)
    {
        CRect rect;
        GetClientRect(rect);
        CRect invertedRect(rect.left, m_LastMousePoint.y, rect.right, m_LastMousePoint.y + 2);

		CDCHandle DC = GetDC();
		if (!DC.IsNull())
		{
			DC.InvertRect(&invertedRect);
			ReleaseDC(DC);
		}
        
        if (m_LeftClickDownPoint != point  && (point.x != 0 || point.y != 0)) // 0 pt fix by email1@bierling.net
        {
            CPoint start;
            if (!GetCellOrigin(m_LeftClickDownCell, &start))
                return;
            
            int nRowHeight = __max(point.y - start.y, m_bAllowRowHide? 0 : 1);

            SetRowHeight(m_LeftClickDownCell.row, nRowHeight);
            ResetScrollBars();
            Invalidate();
        }
    }
    else
    {
	    SendMessageToParent(m_idCurrentCell.row, m_idCurrentCell.col, GVN_SELCHANGED);

        CGridCellBase* pCell = GetCell(m_idCurrentCell.row, m_idCurrentCell.col);
        if (pCell)
            pCell->OnClick(GetPointClicked(m_idCurrentCell.row, m_idCurrentCell.col, point));
        SendMessageToParent(m_LeftClickDownCell.row, m_LeftClickDownCell.col, NM_CLICK);
    }
    
    m_MouseMode = MOUSE_NOTHING;
    
#ifndef _WIN32_WCE_NO_CURSOR
    SetCursor(AtlLoadSysCursor(IDC_ARROW));
#endif
    
    if (!IsValid(m_LeftClickDownCell))
        return;
    
    CWindow Owner = GetOwner(m_hWnd);
    if (Owner.IsWindow())
        Owner.PostMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), BN_CLICKED),
        (LPARAM) m_hWnd);
}

#ifndef _WIN32_WCE
void CGridCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
    DefWindowProc();
	m_bRMouseButtonDown = TRUE;

#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.Hide();  // hide any titletips
#endif
}

// EFW - Added to forward right click to parent so that a context
// menu can be shown without deriving a new grid class.
void CGridCtrl::OnRButtonUp(UINT nFlags, CPoint point)
{
	DefWindowProc();

	m_bRMouseButtonDown = FALSE;

	CCellID FocusCell;
	FocusCell = GetCellFromPt(point);

	EndEditing();		// Auto-destroy any InPlaceEdit's

	// If not a valid cell, pass -1 for row and column
	if(!IsValid(FocusCell))
		SendMessageToParent(-1, -1, NM_RCLICK);
	else
	{
		SetFocusCell(-1,-1);
		SetFocusCell(__max(FocusCell.row, m_nFixedRows),
			__max(FocusCell.col, m_nFixedCols));

		// tell the cell about it
		CGridCellBase* pCell = GetCell(FocusCell.row, FocusCell.col);
		if (pCell)
			pCell->OnRClick(GetPointClicked(FocusCell.row, FocusCell.col, point));

		SendMessageToParent(FocusCell.row, FocusCell.col, NM_RCLICK);
	}
}
#endif

#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
/////////////////////////////////////////////////////////////////////////////
// CGridCtrl printing

// EFW - New print margin support functions
void CGridCtrl::SetPrintMarginInfo(int nHeaderHeight, int nFooterHeight,
	int nLeftMargin, int nRightMargin, int nTopMargin,
	int nBottomMargin, int nGap)
{
	// If any parameter is -1, keep the existing setting
	if(nHeaderHeight > -1)
		m_nHeaderHeight = nHeaderHeight;
	if(nFooterHeight > -1)
		m_nFooterHeight = nFooterHeight;
	if(nLeftMargin > -1)
		m_nLeftMargin = nLeftMargin;
	if(nRightMargin > -1)
		m_nRightMargin = nRightMargin;
	if(nTopMargin > -1)
		m_nTopMargin = nTopMargin;
	if(nBottomMargin > -1)
		m_nBottomMargin = nBottomMargin;
	if(nGap > -1)
		m_nGap = nGap;
}

void CGridCtrl::GetPrintMarginInfo(int &nHeaderHeight, int &nFooterHeight,
	int &nLeftMargin, int &nRightMargin, int &nTopMargin,
	int &nBottomMargin, int &nGap)
{
	nHeaderHeight = m_nHeaderHeight;
	nFooterHeight = m_nFooterHeight;
	nLeftMargin = m_nLeftMargin;
	nRightMargin = m_nRightMargin;
	nTopMargin = m_nTopMargin;
	nBottomMargin = m_nBottomMargin;
	nGap = m_nGap;
}

bool CGridCtrl::Print(CPrinter hPrinter, DEVMODE* pdm/* = nullptr*/)
{
	CWaitCursor wc;

	CPrinterDC	PrintDC(hPrinter, pdm);
	if (PrintDC.IsNull())
		return false;

	CDCHandle CurrentDC = GetDC();
	if (CurrentDC.IsNull())
		return false;

	CPrintJob	printjob;
	UINT		m_nStartPage = 1;
	CString m_strTitle = GetParentWndText(*this);

	// CountPages
	CSize PaperPixelsPerInch(PrintDC.GetDeviceCaps(LOGPIXELSX), PrintDC.GetDeviceCaps(LOGPIXELSY));
	CSize ScreenPixelsPerInch(CurrentDC.GetDeviceCaps(LOGPIXELSX), CurrentDC.GetDeviceCaps(LOGPIXELSY));
	ReleaseDC(CurrentDC);

	// Create the printer font
	int nFontSize = -10;
	CString strFontName = _T("Arial");
	if (m_PrinterFont.IsNull())
		m_PrinterFont.CreateFont(nFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, strFontName);

	CFontHandle OldFont = PrintDC.SelectFont(m_PrinterFont);

	// Get the average character width (in GridCtrl units) and hence the margins
	ATLASSERT(PrintDC.GetTextExtent(_T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSATUVWXYZ"), 52, &m_CharSize));
	m_CharSize.cx /= 52;
	int nMargins = (m_nLeftMargin+m_nRightMargin)*m_CharSize.cx;

	// Get the page sizes (physical and logical)
	m_PaperSize = CSize(PrintDC.GetDeviceCaps(HORZRES), PrintDC.GetDeviceCaps(VERTRES));

	if(m_bWysiwygPrinting)
	{
		m_LogicalPageSize.cx = ScreenPixelsPerInch.cx * m_PaperSize.cx / PaperPixelsPerInch.cx * 3 / 4;
		m_LogicalPageSize.cy = ScreenPixelsPerInch.cy * m_PaperSize.cy / PaperPixelsPerInch.cy * 3 / 4;
	}
	else
	{
		m_PaperSize = CSize(PrintDC.GetDeviceCaps(HORZRES), PrintDC.GetDeviceCaps(VERTRES));

		m_LogicalPageSize.cx = GetVirtualWidth()+nMargins;
#ifdef _WIN32_WCE
		m_LogicalPageSize.cy = (m_LogicalPageSize.cx * m_PaperSize.cy) / m_PaperSize.cx;
#else
		m_LogicalPageSize.cy = MulDiv(m_LogicalPageSize.cx, m_PaperSize.cy, m_PaperSize.cx);
#endif
	}

	m_nPageHeight = m_LogicalPageSize.cy - GetFixedRowHeight() - (m_nHeaderHeight+m_nFooterHeight + 2*m_nGap)*m_CharSize.cy;

	// Get the number of pages. Assumes no row is bigger than the page size.
	int nTotalRowHeight = 0;
	m_nNumPages = 1;
	for (int row = GetFixedRowCount(); row < GetRowCount(); row++)
	{
		nTotalRowHeight += GetRowHeight(row);
		if (nTotalRowHeight > m_nPageHeight) {
			m_nNumPages++;
			nTotalRowHeight = GetRowHeight(row);
		}
	}

	// now, figure out how many additional pages must print out if rows ARE bigger
	// than page size
	int iColumnOffset = 0;
	int i1;
	for(i1=0; i1 < GetFixedColumnCount(); i1++)
	{
		iColumnOffset += GetColumnWidth(i1);
	}
	m_nPageWidth = m_LogicalPageSize.cx - iColumnOffset - nMargins;
	m_nPageMultiplier = 1;

	if(m_bWysiwygPrinting)
	{
		int iTotalRowWidth = 0;
		for(i1 = GetFixedColumnCount(); i1 < GetColumnCount(); i1++)
		{
			iTotalRowWidth += GetColumnWidth(i1);
			if(iTotalRowWidth > m_nPageWidth)
			{
				m_nPageMultiplier++;
				iTotalRowWidth = GetColumnWidth(i1);
			}
		}
		m_nNumPages *= m_nPageMultiplier;
	}
	PrintDC.SelectFont(OldFont);
	printjob.StartPrintJob(false, hPrinter, pdm, this, m_strTitle, m_nStartPage, m_nNumPages);
	return true;
}

void CGridCtrl::PrePrintPage(UINT nPage, HDC hDC)
{
	CDCHandle DC(hDC);
	m_PrintOldFont = DC.SelectFont(m_PrinterFont);
	// Set the page map mode to use GridCtrl units, and setup margin
	DC.SetMapMode(MM_ANISOTROPIC);
	DC.SetWindowExt(m_LogicalPageSize);
	DC.SetViewportExt(m_PaperSize);
	DC.SetWindowOrg(-m_nLeftMargin * m_CharSize.cx, 0);

	// Header
	m_rectDraw.top		= 0;
	m_rectDraw.left		= 0;
	m_rectDraw.right	= m_LogicalPageSize.cx - (m_nLeftMargin + m_nRightMargin) * m_CharSize.cx;
	m_rectDraw.bottom	= m_nHeaderHeight * m_CharSize.cy;

	// PrintHeader
	PrintHeader(nPage, hDC);

	DC.OffsetWindowOrg(0, -m_nHeaderHeight * m_CharSize.cy);
    // Gap between header and column headings
    DC.OffsetWindowOrg(0, -m_nGap * m_CharSize.cy);
    DC.OffsetWindowOrg(0, -GetFixedRowHeight());
}

bool CGridCtrl::PrintPage(UINT nPage, HDC hDC)
{
	CDCHandle DC(hDC);

	// We need to find out which row to start printing for this page.
    int nTotalRowHeight = 0;
    UINT nNumPages = 1;
    UINT m_nCurrPrintRow = GetFixedRowCount();


    // Not only the row, but we need to figure out column, too

    // Can print 4 pages, where page 1 and 2 represent the same rows but
    // with different WIDE columns.
    //
    // .......
    // .1 .2 .  If representing page 3  -->    iPageIfIgnoredWideCols = 2
    // .......                                 iWideColPageOffset = 0
    // .3 .4 .  If representing page 2  -->    iPageIfIgnoredWideCols = 1
    // .......                                 iWideColPageOffset = 1

    int iPageIfIgnoredWideCols = nPage / m_nPageMultiplier;
    int iWideColPageOffset = nPage - (iPageIfIgnoredWideCols * m_nPageMultiplier);
    if(iWideColPageOffset > 0)
        iPageIfIgnoredWideCols++;

    if(iWideColPageOffset == 0)
        iWideColPageOffset = m_nPageMultiplier;
    iWideColPageOffset--;

    // calculate current print row based on iPageIfIgnoredWideCols
    while((int)m_nCurrPrintRow < GetRowCount() && (int)nNumPages < iPageIfIgnoredWideCols)
    {
        nTotalRowHeight += GetRowHeight(m_nCurrPrintRow);
        if (nTotalRowHeight > m_nPageHeight)
		{
            nNumPages++;
            if ((int)nNumPages == iPageIfIgnoredWideCols) break;
            nTotalRowHeight = GetRowHeight(m_nCurrPrintRow);
        }
        m_nCurrPrintRow++;
    }

    m_nPrintColumn = GetFixedColumnCount();
    int iTotalRowWidth = 0;
    int i1, i2;

    // now, calculate which print column to start displaying
    for(i1 = 0; i1 < iWideColPageOffset; i1++)
    {
        for(i2 = m_nPrintColumn; i2 < GetColumnCount(); i2++)
        {
            iTotalRowWidth += GetColumnWidth(i2);
            if(iTotalRowWidth > m_nPageWidth)
            {
                m_nPrintColumn = i2;
                iTotalRowWidth = 0;
                break;
            }
        }
    }

    PrintRowButtons(nPage, hDC);			// print row buttons on each page

	int iColumnOffset = 0;
    for(i1=0; i1 < GetFixedColumnCount(); i1++)
        iColumnOffset += GetColumnWidth(i1);

    // Print the column headings
	m_rectDraw.bottom = GetFixedRowHeight();

	if(m_nPrintColumn == GetFixedColumnCount())
    {
        // have the column headings fcn draw the upper left fixed cells
        //  for the very first columns, only
        DC.OffsetWindowOrg(0, GetFixedRowHeight());

        m_nPageWidth += iColumnOffset;
        m_nPrintColumn = 0;
        PrintColumnHeadings(nPage, hDC);
        m_nPageWidth -= iColumnOffset;
        m_nPrintColumn = GetFixedColumnCount();

        DC.OffsetWindowOrg(-iColumnOffset, -GetFixedRowHeight());
    }
    else
    {
        // changed all of this here to match above almost exactly same
        DC.OffsetWindowOrg(0, +GetFixedRowHeight());

        m_nPageWidth += iColumnOffset;

        // print from column 0 ... last column that fits on the current page
        PrintColumnHeadings(nPage, hDC);
        
        m_nPageWidth -= iColumnOffset;
 
        DC.OffsetWindowOrg(-iColumnOffset, -GetFixedRowHeight());
    }


	if ((int)m_nCurrPrintRow < GetRowCount())
	{
		// Draw as many rows as will fit on the printed page.
		// Clip the printed page so that there is no partially shown
		// row at the bottom of the page (the same row which will be fully
		// shown at the top of the next page).

		BOOL bFirstPrintedRow = TRUE;
		CRect rect;
		rect.bottom = -1;
		while ((int)m_nCurrPrintRow < GetRowCount())
		{
			rect.top = rect.bottom + 1;
			rect.bottom = rect.top + GetRowHeight(m_nCurrPrintRow) - 1;
			if (rect.bottom > m_nPageHeight)
				break;            // Gone past end of page
			rect.right = -1;
			// modified to allow printing of wide grids on multiple pages
			for (int col = m_nPrintColumn; col < GetColumnCount(); col++)
			{
				rect.left = rect.right + 1;
				rect.right = rect.left + GetColumnWidth(col) - 1;

				if(rect.right > m_nPageWidth)
					break;

				CGridCellBase* pCell = GetCell(m_nCurrPrintRow, col);
				if (pCell)
					pCell->PrintCell(DC, m_nCurrPrintRow, col, rect);

				if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
				{
					int nOverlap = (col == 0) ? 0 : 1;
					DC.MoveTo(rect.left - nOverlap, rect.bottom);
					DC.LineTo(rect.right, rect.bottom);
					if (m_nCurrPrintRow == 0)
					{
						DC.MoveTo(rect.left - nOverlap, rect.top);
						DC.LineTo(rect.right, rect.top);
					}
				}
				if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
				{
					int nOverlap = (bFirstPrintedRow) ? 0 : 1;
					DC.MoveTo(rect.right, rect.top - nOverlap);
					DC.LineTo(rect.right, rect.bottom);
					if (col == 0)
					{
						DC.MoveTo(rect.left, rect.top - nOverlap);
						DC.LineTo(rect.left, rect.bottom);
					}
				}
			}
			m_nCurrPrintRow++;
			bFirstPrintedRow = FALSE;
		}
	}
	return true;
}

void CGridCtrl::PostPrintPage(UINT nPage, HDC hDC)
{
	CDCHandle DC(hDC);

    m_rectDraw.bottom = m_nFooterHeight * m_CharSize.cy;
    DC.SetWindowOrg(-m_nLeftMargin * m_CharSize.cx, -m_LogicalPageSize.cy + m_nFooterHeight * m_CharSize.cy);
    PrintFooter(nPage, hDC);

    // SetWindowOrg back for next page
    DC.SetWindowOrg(0, 0);
    DC.SelectFont(m_PrintOldFont);
}

// added by M.Fletcher 12/17/00
void CGridCtrl::PrintFixedRowCells(int nStartColumn, int nStopColumn, int& row, CRect& rect, HDC hDC, BOOL& bFirst)
{
	CDCHandle DC(hDC);

	// print all cells from nStartColumn to nStopColumn on row
	for (int col = nStartColumn; col < nStopColumn; col++)
	{
		rect.left = rect.right + 1;
		rect.right = rect.left + GetColumnWidth(col) - 1;

		if(rect.right > m_nPageWidth)
			break;

		CGridCellBase* pCell = GetCell(row, col);
		if (pCell)
			pCell->PrintCell(DC, row, col, rect);

		if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
		{
			int nOverlap = (col == 0) ? 0 : 1;

			DC.MoveTo(rect.left - nOverlap, rect.bottom);
			DC.LineTo(rect.right, rect.bottom);

			if (row == 0)
			{
				DC.MoveTo(rect.left - nOverlap, rect.top);
				DC.LineTo(rect.right, rect.top);
			}
		}
		if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
		{
			int nOverlap = (row == 0) ? 0 : 1;

			DC.MoveTo(rect.right, rect.top - nOverlap);
			DC.LineTo(rect.right, rect.bottom);

			if(bFirst)
			{
				DC.MoveTo(rect.left, rect.top - nOverlap);
				DC.LineTo(rect.left, rect.bottom);
				bFirst = FALSE;
			}
		}
	} // end of column cells loop
} // end of CGridCtrl::PrintFixedRowCells

void CGridCtrl::PrintColumnHeadings(UINT /*nPage*/, HDC hDC)
{
	CDCHandle DC(hDC);
	//	CFont OldFont = DC.SelectFont(m_PrinterFont);

	CRect rect;
	rect.bottom = -1;

	BOOL bFirst = TRUE;
	BOOL bOriginal;


	// modified to allow column hdr printing of multi-page wide grids
	for (int row = 0; row < GetFixedRowCount(); row++)
	{
		rect.top = rect.bottom + 1;
		rect.bottom = rect.top + GetRowHeight(row) - 1;

		rect.right = -1;

		// if printColumn > fixedcolumncount we are on page 2 or more
		// lets printout those fixed cell headings again the 1 or more that would be missed
		// added by M.Fletcher 12/17/00
		if(m_nPrintColumn>= GetFixedColumnCount())
		{
			bOriginal=bFirst;
			// lets print the missing fixed cells on left first out to last fixed column
			PrintFixedRowCells(0,GetFixedColumnCount(), row, rect, DC, bFirst);
			bFirst=bOriginal;
		}

		// now back to normal business print cells in heading after all fixed columns
		PrintFixedRowCells(m_nPrintColumn, GetColumnCount(), row, rect, DC, bFirst);

	} // end of Row Loop
} // end of CGridCtrl::PrintColumnHeadings

/*****************************************************************************
Prints line of row buttons on each page of the printout.  Assumes that
the window origin is setup before calling

*****************************************************************************/
void CGridCtrl::PrintRowButtons(UINT /*nPage*/, HDC hDC)
{
	CDCHandle DC(hDC);
	//	CFont OldFont = DC.SelectFont(m_PrinterFont);

	CRect rect;
	rect.right = -1;
    UINT m_nCurrPrintRow = GetFixedRowCount();

	BOOL bFirst = TRUE;
	for(int iCol = 0; iCol < GetFixedColumnCount(); iCol++)
	{
		rect.left = rect.right + 1;
		rect.right = rect.left + GetColumnWidth(iCol) - 1;
		rect.bottom = -1;
		for(int iRow = m_nCurrPrintRow; iRow < GetRowCount(); iRow++)
		{
			rect.top = rect.bottom + 1;
			rect.bottom = rect.top + GetRowHeight(iRow) - 1;
			if(rect.bottom > m_nPageHeight)
				break;

			CGridCellBase* pCell = GetCell(iRow, iCol);
			if (pCell)
				pCell->PrintCell(DC, iRow, iCol, rect);

			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_HORZ)
			{
				int Overlap = (iCol == 0) ? 0 : 1;
				DC.MoveTo(rect.left - Overlap, rect.bottom);
				DC.LineTo(rect.right, rect.bottom);
				if(bFirst)
				{
					DC.MoveTo(rect.left - Overlap, rect.top - 1);
					DC.LineTo(rect.right, rect.top - 1);
					bFirst = FALSE;
				}
			}
			if (m_nGridLines == GVL_BOTH || m_nGridLines == GVL_VERT)
			{
				int Overlap = (iRow == 0) ? 0 : 1;
				DC.MoveTo(rect.right, rect.top - Overlap);
				DC.LineTo(rect.right, rect.bottom);
				if (iCol == 0)
				{
					DC.MoveTo(rect.left, rect.top-Overlap);
					DC.LineTo(rect.left, rect.bottom);
				}
			}
		}
	}
}

void CGridCtrl::PrintHeader(UINT nPage, HDC hDC)
{
	CDCHandle DC(hDC);
	CString m_strTitle = GetParentWndText(*this);
    if(!m_strTitle.IsEmpty())
	{
		CFont BoldFont;
		LOGFONT lf;
		m_PrinterFont.GetLogFont(&lf);
		lf.lfWeight = FW_BOLD;
		BoldFont.CreateFontIndirect(&lf);

		CFont NormalFont = DC.SelectFont(BoldFont);
		int nPrevBkMode = DC.SetBkMode(TRANSPARENT);

        DC.DrawText(m_strTitle, m_strTitle.GetLength(), &m_rectDraw, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

		DC.SetBkMode(nPrevBkMode);
		DC.SelectFont(NormalFont);
	}

	// draw ruled-line across top
    DC.SelectStockPen(BLACK_PEN);
    DC.MoveTo(m_rectDraw.left, m_rectDraw.bottom);
    DC.LineTo(m_rectDraw.right, m_rectDraw.bottom);
}

//print footer with a line and date, and page number
void CGridCtrl::PrintFooter(UINT nPage, HDC hDC)
{
	CDCHandle DC(hDC);
    // page numbering on left
    CString strLeft;
    strLeft.Format(_T("Page %d of %d"), nPage, m_nNumPages);

    // date and time on the right
    CString strRight;
	time_t ltime;
	tm today;
	time(&ltime);
	errno_t err = _localtime64_s( &today, &ltime );
	if (!err)
	{
		_tcsftime(strRight.GetBuffer(256), 256, _T("%c"), &today);
		strRight.ReleaseBuffer();
	}

    // draw ruled line on bottom
    DC.SelectStockPen(BLACK_PEN);
    DC.MoveTo(m_rectDraw.left, m_rectDraw.top);
    DC.LineTo(m_rectDraw.right, m_rectDraw.top);

    CFont BoldFont;
    LOGFONT lf;

    //create bold font for header and footer
    m_PrinterFont.GetLogFont(&lf);
    lf.lfWeight = FW_BOLD;
    BoldFont.CreateFontIndirect(&lf);

    CFontHandle NormalFont = DC.SelectFont(BoldFont);
    int nPrevBkMode = DC.SetBkMode(TRANSPARENT);

    // EFW - Bug fix - Force text color to black.  It doesn't always
    // get set to a printable color when it gets here.
    DC.SetTextColor(RGB(0, 0, 0));

    if(!strLeft.IsEmpty())
        DC.DrawText(strLeft, strLeft.GetLength(), &m_rectDraw, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
    if(!strRight.IsEmpty())
        DC.DrawText(strRight, strRight.GetLength(), &m_rectDraw, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

    DC.SetBkMode(nPrevBkMode);
    DC.SelectFont(NormalFont);
}

#endif  // !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)

/////////////////////////////////////////////////////////////////////////////
// CGridCtrl overrideables

void CGridCtrl::OnFixedRowClick(CCellID& cell)
{
    if (!IsValid(cell))
        return;

    if (GetHeaderSort())
    {
        CWaitCursor waiter;
        if (cell.col == GetSortColumn())
            SortItems(cell.col, !GetSortAscending());
        else
            SortItems(cell.col, TRUE);
        Invalidate();
    }

	// Did the user click on a fixed column cell (so the cell was within the overlap of
	// fixed row and column cells) - (fix by David Pritchard)
    if (GetFixedColumnSelection())
    {
        if (cell.col < GetFixedColumnCount())
        {
            m_MouseMode = MOUSE_SELECT_ALL;
            OnSelecting(cell);
        }
        else 
        {
            m_MouseMode = MOUSE_SELECT_COL;
            OnSelecting(cell);
        }
    }
}

void CGridCtrl::OnFixedColumnClick(CCellID& cell)
{
    if (!IsValid(cell))
        return;

//    if (m_bListMode && (GetItemState(cell.row, m_nFixedCols) & GVNI_SELECTED))
//    {
//        OnEditCell(cell.row, cell.col, VK_LBUTTON);
//        return;
//    }

	// Did the user click on a fixed row cell (so the cell was within the overlap of
	// fixed row and column cells) - (fix by David Pritchard)
    if (GetFixedRowSelection())
    {
        if (cell.row < GetFixedRowCount())
        {
            m_MouseMode = MOUSE_SELECT_ALL;
            OnSelecting(cell);
        }
        else
        {
            m_MouseMode = MOUSE_SELECT_ROW;
            OnSelecting(cell);
        }
    }
}

// Gets the extent of the text pointed to by str (no CDC needed)
// By default this uses the selected font (which is a bigger font)
CSize CGridCtrl::GetTextExtent(int nRow, int nCol, LPCTSTR str)
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    if (!pCell)
        return CSize(0, 0);
    else
        return pCell->GetTextExtent(str);
}

// virtual
void CGridCtrl::OnEditCell(int nRow, int nCol, CPoint point, UINT nChar)
{
#ifndef GRIDCONTROL_NO_TITLETIPS
    m_TitleTip.Hide();  // hide any titletips
#endif

    // Can we do it?
    CCellID cell(nRow, nCol);
    if (!IsValid(cell) || !IsCellEditable(nRow, nCol))
        return;

    // Can we see what we are doing?
    EnsureVisible(nRow, nCol);
    if (!IsCellVisible(nRow, nCol))
        return;

    // Where, exactly, are we gonna do it??
    CRect rect;
    if (!GetCellRect(cell, rect))
        return;

    // Check we can edit...
    if (SendMessageToParent(nRow, nCol, GVN_BEGINLABELEDIT) >= 0)
    {
		ATLTRACE(_T("Begin edit cell %d, %d"), nRow, nCol);
        // Let's do it...
        CGridCellBase* pCell = GetCell(nRow, nCol);
        if (pCell)
            pCell->Edit(nRow, nCol, rect, point, IDC_INPLACE_CONTROL, nChar);
    }
}

// virtual
void CGridCtrl::EndEditing()
{
    CCellID cell = GetFocusCell();
    if (!IsValid(cell)) return;
    CGridCellBase *pCell = GetCell(cell.row, cell.col);
    if (pCell)
        pCell->EndEdit();
}

// virtual
void CGridCtrl::OnEndEditCell(int nRow, int nCol, CString str)
{
    CString strCurrentText = GetItemText(nRow, nCol);
    if (strCurrentText != str)
    {
        SetItemText(nRow, nCol, str);
        if (ValidateEdit(nRow, nCol, str) && SendMessageToParent(nRow, nCol, GVN_ENDLABELEDIT) != 0L)
        {
            SetModified(TRUE, nRow, nCol);
            RedrawCell(nRow, nCol);
        }
        else
        {
            SetItemText(nRow, nCol, strCurrentText);
        }
    }

    CGridCellBase* pCell = GetCell(nRow, nCol);
	ATLASSERT(pCell);
	if (pCell)
        pCell->OnEndEdit();
}

// If this returns FALSE then the editing isn't allowed
// virtual
BOOL CGridCtrl::ValidateEdit(int nRow, int nCol, LPCTSTR str)
{
    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return FALSE;

    return pCell->ValidateEdit(str);
}

// virtual
CString CGridCtrl::GetItemText(int nRow, int nCol) const
{
    if (nRow < 0 || nRow >= m_nRows || nCol < 0 || nCol >= m_nCols)
        return _T("");

    CGridCellBase* pCell = GetCell(nRow, nCol);
    ATLASSERT(pCell);
    if (!pCell)
        return _T("");

    return pCell->GetText();
}

void CGridCtrl::ResetVirtualOrder()
{
	m_arRowOrder.resize(m_nRows);  
    for (int i = 0; i < m_nRows; i++)
	{
		m_arRowOrder[i] = i;	
	}
}

void CGridCtrl::Reorder(int From, int To)
{
	// Set line From just after Line To
	ATLASSERT(From >= GetFixedRowCount() && To >= GetFixedRowCount()-1 && From < m_nRows && To < m_nRows);
	int Value = m_arRowOrder[From];
	m_arRowOrder.erase(m_arRowOrder.begin()+From);
	int Offset = (From >= To ? 1 : 0);
	m_arRowOrder.insert(m_arRowOrder.begin()+To+Offset, Value);
}
