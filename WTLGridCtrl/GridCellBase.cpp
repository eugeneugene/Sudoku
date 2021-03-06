#include "stdafx.h"
#include "GridCtrl.h"
#include "GridCellBase.h"
#include "gridctype.h"

// GridCellBase.cpp : implementation file
//
// MFC Grid Control - Main grid cell base class
//
// Provides the implementation for the base cell type of the
// grid control. No data is stored (except for state) but default
// implementations of drawing, printingetc provided. MUST be derived
// from to be used.
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
// For use with CGridCtrl v2.22+
//
// History:
// Ken Bertelson - 12 Apr 2000 - Split CGridCell into CGridCell and CGridCellBase
// C Maunder     - 19 May 2000 - Fixed sort arrow drawing (Ivan Ilinov)
// C Maunder     - 29 Aug 2000 - operator= checks for NULL font before setting (Martin Richter)
// C Maunder     - 15 Oct 2000 - GetTextExtent fixed (Martin Richter)
// C Maunder     -  1 Jan 2001 - Added ValidateEdit
// Yogurt        - 13 Mar 2004 - GetCellExtent fixed
//
// NOTES: Each grid cell should take care of it's own drawing, though the Draw()
//        method takes an "erase background" paramter that is called if the grid
//        decides to draw the entire grid background in on hit. Certain ambient
//        properties such as the default font to use, and hints on how to draw
//        fixed cells should be fetched from the parent grid. The grid trusts the
//        cells will behave in a certain way, and the cells trust the grid will
//        supply accurate information.
//        
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// GridCellBase

CGridCellBase::CGridCellBase()
{
    Reset();
}

CGridCellBase::~CGridCellBase()
{
}

/////////////////////////////////////////////////////////////////////////////
// GridCellBase Operations

void CGridCellBase::Reset()
{
    m_nState  = 0;
	m_ctype   = 0;
}

void CGridCellBase::operator=(const CGridCellBase& cell)
{
	if (this == &cell) return;

    SetGrid(cell.GetGrid());    // do first in case of dependencies

    SetText(cell.GetText());
    SetImage(cell.GetImage());
    SetData(cell.GetData());
    SetState(cell.GetState());
    SetFormat(cell.GetFormat());
    SetTextClr(cell.GetTextClr());
    SetBackClr(cell.GetBackClr());
    SetFont(cell.IsDefaultFont()? nullptr : cell.GetFont());
    SetMargin(cell.GetMargin());
}

/////////////////////////////////////////////////////////////////////////////
// CGridCellBase Attributes

// Returns a pointer to a cell that holds default values for this particular type of cell
CGridCellBase* CGridCellBase::GetDefaultCell() const
{
    if (GetGrid())
        return GetGrid()->GetDefaultCell(IsFixedRow(), IsFixedCol());
    return nullptr;
}


/////////////////////////////////////////////////////////////////////////////
// CGridCellBase Operations

