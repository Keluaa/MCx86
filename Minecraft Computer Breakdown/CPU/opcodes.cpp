﻿
#include "opcodes.h"


using namespace Opcodes;


std::map<U8, std::string> Opcodes::mnemonics = std::map<U8, std::string>{
    { AAA,   "AAA"   },
    { AAD,   "AAD"   },
    { AAM,   "AAM"   },
    { AAS,   "AAS"   },
    { ADC,   "ADC"   },
    { ADD,   "ADD"   },
    { AND,   "AND"   },
    { ARPL,  "ARPL"  },
    { BOUND, "BOUND" },
    { BSF,   "BSF"   },
    { BSR,   "BSR"   },
    { BT,    "BT"    },
    { BTC,   "BTC"   },
    { BTR,   "BTR"   },
    { BTS,   "BTS"   },
    { CBW,   "CBW"   },
    { CLC,   "CLC"   },
    { CLD,   "CLD"   },
    { CLI,   "CLI"   },
    { CLTS,  "CLTS"  },
    { CMC,   "CMC"   },
    { CMP,   "CMP"   },
    { CWD,   "CWD"   },
    { DAA,   "DAA"   },
    { DAS,   "DAS"   },
    { DEC,   "DEC"   },
    { DIV,   "DIV"   },
    { IDIV,  "IDIV"  },
    { IMUL,  "IMUL"  },
    { INC,   "INC"   },
    { LAHF,  "LAHF"  },
    { LEA,   "LEA"   },
    { MOV,   "MOV"   },
    { MOVSX, "MOVSX" },
    { MOVZX, "MOVZX" },
    { MUL,   "MUL"   },
    { NEG,   "NEG"   },
    { NOP,   "NOP"   },
    { NOT,   "NOT"   },
    { OR,    "OR"    },
    { ROT,   "ROT"   },
    { SAHF,  "SAHF"  },
    { SHFT,  "SHFT"  },
    { SBB,   "SBB"   },
    { SETcc, "SETcc" },
    { SHD,   "SHD"   },
    { STC,   "STC"   },
    { STD,   "STD"   },
    { STI,   "STI"   },
    { SUB,   "SUB"   },
    { TEST,  "TEST"  },
    { XCHG,  "XCHG"  },
    { XLAT,  "XLAT"  },
    { XOR,   "XOR"   },
    { HLT,   "HLT"   },
    { IN,    "IN"    },
    { LAR,   "LAR"   },
    { LGDT,  "LGDT"  },
    { LGS,   "LGS"   },
    { LLDT,  "LLDT"  },
    { LMSW,  "LMSW"  },
    { LOCK,  "LOCK"  },
    { LSL,   "LSL"   },
    { LTR,   "LTR"   },
    { OUT,   "OUT"   },
    { POP,   "POP"   },
    { POPF,  "POPF"  },
    { PUSH,  "PUSH"  },
    { PUSHF, "PUSHF" },
    { SGDT,  "SGDT"  },
    { SLDT,  "SLDT"  },
    { SMSW,  "SMSW"  },
    { STR,   "STR"   },
    { VERR,  "VERR"  },
    { WAIT,  "WAIT"  },
    { CMPS,  "CMPS"  },
    { INS,   "INS"   },
    { LODS,  "LODS"  },
    { MOVS,  "MOVS"  },
    { OUTS,  "OUTS"  },
    { SCAS,  "SCAS"  },
    { STOS,  "STOS"  },
    { CALL,  "CALL"  },
    { INT,   "INT"   },
    { IRET,  "IRET"  },
    { Jcc,   "Jcc"   },
    { JMP,   "JMP"   },
    { LEAVE, "LEAVE" },
    { LOOP,  "LOOP"  },
    { REP,   "REP"   },
    { RET,   "RET"   },
    { ENTER, "ENTER" },
    { POPA,  "POPA"  },
    { PUSHA, "PUSHA" },
    { IMULX, "IMULX" },
    { MULX,  "MULX"  },
};
