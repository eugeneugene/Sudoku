#pragma once

class CWinnerDlg : public CDialogImpl<CWinnerDlg>, public CWinDataExchange<CWinnerDlg>
{
	CString m_strWinnerName;
	CString m_strHeader;
public:
	constexpr static int IDD = IDD_WINNERDLG;

	CWinnerDlg() : m_bMsgHandled(FALSE) { }
	~CWinnerDlg() = default;

	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
	{
		CenterWindow();
		DoDataExchange(DDX_LOAD, IDC_EDIT_HEADER);
		return TRUE;
	}

	const char *GetName() const
	{
		return m_strWinnerName;
	}

	void OnOkClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		DoDataExchange(DDX_SAVE, IDC_EDIT_NAME);
		EndDialog(IDOK);
	}

	void OnCancelClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		EndDialog(IDCANCEL);
	}

	void SetHeader(LPCSTR strHeader)
	{
		if (nullptr == strHeader)
			return;
		m_strHeader = strHeader;
	}

	BEGIN_MSG_MAP(CWinnerDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnOkClicked)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancelClicked)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CWinnerDlg)
		DDX_TEXT(IDC_EDIT_HEADER, m_strHeader)
		DDX_TEXT(IDC_EDIT_NAME, m_strWinnerName)
	END_DDX_MAP()
};
