#include "stdafx.h"
#include "task.h"

unsigned CTask::NewGame(const TASK_LEVEL level)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	auto game = UINT_MAX;
	switch(level)
	{
	case TASK_EASY:
	{
		const std::uniform_int_distribution<> dist(0, tasks_easy.size() - 1);
		game = dist(gen);
		_Assign(tasks_easy[game]);
		break;
	}
	case TASK_NORMAL:
	{
		const std::uniform_int_distribution<> dist(0, tasks_normal.size() - 1);
		game = dist(gen);
		_Assign(tasks_normal[game]);
		break;
	}
	case TASK_HARD:
	{
		const std::uniform_int_distribution<> dist(0, tasks_hard.size() - 1);
		game = dist(gen);
		_Assign(tasks_hard[game]);
		break;
	}
	case TASK_INSANE:
	{
		const std::uniform_int_distribution<> dist(0, tasks_insane.size() - 1);
		game = dist(gen);
		_Assign(tasks_insane[game]);
		break;
	}
	default:
		ATLASSERT(FALSE);
	}
	const std::uniform_int_distribution<> dist2(0, 1);
	if (0 != dist2(gen))
		Transpose();
	const std::uniform_int_distribution<> dist4(0, 3);
	Rotate(dist4(gen));
	ExchRows(0, dist4(gen));
	ExchRows(1, dist4(gen));
	ExchRows(2, dist4(gen));
	ExchRows(dist4(gen));
	ExchCols(0, dist4(gen));
	ExchCols(1, dist4(gen));
	ExchCols(2, dist4(gen));
	ExchCols(dist4(gen));
	return game;
}

void CTask::Rotate(const int r)
{
	ATLASSERT(r >= 0 && r < 4);
	switch(r)
	{
	case 1:
		{
			_TASK tt;
			for (auto y = 0; y < 9; y++) for (auto x = 0; x < 9; x++)
			{
				const auto x1 = 8 - y;
				const auto y1 = x;
				tt[GetIndex(x1, y1)] = task[GetIndex(x, y)];
			}
			_Assign(tt);
		}
		break;
	case 2:
		{
			_TASK tt;
			for (auto y = 0; y < 9; y++) for (auto x = 0; x < 9; x++)
			{
				const auto x1 = 8 - x;
				const auto y1 = 8 - y;
				tt[GetIndex(x1, y1)] = task[GetIndex(x, y)];
			}
			_Assign(tt);
		}
		break;
	case 3:
		{
			_TASK tt;
			for (auto y = 0; y < 9; y++) for (auto x = 0; x < 9; x++)
			{
				const auto x1 = y;
				const auto y1 = 8 - x;
				tt[GetIndex(x1, y1)] = task[GetIndex(x, y)];
			}
			_Assign(tt);
		}
		break;
	default:
		// Do nothing
		break;
	}
}

void CTask::Transpose()
{
	_TASK tt;
	for (auto y = 0; y < 9; y++) for (auto x = 0; x < 9; x++)
		tt[GetIndex(x, y)] = task[GetIndex(y, x)];
	_Assign(tt);
}

// Mix:
// 0 - None
// 1 - 0 <==> 1
// 2 - 0 <==> 2
// 3 - 1 <==> 2

void CTask::ExchRows(int i)
{
	ATLASSERT(i >= 0 && i <= 3);
	int dx1, dx2;
	switch(i)
	{
	case 1:
		dx1 = 0;
		dx2 = 3;
		break;
	case 2:
		dx1 = 0;
		dx2 = 6;
		break;
	case 3:
		dx1 = 3;
		dx2 = 6;
		break;
	default:
		// Do nothing
		return;
	}
	for (auto y = 0; y < 9; y++)
	{
		for (auto x = 0; x < 3; x++)
			SwapTask(GetIndex(dx1 + x, y), GetIndex(dx2 + x, y));
	}
}

void CTask::ExchRows(int row, int i)
{
	ATLASSERT(row >= 0 && row < 3);
	ATLASSERT(i >= 0 && i <= 3);
	int dx1, dx2;
	switch(i)
	{
	case 1:
		dx1 = 0;
		dx2 = 1;
		break;
	case 2:
		dx1 = 0;
		dx2 = 2;
		break;
	case 3:
		dx1 = 2;
		dx2 = 1;
		break;
	default:
		// Do nothing
		return;
	}
	for (auto y = 0; y < 9; y++)
		SwapTask(GetIndex(row * 3 + dx1, y), GetIndex(row * 3 + dx2, y));
}

void CTask::ExchCols(int i)
{
	ATLASSERT(i >= 0 && i <= 3);
	int dy1, dy2;
	switch(i)
	{
	case 1:
		dy1 = 0;
		dy2 = 3;
		break;
	case 2:
		dy1 = 0;
		dy2 = 6;
		break;
	case 3:
		dy1 = 3;
		dy2 = 6;
		break;
	default:
		// Do nothing
		return;
	}
	for (auto x = 0; x < 9; x++)
	{
		for (auto y = 0; y < 3; y++)
			SwapTask(GetIndex(x, dy1 + y), GetIndex(x, dy2 + y));
	}
}

void CTask::ExchCols(int col, int i)
{
	ATLASSERT(col >= 0 && col < 3);
	ATLASSERT(i >= 0 && i <= 3);
	int dy1, dy2;
	switch(i)
	{
	case 1:
		dy1 = 0;
		dy2 = 1;
		break;
	case 2:
		dy1 = 0;
		dy2 = 2;
		break;
	case 3:
		dy1 = 2;
		dy2 = 1;
		break;
	default:
		// Do nothing
		return;
	}
	for (auto x = 0; x < 9; x++)
		SwapTask(GetIndex(x, col * 3 + dy1), GetIndex(x, col * 3 + dy2));
}

int CTask::GetIndex(int x, int y)
{
	ATLASSERT(x >= 0 && x < 9);
	ATLASSERT(y >= 0 && y < 9);
	return y * 9 + x;
}

void CTask::SwapTask(size_t i1, size_t i2)
{
	ATLASSERT(i1 >= 0 && i1 < task.size());
	ATLASSERT(i2 >= 0 && i2 < task.size());
	std::swap(task[i1], task[i2]);
}
