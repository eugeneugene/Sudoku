#include "stdafx.h"
#include "MainWnd.h"
#include "MakeSnapShotDlg.h"
#include "WinnerDlg.h"
#include "ShowWinnersDlg.h"

CMainWnd::CMainWnd() :
	m_pFileDlg(new CFileDialog(FALSE, LPCSTR("sdk"), nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_CREATEPROMPT, LPCSTR("Sudoku Files\0*.sdk\0All Files\0*.*\0"))),
	m_tStatusTime(0), m_bAutoCheck(false), m_bMsgHandled(FALSE), m_Direction(CDirection::ZERO)
{
}

CMainWnd::~CMainWnd()
{
	delete m_pFileDlg;
}

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// set icons
	m_hIcon.LoadIcon(IDR_MAINFRAME, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIcon, TRUE);
	m_hIconSmall.LoadIcon(IDR_MAINFRAME, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIconSmall, FALSE);

	m_hAccelerator = AtlLoadAccelerators(IDR_MAINFRAME);

	obj = GetSysColorBrush(COLOR_BTNFACE);

	// register object for message filtering and idle updates
	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	CMenu menu;
	menu.LoadMenu(IDR_MAINFRAME);
	SetMenu(menu);
	m_Popup.Init(m_hWnd);

	CRect m_rectWindow;

	if (!LoadDefault(&m_rectWindow))
		m_bAutoCheck = false;

	if (m_rectWindow.IsRectNull())
		CenterWindow();
	else
		MoveWindow(&m_rectWindow, FALSE);

	CStatic m_GameCtrlWin;
	CStatic m_HintCtrlWin;

	CRect m_rectClient;
	GetClientRect(&m_rectClient);

	CRect GameCtrlRect = { 12, 12, m_rectClient.Width() - 12, m_rectClient.Width() - 12 };
	m_GameCtrlWin.Create(m_hWnd, GameCtrlRect, "GridCtrl", WS_CHILD | WS_VISIBLE);
	m_GameCtrl.SubclassWindow(m_GameCtrlWin);

	CRect HintCtrlRect = { 12, m_rectClient.Width(), m_rectClient.Width() - 12, m_rectClient.Width() + (int)(GameCtrlRect.Width() / 9.0) };
	m_HintCtrlWin.Create(m_hWnd, HintCtrlRect, "HintCtrl", WS_CHILD | WS_VISIBLE);
	m_HintCtrl.SubclassWindow(m_HintCtrlWin);

	UIAddStatusBar(m_Status.Create(m_hWnd));
	int arrPanes[] = { IDR_CURSOR, IDR_COUNT, ID_DEFAULT_PANE, IDR_TIME };
	m_Status.SetPanes(arrPanes, _countof(arrPanes), false);
	SetTimer(ID_TIMER1, 100, NULL);

	LoadTop();

	OnUpdateUndo(m_GameCtrl.CanUndo());
	OnUpdateRedo(m_GameCtrl.CanRedo());
	OnUpdateAutoCheck(m_bAutoCheck);
	OnUpdateCheck(!m_bAutoCheck);

	m_GameCtrl.AutoCheck(m_bAutoCheck);
	m_GameCtrl.OnChanged();
	CModuleVersion vers;
	
	TCHAR szFileName[MAX_PATH + 1];
	GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
	vers.GetFileVersionInfo(szFileName);

	CString s;
	LPTSTR internalname = "Sudoku";
	LPTSTR privatebuild = "Unknown";
	LPTSTR fileversion = "Unknown";
	vers.GetValue("InternalName", &internalname);
	vers.GetValue("PrivateBuild", &privatebuild);
	vers.GetValue("FileVersion", &fileversion);
	s.Format(IDS_VERSION, internalname, privatebuild, fileversion);
#ifdef _DEBUG
	s += " DEBUG";
#endif
	m_strStatus3 = s;
	return TRUE;
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	if (m_hAccelerator != NULL)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccelerator, pMsg))
			return TRUE;
	}
	return FALSE;
}

void CMainWnd::OnDestroy()
{
	StoreDefault();
	StoreTop();
	m_Status.DestroyWindow();
	// unregister message filtering and idle updates
	auto pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveIdleHandler(this);
	pLoop->RemoveMessageFilter(this);
	PostQuitMessage(0);
}

