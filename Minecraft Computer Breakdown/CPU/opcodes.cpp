
#include "opcodes.h"


using namespace Opcodes;


std::map<U8, std::string> Opcodes::mnemonics = std::map<U8, std::string>{
	{ AAA, "AAA" },
	{ AAD, "AAD" },
	{ AAM, "AAM" },
	
	{ ADC, "ADC" },
	{ ADD, "ADD" },
	{ AND, "AND" },
	
	{ CMP, "CMP" },
	
	{ DIV, "DIV" },
	{ IDIV, "IDIV" },
	{ IMUL, "IMUL" },
	
	{ LEA, "LEA" },
	{ MOV, "MOV" },
	{ MOVSX, "MOVSX" },
	{ MOVZX, "MOVZX" },
	{ MUL, "MUL" },
	
	{ NOT, "NOT" },
	{ OR, "OR" },
	
	{ SETcc, "SETcc" },
	
	{ SUB, "SUB" },
	
	{ XCHG, "XCHG" },
	
	{ HLT, "HLT" },
	
	{ POP, "POP" },
	{ PUSH, "PUSH" },
	
	{ CALL, "CALL" },
	{ INT, "INT" },
	
	{ Jcc, "Jcc" },
	{ JMP, "JMP" },
	
	{ RET, "RET" },
};
