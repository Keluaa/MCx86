#pragma once

#include <map>

#include "data_types.h"
#include "instructions.h"


namespace IA32
{
	extern const std::map<U16, Inst> InstTable;
};
