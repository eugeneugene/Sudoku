#pragma once
#include <array>
#include <deque>
#include <list>
#include "GameField.h"

class CSnapShot : public CGameField
{
private:
	std::array<char, SNAPSHOT_NAME_SIZE> m_SnapShotName;
	time_t m_SnapTime;
public:
	CSnapShot() = default;
	CSnapShot(const CSnapShot &snapshot);
	CSnapShot(CSnapShot &&snapshot) noexcept;
	CSnapShot(const CGameField &field, const char *name, time_t time);
	CSnapShot &operator=(const CSnapShot &snaphot);
	CSnapShot &operator=(CSnapShot &&snapshot) noexcept;
	~CSnapShot() = default;

	const char *GetSnapShotName() const
	{
		return m_SnapShotName.data();
	}
	const time_t GetSnapTime() const
	{
		return m_SnapTime;
	}
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive &ar, const unsigned int version)
	{
		ar & boost::serialization::base_object<CGameField>(*this);
		ar & m_SnapShotName & m_SnapTime;
	}
};
BOOST_CLASS_VERSION(CSnapShot, 1)

typedef std::deque<CSnapShot> CSnapShotDeque;
typedef std::list<std::string> CSnapShotNames;