// EFW - Various changes to make it draw cells better when using alternate
// color schemes.  Also removed printing references as that's now done
// by PrintCell() and fixed the sort marker so that it doesn't draw out
// of bounds.
BOOL CGridCellBase::Draw(CDCHandle DC, int nRow, int nCol, CRect rect,  BOOL bEraseBkgnd /*=TRUE*/)
{
    // Note - all through this function we totally brutalise 'rect'. Do not
    // depend on it's value being that which was passed in.

    CGridCtrl* pGrid = GetGrid();
    ATLASSERT(pGrid);

    if (!pGrid || DC.IsNull())
        return FALSE;

    if(rect.Width() <= 0 || rect.Height() <= 0)  // prevents imagelist item from drawing even
        return FALSE;                             //  though cell is hidden

    //TRACE3("Drawing %scell %d, %d\n", IsFixed()? _T("Fixed ") : _T(""), nRow, nCol);

    int nSavedDC = DC.SaveDC();
    DC.SetBkMode(TRANSPARENT);

    // Get the default cell implementation for this kind of cell. We use it if this cell
    // has anything marked as "default"
    CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
    if (!pDefaultCell)
        return FALSE;

    // Set up text and background colours
    COLORREF TextClr, TextBkClr;

    TextClr = (GetTextClr() == CLR_DEFAULT)? pDefaultCell->GetTextClr() : GetTextClr();
    if (GetBackClr() == CLR_DEFAULT)
        TextBkClr = pDefaultCell->GetBackClr();
    else
    {
        bEraseBkgnd = TRUE;
        TextBkClr = GetBackClr();
    }

    // Draw cell background and highlighting (if necessary)
    if (IsFocused() || IsDropHighlighted())
    {
        // Always draw even in list mode so that we can tell where the
        // cursor is at.  Use the highlight colors though.
        if(IsSelected())
        {
            TextBkClr = ::GetSysColor(COLOR_HIGHLIGHT);
            TextClr = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
            bEraseBkgnd = TRUE;
        }

        rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
        if (bEraseBkgnd)
        {
			CBrush brush;
			brush.CreateSolidBrush(TextBkClr);
			ATLASSERT(!brush.IsNull());
			DC.FillRect(&rect, brush);
        }

        // Don't adjust frame rect if no grid lines so that the
        // whole cell is enclosed.
        if(pGrid->GetGridLines() != GVL_NONE)
        {
            rect.right--;
            rect.bottom--;
        }

        if (pGrid->GetFrameFocusCell())
        {
                // Use same color as text to outline the cell so that it shows
                // up if the background is black.
			CBrush brush;
			brush.CreateSolidBrush(TextClr);
			ATLASSERT(!brush.IsNull());
			DC.FrameRect(&rect, brush);
        }
        DC.SetTextColor(TextClr);

        // Adjust rect after frame draw if no grid lines
        if(pGrid->GetGridLines() == GVL_NONE)
        {
            rect.right--;
            rect.bottom--;
        }

		//rect.DeflateRect(0,1,1,1);  - Removed by Yogurt
    }
    else if (IsSelected())
    {
        rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
        DC.FillSolidRect(rect, ::GetSysColor(COLOR_HIGHLIGHT));
        rect.right--; rect.bottom--;
        DC.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        if (bEraseBkgnd)
        {
            rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
            CBrush brush;
			brush.CreateSolidBrush(TextBkClr);
			ATLASSERT(!brush.IsNull());
            DC.FillRect(&rect, brush);
            rect.right--; rect.bottom--;
        }
        DC.SetTextColor(TextClr);
    }

    // Draw lines only when wanted
    if (IsFixed() && pGrid->GetGridLines() != GVL_NONE)
    {
        CCellID FocusCell = pGrid->GetFocusCell();

        // As above, always show current location even in list mode so
        // that we know where the cursor is at.
        BOOL bHiliteFixed = pGrid->GetTrackFocusCell() && pGrid->IsValid(FocusCell) &&
                            (FocusCell.row == nRow || FocusCell.col == nCol);

        // If this fixed cell is on the same row/col as the focus cell,
        // highlight it.
        if (bHiliteFixed)
        {
            rect.right++; rect.bottom++;
            DC.DrawEdge(rect, BDR_SUNKENINNER /*EDGE_RAISED*/, BF_RECT);
            rect.DeflateRect(1,1);
        }
        else
        {
            CPen lightpen, darkpen;
			lightpen.CreatePen(PS_SOLID, 1,  ::GetSysColor(COLOR_3DHIGHLIGHT));
			darkpen.CreatePen(PS_SOLID,  1, ::GetSysColor(COLOR_3DDKSHADOW));
			CPenHandle OldPen = DC.GetCurrentPen();

            DC.SelectPen(lightpen);
            DC.MoveTo(rect.right, rect.top);
            DC.LineTo(rect.left, rect.top);
            DC.LineTo(rect.left, rect.bottom);

            DC.SelectPen(darkpen);
            DC.MoveTo(rect.right, rect.top);
            DC.LineTo(rect.right, rect.bottom);
            DC.LineTo(rect.left, rect.bottom);

            DC.SelectPen(OldPen);
            rect.DeflateRect(1,1);
        }
    }

    // Draw Text and image
	CFontHandle Font = GetFontObject();
	ATLASSERT(!Font.IsNull());
	if (!Font.IsNull())
		DC.SelectFont(Font);
	
    //rect.DeflateRect(GetMargin(), 0); - changed by Yogurt
    rect.DeflateRect(GetMargin(), GetMargin());    
    rect.right++;    
    rect.bottom++;

    if (pGrid->GetImageList() && GetImage() >= 0)
    {
        IMAGEINFO Info;
        if (pGrid->GetImageList()->GetImageInfo(GetImage(), &Info))
        {
            //  would like to use a clipping region but seems to have issue
            //  working with CMemDC directly.  Instead, don't display image
            //  if any part of it cut-off
            //
            // CRgn rgn;
            // rgn.CreateRectRgnIndirect(rect);
            // pDC->SelectClipRgn(&rgn);
            // rgn.DeleteObject();

            /*
            // removed by Yogurt
            int nImageWidth = Info.rcImage.right-Info.rcImage.left+1;
            int nImageHeight = Info.rcImage.bottom-Info.rcImage.top+1;
            if(nImageWidth + rect.left <= rect.right + (int)(2*GetMargin())
                && nImageHeight + rect.top <= rect.bottom + (int)(2*GetMargin()))
            {
                pGrid->GetImageList()->Draw(pDC, GetImage(), rect.TopLeft(), ILD_NORMAL);
            }
            */
            // Added by Yogurt
            int nImageWidth = Info.rcImage.right-Info.rcImage.left;            
            int nImageHeight = Info.rcImage.bottom-Info.rcImage.top;            
            if ((nImageWidth + rect.left <= rect.right) && (nImageHeight + rect.top <= rect.bottom))                
                pGrid->GetImageList()->Draw(DC, GetImage(), rect.TopLeft(), ILD_NORMAL);

            //rect.left += nImageWidth+GetMargin();
        }
    }

    // Draw sort arrow
    if (pGrid->GetSortColumn() == nCol && nRow == 0)
    {
		CSize size;
		BOOL bRes = DC.GetTextExtent(_T("M"), 1, &size);
		ATLASSERT(FALSE != bRes);
        int nOffset = 2;

        // Base the size of the triangle on the smaller of the column
        // height or text height with a slight offset top and bottom.
        // Otherwise, it can get drawn outside the bounds of the cell.
        size.cy -= (nOffset * 2);

        if (size.cy >= rect.Height())
            size.cy = rect.Height() - (nOffset * 2);

        size.cx = size.cy;      // Make the dimensions square

        // Kludge for vertical text
        BOOL bVertical = (GetFont()->lfEscapement == 900);

        // Only draw if it'll fit!
        //if (size.cx + rect.left < rect.right + (int)(2*GetMargin())) - changed / Yogurt
        if (size.cx + rect.left < rect.right)
        {
            int nTriangleBase = rect.bottom - nOffset - size.cy;    // Triangle bottom right
            //int nTriangleBase = (rect.top + rect.bottom - size.cy)/2; // Triangle middle right
            //int nTriangleBase = rect.top + nOffset;                 // Triangle top right

            //int nTriangleLeft = rect.right - size.cx;                 // Triangle RHS
            //int nTriangleLeft = (rect.right + rect.left - size.cx)/2; // Triangle middle
            //int nTriangleLeft = rect.left;                            // Triangle LHS

            int nTriangleLeft;
            if (bVertical)
                nTriangleLeft = (rect.right + rect.left - size.cx)/2; // Triangle middle
            else
                nTriangleLeft = rect.right - size.cx;               // Triangle RHS

            CPen penShadow;
			penShadow.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_3DSHADOW));
            CPen penLight;
			penLight.CreatePen(PS_SOLID, 0, ::GetSysColor(COLOR_3DHILIGHT));
            if (pGrid->GetSortAscending())
            {
                // Draw triangle pointing upwards
                CPenHandle OldPen = DC.SelectPen(penLight);
                DC.MoveTo(nTriangleLeft + 1, nTriangleBase + size.cy + 1);
                DC.LineTo(nTriangleLeft + (size.cx / 2) + 1, nTriangleBase + 1);
                DC.LineTo(nTriangleLeft + size.cx + 1, nTriangleBase + size.cy + 1);
                DC.LineTo(nTriangleLeft + 1, nTriangleBase + size.cy + 1);

                DC.SelectPen(penShadow);
                DC.MoveTo(nTriangleLeft, nTriangleBase + size.cy);
                DC.LineTo(nTriangleLeft + (size.cx / 2), nTriangleBase);
                DC.LineTo(nTriangleLeft + size.cx, nTriangleBase + size.cy);
                DC.LineTo(nTriangleLeft, nTriangleBase + size.cy);
                DC.SelectPen(OldPen);
            }
            else
            {
                // Draw triangle pointing downwards
                CPenHandle OldPen = DC.SelectPen(penLight);
                DC.MoveTo(nTriangleLeft + 1, nTriangleBase + 1);
                DC.LineTo(nTriangleLeft + (size.cx / 2) + 1, nTriangleBase + size.cy + 1);
                DC.LineTo(nTriangleLeft + size.cx + 1, nTriangleBase + 1);
                DC.LineTo(nTriangleLeft + 1, nTriangleBase + 1);
    
                DC.SelectPen(penShadow);
                DC.MoveTo(nTriangleLeft, nTriangleBase);
                DC.LineTo(nTriangleLeft + (size.cx / 2), nTriangleBase + size.cy);
                DC.LineTo(nTriangleLeft + size.cx, nTriangleBase);
                DC.LineTo(nTriangleLeft, nTriangleBase);
                DC.SelectPen(OldPen);
            }
            
            if (!bVertical)
                rect.right -= size.cy;
        }
    }

    // We want to see '&' characters so use DT_NOPREFIX
    GetTextRect(rect);
    rect.right++;    
    rect.bottom++;

    DC.DrawText(GetText(), -1, rect, GetFormat() | DT_NOPREFIX);

    DC.RestoreDC(nSavedDC);

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCellBase Mouse and Cursor events

