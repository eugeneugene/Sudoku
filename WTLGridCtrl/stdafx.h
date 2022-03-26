#pragma once

#include "targetver.h"

#include <locale.h>
#include <wctype.h>

// To avoid to BSCMAKE Error
#pragma component(browser, off, references)
#include <mshtml.h>
#pragma component(browser, on, references)

#include <algorithm>
#include <atlbase.h>
#include <wtl/atlapp.h>
#include <wtl/atlmisc.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <wtl/atlframe.h>
#include <wtl/atlctrls.h>
#include <wtl/atldlgs.h>
#include <wtl/atlcrack.h>
#include <ShellAPI.h>
#include <wtl/atlctrlx.h>
#include <atlfile.h>

//#include <algorithm>
#include <time.h>

#ifdef BEGIN_MSG_MAP
#undef BEGIN_MSG_MAP
#endif

#define BEGIN_MSG_MAP BEGIN_MSG_MAP_EX_
#define BEGIN_MSG_MAP_EX_(theClass) \
public: \
	BOOL m_bMsgHandled; \
	BOOL IsMsgHandled() const \
	{ \
		return m_bMsgHandled; \
	} \
	void SetMsgHandled(BOOL bHandled) \
	{ \
		m_bMsgHandled = bHandled; \
	} \
	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0) override \
	{ \
		BOOL bOldMsgHandled = m_bMsgHandled; \
		BOOL bRet = _ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID); \
		m_bMsgHandled = bOldMsgHandled; \
		return bRet; \
	} \
	BOOL _ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID) \
	{ \
		BOOL bHandled = TRUE; \
		(hWnd); \
		(uMsg); \
		(wParam); \
		(lParam); \
		(lResult); \
		(bHandled); \
		switch(dwMsgMapID) \
		{ \
		case 0:

#if defined(_M_IX86)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_IA64)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined(_M_X64)
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
