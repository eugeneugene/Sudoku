#include "stdafx.h"
#include "GameFieldCtrl.h"
#include "MyTaskDialog.h"
#include "Win32ErrorEx.h"

void CGameFieldCtrl::Load(const char* strFileName)
{
	std::ifstream ifs(strFileName, std::ios_base::in | std::ios_base::binary);
	if (ifs.bad())
		throw std::runtime_error("Can't read from file");
	boost::iostreams::filtering_istreambuf fis;
	fis.push(boost::iostreams::gzip_decompressor());
	fis.push(ifs);
	boost::archive::binary_iarchive bia(fis);
	bia >> boost::serialization::base_object<CGameFieldFileData>(*this);
	ReleaseUndo();
	ReleaseRedo();
	m_Cursor = InvalidCursor;
	OnChanged();
}

void CGameFieldCtrl::Store(const char* strFileName)
{
	std::ofstream ofs(strFileName, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if (!ofs.bad())
		throw std::runtime_error("Can't store to file");
	boost::iostreams::filtering_ostreambuf fos;
	fos.push(boost::iostreams::gzip_compressor());
	fos.push(ofs);

	boost::archive::binary_oarchive boa(fos);
	boa << boost::serialization::base_object<CGameFieldFileData>(*this);
	m_Cursor = InvalidCursor;
}

void CGameFieldCtrl::NewEasy()
{
	CTask task = {};
	auto g = task.NewGame(TASK_EASY);
	Reset(task);
	m_nLevel = TASK_EASY;
#ifdef _DEBUG
	CString s;
	s.Format(IDS_EASYGAME_D, g);
	SetStatus3(s);
#endif
}

void CGameFieldCtrl::NewNormal()
{
	CTask task = {};
	auto g = task.NewGame(TASK_NORMAL);
	Reset(task);
	m_nLevel = TASK_NORMAL;
#ifdef _DEBUG
	CString s;
	s.Format(IDS_NORMALGAME_D, g);
	SetStatus3(s);
#endif
}

void CGameFieldCtrl::NewHard()
{
	CTask task = {};
	auto g = task.NewGame(TASK_HARD);
	Reset(task);
	m_nLevel = TASK_HARD;
#ifdef _DEBUG
	CString s;
	s.Format(IDS_HARDGAME_D, g);
	SetStatus3(s);
#endif
}

void CGameFieldCtrl::NewInsane()
{
	CTask task = {};
	auto g = task.NewGame(TASK_INSANE);
	Reset(task);
	m_nLevel = TASK_INSANE;
#ifdef _DEBUG
	CString s;
	s.Format(IDS_INSANEGAME_D, g);
	SetStatus3(s);
#endif
}

bool CGameFieldCtrl::Check()
{
	return m_GameField.CheckField(&m_FieldError);
}

void CGameFieldCtrl::Init()
{
	ReleaseUndo();
	ReleaseRedo();
	m_SnapShots.clear();
	m_Cursor = InvalidCursor;
	m_GameField.Init();
	time(&m_tStart);
	m_nLevel = TASK_UNKNOWN;
	m_bDirty = false;
	OnChanged();
}

void CGameFieldCtrl::Reset()
{
	ReleaseUndo();
	ReleaseRedo();
	m_SnapShots.clear();
	m_Cursor = InvalidCursor;
	m_GameField.Reset();
	time(&m_tStart);
	m_nLevel = TASK_UNKNOWN;
	m_bDirty = false;
	OnChanged();
}

void CGameFieldCtrl::Reset(const CTask& task)
{
	ReleaseUndo();
	ReleaseRedo();
	m_SnapShots.clear();
	m_Cursor = InvalidCursor;
	m_GameField = task;
	time(&m_tStart);
	m_nLevel = TASK_UNKNOWN;
	m_bDirty = false;
	OnChanged();
}

void CGameFieldCtrl::OnChanged()
{
	m_FieldError.m_nErrorRgn = CFieldError::rgnNoError;
	m_GameField.Count(&counters);
	bool bSolved = false;
	if (IsFull())
	{
		Check();
		if (m_FieldError.m_Error == CFieldError::rgnNoError)
			bSolved = true;
	}
	else
	{
		SetSolved(false);
		if (m_bAutoCheck)
			Check();
	}
	Refresh();

	if (bSolved)
		SetSolved();

	UpdateHintWindow();
	CString s;
	s.LoadString(IDS_READY);
	SetStatus3(s);
}

bool CGameFieldCtrl::MakeSnapShot(const char* name, time_t time)
{
	if (m_SnapShots.size() < SNAPSHOT_MAX_AMOUNT)
	{
		const CSnapShot snap(m_GameField, name, time);
		m_SnapShots.push_back(snap);
		return true;
	}
	return false;
}

void CGameFieldCtrl::LoadSnapShot(unsigned nSnapShot)
{
	if (nSnapShot < m_SnapShots.size())
	{
		ReleaseUndo();
		ReleaseRedo();
		m_GameField = m_SnapShots[nSnapShot].CData();
		OnChanged();
	}
}

void CGameFieldCtrl::GetSnapNames(CSnapShotNames* names) const
{
	names->clear();
	for (const auto& i : m_SnapShots)
	{
		std::stringstream buf;
		CTimeSpan s(i.GetSnapTime());
		buf << i.GetSnapShotName() << " (" << std::setfill('0') << std::setw(2) << s.GetTotalHours() << ':' << std::setfill('0') << std::setw(2) << s.GetMinutes() << ':' << std::setfill('0') << std::setw(2) << s.GetSeconds() << ')';
		names->push_back(buf.str());
	}
}

time_t CGameFieldCtrl::GetTimeSpan() const
{
	if (m_bSolved)
		return m_tSolved;
	if (!m_tStart)
		return 0U;
	time_t now;
	time(&now);
	return now - m_tStart;
}

void CGameFieldCtrl::UpdateHintWindow() const
{
	CWindow Ctrl;
	CWindow Parent = GetParent();
	if (!Parent.IsWindow())
		return;
	Ctrl.m_hWnd = (HWND)Parent.SendMessage(WM_GETHINT, NULL, NULL);
	if (Ctrl.IsWindow())
	{
		if (IsValidCursor())
		{
			const auto i = CGameField::GetIndex(m_Cursor);
			const auto m_nVal = m_GameField.CData()[i].m_nVal;
			const auto m_nHypo = m_GameField.CData()[i].m_nHypo;
			Ctrl.SendMessage(WM_UPDATEHINTCTRL, static_cast<WPARAM>(MAKELONG(m_nVal, m_nMode)), static_cast<LPARAM>(m_nHypo));
		}
		else
			Ctrl.SendMessage(WM_UPDATEHINTCTRL, static_cast<WPARAM>(-1L), static_cast<LPARAM>(-1L));
	}
}

void CGameFieldCtrl::AddUndoRedo(const CCell& cell, const UPOINT& Point)
{
	m_Undo.push_back({ cell, Point });
	UpdateUndo();
}

void CGameFieldCtrl::AddUndo(const CCell& cell, const UPOINT& Point)
{
	AddUndoRedo(cell, Point);
	ReleaseRedo();
}

void CGameFieldCtrl::AddRedo(const CCell& cell, const UPOINT& Point)
{
	m_Redo.push_back({ cell, Point });
	UpdateRedo();
}

void CGameFieldCtrl::Undo()
{
	if (!m_Undo.empty())
	{
		const auto undo(m_Undo.back());
		m_Undo.pop_back();
		const auto j = CGameField::GetIndex(undo.pos);
		AddRedo(m_GameField.CData()[j], undo.pos);
		m_GameField.Data()[j] = undo.cell;
		UpdateUndo();
		UpdateRedo();
		OnChanged();
	}
}

void CGameFieldCtrl::Redo()
{
	if (!m_Redo.empty())
	{
		const auto redo(m_Redo.back());
		m_Redo.pop_back();
		const auto j = CGameField::GetIndex(redo.pos);
		AddUndoRedo(m_GameField.CData()[j], redo.pos);
		m_GameField.Data()[j] = redo.cell;
		UpdateUndo();
		UpdateRedo();
		OnChanged();
	}
}

void CGameFieldCtrl::ReleaseUndo()
{
	m_Undo.clear();
	UpdateUndo();
}

void CGameFieldCtrl::ReleaseRedo()
{
	m_Redo.clear();
	UpdateRedo();
}

void CGameFieldCtrl::UpdateUndo() const
{
	if (!IsWindow())
		return;
	auto parent = GetParent();
	if (!parent.IsWindow())
		return;
	parent.SendMessage(WM_UPDATEUNDO, 0, !m_Undo.empty() ? TRUE : FALSE);
}

void CGameFieldCtrl::UpdateRedo() const
{
	if (!IsWindow())
		return;
	auto parent = GetParent();
	if (!parent.IsWindow())
		return;
	parent.SendMessage(WM_UPDATEREDO, 0, !m_Redo.empty() ? TRUE : FALSE);
}

void CGameFieldCtrl::SetStatus3(LPCSTR str) const
{
	if (!IsWindow())
		return;
	auto parent = GetParent();
	if (!parent.IsWindow())
		return;
	parent.SendMessage(WM_SETSTATUS3, 0, reinterpret_cast<LPARAM>(str));
}

// Подсказка по месту курсора
void CGameFieldCtrl::Hint()
{
	if (!IsValidCursor())
		return;
	if (!m_GameField.Check(m_Cursor, &m_FieldError))
	{
		Refresh();
		return;
	}
	if (m_nLevel != TASK_UNKNOWN)
		m_bDirty = true;
	Hint(m_Cursor);
	Refresh();
	UpdateHintWindow();
}

// Подсказка по переданной точке
void CGameFieldCtrl::Hint(const UPOINT& point)
{
	arHints hints;
	if (m_GameField.Hint(point, &hints))
	{
		const auto index = CGameField::GetIndex(point);
		const auto cell = m_GameField.CData()[index];
		m_GameField.Data()[index].ClearHypothesisAll();
		for (auto i = 1U; i <= 9U; i++)
		{
			if (hints[i - 1])
				m_GameField.Data()[index].SetHypothesis(i);
		}
		// Отложенное добавление в undo только в случае, если были изменения
		if (m_GameField.CData()[index].m_nHypo != cell.m_nHypo)
			AddUndo(cell, point);
	}
}

void CGameFieldCtrl::HintAround(const UPOINT& point)
{
	for (auto x = 1U; x <= 9U; x++)
		Hint(UPOINT{ x, point.y });
	for (auto y = 1U; y <= 9U; y++)
		Hint(UPOINT{ point.x, y });
	const auto s = CGameField::GetSquareIndex(point);
	for (auto c = 1U; c <= 9U; c++)
	{
		const auto i = CGameField::GetIndexSquare(s, c);
		const auto p = CGameField::GetPointIndex(i);
		Hint(p);
	}
}

void CGameFieldCtrl::HintAll()
{
	bool bRefresh = false;

	if (!m_bAutoCheck)
	{
		if (!Check())
			bRefresh = true;
	}
	if (m_FieldError.m_Error == CFieldError::rgnNoError)
	{
		if (m_nLevel != TASK_UNKNOWN)
			m_bDirty = true;
		for (auto y = 1U; y <= 9U; y++)
		{
			for (auto x = 1U; x <= 9U; x++)
				Hint(UPOINT{ x, y });
		}
		UpdateHintWindow();
		bRefresh = true;
	}
	if (bRefresh)
		Refresh();
}

int CGameFieldCtrl::Solve()
{
	CWaitCursor wc;
	bool bChanged;
	m_Cursor = InvalidCursor;
	if (m_nLevel != TASK_UNKNOWN)
		m_bDirty = true;
	int nChanges = -1;

	if (Check())
	{
		nChanges = 0;
		do
		{
			bChanged = false;
			for (auto y = 1U; y <= 9U; y++)
			{
				for (auto x = 1U; x <= 9U; x++)
					Hint(UPOINT{ x, y });
			}

			for (auto y = 1U; y <= 9U; y++)
			{
				for (auto x = 1U; x <= 9U; x++)
				{
					const UPOINT point = { x, y };
					const auto i = m_GameField.GetSingleHint(point);
					if (i != 0)
					{
						AddUndo(m_GameField.CData()[CGameField::GetIndex(point)], point);
						m_GameField.Data()[CGameField::GetIndex(point)].m_nVal = i;
						m_GameField.Data()[CGameField::GetIndex(point)].ClearHypothesisAll();
						bChanged = true;
						nChanges++;
						HintAround(point);
					}
				}
			}

			unsigned i;
			UPOINT point;

			while (i = m_GameField.GetRegionHint(&point))
			{
				AddUndo(m_GameField.CData()[CGameField::GetIndex(point)], point);
				m_GameField.Data()[CGameField::GetIndex(point)].m_nVal = i;
				m_GameField.Data()[CGameField::GetIndex(point)].ClearHypothesisAll();
				bChanged = true;
				nChanges++;
				HintAround(point);
			}
		} while (bChanged);
	}
	OnChanged();

	return nChanges;
}

void CGameFieldCtrl::SetSolved(bool bSolved /*= true*/)
{
	if (bSolved)
	{
		m_Cursor = InvalidCursor;
		m_bSolved = true;
		if (!m_tSolved)
		{
			time_t now;
			time(&now);
			m_tSolved = now - m_tStart;

			if (!IsWindow())
				return;
			auto parent = GetParent();
			if (!parent.IsWindow())
				return;
			parent.SendMessage(WM_SOLVED, m_nLevel, m_bDirty);
		}
	}
	else
	{
		m_bSolved = false;
		m_tSolved = 0;
	}
}

void CGameFieldCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Сначала обрабатываются события, для которых курсор не нужен
	if ('Z' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_FieldError.m_nErrorRgn = CFieldError::rgnNoError;
		Undo();
		return;
	}
	if ('Y' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_FieldError.m_nErrorRgn = CFieldError::rgnNoError;
		Redo();
		return;
	}
	if (VK_ESCAPE == nChar && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_nMode = SDK_DEC;
		m_Cursor = InvalidCursor;
		OnChanged();
		return;
	}
	if ('M' == nChar && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_nMode = (SDK_DEC == m_nMode) ? SDK_HYP : SDK_DEC;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if ('K' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		if (!m_bAutoCheck && !Check())
		{
			Refresh();
		}
		return;
	}
	if ('R' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		Reset();
		return;
	}
#if defined(_DEBUG)
	if ('A' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		CCounters counter1, counter2;
		CMyTaskDialog dlg;
		CString str2;
		CString str1 = "This game is like:\r\n";
		m_GameField.Count(&counter1);
		CString s;
		s.Format("%d %d %d %d %d %d %d %d %d",
			counter1[0], counter1[1], counter1[2], counter1[3], counter1[4],
			counter1[5], counter1[6], counter1[7], counter1[8]);
		SetStatus3(s);

		str1 += "Easy:";
		unsigned found = 0;
		for (auto t : tasks_easy)
		{
			CGameField gf(t);
			gf.Count(&counter2);
			if (counter1 == counter2)
			{
				str2.Format(" %d", ++found);
				str1 += str2;
			}
		}
		if (found)
			str1 += "\r\n";
		else
			str1 += " None\r\n";

		str1 += "Normal:";
		found = 0;
		for (auto t : tasks_normal)
		{
			CGameField gf(t);
			gf.Count(&counter2);
			if (counter1 == counter2)
			{
				str2.Format(" %d", ++found);
				str1 += str2;
			}
		}
		if (found)
			str1 += "\r\n";
		else
			str1 += " None\r\n";

		str1 += "Hard:";
		found = 0;
		for (auto t : tasks_hard)
		{
			CGameField gf(t);
			gf.Count(&counter2);
			if (counter1 == counter2)
			{
				str2.Format(" %d", ++found);
				str1 += str2;
			}
		}
		if (found)
			str1 += "\r\n";
		else
			str1 += " None\r\n";

		str1 += "Insane:";
		found = 0;
		for (auto t : tasks_insane)
		{
			CGameField gf(t);
			gf.Count(&counter2);
			if (counter1 == counter2)
			{
				str2.Format(" %d", ++found);
				str1 += str2;
			}
		}
		if (found)
			str1 += "\r\n";
		else
			str1 += " None\r\n";

		dlg.SetText(str1);
		dlg.DoModal();
		return;
	}
	if ('T' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		CMyTaskDialog dlg;
		CString m_str;
		for (auto y = 1U; y <= 9U; y++)
		{
			CString m_str1;
			m_str1.Format("%d, %d, %d, %d, %d, %d, %d, %d, %d,\r\n",
				m_GameField[CGameField::GetIndexLine(1, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(2, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(3, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(4, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(5, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(6, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(7, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(8, y)].m_nVal,
				m_GameField[CGameField::GetIndexLine(9, y)].m_nVal);
			m_str += m_str1;
		}
		dlg.SetText(m_str);
		dlg.DoModal();
		return;
	}
	if ('H' == nChar && IsCTRLpressed() && IsSHIFTpressed()) // Hint
	{
		HintAll();
		return;
	}
#endif	// _DEBUG
	// События ниже зависят от положения курсора
	if (m_bSolved)
	{
		m_Cursor = InvalidCursor;
		return;
	}
	if (!IsValidCursor())
	{
		SetMsgHandled(FALSE);
		return;
	}
	if ('H' == nChar && IsCTRLpressed() && !IsSHIFTpressed()) // Hint
	{
		Hint();
		return;
	}
	if ('0' == nChar && SDK_DEC == m_nMode && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		const auto i = CGameField::GetIndex(m_Cursor);
		if (!m_GameField[i].ReadOnly())
		{
			AddUndo(m_GameField[i], m_Cursor);
			m_GameField[i].m_nVal = 0U;
			OnChanged();
		}
		return;
	}
	if (((nChar >= '1' && nChar <= '9') || (nChar >= VK_NUMPAD1 && nChar <= VK_NUMPAD9)) && !IsSHIFTpressed())
	{
		CELLVAL v = 0U;
		if ('1' == nChar || VK_NUMPAD1 == nChar) v = 1U;
		else if ('2' == nChar || VK_NUMPAD2 == nChar) v = 2U;
		else if ('3' == nChar || VK_NUMPAD3 == nChar) v = 3U;
		else if ('4' == nChar || VK_NUMPAD4 == nChar) v = 4U;
		else if ('5' == nChar || VK_NUMPAD5 == nChar) v = 5U;
		else if ('6' == nChar || VK_NUMPAD6 == nChar) v = 6U;
		else if ('7' == nChar || VK_NUMPAD7 == nChar) v = 7U;
		else if ('8' == nChar || VK_NUMPAD8 == nChar) v = 8U;
		else if ('9' == nChar || VK_NUMPAD9 == nChar) v = 9U;
		else ATLASSERT(FALSE);
		const auto i = CGameField::GetIndex(m_Cursor);
		if (!m_GameField[i].ReadOnly())
		{
			if (SDK_DEC == m_nMode && !IsCTRLpressed() && v != m_GameField[i].m_nVal)
			{
				AddUndo(m_GameField[i], m_Cursor);
				m_GameField[i].m_nVal = v;
				m_GameField[i].ClearHypothesisAll();
				OnChanged();
			}
			if ((SDK_HYP == m_nMode && 0 == m_GameField[i].m_nVal) || (SDK_DEC == m_nMode && IsCTRLpressed()))
			{
				AddUndo(m_GameField[i], m_Cursor);
				m_GameField[i].InvertHypothesis(v);
				OnChanged();
			}
		}
		return;
	}
	if (VK_DELETE == nChar && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		const auto i = CGameField::GetIndex(m_Cursor);
		if (!m_GameField[i].ReadOnly())
		{
			AddUndo(m_GameField[i], m_Cursor);
			m_GameField[i].m_nVal = 0U;
			m_GameField[i].ClearHypothesisAll();
			OnChanged();
		}
		return;
	}
	if (VK_LEFT == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
		{
			if (0U == --m_Cursor.x)
				m_Cursor.x = 9U;
		}
		else // IsCTRLpressed
			m_Cursor.x = 1U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_RIGHT == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
		{
			if (10U == ++m_Cursor.x)
				m_Cursor.x = 1U;
		}
		else // IsCTRLpressed
			m_Cursor.x = 9U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_UP == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
		{
			if (0U == --m_Cursor.y)
				m_Cursor.y = 9U;
		}
		else // IsCTRLpressed
			m_Cursor.y = 1U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_DOWN == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
		{
			if (10U == ++m_Cursor.y)
				m_Cursor.y = 1U;
		}
		else // IsCTRLpressed
			m_Cursor.y = 9U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_HOME == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
			m_Cursor = UPOINT{ 1U, 1U };
		else
			m_Cursor.x = 1U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_END == nChar && !IsSHIFTpressed())
	{
		if (!IsCTRLpressed())
			m_Cursor = UPOINT{ 9U, 9U };
		else
			m_Cursor.x = 9U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_PRIOR == nChar && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_Cursor.y = 1U;
		Refresh();
		UpdateHintWindow();
		return;
	}
	if (VK_NEXT == nChar && !IsCTRLpressed() && !IsSHIFTpressed())
	{
		m_Cursor.y = 9U;
		Refresh();
		UpdateHintWindow();
		return;
	}
#ifdef _DEBUG
	if ('R' == nChar && IsCTRLpressed() && !IsSHIFTpressed())
	{
		const auto i = CGameField::GetIndex(m_Cursor);
		if (m_GameField[i].m_nVal != 0U)
		{
			AddUndo(m_GameField[i], m_Cursor);
			if (m_GameField[i].ReadOnly())
				m_GameField[i].ReadOnly(false);
			else
				m_GameField[i].ReadOnly(true);
			OnChanged();
		}
		return;
	}
#endif	// _DEBUG
	SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnLButtonDown(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (TrackCursor(point))
	{
		SetCapture();
		m_bLButtonDown = true;
	}
	else
		SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnLButtonUp(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;

	if (m_bLButtonDown)
	{
		m_bLButtonDown = false;
		TrackCursor(point);
	}
	ReleaseCapture();
}

void CGameFieldCtrl::OnLButtonDblClk(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (IsValidPoint(point))
	{
		m_nMode = (SDK_DEC == m_nMode) ? SDK_HYP : SDK_DEC;
		Refresh();
	}
	else
		SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnRButtonDown(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (TrackCursor(point))
	{
		SetCapture();
		m_bRButtonDown = true;
	}
	else
		SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnRButtonUp(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (m_bRButtonDown)
	{
		m_bRButtonDown = false;
		TrackCursor(point);
	}
	ReleaseCapture();
}

void CGameFieldCtrl::OnRButtonDblClk(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (IsValidPoint(point))
	{
		m_nMode = (SDK_DEC == m_nMode) ? SDK_HYP : SDK_DEC;
		Refresh();
	}
	else
		SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnMouseMove(UINT nFlags, POINT point)
{
	if (m_bSolved)
		return;
	if (m_bLButtonDown || m_bRButtonDown)
	{
		if (!TrackCursor(point))
			SetMsgHandled(FALSE);
	}
}

bool CGameFieldCtrl::TrackCursor(const POINT& point)
{
	UPOINT Cursor;
	if (!IsValidPoint(point, &Cursor))
		return false;
	if (!m_bSolved)
	{
		m_Cursor = Cursor;
		Refresh();
		UpdateHintWindow();
	}
	return true;
}

bool CGameFieldCtrl::IsValidPoint(const POINT& point, _Out_ UPOINT *Cursor /*= nullptr*/) const
{
	CRect rect;
	GetClientRect(&rect);
	rect.bottom -= 2L;
	rect.right -= 2L;

	CPoint clt = point;
	if (!m_bCaptured)
		::MapWindowPoints(GetParent(), m_hWnd, &clt, 1);

	if (!rect.PtInRect(clt))
		return false;

	UPOINT cursor = { unsigned(double(clt.x) / double(rect.right) * 9.0) + 1U, unsigned(double(clt.y) / double(rect.bottom) * 9.0) + 1U };
	if (!IsValidCursor(cursor))
		return false;
	if (Cursor)
		*Cursor = cursor;
	return true;
}

void CGameFieldCtrl::OnPaint(CDCHandle /*dc*/)
{
	CRect rect;
	GetClientRect(rect);
	//ATLTRACE("CGameFieldCtrl::OnPaint: rectClient.Width = %i rectClient.Height = %i\r\n", rect.Width(), rect.Height());

#if defined(_DEBUG_PAINT)
	CPaintDC dc(m_hWnd);
#else
	const CPaintDC paintdc(m_hWnd);
	CMemoryDC dc(paintdc, rect);
#endif

	rect.right -= 1L; rect.bottom -= 1L;
	const double b3 = rect.Height() / 3.0;
	const double r3 = rect.Width() / 3.0;
	const double b9 = rect.Height() / 9.0;
	const double r9 = rect.Width() / 9.0;

	CPen pen;
	CPenHandle oldpen;
	CBrush brush;

	CLogFont biglogfont;
	CLogFont smalllogfont;
	CFont bigfont, smallfont;
	CFontHandle oldfont;

	biglogfont.lfHeight = static_cast<int>(b9);
	strcpy_s(static_cast<char *>(biglogfont.lfFaceName), LF_FACESIZE, "Arial");
	bigfont.CreateFontIndirect(&biglogfont);

	smalllogfont.lfHeight = int(b9 / 3);
	strcpy_s(static_cast<char *>(smalllogfont.lfFaceName), LF_FACESIZE, "Arial");
	smallfont.CreateFontIndirect(&smalllogfont);

	for (auto y = 1U; y <= 9U; y++) for (auto x = 1U; x <= 9U; x++)
	{
		CRect cell(r9 * (double(x) - 1.0) + 1.0, b9 * (double(y) - 1.0) + 1.0, r9 * double(x), b9 * double(y));
		CRect cell2((double(x) - 1.0) * r9 + 2.0, (double(y) - 1.0) * b9 + 2.0, double(x) * r9, double(y) * b9);

		// Рисуем фон каждой клеточки
		COLORREF bgcolor = BACKGRND;
		COLORREF fgcolor = DIGIT_RO;
		COLORREF frcolor = CURSOR_DEC;
		if (m_bSolved)
		{
			bgcolor = BACKGRND_SOLVED;
			fgcolor = DIGIT_SOLVED;
		}
		else
		{
			const auto bError = m_FieldError.IsPointInRegion(UPOINT{ x, y });
			bool bShadow = false;
#if defined(DRAW_SHADOW)
			bShadow = true;
#endif
			if (m_Cursor.x == x && m_Cursor.y == y)
			{
				bShadow = false;
				if (bError)
					bgcolor = CURSOR_ERR_BRUSH;
				else
					bgcolor = CURSOR_BRUSH;
			}
			else
			{
				if (bError)
					bgcolor = BACKGRND_ERR;
			}
			if ((m_Cursor.x == x || m_Cursor.y == y) && bShadow)
			{
				if (bError)
					bgcolor = CURSOR_SHA_ERR_BRUSH;
				else
					bgcolor = CURSOR_SHA_BRUSH;
			}
		}
		brush.CreateSolidBrush(bgcolor);
		dc.FillRect(cell, brush);
		brush.DeleteObject();

		// Рисуем курсор
		if (m_Cursor.x == x && m_Cursor.y == y && !m_bSolved)
		{
			if (SDK_DEC == m_nMode)
				pen.CreatePen(PS_SOLID, 2, CURSOR_DEC);
			else
				pen.CreatePen(PS_SOLID, 2, CURSOR_HYP);
			oldpen = dc.SelectPen(pen);
			auto obj = SelectObject(dc, GetStockObject(NULL_BRUSH));
			dc.Rectangle(cell2);
			dc.SelectPen(oldpen);
			pen.DeleteObject();
		}

		// Рисуем цифры
		dc.SetBkMode(TRANSPARENT);
		const auto i = CGameField::GetIndexLine(x, y);
		const auto v = m_GameField[i].m_nVal;
		ATLASSERT(v >= 0U && v <= 9U);
		if (v >= 1U && v <= 9U)
		{
			oldfont = dc.SelectFont(bigfont);
			if (m_GameField[i].ReadOnly())
				dc.SetTextColor(DIGIT_RO);
			else
				dc.SetTextColor(DIGIT_DEC);
			char text[2] = { Num2Char(v), '\0' };
			dc.DrawText(static_cast<LPCSTR>(text), 1, &cell2, DT_CENTER | DT_VCENTER);
			dc.SelectFont(oldfont);
		}
		else // v == 0
		{
			const auto h = m_GameField[i].m_nHypo;
			if (h)
			{
				for (auto hi = 1U, hi0 = 0U; hi <= 9U; hi++, hi0++)
				{
					if (m_GameField[i].GetHypothesis(hi))
					{
						const auto hx = hi0 % 3;
						const auto hy = hi0 / 3;
						const auto x1 = (x - 1.0) * r9 + (hx * r9 / 3.0) + 1.0;
						const auto y1 = (y - 1.0) * b9 + (hy * b9 / 3.0) + 1.0;
						const auto x2 = r9 / 3.0 + x1 + 1;
						const auto y2 = b9 / 3.0 + y1 + 1;
						CRect rect(x1, y1, x2, y2);
						oldfont = dc.SelectFont(smallfont);
						dc.SetTextColor(DIGIT_HYP);
						char text[2] = { Num2Char(hi), '\0' };
						dc.DrawText(static_cast<LPCSTR>(text), 1, &rect, DT_CENTER | DT_NOCLIP | DT_VCENTER);
						dc.SelectFont(oldfont);
					}
				}
			}
		}
	}

	// Рисуем линии
	// Рисуем тонкие линии
	pen.CreatePen(PS_SOLID, 1, THINLINE);
	oldpen = dc.SelectPen(pen);

	dc.MoveTo(r9, rect.top); // Рисуем вертикальные тонкие линии
	dc.LineTo(r9, rect.bottom);
	dc.MoveTo(r9 * 2.0, rect.top);
	dc.LineTo(r9 * 2.0, rect.bottom);

	dc.MoveTo(r9 * 4.0, rect.top);
	dc.LineTo(r9 * 4.0, rect.bottom);
	dc.MoveTo(r9 * 5.0, rect.top);
	dc.LineTo(r9 * 5.0, rect.bottom);

	dc.MoveTo(r9 * 7.0, rect.top);
	dc.LineTo(r9 * 7.0, rect.bottom);
	dc.MoveTo(r9 * 8.0, rect.top);
	dc.LineTo(r9 * 8.0, rect.bottom);

	dc.MoveTo(rect.left, b9); // Рисуем горизонтальные тонкие линии
	dc.LineTo(rect.right, b9);
	dc.MoveTo(rect.left, b9 * 2.0);
	dc.LineTo(rect.right, b9 * 2.0);

	dc.MoveTo(rect.left, b9 * 4.0);
	dc.LineTo(rect.right, b9 * 4.0);
	dc.MoveTo(rect.left, b9 * 5.0);
	dc.LineTo(rect.right, b9 * 5.0);

	dc.MoveTo(rect.left, b9 * 7.0);
	dc.LineTo(rect.right, b9 * 7.0);
	dc.MoveTo(rect.left, b9 * 8.0);
	dc.LineTo(rect.right, b9 * 8.0);
	dc.SelectPen(oldpen);
	pen.DeleteObject();

	pen.CreatePen(PS_SOLID, 3, THICKLINE);
	oldpen = dc.SelectPen(pen);

	// Рисуем толстую рамку
	CRect frame;
	GetClientRect(frame);
	auto obj = SelectObject(dc, GetStockObject(NULL_BRUSH));
	dc.Rectangle(frame);
	DeleteObject(obj);

	dc.MoveTo(r3, rect.top); // Рисуем вертикальные толстые линии
	dc.LineTo(r3, rect.bottom);
	dc.MoveTo(r3 * 2.0, rect.top);
	dc.LineTo(r3 * 2.0, rect.bottom);

	dc.MoveTo(rect.left, b3); // Рисуем горизонтальные толстые линии
	dc.LineTo(rect.right, b3);
	dc.MoveTo(rect.left, b3 * 2.0);
	dc.LineTo(rect.right, b3 * 2.0);
	dc.SelectPen(oldpen);
	pen.DeleteObject();

	bigfont.DeleteObject();
	smallfont.DeleteObject();

	//SetMsgHandled(FALSE);
}

void CGameFieldCtrl::OnHintDown(CELLVAL nHint)
{
	if (IsValidCursor())
	{
		const auto i = CGameField::GetIndex(m_Cursor);
		if (!m_GameField[i].ReadOnly())
		{
			if (SDK_DEC == m_nMode)
			{
				AddUndo(m_GameField[i], m_Cursor);
				m_GameField[i].m_nVal = nHint;
			}
			else if (!m_GameField[i].m_nVal)
			{
				AddUndo(m_GameField[i], m_Cursor);
				m_GameField[i].InvertHypothesis(nHint);
			}
			OnChanged();
		}
	}
}

void CGameFieldCtrl::SetCapture()
{
	if (!m_bCaptured)
	{
		CWindow::SetCapture();
		m_bCaptured = true;
	}
}

void CGameFieldCtrl::ReleaseCapture()
{
	if (m_bCaptured)
	{
		::ReleaseCapture();
		m_bCaptured = false;
	}
}
