#pragma once
#include <string>

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
private:
	std::string m_strVersion;

public:
	constexpr static int IDD = IDD_ABOUTBOX;

	CAboutDlg() = default;
	~CAboutDlg() = default;
	BOOL OnInitDialog(CWindow /*wndFocus*/, LPARAM /*lInitParam*/)
	{
		::SetDlgItemText(m_hWnd, IDC_STATIC_VERSION, m_strVersion.c_str());
		CenterWindow(GetParent());
		return TRUE;
	}

	void OnCloseCmd(UINT /*uNotifyCode*/, int nID, CWindow /*wndCtl*/)
	{
		EndDialog(nID);
	}
	void SetVersion(LPCTSTR strVersion)
	{
		m_strVersion = strVersion;
		if (::IsWindow(m_hWnd))
			::SetDlgItemText(m_hWnd, IDC_STATIC_VERSION, m_strVersion.c_str());
	}

protected:
	BEGIN_MSG_MAP(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDOK, OnCloseCmd)
	END_MSG_MAP()
};
