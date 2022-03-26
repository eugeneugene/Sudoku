#pragma once

#include "targetver.h"

#include "resource.h"

#if defined(_DEBUG)
#define BOOST_LIB_DIAGNOSTIC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>  
#endif

//#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

#pragma component(browser, off, references)
#include <atlbase.h>
#pragma warning(disable : 4996)
#include <wtl/atlapp.h>
#pragma warning(default : 4996)
#pragma component(browser, on, references)

extern CAppModule _Module;

#include <atlwin.h>

#include <wtl/atlcrack.h>
#include <wtl/atlctrls.h>
#include <shellapi.h>	// Эту херню нужно вставлять перед atlctrlx.h
#include <wtl/atlctrlx.h>
#include <wtl/atlframe.h>
#include <wtl/atlddx.h>
#include <wtl/atlmisc.h>
#include <atltime.h>
#include <wtl/atldlgs.h>

#include <iomanip>
#include <sstream>
#include <fstream>
#include <random>
#include <boost/filesystem.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>

#define DRAW_SHADOW
//#define _DEBUG_PAINT

constexpr auto BACKGRND =				RGB(255, 255, 240);
constexpr auto BACKGRND_SOLVED =		RGB(240, 255, 240);
constexpr auto THINLINE =				RGB(127, 127, 127);
constexpr auto THICKLINE				RGB(  0,   0,   0);
constexpr auto CURSOR_DEC_BRUSH =		RGB(220, 220, 255);
constexpr auto CURSOR_DEC =				RGB(  0,   0, 240);
constexpr auto CURSOR_HYP_BRUSH =		RGB(255, 220, 220);
constexpr auto CURSOR_HYP =				RGB(240,   0,   0);
constexpr auto CURSOR_ERR_BRUSH =		RGB(255,  64, 127);
constexpr auto DIGIT_RO =				RGB(  0,   0,   0);
constexpr auto DIGIT_DEC =				RGB(  0,   0, 240);
constexpr auto DIGIT_HYP =				RGB(240,  64,  64);
constexpr auto DIGIT_SOLVED =			RGB(240,  64,  64);
constexpr auto BACKGRND_ERR =			RGB(255,  64,  64);

#if defined(DRAW_SHADOW)
constexpr auto CURSOR_BRUSH =			RGB(200, 220, 235);
constexpr auto CURSOR_SHA_BRUSH =		RGB(220, 240, 255);
constexpr auto CURSOR_SHA_ERR =			RGB(240,  64, 255);
constexpr auto CURSOR_SHA_ERR_BRUSH =	RGB(255,  64, 255);
#endif

typedef UINT8 CELLVAL;
typedef UINT16 CELLHYPO;

typedef struct tagUPOINT
{
	unsigned int x;
	unsigned int y;
} UPOINT;

typedef std::array<bool, 9U> arHints;

inline char Num2Char(const unsigned n)
{
	ATLASSUME(n < 10);
	const char strNum2Char[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	return strNum2Char[n % 10];
}

constexpr unsigned ROBIT = 0x8000;
constexpr int SDK_DEC				= 0;
constexpr int SDK_HYP				= 1;
constexpr int SNAPSHOT_NAME_SIZE	= 20;
constexpr int SNAPSHOT_MAX_AMOUNT	= 20;
constexpr int SNAPSHOT_USER_ID		= 0x9000;
constexpr int HISCORE_MAX_AMOUNT	= 10;
constexpr int HISCORE_NAME_SIZE		= 40;

constexpr int MAGIC					= 0xAAFF55BB;
constexpr int WM_UPDATEHINTCTRL		= WM_USER + 1;
constexpr int WM_HINTDOWN			= WM_USER + 2;
constexpr int WM_UPDATEUNDO			= WM_USER + 3;
constexpr int WM_UPDATEREDO			= WM_USER + 4;
constexpr int WM_SETSTATUS3			= WM_USER + 5;
constexpr int WM_SOLVED				= WM_USER + 6;
constexpr int WM_GETFIELD           = WM_USER + 7;
constexpr int WM_GETHINT            = WM_USER + 8;
constexpr int ID_TIMER1				= 1;

constexpr UPOINT InvalidCursor		= { 0U, 0U };

#define GRIDBOLDFIXEDROW
//#define GRIDBOLDCELL
//#define GRIDMONOFIXEDROW
#define GRIDMONOCELL
#define GRIDFIXEDROWMARGIN 3
#define GRIDCELLMARGIN 2

constexpr char defaultname[]		= "default.dat";
constexpr char topname[]			= "top.dat";


// void OnUpdateCtrl(WPARAM nVal, LPARAM nHypo)
#define MSG_WM_UPDATEHINTCTRL(func) \
	if (WM_UPDATEHINTCTRL == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func(wParam, lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

// void OnHintDown(int nHint)
#define MSG_WM_HINTDOWN(func) \
	if (WM_HINTDOWN == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func((int) lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

// void OnUpdateUndo(BOOL bEnabled)
#define MSG_WM_UPDATEUNDO(func) \
	if (WM_UPDATEUNDO == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func((BOOL) lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

// void OnUpdateRedo(BOOL bEnabled)
#define MSG_WM_UPDATEREDO(func) \
	if (WM_UPDATEREDO == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func((BOOL) lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

// void OnSetStatus3(LPCSTR strStatus3)
#define MSG_WM_SETSTATUS3(func) \
	if (WM_SETSTATUS3 == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func((LPCSTR) lParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

// void OnSolved(int Level)
#define MSG_WM_SOLVED(func) \
	if (WM_SOLVED == uMsg) \
{ \
	SetMsgHandled(TRUE); \
	func(wParam); \
	if(IsMsgHandled()) \
	return TRUE; \
}

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
