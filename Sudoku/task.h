#pragma once
#include <array>

typedef std::array<CELLVAL, 81> _TASK;

typedef std::array<_TASK, 8> _TASK_EASY;
typedef std::array<_TASK, 8> _TASK_NORMAL;
typedef std::array<_TASK, 8> _TASK_HARD;
typedef std::array<_TASK, 8> _TASK_INSANE;

extern const _TASK_EASY tasks_easy;
extern const _TASK_NORMAL tasks_normal;
extern const _TASK_HARD tasks_hard;
extern const _TASK_INSANE tasks_insane;

enum TASK_LEVEL { TASK_UNKNOWN, TASK_EASY, TASK_NORMAL, TASK_HARD, TASK_INSANE };

class CTask
{
	_TASK task;
public:
	CTask() = default;
	~CTask() = default;
	CTask(const CTask &t) { _Assign(t.task); }
	CTask(CTask &&t) noexcept { _Assign(t.task); }

private:
	void _Assign(const _TASK &t) { task = t; }
	void _Assign(_TASK &&t)
	{
		task.swap(t);
	}
	void Rotate(int r);
	void Transpose();
	void ExchRows(int i);
	void ExchRows(int row, int i);
	void ExchCols(int i);
	void ExchCols(int col, int i);
	static int GetIndex(int x, int y);
	void SwapTask(size_t i1, size_t i2);
public:
	CTask &operator=(const CTask &t)
	{
		_Assign(t.task);
		return *this;
	}
	CTask &operator=(CTask &&t) noexcept
	{
		task.swap(t.task);
		return *this;
	}
	CELLVAL operator[](const size_t i) const
	{
		return task[i];
	}
	unsigned NewGame(TASK_LEVEL level);
};