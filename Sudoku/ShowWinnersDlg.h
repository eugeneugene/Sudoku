#pragma once
#include "MyGridCtrl.h"
#include "HiScores.h"

class CShowWinnersDlg : public CDialogImpl<CShowWinnersDlg>, public CWinDataExchange<CShowWinnersDlg>
{
	HiScoreTable *m_HiScores;
	CMyGridCtrl m_Grid;

	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/);
	void InitGrid();
	static LONG GetFontHeightFromPoints(int points);

	void OnOkClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		EndDialog(IDOK);
	}

	void OnCancelClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		EndDialog(IDCANCEL);
	}

public:
	constexpr static int IDD = IDD_SHOWWINNERSDLG;

	CShowWinnersDlg() : m_HiScores(nullptr) { }
	~CShowWinnersDlg() = default;

	void HiScores(HiScoreTable *hs)
	{
		m_HiScores = hs;
	}

	BEGIN_MSG_MAP(CShowWinnersDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOkClicked)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancelClicked)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CShowWinnersDlg)
		DDX_CONTROL(IDC_WINNERS, m_Grid);
	END_DDX_MAP()
};
