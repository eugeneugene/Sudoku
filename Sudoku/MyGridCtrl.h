#pragma once

#include "../WTLGridCtrl/GridCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CMyGridCtrl window

class CMyGridCtrl : public CGridCtrl
{
// Construction
public:
	CMyGridCtrl() : CGridCtrl() {};

// Attributes
public:

// Operations
public:

// Overrides
public:
#if !defined(_WIN32_WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	virtual void PrintHeader(UINT nPage, HDC hDC)
	{
		CDCHandle DC(hDC);
		// Extended Password Generator
		// Copyright
		char *buf = nullptr;
		std::string strVersionString;
		GetVersion(&strVersionString);

		std::string strCopyright;
		if (pModuleVersion->GetValue("LegalCopyright", &buf))
			strCopyright = buf;

		CFont BoldFont;
		LOGFONT lf;

		//create bold font for header and footer
		VERIFY(m_PrinterFont.GetLogFont(&lf));
		lf.lfWeight = FW_BOLD;
		VERIFY(BoldFont.CreateFontIndirect(&lf));

		CFontHandle NormalFont = DC.SelectFont(BoldFont);
		int nPrevBkMode = DC.SetBkMode(TRANSPARENT);

		DC.DrawText(strVersionString.c_str(), -1, &m_rectDraw, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);
		DC.DrawText(strCopyright.c_str(), -1, &m_rectDraw, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER);

		DC.SetBkMode(nPrevBkMode);
		DC.SelectFont(NormalFont);
		BoldFont.DeleteObject();

		// draw ruled-line across top
		DC.SelectStockPen(BLACK_PEN);
		DC.MoveTo(m_rectDraw.left, m_rectDraw.bottom);
		DC.LineTo(m_rectDraw.right, m_rectDraw.bottom);
	}
#endif

// Implementation
public:
	BOOL EditCell(int nRow, int nCol)
	{
#ifndef GRIDCONTROL_NO_TITLETIPS
		m_TitleTip.Hide();  // hide any titletips
#endif

		// Can we do it?
		CCellID cell(nRow, nCol);
		if (!IsValid(cell) || !IsCellEditable(nRow, nCol))
			return FALSE;

		// Can we see what we are doing?
		EnsureVisible(nRow, nCol);
		if (!IsCellVisible(nRow, nCol))
			return FALSE;

		// Where, exactly, are we gonna do it??
		CRect rect;
		if (!GetCellRect(cell, rect))
			return FALSE;

		// Check we can edit...
		if (SendMessageToParent(nRow, nCol, GVN_BEGINLABELEDIT) >= 0)
		{
			// Let's do it...
			CGridCellBase* pCell = GetCell(nRow, nCol);
			if (pCell)
				return pCell->Edit(nRow, nCol, rect, CPoint(-1, -1), IDC_INPLACE_CONTROL, VK_LBUTTON);
		}
		return FALSE;
	}
};
