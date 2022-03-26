#pragma once
#include "ModuleVersion.h"
#include "AboutDlg.h"

#include "GameFieldCtrl.h"
#include "LoadSnapShotPopup.h"
#include "HintCtrl.h"
#include "HiScores.h"
#include "Win32ErrorEx.h"

typedef CWinTraits<WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX, WS_EX_WINDOWEDGE> tMainWndTraits;
enum class CDirection { ZERO, UP, RIGHT, DOWN, LEFT };

class CMainWnd : public CWindowImpl<CMainWnd, CWindow, tMainWndTraits>,
	public CUpdateUI<CMainWnd>,
	public CIdleHandler,
	public CMessageFilter
{
private:
	CIcon m_hIcon;
	CIcon m_hIconSmall;
	CAcceleratorHandle m_hAccelerator;
	CLoadSnapShotPopup m_Popup;

	CGameFieldCtrl m_GameCtrl;
	CHintCtrl m_HintCtrl;
	CMultiPaneStatusBarCtrl m_Status;

	CString m_strFileName;
	CFileDialog *m_pFileDlg;
	CBrushHandle obj;

	CString m_strStatusCursor;
	CString m_strStatusCount;
	CString m_strStatus3Prev;
	CString m_strStatus3;
	time_t m_tStatusTime;

	bool m_bAutoCheck;

	HiScoreTable hsEasy;
	HiScoreTable hsNormal;
	HiScoreTable hsHard;
	HiScoreTable hsInsane;

	const CPoint m_ptMinTrackSize = { 125, 150 };

	CDirection m_Direction;

public:
	BOOL OnIdle() override;
	CMainWnd();
	~CMainWnd();
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void OnDestroy();
	void OnSizing(UINT fwSide, LPRECT pRect);
	void OnExitSizeMove()
	{
		m_Direction = CDirection::ZERO;
	}
	void OnCommand(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/);
	void OnClose()
	{
		DestroyWindow();
	}
	void OnUpdateUndo(BOOL bEnabled)
	{
		UIEnable(ID_EDIT_UNDO, bEnabled);
	}
	void OnUpdateRedo(BOOL bEnabled)
	{
		UIEnable(ID_EDIT_REDO, bEnabled);
	}
	void OnUpdateAutoCheck(BOOL bChecked)
	{
		UISetCheck(ID_GAME_AUTOCHECK, bChecked);
	}
	void OnUpdateCheck(BOOL bEnabled)
	{
		UIEnable(ID_GAME_CHECK, bEnabled);
	}
	void OnSetStatus3(LPCSTR strStatus3)
	{
		m_strStatus3 = strStatus3;
	}

private:
	void ShowException(LPCSTR strMsg, LPCSTR strCaption);
	void ShowException(const CWin32Error &e, LPCSTR strCaption);
	void OnSnapShot(int nID)
	{
		m_GameCtrl.LoadSnapShot(nID - SNAPSHOT_USER_ID);
	}
	void StoreDefault() const;
	bool LoadDefault(CRect *m_rectWindow);
	void StoreTop() const;
	bool LoadTop();
	void OnSolved(int nLevel);
	void OnTimer(UINT_PTR nIDEvent);

	LRESULT OnGetField(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_GameCtrl.m_hWnd;
	}

	LRESULT OnGetHint(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return (LRESULT)m_HintCtrl.m_hWnd;
	}

	void OnPaint(const CDCHandle& hdc)
	{
		CDCHandle dc = hdc;
		PAINTSTRUCT m_ps = {};

		if (!hdc)
			dc = ::BeginPaint(m_hWnd, &m_ps);

		if (!hdc)
			::EndPaint(m_hWnd, &m_ps);
	}

public:
	DECLARE_WND_CLASS_EX("Sudoku2010WndClass", 0, COLOR_BTNFACE)

	BEGIN_UPDATE_UI_MAP(CMainDlg)
		UPDATE_ELEMENT(ID_EDIT_UNDO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_EDIT_REDO, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(0, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(1, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(2, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(3, UPDUI_STATUSBAR)
		UPDATE_ELEMENT(ID_GAME_AUTOCHECK, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_CHECK, UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_GAME_HINT, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZING(OnSizing)
		MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_COMMAND(OnCommand)
		MSG_WM_UPDATEUNDO(OnUpdateUndo)
		MSG_WM_UPDATEREDO(OnUpdateRedo)
		MSG_WM_SETSTATUS3(OnSetStatus3)
		MSG_WM_SOLVED(OnSolved)
		MESSAGE_HANDLER_EX(WM_GETFIELD, OnGetField)
		MESSAGE_HANDLER_EX(WM_GETHINT, OnGetHint)
		CHAIN_MSG_MAP(CUpdateUI<CMainWnd>)
		CHAIN_MSG_MAP_MEMBER(m_GameCtrl)
		CHAIN_MSG_MAP_MEMBER(m_HintCtrl)
		CHAIN_MSG_MAP_MEMBER(m_Status)
	END_MSG_MAP()
};
