
#include "instructions.h"


InstData Inst::getInstData() const
{
	// All OpSizes are DW by default, and the address is 0
	return InstData{ .imm = immediate_value };
}