// Not yet implemented
void CGridCellBase::OnMouseEnter()
{
    //AtlTrace(_T("Mouse entered cell\n"));
}

void CGridCellBase::OnMouseOver()
{
    //AtlTrace(_T("Mouse over cell\n"));
}

// Not Yet Implemented
void CGridCellBase::OnMouseLeave()
{
    //AtlTrace(_T("Mouse left cell\n"));
}

void CGridCellBase::OnClick(CPoint PointCellRelative)
{
    //AtlTrace(_T("Mouse Left btn up in cell at x=%i y=%i\n"), PointCellRelative.x, PointCellRelative.y);
}

void CGridCellBase::OnClickDown(CPoint PointCellRelative)
{
	//AtlTrace(_T("Mouse Left btn down in cell at x=%i y=%i\n"), PointCellRelative.x, PointCellRelative.y);
}

void CGridCellBase::OnRClick(CPoint PointCellRelative)
{
    //AtlTrace(_T("Mouse right-clicked in cell at x=%i y=%i\n"), PointCellRelative.x, PointCellRelative.y);
}

void CGridCellBase::OnDblClick(CPoint PointCellRelative)
{
    //AtlTrace(_T("Mouse double-clicked in cell at x=%i y=%i\n"), PointCellRelative.x, PointCellRelative.y);
}

