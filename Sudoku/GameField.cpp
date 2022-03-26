#include "StdAfx.h"
#include "GameField.h"

bool CFieldError::IsPointInRegion(const UPOINT &point) const
{
	if (rgnNoError == m_Error || 0 == m_nErrorRgn)
		return false;

	switch (m_Error)
	{
	case rgnLine:
		if (point.y == m_nErrorRgn)
			return true;
		break;
	case rgnRow:
		if (point.x == m_nErrorRgn)
			return true;
		break;
	case rgnSquare:
		if (CGameField::GetSquareIndex(point) == static_cast<unsigned>(m_nErrorRgn))
			return true;
		break;
	default:
		return false;
	}
	return false;
}

void CGameField::Reset()
{
	for (auto &i : m_FieldData)
	{
		i.ClearHypothesisAll();
		if (!i.ReadOnly())
			i.m_nVal = 0;
	}
}

bool CGameField::CheckLine(unsigned line) const
{
	bool n1 = false; bool n2 = false; bool n3 = false;
	bool n4 = false; bool n5 = false; bool n6 = false;
	bool n7 = false; bool n8 = false; bool n9 = false;
	for (auto row = 1U; row <= 9U; row++)
	{
		switch (m_FieldData[GetIndexLine(row, line)].m_nVal)
		{
		case 1:
			if (n1)
				return false;
			n1 = true;
			break;
		case 2:
			if (n2)
				return false;
			n2 = true;
			break;
		case 3:
			if (n3)
				return false;
			n3 = true;
			break;
		case 4:
			if (n4)
				return false;
			n4 = true;
			break;
		case 5:
			if (n5)
				return false;
			n5 = true;
			break;
		case 6:
			if (n6)
				return false;
			n6 = true;
			break;
		case 7:
			if (n7)
				return false;
			n7 = true;
			break;
		case 8:
			if (n8)
				return false;
			n8 = true;
			break;
		case 9:
			if (n9)
				return false;
			n9 = true;
			break;
		}
	}
	return true;
}

bool CGameField::CheckRow(unsigned row) const
{
	bool n1 = false; bool n2 = false; bool n3 = false;
	bool n4 = false; bool n5 = false; bool n6 = false;
	bool n7 = false; bool n8 = false; bool n9 = false;
	for (auto line = 1U; line <= 9U; line++)
	{
		switch (m_FieldData[GetIndexLine(row, line)].m_nVal)
		{
		case 1:
			if (n1)
				return false;
			n1 = true;
			break;
		case 2:
			if (n2)
				return false;
			n2 = true;
			break;
		case 3:
			if (n3)
				return false;
			n3 = true;
			break;
		case 4:
			if (n4)
				return false;
			n4 = true;
			break;
		case 5:
			if (n5)
				return false;
			n5 = true;
			break;
		case 6:
			if (n6)
				return false;
			n6 = true;
			break;
		case 7:
			if (n7)
				return false;
			n7 = true;
			break;
		case 8:
			if (n8)
				return false;
			n8 = true;
			break;
		case 9:
			if (n9)
				return false;
			n9 = true;
			break;
		}
	}
	return true;
}

bool CGameField::CheckSquare(unsigned square) const
{
	bool n1 = false; bool n2 = false; bool n3 = false;
	bool n4 = false; bool n5 = false; bool n6 = false;
	bool n7 = false; bool n8 = false; bool n9 = false;
	for (auto c = 1U; c <= 9U; c++)
	{
		switch (m_FieldData[GetIndexSquare(square, c)].m_nVal)
		{
		case 1:
			if (n1)
				return false;
			n1 = true;
			break;
		case 2:
			if (n2)
				return false;
			n2 = true;
			break;
		case 3:
			if (n3)
				return false;
			n3 = true;
			break;
		case 4:
			if (n4)
				return false;
			n4 = true;
			break;
		case 5:
			if (n5)
				return false;
			n5 = true;
			break;
		case 6:
			if (n6)
				return false;
			n6 = true;
			break;
		case 7:
			if (n7)
				return false;
			n7 = true;
			break;
		case 8:
			if (n8)
				return false;
			n8 = true;
			break;
		case 9:
			if (n9)
				return false;
			n9 = true;
			break;
		}
	}
	return true;
}

