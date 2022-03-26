#pragma once
#include <array>

class CCell
{
public:
	CELLVAL m_nVal;
	CELLHYPO m_nHypo;
	CCell() : m_nVal(0U), m_nHypo(0U) {}
	CCell(const CCell &c) : m_nVal(c.m_nVal), m_nHypo(c.m_nHypo) {}
	CCell(CCell &&cell) noexcept : m_nVal(cell.m_nVal), m_nHypo(cell.m_nHypo) {}
	~CCell() = default;
	CCell &operator=(const CCell &cell)
	{
		m_nVal = cell.m_nVal;
		m_nHypo = cell.m_nHypo;
		return *this;
	}
	CCell &operator=(CCell &&cell) noexcept
	{
		m_nVal = cell.m_nVal;
		m_nHypo = cell.m_nHypo;
		return *this;
	}
	void Init()
	{
		m_nVal = 0U;
		m_nHypo = 0U;
	}
	bool GetHypothesis(unsigned bit) const
	{
		ATLASSUME(bit >= 1U && bit <= 9U);
		const auto b = 1U << (bit - 1U);
		return m_nHypo & b;
	}
	void SetHypothesis(unsigned bit, BOOL hypothesis = TRUE)
	{
		ATLASSUME(bit >= 1U && bit <= 9U);
		if (FALSE == hypothesis)
			ClearHypothesis(bit);
		else
		{
			const auto b = 1U << (bit - 1U);
			m_nHypo |= b;
		}
	}
	void ClearHypothesis(unsigned bit)
	{
		ATLASSUME(bit >= 1U && bit <= 9U);
		const auto b = 1U << (bit - 1U);
		m_nHypo &= ~b;
	}
	void SetHypothesisAll()
	{
		m_nHypo |= 0x1FFU;	// 9 bits
	}
	void ClearHypothesisAll()
	{
		m_nHypo &= ~0x1FFU;	// 9 bits
	}
	void InvertHypothesis(unsigned bit)
	{
		ATLASSUME(bit >= 1U && bit <= 9U);
		const auto b = 1U << (bit - 1U);
		m_nHypo ^= b;
	}
	bool ReadOnly() const
	{
		return m_nHypo & ROBIT;
	}
	void ReadOnly(bool r)
	{
		if (r)
			m_nHypo |= ROBIT;
		else
			m_nHypo &= ~ROBIT;
	}
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive &ar, const unsigned int /*version*/)
	{
		ar & m_nVal & m_nHypo;
	}
};
BOOST_CLASS_VERSION(CCell, 1)

struct CCounters : public std::array<unsigned, 9U>
{
	unsigned total;
	unsigned readonly;
	CCounters() : total(0U), readonly(0U) {}
};

typedef std::array<CCell, 81U> CCellArray;
