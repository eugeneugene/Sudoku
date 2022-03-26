#pragma once
#include <atlwin.h>
#include "GameField.h"

class CHintCtrl : public CWindowImpl<CHintCtrl, CStatic>
{
private:
	CCell m_Cell;
	int m_nMode;
	bool m_bInit;

public:
	CHintCtrl() : m_nMode(0), m_bInit(false), m_bMsgHandled(FALSE) {}
	void OnLButtonDown(UINT /*nFlags*/, const POINT &point);
	void OnUpdateCtrl(WPARAM nVal, LPARAM nHypo);
	void OnPaint(CDCHandle /*dc*/);
private:
	void Refresh()
	{
		InvalidateRect(NULL, TRUE);
	}
public:
	DECLARE_WND_SUPERCLASS(NULL, CStatic::GetWndClassName())

	BEGIN_MSG_MAP_EX(CHintCtrl)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_UPDATEHINTCTRL(OnUpdateCtrl)
	END_MSG_MAP()
};
