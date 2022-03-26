#pragma once

class CMakeSnapShotDlg : public CDialogImpl<CMakeSnapShotDlg>
{
	CString m_strSnapShotName;
public:
	constexpr static int IDD = IDD_SNAPSHOTDLG;

	CMakeSnapShotDlg() : m_bMsgHandled(FALSE) {}

	~CMakeSnapShotDlg() = default;

	const char *GetName() const
	{
		return m_strSnapShotName;
	}

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		wndFocus = GetDlgItem(IDC_EDIT_NAME);
		return TRUE;
	}

	BEGIN_MSG_MAP(CMakeSnapShotDlg)
		COMMAND_ID_HANDLER_EX(IDOK, OnOkClicked)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancelClicked)
		MSG_WM_INITDIALOG(OnInitDialog)
	END_MSG_MAP()

	void OnOkClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		GetDlgItemText(IDC_EDIT_NAME, m_strSnapShotName);
		EndDialog(IDOK);
	}
	void OnCancelClicked(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wndCtl*/)
	{
		EndDialog(IDCANCEL);
	}
};
