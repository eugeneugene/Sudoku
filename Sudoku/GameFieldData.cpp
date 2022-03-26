#include "stdafx.h"
#include "GameFieldData.h"

CGameFieldFileData &CGameFieldFileData::operator=(CGameFieldFileData &&data) noexcept
{
	m_GameField = std::move(data.m_GameField);
	m_SnapShots = std::move(data.m_SnapShots);
	m_tStart = data.m_tStart;
	m_tSolved = data.m_tSolved;
	m_nLevel = data.m_nLevel;
	m_bDirty = data.m_bDirty;
	return *this;
}
