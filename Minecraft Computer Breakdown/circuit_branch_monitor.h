#pragma once

#include <set>

#include "exceptions.h"


class CircuitBranchAlreadyUsed : public ExceptionWithMsg
{
public:
	CircuitBranchAlreadyUsed() noexcept
	{
		this->msg = "Cannot use this branch, since it has already been used during this clock cycle.";
	}
};


class CircuitBranchMonitor
{
	bool monitor = false;

	std::set<const char*> usages;

public:

	constexpr void use_branch(const char* function)
	{
		if (!monitor) return;

		if (usages.find(function) != usages.end()) {
			// this function has already been used during this clock cycle
			throw CircuitBranchAlreadyUsed();
		}
		else {
			usages.insert(function);
		}
	}

	void reset() { usages.clear(); }

	const std::set<const char*>& get_usages() const { return usages; }

	bool is_monitoring() const { return monitor; }
	void start_monitoring() { monitor = true; }
	void stop_monitoring() { monitor = false; }
};

#define _USE_BRANCH_EVAL(macro) macro
#define USE_BRANCH(monitor) _USE_BRANCH_EVAL(monitor.use_branch(__func__));
