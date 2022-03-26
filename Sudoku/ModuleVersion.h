////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
//
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

#pragma once

// tell linker to link with version.lib for VerQueryValue, etc.
#pragma comment(linker, "/defaultlib:version.lib")


//////////////////
// CModuleVersion version info about a module.
// To use:
//
// CModuleVersion ver
// if (ver.GetFileVersionInfo("_T("mymodule))) {
//		// info is in ver, you can call GetValue to get variable info like
//		CString s = ver.GetValue(_T("CompanyName"));
// }
//
// You can also call the static fn DllGetVersion to get DLLVERSIONINFO.
//
class CModuleVersion : public VS_FIXEDFILEINFO {
protected:
	BYTE* m_pVersionInfo;	// all version info

	struct TRANSLATION {
		WORD langID;			// language ID
		WORD charset;			// character set (code page)
	} m_translation{};

public:
	CModuleVersion() : tagVS_FIXEDFILEINFO(), m_pVersionInfo(nullptr)
	{}
	virtual ~CModuleVersion()
	{
		delete[] m_pVersionInfo;
	}

	BOOL GetFileVersionInfo(LPCTSTR modulename);
	BOOL GetValue(LPCTSTR lpKeyName, LPTSTR *lpszRetVal) const;
	static BOOL DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi);
};
