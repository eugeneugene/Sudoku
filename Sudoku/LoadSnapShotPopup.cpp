#include "stdafx.h"
#include "LoadSnapShotPopup.h"

void CLoadSnapShotPopup::MakeGrey()
{
	CMenuItemInfo info;
	info.fMask = MIIM_SUBMENU | MIIM_STATE;
	info.hSubMenu = NULL;
	info.fState = MFS_GRAYED;
	m_SubMenu.SetMenuItemInfo(ID_GAME_LOADSNAPSHOT, FALSE, &info);
}
void CLoadSnapShotPopup::MakeSubMenu()
{
	CMenuItemInfo info;
	info.fMask = MIIM_SUBMENU | MIIM_STATE;
	info.hSubMenu = m_PopupMenu;
	info.fState = MFS_ENABLED;
	BOOL r = m_SubMenu.SetMenuItemInfo(ID_GAME_LOADSNAPSHOT, FALSE, &info);
	ATLASSERT(r);
}

CLoadSnapShotPopup::~CLoadSnapShotPopup()
{
	if (m_PopupMenu)
		m_PopupMenu.DestroyMenu();
}
void CLoadSnapShotPopup::Init(const HWND hWnd)
{
	CMenuHandle MainMenu{ ::GetMenu(hWnd) };
	m_SubMenu = MainMenu.GetSubMenu(1);
	MakeGrey();
}
void CLoadSnapShotPopup::Reset()
{
	if (nItems)
	{
		if (m_PopupMenu)
			m_PopupMenu.DestroyMenu();
		MakeGrey();
		nItems = 0;
	}
}
void CLoadSnapShotPopup::AddItem(LPCSTR sItem)
{
	if (!m_PopupMenu)
		m_PopupMenu.CreatePopupMenu();
	m_PopupMenu.AppendMenu(MF_STRING, SNAPSHOT_USER_ID + nItems, sItem);
	if (!nItems)
		MakeSubMenu();
	nItems++;
}