// Return TRUE if you set the cursor
BOOL CGridCellBase::OnSetCursor()
{
#ifndef _WIN32_WCE_NO_CURSOR
    SetCursor(AtlLoadSysCursor(IDC_ARROW));
#endif
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCellBase editing

void CGridCellBase::OnEndEdit() 
{
	ATLASSERT(FALSE); 
}

BOOL CGridCellBase::ValidateEdit(LPCTSTR str)
{
	for (size_t i = 0; i < _tcslen(str); i++)
		if (!_IsCType(str[i], GetCType(), GridLocale))
			return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CGridCellBase Sizing

BOOL CGridCellBase::GetTextRect(LPRECT pRect)  // i/o:  i=dims of cell rect; o=dims of text rect
{
    if (GetImage() >= 0)
    {
        IMAGEINFO Info;

        CGridCtrl* pGrid = GetGrid();
        CImageList* pImageList = pGrid->GetImageList();
        
        if (pImageList && pImageList->GetImageInfo(GetImage(), &Info))
        {
            int nImageWidth = Info.rcImage.right-Info.rcImage.left+1;
            pRect->left += nImageWidth + GetMargin();
        }
    }

    return TRUE;
}

// By default this uses the selected font (which is a bigger font)
CSize CGridCellBase::GetTextExtent(LPCTSTR szText, CDCHandle DC /*= CDCHandle()*/)
{
    CGridCtrl* pGrid = GetGrid();
    ATLASSERT(pGrid);

    BOOL bReleaseDC = FALSE;
    if (DC.IsNull() || szText == nullptr)
    {
        if (szText)
			DC = pGrid->GetDC();
        if (DC.IsNull() || szText == nullptr) 
        {
            CGridDefaultCell* pDefCell = (CGridDefaultCell*) GetDefaultCell();
            ATLASSERT(pDefCell);
            return CSize(pDefCell->GetWidth(), pDefCell->GetHeight());
        }
        bReleaseDC = TRUE;
    }

    CFontHandle OldFont, Font = GetFontObject();
    if (!Font.IsNull())
        OldFont = DC.SelectFont(Font);

    CSize size;
    int nFormat = GetFormat();

    // If the cell is a multiline cell, then use the width of the cell
    // to get the height
    if ((nFormat & DT_WORDBREAK) && !(nFormat & DT_SINGLELINE))
    {
        CString str = szText;
        int nMaxWidth = 0;
        while (TRUE)
        {
            int nPos = str.Find(_T('\n'));
            CString TempStr = (nPos < 0)? str : str.Left(nPos);
			CSize size;
			BOOL bRes = DC.GetTextExtent(TempStr, (int) _tcslen(TempStr), &size);
			ATLASSERT(bRes);
            int nTempWidth = size.cx;
            if (nTempWidth > nMaxWidth)
                nMaxWidth = nTempWidth;

            if (nPos < 0)
                break;
            str = str.Mid(nPos + 1);    // Bug fix by Thomas Steinborn
        }
        
        CRect rect;
        rect.SetRect(0,0, nMaxWidth+1, 0);
        DC.DrawText(szText, -1, rect, nFormat | DT_CALCRECT);
        size = rect.Size();
    }
    else
        DC.GetTextExtent(szText, -1, &size);

    // Removed by Yogurt
    //TEXTMETRIC tm;
    //pDC->GetTextMetrics(&tm);
    //size.cx += (tm.tmOverhang);

    if (!OldFont.IsNull())
        DC.SelectFont(OldFont);
    
    size += CSize(2*GetMargin(), 2*GetMargin());

    // Kludge for vertical text
    LOGFONT *pLF = GetFont();
    if (pLF->lfEscapement == 900 || pLF->lfEscapement == -900)
    {
        int nTemp = size.cx;
        size.cx = size.cy;
        size.cy = nTemp;
        size += CSize(0, 4*GetMargin());
    }
    
    if (bReleaseDC)
        pGrid->ReleaseDC(DC);

    return size;
}


CSize CGridCellBase::GetCellExtent(CDCHandle DC)
{    
    CSize size = GetTextExtent(GetText(), DC);    
    CSize ImageSize(0,0);    
    
    int nImage = GetImage();    
    if (nImage >= 0)    
    {        
        CGridCtrl* pGrid = GetGrid();        
        ATLASSERT(pGrid);        
        IMAGEINFO Info;        
        if (pGrid->GetImageList() && pGrid->GetImageList()->GetImageInfo(nImage, &Info))         
        {            
            ImageSize = CSize(Info.rcImage.right-Info.rcImage.left,                                 
                Info.rcImage.bottom-Info.rcImage.top);            
            if (size.cx > 2*(int)GetMargin ())                
                ImageSize.cx += GetMargin();            
            ImageSize.cy += 2*(int)GetMargin ();        
        }    
    }    
    size.cx += ImageSize.cx + 1;    
    size.cy = __max(size.cy, ImageSize.cy) + 1;
    if (IsFixed())    
    {        
        size.cx++;        
        size.cy++;    
    }    
    return size;
}

// EFW - Added to print cells so that grids that use different colors are
// printed correctly.
BOOL CGridCellBase::PrintCell(CDCHandle DC, int /*nRow*/, int /*nCol*/, CRect rect)
{
#if defined(_WIN32_WCE_NO_PRINTING) || defined(GRIDCONTROL_NO_PRINTING)
    return FALSE;
#else
    COLORREF crFG, crBG;
    GV_ITEM Item;

    CGridCtrl* pGrid = GetGrid();
    if (!pGrid || DC.IsNull())
        return FALSE;

    if(rect.Width() <= 0
        || rect.Height() <= 0)  // prevents imagelist item from drawing even
        return FALSE;           //  though cell is hidden

    int nSavedDC = DC.SaveDC();

    DC.SetBkMode(TRANSPARENT);

    if (pGrid->GetShadedPrintOut())
    {
        // Get the default cell implementation for this kind of cell. We use it if this cell
        // has anything marked as "default"
        CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
        if (!pDefaultCell)
            return FALSE;

        // Use custom color if it doesn't match the default color and the
        // default grid background color.  If not, leave it alone.
        if(IsFixed())
            crBG = (GetBackClr() != CLR_DEFAULT) ? GetBackClr() : pDefaultCell->GetBackClr();
        else
            crBG = (GetBackClr() != CLR_DEFAULT && GetBackClr() != pDefaultCell->GetBackClr()) ?
                GetBackClr() : CLR_DEFAULT;

        // Use custom color if the background is different or if it doesn't
        // match the default color and the default grid text color.  
        if(IsFixed())
            crFG = (GetBackClr() != CLR_DEFAULT) ? GetTextClr() : pDefaultCell->GetTextClr();
        else
            crFG = (GetBackClr() != CLR_DEFAULT) ? GetTextClr() : pDefaultCell->GetTextClr();

        // If not printing on a color printer, adjust the foreground color
        // to a gray scale if the background color isn't used so that all
        // colors will be visible.  If not, some colors turn to solid black
        // or white when printed and may not show up.  This may be caused by
        // coarse dithering by the printer driver too (see image note below).
        if(DC.GetDeviceCaps(NUMCOLORS) == 2 && crBG == CLR_DEFAULT)
            crFG = RGB(GetRValue(crFG) * 0.30, GetGValue(crFG) * 0.59,
                GetBValue(crFG) * 0.11);

        // Only erase the background if the color is not the default
        // grid background color.
        if(crBG != CLR_DEFAULT)
        {
            CBrush brush;
			brush.CreateSolidBrush(crBG);
            rect.right++; rect.bottom++;
            DC.FillRect(rect, brush);
            rect.right--; rect.bottom--;
        }
    }
    else
    {
        crBG = CLR_DEFAULT;
        crFG = RGB(0, 0, 0);
    }

    DC.SetTextColor(crFG);

    CFontHandle Font = GetFontObject();
    if (!Font.IsNull())
        DC.SelectFont(Font);

    /*
    // ***************************************************
    // Disabled - if you need this functionality then you'll need to rewrite.
    // Create the appropriate font and select into DC.
    CFont Font;
    // Bold the fixed cells if not shading the print out.  Use italic
    // font it it is enabled.
    const LOGFONT* plfFont = GetFont();
    if(IsFixed() && !pGrid->GetShadedPrintOut())
    {
        Font.CreateFont(plfFont->lfHeight, 0, 0, 0, FW_BOLD, plfFont->lfItalic, 0, 0,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
#ifndef _WIN32_WCE
            PROOF_QUALITY,
#else
            DEFAULT_QUALITY,
#endif
            VARIABLE_PITCH | FF_SWISS, plfFont->lfFaceName);
    }
    else
        Font.CreateFontIndirect(plfFont);

    pDC->SelectObject(&Font);
    // ***************************************************
    */

    // Draw lines only when wanted on fixed cells.  Normal cell grid lines
    // are handled in OnPrint.
    if(pGrid->GetGridLines() != GVL_NONE && IsFixed())
    {
        CPen lightpen, darkpen;
		lightpen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT));
        darkpen.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
		CPenHandle OldPen(DC.GetCurrentPen());

        DC.SelectPen(lightpen);
        DC.MoveTo(rect.right, rect.top);
        DC.LineTo(rect.left, rect.top);
        DC.LineTo(rect.left, rect.bottom);
		
		DC.SelectPen(darkpen);
        DC.MoveTo(rect.right, rect.top);
        DC.LineTo(rect.right, rect.bottom);
        DC.LineTo(rect.left, rect.bottom);

        rect.DeflateRect(1,1);
        DC.SelectPen(OldPen);
    }

    rect.DeflateRect(GetMargin(), 0);

    if(pGrid->GetImageList() && GetImage() >= 0)
    {
        // NOTE: If your printed images look like fuzzy garbage, check the
        //       settings on your printer driver.  If it's using coarse
        //       dithering and/or vector graphics, they may print wrong.
        //       Changing to fine dithering and raster graphics makes them
        //       print properly.  My HP 4L had that problem.

        IMAGEINFO Info;
        if(pGrid->GetImageList()->GetImageInfo(GetImage(), &Info))
        {
            int nImageWidth = Info.rcImage.right-Info.rcImage.left;
            pGrid->GetImageList()->Draw(DC, GetImage(), rect.TopLeft(), ILD_NORMAL);
            rect.left += nImageWidth+GetMargin();
        }
    }

    // Draw without clipping so as not to lose text when printed for real
	// DT_NOCLIP removed 01.01.01. Slower, but who cares - we are printing!
    DrawText(DC, GetText(), -1, rect,
        GetFormat() | /*DT_NOCLIP | */ DT_NOPREFIX);

    DC.RestoreDC(nSavedDC);

    return TRUE;
#endif
}

/*****************************************************************************
Callable by derived classes, only
*****************************************************************************/
LRESULT CGridCellBase::SendMessageToParent(int nRow, int nCol, int nMessage)
{
    CGridCtrl* pGrid = GetGrid();
    if(pGrid)
        return pGrid->SendMessageToParent(nRow, nCol, nMessage);
    else
        return 0;
}