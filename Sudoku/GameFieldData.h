#pragma once
#include <boost/serialization/split_member.hpp>
#include "GameField.h"
#include "GameFieldHeader.h"
#include "SnapShot.h"

struct CUndo
{
	CCell cell;
	UPOINT pos;
private:
	friend class boost::serialization::access;
	template <class Archive> void serialize(Archive& ar, const unsigned int /*version*/)
	{
		ar & cell & pos.x & pos.y;
	}
};
BOOST_CLASS_VERSION(CUndo, 1)

class CGameFieldFileData
{
protected:
	CGameField m_GameField;
	CSnapShotDeque m_SnapShots;
	time_t m_tStart;
	time_t m_tSolved;
	int m_nLevel;
	bool m_bDirty;

public:
	CGameFieldFileData() : m_tStart(0), m_tSolved(0), m_nLevel(0), m_bDirty(false) {}
	CGameFieldFileData(const CGameFieldFileData &data) : m_GameField(data.m_GameField), m_SnapShots(data.m_SnapShots), m_tStart(data.m_tStart),
		m_tSolved(data.m_tSolved), m_nLevel(data.m_nLevel), m_bDirty(data.m_bDirty) {}
	CGameFieldFileData(CGameFieldFileData &&data) noexcept : m_GameField(std::move(data.m_GameField)),
		m_SnapShots(std::move(data.m_SnapShots)), m_tStart(std::move(data.m_tStart)), m_tSolved(std::move(data.m_tSolved)),
		m_nLevel(std::move(data.m_nLevel)), m_bDirty(std::move(data.m_bDirty)) {}
	CGameFieldFileData &operator=(const CGameFieldFileData &data)
	{
		m_GameField = data.m_GameField;
		m_SnapShots = data.m_SnapShots;
		m_tStart = data.m_tStart;
		m_tSolved = data.m_tSolved;
		m_nLevel = data.m_nLevel;
		m_bDirty = data.m_bDirty;
		return *this;
	}
	CGameFieldFileData &operator=(CGameFieldFileData &&data) noexcept;
	~CGameFieldFileData() = default;

	bool IsDirty() { return m_bDirty; }
	void SetDirty() { m_bDirty = true; }

private:
	friend class boost::serialization::access;
	template <class Archive> void save(Archive& ar, const unsigned int /*version*/) const
	{
		CGameFieldHeader header;
		header.Start(m_tStart);
		header.Solved(m_tSolved);
		header.Level(m_nLevel);
		header.Dirty(m_bDirty);
		ar & header & m_GameField & m_SnapShots;
	}
	template <class Archive> void load(Archive& ar, const unsigned int /*version*/)
	{
		CGameFieldHeader header;
		ar & header;
		if (!header.Check())
			throw std::exception("Unknown header");
		ar & m_GameField & m_SnapShots;
		m_tStart = header.Start();
		m_tSolved = header.Solved();
		m_nLevel = header.Level();
		m_bDirty = header.Dirty();
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};
BOOST_CLASS_VERSION(CGameFieldFileData, 1)

class CGameFieldSerialData : public CGameFieldFileData
{
protected:
	CGameFieldSerialData() = default;
	~CGameFieldSerialData() = default;
	std::deque<CUndo> m_Undo;
	std::deque<CUndo> m_Redo;
private:
	friend class boost::serialization::access;
	template <class Archive> void serialize(Archive& ar, const unsigned int /*version*/)
	{
		ar & boost::serialization::base_object<CGameFieldFileData>(*this);
		ar & m_Undo & m_Redo;
	}
};
BOOST_CLASS_VERSION(CGameFieldSerialData, 1)
