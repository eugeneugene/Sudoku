#pragma once

class CLoadSnapShotPopup
{
	CMenuHandle m_PopupMenu;
	CMenuHandle m_SubMenu;
	bool bHasItems;
	unsigned nItems;

	void MakeGrey();
	void MakeSubMenu();
public:
	CLoadSnapShotPopup() : m_PopupMenu(NULL), m_SubMenu(NULL), bHasItems(false), nItems(0) {}
	~CLoadSnapShotPopup();
	void Init(HWND hWnd);
	void Reset();
	void AddItem(LPCSTR sItem);
};