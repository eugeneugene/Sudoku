#include "stdafx.h"
#include "ShowWinnersDlg.h"

BOOL CShowWinnersDlg::OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
{
	CenterWindow();
	DoDataExchange(DDX_LOAD);

	if (m_Grid.m_hWnd == NULL)
		return FALSE;

	InitGrid();
	m_Grid.SetEditable(FALSE);
	m_Grid.SetFixedRowCount(1);
	m_Grid.SetColumnCount(3);

	GV_ITEM item;
	item.row = item.col = 0;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.strText = "N";
	item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
	m_Grid.SetItem(&item);
	item.col++;
	item.strText = "Name";
	m_Grid.SetItem(&item);
	item.col++;
	item.strText = "Time";
	m_Grid.SetItem(&item);

	CRect r;
	m_Grid.GetClientRect(&r);

	constexpr int c1 = 1;
	constexpr int c2 = 15;
	constexpr int c3 = 4;
	auto w1 = MulDiv(r.Width(), c1, c1 + c2 + c3);
	auto w2 = MulDiv(r.Width(), c2, c1 + c2 + c3);
	auto w3 = MulDiv(r.Width(), c3, c1 + c2 + c3);
	while ((w1 + w2 + w3) > r.Width()) w3--;
	m_Grid.SetColumnWidth(0, w1);
	m_Grid.SetColumnWidth(1, w2);
	m_Grid.SetColumnWidth(2, w3);

	if (m_HiScores)
	{
		int i = 1;
		m_Grid.SetRowCount(m_HiScores->size() + 1);
		for (auto &it = m_HiScores->cbegin(); it != m_HiScores->cend(); it++, i++)
		{
			std::stringstream sstr;
			sstr << i;
			CTimeSpan span(it->m_Time);
			std::stringstream buf;
			buf << std::setfill('0') << std::setw(2) << span.GetTotalHours() << ':' << std::setfill('0') << std::setw(2) << span.GetMinutes() << ':' << std::setfill('0') << std::setw(2) << span.GetSeconds();
			m_Grid.SetItemText(i, 0, sstr.str().c_str());
			m_Grid.SetItemText(i, 1, it->m_HiScoreEntryName.data());
			m_Grid.SetItemText(i, 2, buf.str().c_str());
		}
	}
	return TRUE;
}

void CShowWinnersDlg::InitGrid()
{
	LOGFONT *lf = 0;
	CGridDefaultCell *cell = 0;

#ifdef GRIDMONOFIXEDROW
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(TRUE, FALSE);
	lf = cell->GetFont();
	strncpy(lf->lfFaceName, "Courier New", 32);
	lf->lfHeight = GetFontHeightFromPoints(10);
	cell->SetFont(lf);
#endif
#ifdef GRIDMONOCELL
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(FALSE, FALSE);
	lf = cell->GetFont();
	strncpy_s(lf->lfFaceName, "Courier New", 32);
	lf->lfHeight = GetFontHeightFromPoints(10);
	cell->SetFont(lf);
#endif
#ifdef GRIDBOLDFIXEDROW
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(TRUE, FALSE);
	lf = cell->GetFont();
	lf->lfWeight = FW_BOLD;
	cell->SetFont(lf);
#endif
#ifdef GRIDBOLDCELL
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(FALSE, FALSE);
	lf = cell->GetFont();
	lf->lfWeight = FW_BOLD;
	cell->SetFont(lf);
#endif
#if defined(GRIDFIXEDROWMARGIN) && GRIDFIXEDROWMARGIN != 0
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(TRUE, FALSE);
	cell->SetMargin(GRIDFIXEDROWMARGIN);
#endif
#if defined(GRIDCELLMARGIN) && GRIDCELLMARGIN != 0
	cell = (CGridDefaultCell *)m_Grid.GetDefaultCell(FALSE, FALSE);
	cell->SetMargin(GRIDCELLMARGIN);
#endif

	m_Grid.SetListMode(TRUE);
	m_Grid.SetEditable(FALSE);
	m_Grid.EnableTitleTips(TRUE);
	m_Grid.SetColumnResize(FALSE);
	m_Grid.SetRowResize(FALSE);
	m_Grid.EnableDragAndDrop(TRUE);
}

LONG CShowWinnersDlg::GetFontHeightFromPoints(int points)
{
	HDC dc = ::GetDC(nullptr);
	int res = -MulDiv(points, GetDeviceCaps(dc, LOGPIXELSY), 72);
	::ReleaseDC(nullptr, dc);
	return res;
}
