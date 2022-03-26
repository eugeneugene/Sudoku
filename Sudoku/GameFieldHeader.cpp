#include "stdafx.h"
#include "GameFieldHeader.h"

CGameFieldHeader& CGameFieldHeader::operator=(CGameFieldHeader&& header) noexcept
{
	m_nMagic = header.m_nMagic;
	m_nGameFieldSize = header.m_nGameFieldSize;
	m_tStart = header.m_tStart;
	m_tSolved = header.m_tSolved;
	m_nLevel = header.m_nLevel;
	m_bDirty = header.m_bDirty;
	return *this;
}

bool CGameFieldHeader::Check() const
{
	const CGameFieldHeader h;
	return (*this == h);
}