void CMainWnd::OnSizing(UINT fwSide, LPRECT _pRect)
{
	CRect *pRect{ static_cast<CRect*>(_pRect) };
	CRect rect;
	GetWindowRect(&rect);

	CRect m_GameCtrlClntRect;
	m_GameCtrl.GetClientRect(&m_GameCtrlClntRect);
	m_GameCtrl.MapWindowPoints(m_hWnd, &m_GameCtrlClntRect);
	CRect m_HintCtrlClntRect;
	m_HintCtrl.GetClientRect(&m_HintCtrlClntRect);
	m_HintCtrl.MapWindowPoints(m_hWnd, &m_HintCtrlClntRect);

	int dx = pRect->Width() - rect.Width();
	int dy = pRect->Height() - rect.Height();

	if (CDirection::ZERO == m_Direction)
	{
		if (dx == 0 && dy < 0)
			m_Direction = CDirection::UP;
		else if (dx > 0 && dy < 0)
		{
			if (dx > -dy)
				m_Direction = CDirection::RIGHT;
			else
				m_Direction = CDirection::UP;
		}
		else if (dx > 0 && dy == 0)
			m_Direction = CDirection::RIGHT;
		else if (dx > 0 && dy > 0)
		{
			if (dx > dy)
				m_Direction = CDirection::RIGHT;
			else
				m_Direction = CDirection::DOWN;
		}
		else if (dx == 0 && dy > 0)
			m_Direction = CDirection::DOWN;
		else if (dx < 0 && dy > 0)
		{
			if (-dx > dy)
				m_Direction = CDirection::LEFT;
			else
				m_Direction = CDirection::DOWN;
		}
		else if (dx < 0 && dy == 0)
			m_Direction = CDirection::LEFT;
		else if (dx < 0 && dy < 0)
		{
			if (dx < dy)
				m_Direction = CDirection::LEFT;
			else
				m_Direction = CDirection::UP;
		}
	}

	switch (m_Direction)
	{
	case CDirection::LEFT:
	case CDirection::RIGHT:
		if (true)
		{
			if (pRect->top != rect.top)
				pRect->top = rect.top;
			m_GameCtrlClntRect.right = m_GameCtrlClntRect.bottom = pRect->Width() - 28;	// В квадрате ширина равна высоте
			m_HintCtrlClntRect.top = m_GameCtrlClntRect.bottom + 12;
			m_HintCtrlClntRect.right = m_GameCtrlClntRect.right;
			m_HintCtrlClntRect.bottom = m_HintCtrlClntRect.top + m_HintCtrlClntRect.Width() / 9;
			int d = m_HintCtrlClntRect.bottom + 99;
			if (WMSZ_LEFT == fwSide || WMSZ_RIGHT == fwSide)
			{
				pRect->bottom = pRect->top + d;
			}
			else
			{
				if (WMSZ_TOPLEFT == fwSide || WMSZ_TOPRIGHT == fwSide)
					pRect->top = pRect->bottom - d;
				else
					pRect->bottom = pRect->top + d;
			}
		}
		break;

	case CDirection::UP:
	case CDirection::DOWN:
		if (true)
		{
			if (pRect->left != rect.left)
				pRect->left = rect.left;
			int d = MulDiv(pRect->Height() - 123, 9, 10) + 41;		// Вычислено опытным путём
			if (WMSZ_TOP == fwSide || WMSZ_BOTTOM == fwSide)
			{
				pRect->right = pRect->left + d;
			}
			else
			{
				if (WMSZ_TOPLEFT == fwSide || WMSZ_BOTTOMLEFT == fwSide)
					pRect->left = pRect->right - d;
				else
					pRect->right = pRect->left + d;
			}
			m_GameCtrlClntRect.right = m_GameCtrlClntRect.bottom = pRect->Width() - 28;	// В квадрате ширина равна высоте
			m_HintCtrlClntRect.top = m_GameCtrlClntRect.bottom + 12;
			m_HintCtrlClntRect.right = m_GameCtrlClntRect.right;
			m_HintCtrlClntRect.bottom = m_HintCtrlClntRect.top + m_HintCtrlClntRect.Width() / 9;
			break;
		}
	}

	m_GameCtrl.MoveWindow(m_GameCtrlClntRect, FALSE);
	m_GameCtrl.Invalidate(FALSE);
	m_HintCtrl.MoveWindow(m_HintCtrlClntRect, FALSE);
	m_HintCtrl.Invalidate(FALSE);
	Invalidate();
}

