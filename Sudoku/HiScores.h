#pragma once
#include <list>
#include <array>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/array.hpp>

struct HiScoreEntry
{
	std::array<char, HISCORE_NAME_SIZE> m_HiScoreEntryName;
	time_t m_Time;
	HiScoreEntry() : m_Time(0) { }
	HiScoreEntry(const char *name, time_t time)
	{
		const auto len = strlen(name);
		size_t i = 0;
		for (auto it = m_HiScoreEntryName.begin(); it < m_HiScoreEntryName.end(); it++)
		{
			*it = (i > len) ? '\0' : name[i++];
		}

		m_Time = time;
	}
	bool operator<(const HiScoreEntry& hs)
	{
		return m_Time < hs.m_Time;
	}
	/*bool operator>(const HiScoreEntry& hs)
	{
		return m_Time > hs.m_Time;
	}
	bool operator==(const HiScoreEntry& hs)
	{
		return m_Time == hs.m_Time;
	}*/
private:
	friend class boost::serialization::access;
	template <class Archive> void serialize(Archive& ar, const unsigned int /*version*/)
	{
		ar & m_HiScoreEntryName & m_Time;
	}
};
BOOST_CLASS_VERSION(HiScoreEntry, 1)

typedef std::list<HiScoreEntry> HiScoreTable;
