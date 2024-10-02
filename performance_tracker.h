#include <iostream>
#include <chrono>
using namespace std::chrono;

class PerformanceTracker
{
	steady_clock::time_point _startTime;
	steady_clock::time_point _stopTime;

	long _executeCount;
	long long _elapsedTotal;
	std::string _name = "";

public:

	PerformanceTracker()
	{
		_executeCount = 0;
		_elapsedTotal = 0;
	}

	PerformanceTracker(std::string name)
	{
		_executeCount = 0;
		_elapsedTotal = 0;
		_name = name;
	}

	void Start()
	{
		_startTime = steady_clock::now();
	}

	void Stop()
	{
		_executeCount++;
		_elapsedTotal += (steady_clock::now() - _startTime).count();

		if (_executeCount % 100 == 0)
		{
			auto millisecondsTotal = _elapsedTotal / 1000000;
			std::cout << _name <<" Millisec per draw: " << millisecondsTotal / _executeCount << ", _executeCount: "<< _executeCount << std::endl;
		}
	}

	void GetStats(long& executeCount, long& millisecondsTotal)
	{
		executeCount = _executeCount;
		millisecondsTotal = _elapsedTotal / 1000;
	}
};