bool CGameField::Check(const UPOINT &point, _Out_ CFieldError *pError) const
{
	pError->m_Error = CFieldError::rgnNoError;
	pError->m_nErrorRgn = 0U;
	if (!CheckLine(point.y))
	{
		pError->m_Error = CFieldError::rgnLine;
		pError->m_nErrorRgn = point.y;
		return false;
	}
	if (!CheckRow(point.x))
	{
		pError->m_Error = CFieldError::rgnRow;
		pError->m_nErrorRgn = point.x;
		return false;
	}
	if (!CheckSquare(GetSquareIndex(point)))
	{
		pError->m_Error = CFieldError::rgnSquare;
		pError->m_nErrorRgn = GetSquareIndex(point);
		return false;
	}
	return true;
}

bool CGameField::CheckField(_Out_ CFieldError *pError) const
{
	for (auto y = 1U; y <= 9U; y++) for (auto x = 1U; x <= 9U; x++)
	{
		if (!Check(UPOINT{ x, y }, pError))
			return false;
	}
	return true;
}

bool CGameField::Hint(const UPOINT &point, _Out_ arHints *parHints) const
{
	if (m_FieldData[GetIndex(point)].m_nVal)
		return false;

	for (auto i = 0U; i < 9U; i++)
		(*parHints)[i] = true;
	for (auto i = 1U; i <= 9U; i++)
	{
		const auto v = m_FieldData[GetIndexLine(point.x, i)].m_nVal;
		if (v)
			(*parHints)[v - 1] = false;
	}
	for (auto i = 1U; i <= 9U; i++)
	{
		const auto v = m_FieldData[GetIndexLine(i, point.y)].m_nVal;
		if (v)
			(*parHints)[v - 1] = false;
	}
	const auto s = GetSquareIndex(point);
	for (auto i = 1U; i <= 9U; i++)
	{
		const auto v = m_FieldData[GetIndexSquare(s, i)].m_nVal;
		if (v)
			(*parHints)[v - 1] = false;
	}
	return true;
}

// Возвращает номер подсказки, если эта единственная подсказка в этой ячейке, либо 0
unsigned CGameField::GetSingleHint(const UPOINT &point) const
{
	auto h = 0U;
	if (!m_FieldData[GetIndex(point)].m_nVal)
	{
		for (auto i = 1U; i <= 9U; i++)
		{
			if (m_FieldData[GetIndex(point)].GetHypothesis(i))
			{
				if (h)
					return 0U;
				h = i;
			}
		}
	}
	return h;
}

// Подчет количества значений в регионе
// Если равно 1, то это единственная такая цифра в данной области (строке, столбце, квадрате)
unsigned CGameField::GetRegionHint(_Out_ UPOINT *point) const
{
	// Двигаемся по строкам
	for (auto y = 1U; y <= 9U; y++)
	{
		for (auto i = 1U; i <= 9U; i++)
		{
			auto k = 0U;
			for (auto x = 1U; x <= 9U; x++)
			{
				const auto index = GetIndexLine(x, y);
				if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
					k++;
			}
			if (1U == k)	// В этой строке i встречается только один раз
			{
				unsigned x;
				for (x = 1U; x <= 9U; x++)
				{
					const auto index = GetIndexLine(x, y);
					if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
						break;
				}
				ATLASSUME(x >= 1U && x <= 9U);
				point->x = x;
				point->y = y;
				return i;
			}
		}
	}

	// Двигаемся по столбцам
	for (auto x = 1U; x <= 9U; x++)
	{
		for (auto i = 1U; i <= 9U; i++)
		{
			auto k = 0U;
			for (auto y = 1U; y <= 9U; y++)
			{
				const auto index = GetIndexLine(x, y);
				if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
					k++;
			}
			if (1U == k)	// В этом столбце i встречается только один раз
			{
				unsigned y;
				for (y = 1U; y <= 9U; y++)
				{
					const auto index = GetIndexLine(x, y);
					if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
						break;
				}
				ATLASSUME(y >= 1U && y <= 9U);
				point->x = x;
				point->y = y;
				return i;
			}
		}
	}

	// Двигаемся по квадратам
	for (auto s = 1U; s <= 9U; s++)
	{
		for (auto i = 1U; i <= 9U; i++)
		{
			unsigned k = 0U;
			for (auto c = 1U; c <= 9U; c++)
			{
				const auto index = GetIndexSquare(s, c);
				if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
					k++;
			}
			if (1U == k) // В этом квадрате i встречается только один раз
			{
				unsigned c;
				for (c = 1U; c <= 9U; c++)
				{
					const auto index = GetIndexSquare(s, c);
					if (!m_FieldData[index].m_nVal && m_FieldData[index].GetHypothesis(i))
						break;
				}
				ATLASSUME(c >= 1U && c <= 9U);
				const auto p = GetPointIndex(GetIndexSquare(s, c));
				point->x = p.x;
				point->y = p.y;
				return i;
			}
		}
	}
	return 0U;
}

