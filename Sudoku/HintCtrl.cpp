#include "stdafx.h"
#include "HintCtrl.h"
#include "MainWnd.h"

void CHintCtrl::OnLButtonDown(UINT /*nFlags*/, const POINT &point)
{
	CRect rect;
	GetClientRect(rect);
	rect.bottom -= 2;
	rect.right -= 2;

	CPoint clt = point;
	::MapWindowPoints(GetParent(), m_hWnd, &clt, 1);
	if (!rect.PtInRect(clt))
	{
		SetMsgHandled(FALSE);
		return;
	}
	const int x = int(double(clt.x) / double(rect.right) * 9.0) + 1;
	if (x < 1 || x > 9)
	{
		SetMsgHandled(FALSE);
		return;
	}
	CWindow Ctrl;
	CWindow Parent = GetParent();
	if (!Parent.IsWindow())
		return;
	Ctrl.m_hWnd = (HWND)Parent.SendMessage(WM_GETFIELD, NULL, NULL);
	Ctrl.IsWindow() && Ctrl.SendMessage(WM_HINTDOWN, 0, static_cast<LPARAM>(x));
}

void CHintCtrl::OnUpdateCtrl(WPARAM nVal, LPARAM nHypo)
{
	if (-1 == nVal && -1 == nHypo)
		m_bInit = false;
	else
	{
		m_Cell.m_nVal = static_cast<CELLVAL>(LOWORD(nVal));
		m_Cell.m_nHypo = static_cast<CELLHYPO>(nHypo);
		m_nMode = HIWORD(nVal);
		m_bInit = true;
	}
	Refresh();
}

void CHintCtrl::OnPaint(CDCHandle /*dc*/)
{
	CRect rect;
	GetClientRect(rect);

#if defined(_DEBUG_PAINT)
	CPaintDC dc(m_hWnd);
#else
	const CPaintDC paintdc(m_hWnd);
	CMemoryDC dc(paintdc, rect);
#endif

	CPen pen;
	CBrush brush;
	const double b9 = rect.Width() / 9.0;

	brush.CreateSolidBrush(BACKGRND);
	CBrushHandle oldbrush = dc.SelectBrush(brush);
	pen.CreatePen(PS_SOLID, 1, THINLINE);
	CPenHandle oldpen = dc.SelectPen(pen);

	dc.Rectangle(rect);
	for (auto i = 1.0; i < 9.0; i++)		// Вертикальные линии
	{
		int x = rect.left + int(b9 * i);
		dc.MoveTo(x, rect.top);
		dc.LineTo(x, rect.bottom);
	}

	dc.SelectBrush(oldbrush);
	brush.DeleteObject();

	const int val = m_Cell.m_nVal;
	if (m_bInit && !(0 != val && SDK_HYP == m_nMode))	// Если режим предположения, а значение уже введено, то ничего рисовать не нужно
	{
		CLogFont biglogfont;
		CFont bigfont;

		biglogfont.lfHeight = rect.Height();
		strcpy_s(static_cast<char *>(biglogfont.lfFaceName), LF_FACESIZE, "Arial");
		bigfont.CreateFontIndirect(&biglogfont);
		dc.SetBkMode(TRANSPARENT);
		CFontHandle oldfont = dc.SelectFont(bigfont);

		if (m_Cell.ReadOnly())	// Ничего вводить нельзя, один вариант
		{
			CRect rect1(rect);
			rect1.left = rect.left + int(b9 * double(val - 1));
			rect1.right = rect1.left + int(b9);
			dc.SetTextColor(DIGIT_RO);
			char text[2];
			text[0] = Num2Char(val);
			text[1] = '\0';
			dc.DrawText(static_cast<LPCSTR>(text), 1, &rect1, DT_CENTER | DT_VCENTER);
		}
		else
		{
			if (SDK_DEC == m_nMode)
			{
				for (int i = 0; i < 9; i++)
				{
					int v = i + 1;
					CRect rect1(rect);
					rect1.left = rect.left + int(b9 * double(i));
					rect1.right = rect1.left + int(b9);
					if (m_Cell.GetHypothesis(v))
						dc.SetTextColor(DIGIT_HYP);
					else
						dc.SetTextColor(DIGIT_DEC);
					char text[2];
					text[0] = Num2Char(v);
					text[1] = '\0';
					dc.DrawText(static_cast<LPCSTR>(text), 1, &rect1, DT_CENTER | DT_VCENTER);
				}
			}
			else
			{
				for (int i = 0; i < 9; i++)
				{
					int v = i + 1;
					CRect rect1(rect);
					rect1.left = rect.left + int(b9 * double(i));
					rect1.right = rect1.left + int(b9);
					dc.SetTextColor(DIGIT_HYP);
					char text[2];
					text[0] = Num2Char(v);
					text[1] = '\0';
					dc.DrawText(static_cast<LPCSTR>(text), 1, &rect1, DT_CENTER | DT_VCENTER);
				}
			}
		}
		dc.SelectFont(oldfont);
	}
	dc.SelectPen(oldpen);
	pen.DeleteObject();

	//SetMsgHandled(false);
}
