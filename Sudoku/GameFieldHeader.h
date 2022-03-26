#pragma once
#include <boost/serialization/split_member.hpp>
#include "GameField.h"

class CGameFieldHeader
{
	long m_nMagic;
	size_t m_nGameFieldSize;
	time_t m_tStart;
	time_t m_tSolved;
	int m_nLevel;
	bool m_bDirty;
public:
	CGameFieldHeader() : m_nMagic(MAGIC), m_nGameFieldSize(sizeof(CGameField)), m_tStart(0), m_tSolved(0), m_nLevel(0), m_bDirty(false) {}
	CGameFieldHeader(const CGameFieldHeader &header) : m_nMagic(header.m_nMagic), m_nGameFieldSize(header.m_nGameFieldSize),
		m_tStart(header.m_tStart), m_tSolved(header.m_tSolved), m_nLevel(header.m_nLevel), m_bDirty(header.m_bDirty) {}
	CGameFieldHeader(CGameFieldHeader &&header) noexcept : m_nMagic(header.m_nMagic), m_nGameFieldSize(header.m_nGameFieldSize),
		m_tStart(header.m_tStart), m_tSolved(header.m_tSolved), m_nLevel(header.m_nLevel), m_bDirty(header.m_bDirty) {}
	CGameFieldHeader &operator=(const CGameFieldHeader& header)
	{
		m_nMagic = header.m_nMagic;
		m_nGameFieldSize = header.m_nGameFieldSize;
		m_tStart = header.m_tStart;
		m_tSolved = header.m_tSolved;
		m_nLevel = header.m_nLevel;
		m_bDirty = header.m_bDirty;
		return *this;
	}
	CGameFieldHeader& operator=(CGameFieldHeader&& header) noexcept;
	~CGameFieldHeader() = default;
	bool Check() const;
	bool operator==(const CGameFieldHeader &h) const
	{
		return m_nMagic == MAGIC && m_nGameFieldSize == h.m_nGameFieldSize;
	}
	bool operator!=(const CGameFieldHeader &h) const
	{
		return !(*this == h);
	}
	time_t Start() const { return m_tStart; }
	time_t Start(const time_t Elapsed) { return m_tStart = Elapsed; }
	time_t Solved() const { return m_tSolved; }
	time_t Solved(const time_t Solved) { return m_tSolved = Solved; }
	int Level() const { return m_nLevel; }
	int Level(const UINT8 nLevel) { return m_nLevel = nLevel; }
	bool Dirty() const { return m_bDirty; }
	bool Dirty(const bool Dirty) { return m_bDirty = Dirty; }

private:
	friend class boost::serialization::access;
	template <class Archive> void save(Archive& ar, const unsigned int /*version*/) const
	{
		char level = m_nLevel;
		char dirty = m_bDirty;
		ar & m_nMagic & m_nGameFieldSize & m_tStart & m_tSolved & level & dirty;
	}
	template <class Archive> void load(Archive& ar, const unsigned int /*version*/)
	{
		char level = m_nLevel;
		char dirty = m_bDirty;
		ar & m_nMagic & m_nGameFieldSize & m_tStart & m_tSolved & level & dirty;
		m_nLevel = level;
		m_bDirty = dirty;
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};
BOOST_CLASS_VERSION(CGameFieldHeader, 1)
