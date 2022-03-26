#pragma once
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "task.h"
#include "Cell.h"

struct CFieldError
{
	enum { rgnNoError = 0, rgnLine, rgnRow, rgnSquare } m_Error;
	unsigned m_nErrorRgn;
	CFieldError() : m_Error(rgnNoError), m_nErrorRgn(0U) {}
	~CFieldError() = default;
	void Init()
	{
		m_Error = rgnNoError;
		m_nErrorRgn = 0U;
	}
	bool IsPointInRegion(const UPOINT &point) const;
};

class CGameField
{
	CCellArray m_FieldData = {};

	bool CheckLine(unsigned line) const;
	bool CheckRow(unsigned row) const;
	bool CheckSquare(unsigned square) const;

public:
	CGameField() = default;
	CGameField(const CGameField &gamefield) : m_FieldData(gamefield.m_FieldData) {}
	CGameField(CGameField &&gamefield) noexcept : m_FieldData(std::move(gamefield.m_FieldData)) {}
	explicit CGameField(const _TASK &t) { _Assign(t); }
	explicit CGameField(const CTask &t) { _Assign(t); }
	~CGameField() = default;

	CGameField &operator=(const CCellArray &array)
	{
		_Assign(array);
		return *this;
	}

	CGameField &operator=(const CGameField &gamefield)
	{
		_Assign(gamefield);
		return *this;
	}

	CGameField &operator=(CGameField &&gamefield) noexcept
	{
		_Assign(std::move(gamefield));
		return *this;
	}

	CGameField &operator=(const _TASK &t)
	{
		_Assign(t);
		return *this;
	}

	CGameField &operator=(const CTask &t)
	{
		_Assign(t);
		return *this;
	}

protected:
	void _Assign(const CCellArray &array) { m_FieldData = array; }
	void _Assign(const CGameField &gamefield) { m_FieldData = gamefield.m_FieldData; }
	void _Assign(CGameField &&gamefield) { m_FieldData.swap(gamefield.m_FieldData); }
	void _Assign(const _TASK &t);
	void _Assign(const CTask &t);

public:
	void Init()
	{
		m_FieldData = {};
	}
	bool Check(const UPOINT &point, _Out_ CFieldError *pError) const;
	bool CheckField(CFieldError _Out_ *pError) const;
	bool Hint(const UPOINT &point, _Out_ arHints *parHints) const;
	unsigned GetSingleHint(const UPOINT &point) const;
	unsigned GetRegionHint(_Out_ UPOINT *point) const;
	void Count(_Out_ CCounters *counters) const;
	void Reset();
	const CCellArray& CData() const
	{
		return m_FieldData;
	}
	CCellArray& Data()
	{
		return m_FieldData;
	}
	CCell &operator[](size_t i)
	{
		return m_FieldData[i];
	}
	// Static members
	static UPOINT GetPointIndex(unsigned i);
	static unsigned GetIndex(const UPOINT &point);
	static unsigned GetIndexLine(unsigned row, unsigned line);
	static unsigned GetIndexSquare(unsigned square, unsigned cell);
	static unsigned GetSquareIndex(unsigned i);
	static unsigned GetSquareIndex(const UPOINT &point);
	static unsigned GetCellIndex(unsigned i);
	static unsigned GetCellIndex(const UPOINT &point);
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive &ar, const unsigned int /*version*/)
	{
		ar & m_FieldData;
	}
};
BOOST_CLASS_VERSION(CGameField, 1)