void CMainWnd::OnCommand(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
{
	switch (nID)
	{
	case ID_APP_EXIT:
		DestroyWindow();
		break;
	case ID_APP_ABOUT:
	{
		CAboutDlg dlg;
		CModuleVersion vers;

		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		vers.GetFileVersionInfo(szFileName);

		CString s;
		LPTSTR internalname;
		LPTSTR privatebuild;
		LPTSTR fileversion;
		vers.GetValue("InternalName", &internalname);
		vers.GetValue("PrivateBuild", &privatebuild);
		vers.GetValue("FileVersion", &fileversion);
		s.Format(IDS_VERSION, internalname, privatebuild, fileversion);
#ifdef _DEBUG
		s += " DEBUG";
#endif
		dlg.SetVersion(s);
		dlg.DoModal();
	}
	break;
	case ID_FILE_SAVE_AS:
	lblFileSaveAs:
		m_pFileDlg->m_bOpenFileDialog = FALSE;
		m_pFileDlg->m_ofn.lpstrFile = m_strFileName.GetBufferSetLength(_MAX_PATH);
		m_pFileDlg->m_ofn.Flags |= OFN_CREATEPROMPT;
		if (IDOK != m_pFileDlg->DoModal(m_hWnd))
			break;
		m_strFileName.ReleaseBuffer();
	case ID_FILE_SAVE:
		if (m_strFileName.IsEmpty())
			goto lblFileSaveAs;
		try
		{
			m_GameCtrl.Store(m_strFileName);
		}
		catch (CWin32Error &e)
		{
			CString str;
			str.LoadString(IDS_FILESAVEERROR);
			ShowException(e, str);
		}
		catch (std::exception &e)
		{
			CString str;
			str.LoadString(IDS_FILESAVEERROR);
			ShowException(e.what(), str);
		}
		break;
	case ID_FILE_OPEN:
	{
		m_pFileDlg->m_bOpenFileDialog = TRUE;
		m_pFileDlg->m_ofn.lpstrFile = m_strFileName.GetBufferSetLength(_MAX_PATH);
		m_pFileDlg->m_ofn.Flags &= ~OFN_CREATEPROMPT;
		if (IDOK != m_pFileDlg->DoModal(m_hWnd))
			break;
		m_strFileName.ReleaseBuffer();
		try
		{
			m_GameCtrl.Load(m_strFileName);
			CSnapShotNames m_names;
			m_GameCtrl.GetSnapNames(&m_names);
			m_Popup.Reset();
			for (const auto &name : m_names)
			{
				m_Popup.AddItem(name.c_str());
			}
		}
		catch (CWin32Error &e)
		{
			CString str;
			str.LoadString(IDS_FILEOPENERROR);
			ShowException(e, str);
		}
		catch (std::exception &e)
		{
			CString str;
			str.LoadString(IDS_FILEOPENERROR);
			ShowException(e.what(), str);
		}
	}
	break;
	case ID_NEW_BLANK:
		m_Popup.Reset();
		m_GameCtrl.Init();
		m_strFileName.Empty();
		break;
	case ID_NEW_EASY:
		m_Popup.Reset();
		m_GameCtrl.NewEasy();
		m_strFileName.Empty();
		break;
	case ID_NEW_NORMAL:
		m_Popup.Reset();
		m_GameCtrl.NewNormal();
		m_strFileName.Empty();
		break;
	case ID_NEW_HARD:
		m_Popup.Reset();
		m_GameCtrl.NewHard();
		m_strFileName.Empty();
		break;
	case ID_NEW_INSANE:
		m_Popup.Reset();
		m_GameCtrl.NewInsane();
		m_strFileName.Empty();
		break;
	case ID_EDIT_UNDO:
		m_GameCtrl.Undo();
		break;
	case ID_EDIT_REDO:
		m_GameCtrl.Redo();
		break;
	case ID_GAME_HINT:
		m_GameCtrl.Hint();
		break;
	case ID_GAME_CHECK:
		m_GameCtrl.Check();
		break;
	case ID_GAME_SOLVE:
		if(true)
		{
			int nChanges = m_GameCtrl.Solve();
#if defined(_DEBUG)
			std::stringstream ss;
			ss << "Made changes: " << nChanges;
			UISetText(2, ss.str().c_str(), TRUE);
#endif
		}
		break;
	case ID_GAME_AUTOCHECK:
		m_bAutoCheck = !m_GameCtrl.AutoCheck();
		OnUpdateAutoCheck(m_bAutoCheck);
		OnUpdateCheck(!m_bAutoCheck);
		m_GameCtrl.AutoCheck(m_bAutoCheck);
		m_GameCtrl.Check();
		break;
	case ID_GAME_RESET:
		m_GameCtrl.Reset();
		break;
	case ID_GAME_MAKEASNAPSHOT:
	{
		CMakeSnapShotDlg dlg;
		if (dlg.DoModal() == IDOK)
		{
			std::string name(dlg.GetName());
			boost::trim(name);
			if (!name.empty())
			{
				CTimeSpan s(m_GameCtrl.GetTimeSpan());
				if (m_GameCtrl.MakeSnapShot(name.c_str(), s.GetTotalSeconds()))
				{
					std::stringstream buf;
					buf << name << " (" << std::setfill('0') << std::setw(2) << s.GetTotalHours() << ':' << std::setfill('0') << std::setw(2) << s.GetMinutes() << ':' << std::setfill('0') << std::setw(2) << s.GetSeconds() << ')';
					m_Popup.AddItem(buf.str().c_str());
				}
				else
					AtlMessageBox(m_hWnd, IDS_MAX_SNAPSHOT_AMOUNT, IDS_MAX_SNAPSHOT_AMOUNT_TITLE);
			}
		}
	}
	break;
	case ID_SHOWTOPWINNERS_EASYGAME:
	{
		CShowWinnersDlg dlg;
		dlg.HiScores(&hsEasy);
		dlg.DoModal();
	}
	break;
	case ID_SHOWTOPWINNERS_NORMALGAME:
	{
		CShowWinnersDlg dlg;
		dlg.HiScores(&hsNormal);
		dlg.DoModal();
	}
	break;
	case ID_SHOWTOPWINNERS_HARDGAME:
	{
		CShowWinnersDlg dlg;
		dlg.HiScores(&hsHard);
		dlg.DoModal();
	}
	break;
	case ID_SHOWTOPWINNERS_INSANEGAME:
	{
		CShowWinnersDlg dlg;
		dlg.HiScores(&hsInsane);
		dlg.DoModal();
	}
	break;
	default:
		if (nID >= SNAPSHOT_USER_ID && nID < SNAPSHOT_USER_ID + SNAPSHOT_MAX_AMOUNT)
			OnSnapShot(nID);
	}
}

void CMainWnd::ShowException(const LPCSTR strMsg, const LPCSTR strCaption)
{
	MessageBox(strMsg, strCaption, MB_OK | MB_ICONERROR | MB_APPLMODAL);
}

void CMainWnd::ShowException(const CWin32Error &e, const LPCSTR strCaption)
{
	std::stringstream str;
	str << "OS Error: (" << e.ErrorCode() << ") " << e.Description();
	ShowException(str.str().c_str(), strCaption);
}

void CMainWnd::StoreDefault() const
{
	try
	{
		CModuleVersion vers;

		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		vers.GetFileVersionInfo(szFileName);

		char *internalname;
		if (!vers.GetValue("InternalName", &internalname))
			return;
		char localappdata[MAX_PATH];
		if (!GetEnvironmentVariable("LOCALAPPDATA", localappdata, MAX_PATH))
			return;

		boost::filesystem::path p(localappdata);
		if (!exists(p))
			return;

		p /= boost::filesystem::path(internalname);
		if (!exists(p))
		{
			if (!create_directory(p))
				return;
		}

		auto f(p);
		f /= defaultname;

		std::ofstream ofs(f.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if (!ofs.good())
			return;

		boost::iostreams::filtering_ostreambuf fos;
		fos.push(boost::iostreams::gzip_compressor());
		fos.push(ofs);

		CRect m_rectWindow;
		GetWindowRect(&m_rectWindow);

		boost::archive::binary_oarchive boa(fos);
		boa << m_GameCtrl;
		boa << m_rectWindow.left << m_rectWindow.top << m_rectWindow.Width() << m_rectWindow.Height();
		boa << m_bAutoCheck;
	}
	catch (std::exception ex)
	{
		ATLTRACE("StoreDefault: Exception occurred: %s", ex.what());
		return;
	}
	catch (...)
	{
		ATLTRACE("StoreDefault: Unknown exception occurred");
		return;
	}
}

bool CMainWnd::LoadDefault(CRect *m_rectWindow)
{
	CGameFieldCtrl GameCtrl;
	CRect rect;
	bool bAutoCheck;
	try
	{
		CModuleVersion vers;

		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		vers.GetFileVersionInfo(szFileName);

		char *internalname;
		if (!vers.GetValue("InternalName", &internalname))
			return false;
		char localappdata[MAX_PATH];
		if (!GetEnvironmentVariable("LOCALAPPDATA", localappdata, MAX_PATH))
			return false;

		boost::filesystem::path p(localappdata);
		if (!exists(p))
			return false;

		p /= boost::filesystem::path(internalname);
		if (!exists(p))
			return false;

		auto f(p);
		f /= defaultname;

		std::ifstream ifs(f.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!ifs.good())
			return false;
		boost::iostreams::filtering_istreambuf fis;
		fis.push(boost::iostreams::gzip_decompressor());
		fis.push(ifs);
		boost::archive::binary_iarchive bia(fis);
		bia >> GameCtrl;
		int Width, Height;
		bia >> rect.left >> rect.top >> Width >> Height;
		rect.right = rect.left + Width;
		rect.bottom = rect.top + Height;
		bia >> bAutoCheck;
	}
	catch (std::exception ex)
	{
		ATLTRACE("LoadDefault: Exception occurred: %s", ex.what());
		return false;
	}
	catch (...)
	{
		ATLTRACE("LoadDefault: Unknown exception occurred");
		return false;
	}
	m_GameCtrl = GameCtrl;
	m_rectWindow->CopyRect(&rect);
	m_bAutoCheck = bAutoCheck;

	CSnapShotNames m_names;
	m_GameCtrl.GetSnapNames(&m_names);
	m_Popup.Reset();
	for (const auto &name : m_names)
	{
		m_Popup.AddItem(name.c_str());
	}
	return true;
}

void CMainWnd::StoreTop() const
{
	try
	{
		CModuleVersion vers;

		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		vers.GetFileVersionInfo(szFileName);

		char *internalname;
		if (!vers.GetValue("InternalName", &internalname))
			return;
		char localappdata[MAX_PATH];
		if (!GetEnvironmentVariable("LOCALAPPDATA", localappdata, MAX_PATH))
			return;

		boost::filesystem::path p(localappdata);
		if (!exists(p))
			return;

		p /= boost::filesystem::path(internalname);
		if (!exists(p))
		{
			if (!create_directory(p))
				return;
		}

		auto f(p);
		f /= topname;

		std::ofstream ofs(f.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		if (!ofs.good())
			return;

		boost::iostreams::filtering_ostreambuf fos;
		fos.push(boost::iostreams::gzip_compressor());
		fos.push(ofs);

		boost::archive::binary_oarchive boa(fos);
		boa << hsEasy << hsNormal << hsHard << hsInsane;
	}
	catch (std::exception ex)
	{
		ATLTRACE("LoadDefault: Exception occurred: %s", ex.what());
		return;
	}
	catch (...)
	{
		ATLTRACE("LoadDefault: Unknown exception occurred");
		return;
	}
}

bool CMainWnd::LoadTop()
{
	HiScoreTable hsEasy1;
	HiScoreTable hsNormal1;
	HiScoreTable hsHard1;
	HiScoreTable hsInsane1;

	try
	{
		CModuleVersion vers;

		TCHAR szFileName[MAX_PATH + 1];
		GetModuleFileName(NULL, szFileName, MAX_PATH + 1);
		vers.GetFileVersionInfo(szFileName);

		char *internalname;
		if (!vers.GetValue("InternalName", &internalname))
			return false;
		char localappdata[MAX_PATH];
		if (!GetEnvironmentVariable("LOCALAPPDATA", localappdata, MAX_PATH))
			return false;

		boost::filesystem::path p(localappdata);
		if (!exists(p))
			return false;

		p /= boost::filesystem::path(internalname);
		if (!exists(p))
		{
			return false;
		}

		auto f(p);
		f /= topname;

		std::ifstream ifs(f.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!ifs.good())
			return false;
		boost::iostreams::filtering_istreambuf fis;
		fis.push(boost::iostreams::gzip_decompressor());
		fis.push(ifs);
		boost::archive::binary_iarchive bia(fis);
		bia >> hsEasy1 >> hsNormal1 >> hsHard1 >> hsInsane1;
	}
	catch (std::exception ex)
	{
		ATLTRACE("LoadDefault: Exception occurred: %s", ex.what());
		return false;
	}
	catch (...)
	{
		ATLTRACE("LoadDefault: Unknown exception occurred");
		return false;
	}

	hsEasy = hsEasy1;
	hsNormal = hsNormal1;
	hsHard = hsHard1;
	hsInsane = hsInsane1;

	return true;
}

void CMainWnd::OnSolved(int nLevel)
{
	bool m_bDirty = m_GameCtrl.IsDirty();
	ATLTRACE("OnSolved Level: %d Dirty: %s\n", nLevel, m_bDirty ? "true" : "false");
	if (nLevel != TASK_EASY && nLevel != TASK_NORMAL && nLevel != TASK_HARD && nLevel != TASK_INSANE)
		return;

#if !defined(_DEBUG)
	if (!m_bDirty)
		return;
	m_GameCtrl.SetDirty();
#endif

	bool bWinner = false;
	const CTimeSpan span(m_GameCtrl.GetTimeSpan());
	HiScoreTable *HiScore = nullptr;
	int level = 0;
	switch (nLevel)
	{
	case TASK_EASY:
		HiScore = &hsEasy;
		level = IDS_EASYGAME;
		break;
	case TASK_NORMAL:
		HiScore = &hsNormal;
		level = IDS_NORMALGAME;
		break;
	case TASK_HARD:
		HiScore = &hsHard;
		level = IDS_HARDGAME;
		break;
	case TASK_INSANE:
		HiScore = &hsInsane;
		level = IDS_INSANEGAME;
		break;
	}
	ATLASSUME(HiScore);
	if (HiScore->size() < HISCORE_MAX_AMOUNT)
		bWinner = true;
	else
		for_each(HiScore->cbegin(), HiScore->cend(), [&bWinner, span](const HiScoreEntry &h) { if (h.m_Time > span.GetTotalSeconds()) { bWinner = true; }});
	if (bWinner)
	{
		ATLTRACE("We have a winner! Level: %d Time: %d\n", nLevel, span.GetTotalSeconds());
		CWinnerDlg m_namedlg;
		CString strHeader;
		CString strGame;
		strGame.LoadString(level);
		std::stringstream buf;
		buf << std::setfill('0') << std::setw(2) << span.GetTotalHours() << ':' << std::setfill('0') << std::setw(2) << span.GetMinutes() << ':' << std::setfill('0') << std::setw(2) << span.GetSeconds();
		strHeader.Format(IDS_WINNER_HEADER, strGame, buf.str());
		m_namedlg.SetHeader(strHeader);
		auto res = m_namedlg.DoModal();
		if (IDOK == res)
		{
			HiScoreEntry entry(m_namedlg.GetName(), span.GetTotalSeconds());
			HiScore->push_back(entry);
			HiScore->sort();
		}
	}
	UISetText(2, "Game over", TRUE);
}

BOOL CMainWnd::OnIdle()
{
	CString strFormat;
	bool bNeedUpdate = false;	// Flicker free update
	strFormat.Format(IDS_CURSORFORMAT, m_GameCtrl.GetCursorX(), m_GameCtrl.GetCursorY());
	if (m_GameCtrl.isDirty())
		strFormat += '*';
	if (strFormat != m_strStatusCursor)
	{
		bNeedUpdate = true;
		UISetText(0, m_strStatusCursor = strFormat);
	}
	strFormat.Format(IDS_COUNTFORMAT, m_GameCtrl.Count(), m_GameCtrl.ReadOnly());
	if (strFormat != m_strStatusCount)
	{
		bNeedUpdate = true;
		UISetText(1, m_strStatusCount = strFormat);
	}
	if (m_strStatus3Prev != m_strStatus3)
	{
		bNeedUpdate = true;
		UISetText(2, m_strStatus3Prev = m_strStatus3);
	}
	if (bNeedUpdate)
		UIUpdateStatusBar();
	return FALSE;
}

void CMainWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != ID_TIMER1)
		return;
	bool bNeedUpdate = false;
	const auto span = m_GameCtrl.GetTimeSpan();
	if (m_tStatusTime != span)
	{
		bNeedUpdate = true;
		CTimeSpan s(span);
		std::stringstream buf;
		buf << std::setfill('0') << std::setw(2) << s.GetTotalHours() << ':' << std::setfill('0') << std::setw(2) << s.GetMinutes() << ':' << std::setfill('0') << std::setw(2) << s.GetSeconds();
		UISetText(3, buf.str().c_str());
		m_tStatusTime = span;
	}
	if (bNeedUpdate)
		UIUpdateStatusBar();
}
