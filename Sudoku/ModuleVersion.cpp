////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//
#include "StdAfx.h"
#include "ModuleVersion.h"


//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL CModuleVersion::GetFileVersionInfo(LPCTSTR modulename)
{
	m_translation.charset = 1252;		// default = ANSI code page
	//memset(static_cast<VS_FIXEDFILEINFO*>(this), 0, sizeof(VS_FIXEDFILEINFO));
	auto& v = *static_cast<VS_FIXEDFILEINFO*>(this);
	v = {};

	// get module handle
	TCHAR filename[_MAX_PATH];
	const auto hModule = ::GetModuleHandle(modulename);
	if (nullptr == hModule && nullptr != modulename)
		return FALSE;

	// get module file name
	auto len = GetModuleFileName(hModule, filename, sizeof(filename)/sizeof(filename[0]));
	if (len <= 0)
		return FALSE;

	// read file version info
	DWORD dwDummyHandle; // will always be set to zero
	len = GetFileVersionInfoSize(filename, &dwDummyHandle);
	if (len <= 0)
		return FALSE;

	m_pVersionInfo = new BYTE[len]; // allocate version info
	if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
		return FALSE;

	LPVOID lpvi;
	UINT iLen;
	if (!VerQueryValue(m_pVersionInfo, _T("\\"), &lpvi, &iLen))
		return FALSE;

	// copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
	*static_cast<VS_FIXEDFILEINFO*>(this) = *static_cast<VS_FIXEDFILEINFO*>(lpvi);

	// Get translation info
	if (VerQueryValue(m_pVersionInfo, _T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4)
		m_translation = *static_cast<TRANSLATION*>(lpvi);

	return dwSignature == VS_FFI_SIGNATURE;
}


//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// returns the value as a CString.
//
BOOL CModuleVersion::GetValue(const LPCTSTR lpKeyName, LPTSTR *lpszRetVal) const
{
	if (m_pVersionInfo)
	{
		// To get a string value must pass query in the form
		//
		//    "\StringFileInfo\<langID><codepage>\keyname"
		//
		// where <lang-codepage> is the languageID concatenated with the
		// code page, in hex. Wow.
		//
		TCHAR query[128];
		_stprintf_s(query, _countof(query), _T("\\StringFileInfo\\%04x%04x\\%s"),
			m_translation.langID,
			m_translation.charset,
			lpKeyName);

		UINT iLenVal;
		static LPTSTR lpVal;
		if (VerQueryValue(m_pVersionInfo, query,
			reinterpret_cast<LPVOID *>(&lpVal), &iLenVal))
		{
			*lpszRetVal = lpVal;
			return TRUE;
		}
	}
	return FALSE;
}

// typedef for DllGetVersion proc
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

/////////////////
// Get DLL Version by calling DLL's DllGetVersion proc
//
BOOL CModuleVersion::DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi)
{
	const auto hinst = LoadLibrary(modulename);
	if (!hinst)
		return FALSE;

	// Must use GetProcAddress because the DLL might not implement 
	// DllGetVersion. Depending upon the DLL, the lack of implementation of the 
	// function may be a version marker in itself.
	//
	const auto pDllGetVersion =
		reinterpret_cast<DLLGETVERSIONPROC>(GetProcAddress(hinst, "DllGetVersion"));

	if (!pDllGetVersion)
		return FALSE;

	//memset(&dvi, 0, sizeof(dvi));			 // clear
	dvi = {};
	dvi.cbSize = sizeof(dvi);				 // set size for Windows

	return SUCCEEDED((*pDllGetVersion)(&dvi));
}
