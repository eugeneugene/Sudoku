#pragma once
#include "GameFieldData.h"
#include "task.h"

class CGameFieldCtrl : public CWindowImpl<CGameFieldCtrl, CStatic>, public CGameFieldSerialData
{
private:
	UPOINT m_Cursor;	// [1 .. 9]
	int m_nMode;
	CFieldError m_FieldError;
	bool m_bAutoCheck;
	bool m_bLButtonDown;
	bool m_bRButtonDown;
	bool m_bSolved;
	bool m_bCaptured;

	void UpdateHintWindow() const;
	void AddUndoRedo(const CCell &cell, const UPOINT &Point);
	void AddUndo(const CCell &cell, const UPOINT &Point);
	void AddRedo(const CCell &cell, const UPOINT &Point);
	void UpdateUndo() const;
	void UpdateRedo() const;
	void ReleaseUndo();
	void ReleaseRedo();
	void Hint(const UPOINT &point);
	void HintAround(const UPOINT &point);
	void SetStatus3(LPCSTR str) const;
	CCounters counters;

public:
	CGameFieldCtrl() : m_Cursor(InvalidCursor), m_nMode(SDK_DEC), m_bAutoCheck(false),
		m_bLButtonDown(false), m_bRButtonDown(false), m_bSolved(false), m_bCaptured(false), m_bMsgHandled(false) {}
	~CGameFieldCtrl()
	{
		m_Undo.clear();
		m_Redo.clear();
		m_SnapShots.clear();
	}

	bool isDirty() const { return m_bDirty; }
	int GetCursorX() const { return m_Cursor.x; }
	int GetCursorY() const { return m_Cursor.y; }
	bool AutoCheck() const { return m_bAutoCheck; }
	bool AutoCheck(bool bAutoCheck) { return m_bAutoCheck = bAutoCheck; }
	bool CanUndo() const { return !m_Undo.empty(); }
	bool CanRedo() const { return !m_Redo.empty(); }
	unsigned Count()
	{
		return counters.total;
	}
	unsigned ReadOnly()
	{
		return counters.readonly;
	}

	void NewEasy();
	void NewNormal();
	void NewHard();
	void NewInsane();
	bool Check();
	void Init();
	void Reset();
	void Reset(const CTask &task);
	void OnChanged();
	bool MakeSnapShot(const char* name, time_t time);
	void LoadSnapShot(unsigned nSnapShot);
	void GetSnapNames(CSnapShotNames *names) const;
	time_t GetTimeSpan() const;
	void Store(const char *strFileName);
	void Load(const char *strFileName);
	void Undo();
	void Redo();
	void Hint();
	int Solve();
	void HintAll();

private:
	void Refresh()
	{
		InvalidateRect(NULL, TRUE);
	}
	bool IsFull() const
	{
		return (81 == counters.total) ? true : false;
	}
	void SetSolved(bool bSolved = true);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, POINT point);
	void OnLButtonUp(UINT nFlags, POINT point);
	void OnLButtonDblClk(UINT nFlags, POINT /*point*/);
	void OnRButtonDown(UINT nFlags, POINT point);
	void OnRButtonUp(UINT nFlags, POINT point);
	void OnRButtonDblClk(UINT nFlags, POINT /*point*/);
	void OnMouseMove(UINT nFlags, POINT point);
	bool TrackCursor(const POINT &point);
	void OnPaint(CDCHandle /*dc*/);
	void OnHintDown(CELLVAL nHint);
	void SetCapture();
	void ReleaseCapture();
	bool IsValidPoint(const POINT& point, _Out_ UPOINT *Cursor = nullptr) const;
	bool IsValidCursor() const
	{
		return m_Cursor.x > 0U && m_Cursor.y > 0U && m_Cursor.x < 10U && m_Cursor.y < 10U;
	}
	static bool IsValidCursor(const UPOINT &Cursor)
	{
		return Cursor.x > 0U && Cursor.y > 0U && Cursor.x < 10U && Cursor.y < 10U;
	}
	bool IsSHIFTpressed() { return bool(GetKeyState(VK_SHIFT) & 128); }
	bool IsCTRLpressed()  { return bool(GetKeyState(VK_CONTROL) & 128); }
public:
	DECLARE_WND_SUPERCLASS(NULL, CStatic::GetWndClassName())

	BEGIN_MSG_MAP(CGameFieldCtrl)
		// Карта сообщений
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_RBUTTONUP(OnRButtonUp)
		MSG_WM_RBUTTONDBLCLK(OnRButtonDblClk)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_HINTDOWN(OnHintDown)
	END_MSG_MAP()
};