void CGameField::Count(_Out_ CCounters *counters) const
{
	std::for_each(counters->begin(), counters->end(), [](unsigned &i) { i = 0U; });
	counters->total = 0U;
	counters->readonly = 0U;
	for (const auto &f : m_FieldData)
	{
		const auto val = f.m_nVal;
		if (val)
		{
			(*counters)[val - 1]++;
			counters->total++;
			if (f.ReadOnly())
				counters->readonly++;
		}
	}
}

void CGameField::_Assign(const _TASK &t)
{
	auto j = 0U;
	for (auto i = m_FieldData.begin(); i < m_FieldData.end() && j < t.size(); i++, j++)
	{
		i->m_nVal = t[j];
		i->ClearHypothesisAll();
		i->ReadOnly(t[j]);
	}
}

void CGameField::_Assign(const CTask &t)
{
	auto j = 0U;
	for (auto i = m_FieldData.begin(); i < m_FieldData.end() && j < 81U; i++, j++)
	{
		i->m_nVal = t[j];
		i->ClearHypothesisAll();
		i->ReadOnly(t[j]);
	}
}

UPOINT CGameField::GetPointIndex(unsigned i)
{
	const UPOINT point{ (i % 9U) + 1U, (i / 9U) + 1U };
	return point;
}

unsigned CGameField::GetIndex(const UPOINT &point)
{
	return GetIndexLine(point.x, point.y);
}

unsigned CGameField::GetIndexLine(unsigned row, unsigned line)		// 1 <= row <= 9, 1 <= line <= 9
{
	ATLASSUME(line >= 1U && line <= 9U);
	ATLASSUME(row >= 1U && row <= 9U);
	return --line * 9U + --row;
}

unsigned CGameField::GetIndexSquare(unsigned square, unsigned cell)
{
	ATLASSUME(square >= 1U && square <= 9U);
	ATLASSUME(cell >= 1U && cell <= 9U);
	/*
	1 2 3
	4 5 6
	7 8 9
	*/
	const auto cy = (cell - 1U) / 3U;
	const auto cx = (cell - 1U) % 3U;
	const auto sy = (square - 1U) / 3U;
	const auto sx = (square - 1U) % 3U;
	return (sy * 3U + cy) * 9U + sx * 3U + cx;
}

unsigned CGameField::GetSquareIndex(unsigned i)
{
	return GetSquareIndex(GetPointIndex(i));
}

unsigned CGameField::GetSquareIndex(const UPOINT &point)
{
	ATLASSUME(point.x >= 1U && point.x <= 9U);
	ATLASSUME(point.y >= 1U && point.y <= 9U);
	const auto x = (point.x - 1U) / 3U;
	const auto y = (point.y - 1U) / 3U;
	return (y * 3U + x) + 1U;
}

unsigned CGameField::GetCellIndex(unsigned i)
{
	return GetCellIndex(GetPointIndex(i));
}

unsigned CGameField::GetCellIndex(const UPOINT &point)
{
	ATLASSUME(point.x >= 1U && point.x <= 9U);
	ATLASSUME(point.y >= 1U && point.y <= 9U);
	const auto x = (point.x - 1U) % 3U;
	const auto y = ((point.y - 1U) * 3U) % 9U;
	return (y * 3U + x) + 1U;
}
