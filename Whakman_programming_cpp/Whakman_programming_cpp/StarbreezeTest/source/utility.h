#pragma once

#include <Windows.h>

class Timer {
public:
	Timer()
	: _current_time(0)
	{
		QueryPerformanceFrequency(&_freq);
		QueryPerformanceCounter(&_start_time);
	}

	void snapshot() {
		LARGE_INTEGER snapshot_time;

		QueryPerformanceCounter(&snapshot_time);
		LARGE_INTEGER elapsed;
		elapsed.QuadPart = snapshot_time.QuadPart - _start_time.QuadPart;
		elapsed.QuadPart *= 1000000;
		elapsed.QuadPart /= _freq.QuadPart;
		_current_time = elapsed.QuadPart / 1000000.0f;
	}

	float time() {
		return _current_time;
	}

private:
	LARGE_INTEGER	_freq;
	LARGE_INTEGER	_start_time;	
	float			_current_time;
};
