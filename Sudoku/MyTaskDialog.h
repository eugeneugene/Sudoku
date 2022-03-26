#pragma once

#if defined(_DEBUG)

class CMyTaskDialog : public CDialogImpl<CMyTaskDialog>
{
	CString m_strText;
protected:
	BEGIN_MSG_MAP(CTaskDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_COMMAND(OnCommand)
	END_MSG_MAP()
	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
	{
		CenterWindow();
		auto wnd = static_cast<CEdit>(GetDlgItem(IDC_DATAEDIT));
		wnd.SetWindowText(m_strText);
		wnd.SetFocus();
		wnd.SetSelAll();
		return FALSE;
	}
	void OnClose()
	{
		EndDialog(IDCANCEL);
	}
	void OnCommand(UINT /*uNotifyCode*/, const int nID, CWindow /*wndCtl*/)
	{
		if (IDC_CLOSE == nID)
			EndDialog(IDOK);
	}

	const int IDD = IDD_TASKDLG;
	void SetText(const LPCSTR strText)
	{
		m_strText = strText;
	}
};

#endif	// _DEBUG
