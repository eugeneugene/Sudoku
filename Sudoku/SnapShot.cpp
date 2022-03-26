#include "stdafx.h"
#include "SnapShot.h"

CSnapShot::CSnapShot(const CSnapShot &snapshot) : CGameField(snapshot)
{
	m_SnapShotName = snapshot.m_SnapShotName;
	m_SnapTime = snapshot.m_SnapTime;
}

CSnapShot::CSnapShot(CSnapShot &&snapshot) noexcept : CGameField(std::move(snapshot))
{
	m_SnapShotName.swap(snapshot.m_SnapShotName);
	std::swap(m_SnapTime, snapshot.m_SnapTime);
}

CSnapShot::CSnapShot(const CGameField &field, const char *name, time_t time) : CGameField(field), m_SnapTime(time)
{
	const auto len = strlen(name);
	size_t i = 0;
	for (auto it = m_SnapShotName.begin(); it < m_SnapShotName.end(); it++)
	{
		*it = (i > len) ? '\0' : name[i++];
	}
}

CSnapShot &CSnapShot::operator=(const CSnapShot &snaphot)
{
	m_SnapShotName = snaphot.m_SnapShotName;
	m_SnapTime = snaphot.m_SnapTime;
	_Assign(snaphot);
	return *this;
}

CSnapShot &CSnapShot::operator=(CSnapShot &&snapshot) noexcept
{
	m_SnapShotName.swap(snapshot.m_SnapShotName);
	std::swap(m_SnapTime, snapshot.m_SnapTime);
	_Assign(std::move(snapshot));
	return *this;
}
