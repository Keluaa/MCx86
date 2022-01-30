
#include "../ALU.hpp"
#include "CPU.h"
#include "opcodes.h"


/**
 * Executes the instruction specified by its opcode. This method handles all 'simple' instructions.
 *
 * @param data Holds instruction information
 * @param flags EFLAGS register
 * @param ret Return value of the instruction
 * @param ret_2 Additional return value of the instruction
 */
void CPU::execute_arithmetic_instruction(const U8 opcode, const InstData data, EFLAGS& flags, U32& ret, U32& ret_2)
{
	switch (opcode) // we could consider only the first 7 bits of the opcode
	{
	case Opcodes::AAA:
	{
		// if AF is set or if the first 4 bits of AL are greater than 9
		if (flags.get(EFLAGS::AF) || ALU::compare_greater_or_equal(U8(data.op1) & 0x0F, 10)) {
			// we do
			// AL <- (AL + 6) and 0x0F
			// AH <- AH + 1
			// but in one step:
			// AX <- (AH + 1) : (AL + 6) & 0x0F
			// or:
			// AX <- (AX + 0x0106) & 0x0F0F <- 0F0F mask because we are doing BCD operations
			ret = ALU::add_no_carry(data.op1, (U16) 0x0106) & 0x0F0F;

			flags.set(EFLAGS::AF | EFLAGS::CF);
		}
		else {
			ret = data.op1; // keep AX intact

			flags.clear(EFLAGS::AF | EFLAGS::CF);
		}
		break;
	}
	case Opcodes::AAD:
	{
		U8 AH_val = (data.op1 & 0xFF00) >> 8;
		U8 AL_val = data.op1 & 0xFF;

		AL_val = ALU::add_no_carry(ALU::multiply_no_overflow(AH_val, (U8) 10), AL_val);

		flags.update_sign_flag(AL_val, OpSize::B);
		flags.update_zero_flag(AL_val);
		flags.update_parity_flag(AL_val);

		ret = AL_val;
		break;
	}
	case Opcodes::AAM:
	{
		U8 q, r;
		bit div_by_zero;
		ALU::unsigned_divide(U8(data.op1 & 0xFF), (U8)10, q, r, div_by_zero);

		ret = (q << 8) | r;

        flags.update_sign_flag(r, OpSize::B);
        flags.update_zero_flag(r);
        flags.update_parity_flag(r);
		break;
	}
	case Opcodes::AAS:
	{
		// if AF is set or if the first 4 bits of AL are greater than 9
		if (flags.get(EFLAGS::AF) || ALU::compare_greater_or_equal(data.op1 & 0x0F, 10)) {
			// we do:
			// AL <- (AL - 6) & 0x0F
			// AH <- AH - 1
			// but in one step:
			// AX <- (AX + ((-1 << 8) + (-6 & 0x0F))) & 0xFF0F
			// notice: -6 & 0x0F = 0x0A
			// and then: (-1 << 8) + (-6 & 0x0F) = 0xFF0A
			ret = ALU::add_no_carry(U16(data.op1), (U16) 0xFF0A) & 0xFF0F;

			flags.set(EFLAGS::AF | EFLAGS::CF);
		}
		else {
		    flags.clear(EFLAGS::AF | EFLAGS::CF);
		}
		break;
	}
	case Opcodes::ADC:
	{
	    bit carry = flags.get(EFLAGS::CF);
		ret = ALU::add(data.op1, data.op2, carry);

        flags.update_status_flags(data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::ADD:
	{
		bit carry = 0;
		ret = ALU::add(data.op1, data.op2, carry);

        flags.update_status_flags(data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry);
		break;
	}
	case Opcodes::AND:
	{
		ret = ALU::and_(data.op1, data.op2);

		flags.clear(EFLAGS::CF | EFLAGS::OF);
        flags.update_sign_flag(ret, data.op1_size);
        flags.update_zero_flag(ret);
        flags.update_parity_flag(ret);
		break;
	}
	case Opcodes::ARPL:
	{
		// compare the first two bits of the operands, and branch if op1 < op2
		if (!ALU::compare_greater_or_equal(data.op1 & 0x03, data.op2 & 0x03)) {
			// copy the first two bits of op2 to the first two bits of op1
			ret = (data.op1 & (0xFF - 0x03)) | (data.op2 & 0x03);

			flags.set(EFLAGS::ZF);
		}
		else {
		    flags.clear(EFLAGS::ZF);
		}
		break;
	}
	case Opcodes::BOUND:
	{
		if (!ALU::compare_greater_or_equal(data.op1, data.op2) || ALU::compare_greater(data.op1, data.op3)) {
            throw_exception(Interrupts::BoundException);
		}
		break;
	}
	case Opcodes::BSF:
	{
		bit is_zero;
		ret = ALU::get_first_set_bit_index(data.op1, is_zero);

		flags.set_val(EFLAGS::ZF, is_zero);
		break;
	}
	case Opcodes::BSR:
	{
		bit is_zero;
		ret = ALU::get_last_set_bit_index(data.op1, is_zero);

        flags.set_val(EFLAGS::ZF, is_zero);
		break;
	}
	case Opcodes::BT:
	{
		if (data.op2 > 32) {
			// see the manual! (p. 268)
            throw_NYI("The BT instruction has an incomplete implementation for memory operands.");

			/*
			16 bits:
				15-4 3-0
				|	 |-> index of the bit to use from the 2 bytes fetched from memory
				|-> additional offset for the memory operand, x2

			32 bits:
				31-5 4-0
				|	 |-> index of the bit to use from the 4 bytes fetched from memory
				|-> additional offset for the memory operand, x4
			*/
		}

		bit bit_val = ALU::get_bit_at(data.op1, data.op2 & 0b11111); // op2 mod 32
		flags.set_val(EFLAGS::CF, bit_val);
		break;
	}
	case Opcodes::BTC:
	{
		if (data.op2 > 32) {
            throw_NYI("The BTC instruction has an incomplete implementation for memory operands.");
		}

		bit bit_val = ALU::get_bit_at(data.op1, data.op2);
		ret = data.op1;
        ALU::get_and_set_bit_at(ret, data.op2, !bit_val);
        flags.set_val(EFLAGS::CF, bit_val);
		break;
	}
	case Opcodes::BTR:
	{
		if (data.op2 > 32) {
            throw_NYI("The BTR instruction has an incomplete implementation for memory operands.");
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 0);
        flags.set_val(EFLAGS::CF, bit_val);
		break;
	}
	case Opcodes::BTS:
	{
		if (data.op2 > 32) {
            throw_NYI("The BTS instruction has an incomplete implementation for memory operands.");
		}

		ret = data.op1;
		bit bit_val = ALU::get_and_set_bit_at(ret, data.op2, 1);
        flags.set_val(EFLAGS::CF, bit_val);
		break;
	}
	case Opcodes::CBW:
	{
	    // The OpSize of op1 is the one we want to convert to, the previous size is half of it
		if (data.op1_size == OpSize::W) {
			U8 AL = data.op1;
			ret = ALU::sign_extend((U16) AL, OpSize::B);
		}
		else {
			U16 AX = data.op1;
			ret = ALU::sign_extend((U32) AX, OpSize::W);
		}
		break;
	}
	case Opcodes::CLC:
	{
	    flags.clear(EFLAGS::CF);
		break;
	}
	case Opcodes::CLD:
	{
        flags.clear(EFLAGS::DF);
		break;
	}
	case Opcodes::CLI:
	{
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		flags.clear(EFLAGS::IF);
		break;
	}
	case Opcodes::CLTS:
	{
		// this instruction needs special treatment
		WARNING("Permission check missing, this instruction should sometimes raise an exception"); // TODO
		CR0 control_register = registers.get_CR0();
		control_register.clear(CR0::TS);
		registers.set_CR0(control_register);
		break;
	}
	case Opcodes::CMC:
	{
	    flags.value ^= EFLAGS::CF;
		break;
	}
	case Opcodes::CMP:
	{
		bit carry;
		U32 val = ALU::sub(data.op1, data.op2, carry);
        flags.update_status_flags(data.op1, data.op2, val, data.op1_size, data.op2_size, data.op1_size, carry, 1);
		break;
	}
	case Opcodes::CWD:
	{
		if (data.op2_size == OpSize::DW) {
			if (ALU::check_is_negative(data.op2, OpSize::DW)) {
				ret = 0xFFFFFFFF;
			}
			else {
				ret = 0;
			}
		}
		else {
			if (ALU::check_is_negative(data.op2, OpSize::W)) {
				ret = 0xFFFF;
			}
			else {
				ret = 0;
			}
		}
		break;
	}
	case Opcodes::DAA:
	{
		bit carry = 0;
		if (flags.get(EFLAGS::AF) || ALU::compare_greater_or_equal(U8(data.op1) & 0x0F, 10)) {
			ret = ALU::add(U8(data.op1), U8(6), carry); // AL += 6
			flags.set(EFLAGS::AF);
		}
		else {
            flags.clear(EFLAGS::AF);
		}

		carry |= flags.get(EFLAGS::CF);
		if (carry || ALU::compare_greater_or_equal(U8(data.op1), U8(0x10))) {
			ret = ALU::add_no_carry(U8(ret), U8(0x60)); // AL += 0x60
			flags.set(EFLAGS::CF);
		}
		else {
		    flags.clear(EFLAGS::CF);
		}

		flags.update_parity_flag(ret);
		flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
		break;
	}
	case Opcodes::DAS:
	{
		bit carry = 0;
		if (flags.get(EFLAGS::AF) || ALU::compare_greater_or_equal(U8(data.op1 & 0x0F), U8(10))) {
			ret = ALU::sub(U8(data.op1), U8(6), carry); // AL -= 6
            flags.set(EFLAGS::AF);
		}
		else {
            flags.clear(EFLAGS::AF);
		}

		carry |= flags.get(EFLAGS::CF);
		if (carry || ALU::compare_greater_or_equal(U8(data.op1), U8(0x10))) {
			ret = ALU::sub_no_carry(U8(ret), U8(0x60)); // AL -= 0x60
            flags.set(EFLAGS::CF);
		}
		else {
            flags.clear(EFLAGS::CF);
		}

        flags.update_parity_flag(ret);
        flags.update_sign_flag(ret, data.op1_size);
        flags.update_zero_flag(ret);
		break;
	}
	case Opcodes::DEC:
	{
		ret = ALU::sub_no_carry(data.op1, U8(1));

        flags.update_overflow_flag(data.op1, U8(-1), ret, data.op1_size, OpSize::B, data.op1_size);
		flags.update_parity_flag(ret);
		flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
        flags.update_adjust_flag(data.op1, -1, 1);
		break;
	}
	case Opcodes::DIV:
	{
		U32 r, q, n = data.op1, d = data.op2;
		bit div_by_zero = false;
		ALU::unsigned_divide(n, d, q, r, div_by_zero);

		if (div_by_zero) {
            throw_exception(Interrupts::DivideError);
		}

		ret = q;
        ret_2 = r;
		break;
	}
	case Opcodes::IDIV:
	{
		U32 r, q, n = data.op1, d = data.op2;
		bit div_by_zero = false;
		ALU::signed_divide(n, d, q, r, div_by_zero);

		if (div_by_zero) {
            throw_exception(Interrupts::DivideError);
		}

		ret = q;
        ret_2 = r;
		break;
	}
	case Opcodes::IMUL:
	{
		bit overflow = 0;
		ret = ALU::multiply(data.op1, data.op2, overflow);

		// set both the carry and the overflow flag
		flags.set_val(EFLAGS::OF | EFLAGS::CF, overflow);
		break;
	}
	case Opcodes::IMULX:
	{
		// TODO: change this to perform full 64 bit multiplication instead
		// this implementation exactly what is not happening in the circuit implementation:
		// here we cast everything to 64 bits, perform 64 bit multiplication,
		// and return the last 32 bits of the result.
		// In the circuit the circuit of this instruction is placed next to the circuit of IMUL,
		// and the carries from the adders are stored, and when using IMULX,
		// we use them to compute the remaining 32 bits of the result.
		U64 a = I64(I32(data.op1)), b = I64(I32(data.op2)), r;
		bit overflow = 0;
		r = ALU::multiply(a, b, overflow);
		ret = U32(r >> 32);

		// set both the carry and the overflow flag
        flags.set_val(EFLAGS::OF | EFLAGS::CF, overflow);
		break;
	}
	case Opcodes::INC:
	{
		ret = ALU::add_no_carry(data.op1, 1);

        flags.update_overflow_flag(data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size);
		flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
        flags.update_parity_flag(ret);
        flags.update_adjust_flag(data.op1, data.op2);
		break;
	}
	case Opcodes::LAHF:
	{
	    ret = flags.value & 0xFF;
		break;
	}
	case Opcodes::LEA:
	{
		ret = data.address;
		break;
	}
	case Opcodes::MOV:
	{
		ret = data.op2;
		// TODO : segment change checks, same for control registers, and other things...
		break;
	}
	case Opcodes::MOVSX:
	{
		ret = ALU::sign_extend(data.op2, data.op2_size);
		break;
	}
	case Opcodes::MOVZX:
	{
		ret = data.op2;
		break;
	}
	case Opcodes::MUL:
	{
		bit overflow = 0;
		ret = ALU::multiply(data.op1, data.op2, overflow);

		// TODO : remove this if the overflow flag works
		/*
		// update the flags
		bit carry = 0;
		switch(data.op1_size)
		{
		case OpSize::B:
			carry = bool(ret & 0xFF00);
			break;
		case OpSize::W:
			carry = bool(ret & 0xFFFF0000);
			break;
		case OpSize::DW:
			throw BadInstruction("MUL does not support 64bit results. Use MULX.", registers.EIP);
		default:
			break; // handled in MULX
		}
		*/
		if (data.op1_size == OpSize::DW) { // TODO : change? remove?
			throw BadInstruction("MUL does not support 64bit results. Use MULX.", registers.EIP);
		}

		// set both the carry and the overflow flag
		flags.set_val(EFLAGS::CF | EFLAGS::OF, overflow);
		break;
	}
	case Opcodes::MULX:
	{
		// in the circuit, the algorithm used to get a 64bit result is optimized
		// because the high bits of the operands are 0
		U64 a = data.op1, b = data.op2, r;
		bit overflow = 0;
		r = ALU::multiply(a, b, overflow);
		ret = U32(r);
        ret_2 = U32(r >> 32);

		// set both the carry and the overflow flag
        flags.set_val(EFLAGS::CF | EFLAGS::OF, overflow);
		break;
	}
	case Opcodes::NEG:
	{
		ret = ALU::negate(data.op1);

        flags.set_val(EFLAGS::CF, ALU::check_equal_zero(data.op1));
		flags.clear(EFLAGS::OF); // no overflow is possible
		flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
		flags.update_parity_flag(ret);
		break;
	}
	case Opcodes::NOP:
	{
		break;
	}
	case Opcodes::NOT:
	{
		ret = ALU::not_(data.op1);
		break;
	}
	case Opcodes::OR:
	{
		ret = ALU::or_(data.op1, data.op2);

		flags.clear(EFLAGS::CF | EFLAGS::OF);
		flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
		flags.update_parity_flag(ret);
		break;
	}
	case Opcodes::ROT:
	{
		// To encode this operation in one opcode, we use the immediate to specify the operation:
		// op1: dest, op2: source register value if any
		// op3: bit field with the count if any:
		//  - bits 0-4: count
		//  - 5: use the previous 5 bits as count instead of op2
		//  - 6: rotate left (else right)
		//  - 7: include the carry in the rotations (RC- instructions)
		U8 count = data.op3 & 0b11111;
		bit use_imm = data.op3 & (1 << 5);
		bit rot_left = data.op3 & (1 << 6);
		bit rot_carry = data.op3 & (1 << 7);

		if (!use_imm) {
			count = data.op2;
		}

        U8 last_bit_pos = 0;
		switch (data.op1_size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
        case OpSize::UNKNOWN: throw BadInstruction("Incorrect Operand Size", registers.EIP);
		}

		bit carry = flags.get(EFLAGS::CF);
        bit overflow;
		if (rot_carry) {
			if (rot_left) {
				ret = ALU::rotate_left_carry(data.op1, carry, count, data.op1_size);
				overflow = carry != bit(ret & (1 << last_bit_pos));
			}
			else {
				ret = ALU::rotate_right_carry(data.op1, carry, count, data.op1_size);
				overflow = bit(ret & (1 << last_bit_pos)) != bit(ret & (1 << (last_bit_pos - 1)));
			}
		}
		else {
			if (rot_left) {
				ret = ALU::rotate_left(data.op1, carry, count, data.op1_size);
				overflow = carry != bit(ret & (1 << last_bit_pos));
			}
			else {
				ret = ALU::rotate_right(data.op1, carry, count, data.op1_size);
				overflow = bit(ret & (1 << last_bit_pos)) != bit(ret & (1 << (last_bit_pos - 1)));
			}
		}

		flags.set_val(EFLAGS::OF, overflow);
		flags.set_val(EFLAGS::CF, carry);
		break;
	}
    case Opcodes::SAHF:
	{
        // store only SF, ZF, AF, PF and CF from AH
        flags.value = (flags.value & 0xFF) | (data.op1 & 0b11010101);
		break;
	}
    case Opcodes::SHFT:
    {
        // To encode this operation in one opcode, we use the immediate to specify the operation:
		// op1: dest, op2: source register value if any
		// op3: bit field with the count if any:
		//  - bits 0-4: count
		//  - 5: use the previous 5 bits as count instead of op2
		//  - 6: shift left (else right)
        //  - 7: keep the sign (only for right shifts)
		U8 count = data.op3 & 0b11111;
		bit use_imm = data.op3 & (1 << 5);
		bit shift_left = data.op3 & (1 << 6);
        bit keep_sign = data.op3 & (1 << 7);

        if (!use_imm) {
            count = data.op2;
        }

        U8 last_bit_pos = 0;
		switch (data.op1_size)
		{
		case OpSize::DW: last_bit_pos = sizeof(U32) * 8 - 1; break;
		case OpSize::W:  last_bit_pos = sizeof(U16) * 8 - 1; break;
		case OpSize::B:  last_bit_pos = sizeof(U8)  * 8 - 1; break;
        case OpSize::UNKNOWN: throw BadInstruction("Incorrect Operand Size", registers.EIP);
		}

        bit carry = 0;
        bit overflow;
        if (shift_left) {
            ret = ALU::shift_left(data.op1, carry, count, data.op1_size);
			overflow = bool(ret & (1 << last_bit_pos)) != carry;
        }
        else {
            ret = ALU::shift_right(data.op1, carry, count, data.op1_size, keep_sign);
            if (keep_sign) {
				overflow = 0;
            }
            else {
				overflow = bool(data.op1 & (1 << last_bit_pos));
            }
        }

        flags.set_val(EFLAGS::OF, overflow);
        flags.set_val(EFLAGS::CF, carry);
        flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
		flags.update_parity_flag(ret);
		break;
    }
    case Opcodes::SBB:
	{
		U32 op_2 = ALU::sign_extend(data.op2, data.op2_size);

		bit carry = flags.get(EFLAGS::CF);
        op_2 = ALU::add(op_2, 0, carry);
		ret = ALU::sub(data.op1, op_2, carry);

        flags.update_status_flags(data.op1, data.op2, ret, data.op1_size, data.op2_size, data.op1_size, carry, 1);
		break;
	}
	case Opcodes::SETcc:
	{
		U8 condition = data.imm;
		switch (condition)
		{
        case 0b0000: ret =  flags.get(EFLAGS::OF); break;              // Overflow                   OF = 1
        case 0b0001: ret = !flags.get(EFLAGS::OF); break;              // Not overflow               OF = 0
        case 0b0010: ret =  flags.get(EFLAGS::CF); break;              // Below | Carry | Not above or equal     CF = 1
        case 0b0011: ret = !flags.get(EFLAGS::CF); break;              // Above or equal | Not below | Not carry CF = 0
        case 0b0100: ret =  flags.get(EFLAGS::ZF); break;              // Equal | Zero               ZF = 1
        case 0b0101: ret = !flags.get(EFLAGS::ZF); break;              // Not equal | Not zero       ZF = 0
        case 0b0110: ret =  flags.get(EFLAGS::ZF | EFLAGS::CF); break; // Below or equal | Not above ZF = 1 || CF = 1
		case 0b0111: ret = !flags.get(EFLAGS::ZF | EFLAGS::CF); break; // Above | Not below or equal ZF = 0 && CF = 0
        case 0b1000: ret =  flags.get(EFLAGS::SF); break;              // Sign                       SF = 1
        case 0b1001: ret = !flags.get(EFLAGS::SF); break;              // Not sign                   SF = 0
        case 0b1010: ret =  flags.get(EFLAGS::PF); break;              // Parity | Parity even       PF = 1
        case 0b1011: ret = !flags.get(EFLAGS::PF); break;              // Not parity | Parity odd    PF = 0
        case 0b1100: ret =   flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF);  break; // Less | Not greater or equal SF != OF
        case 0b1101: ret = !(flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF)); break; // Greater or Equal | Not less SF == OF
        case 0b1110: ret =   flags.get(EFLAGS::ZF) |  (flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF)); break; // Less or equal | Not greater ZF = 1 ||  SF != OF
        case 0b1111: ret =  !flags.get(EFLAGS::ZF) & !(flags.get(EFLAGS::SF) ^ flags.get(EFLAGS::OF)); break; // Greater | Not less or equal ZF = 0 && (SF == OF)
        default:
            throw BadInstruction("Invalid Jump Type", registers.EIP);
		}
		break;
	}
	case Opcodes::SHD:
	{
		U8 count = data.op3 & 0b11111;
		bit shift_left = data.op3 & (1 << 5);

		bit carry = 0;
		if (shift_left) {
			U64 merged = U64(data.op1) << 32;
			if (data.op2_size == OpSize::DW) {
				merged |= data.op2;
			}
			else { // W operand
				merged |= static_cast<U64>(U16(data.op2)) << 16;
			}
			merged = ALU::shift_left(merged, carry, count);
			ret = U32(merged >> 32);
		}
		else {
			U64 merged = U64(data.op2) << 32;
			if (data.op1_size == OpSize::DW) {
				merged |= data.op1;
			}
			else { // W operand
				merged |= static_cast<U64>(U16(data.op1)) << 16;
			}
			merged = ALU::shift_right(merged, carry, count);
			if (data.op1_size == OpSize::DW) {
				ret = U32(merged);
			}
			else {
				ret = U32(merged >> 16);
			}
		}

		flags.set_val( EFLAGS::CF, carry);
        flags.update_sign_flag(ret, data.op1_size);
		flags.update_zero_flag(ret);
		flags.update_parity_flag(ret);
		break;
	}
	case Opcodes::STC:
	{
	    flags.set(EFLAGS::CF);
		break;
	}
	case Opcodes::STD:
	{
	    flags.set(EFLAGS::DF);
		break;
	}
	case Opcodes::STI:
	{
		// TODO: permission checks
        flags.set(EFLAGS::IF);
		break;
	}
	case Opcodes::SUB:
	{
        OpSize op_2_size = data.op2_size;
        U32 op_2;
        if (data.op1_size != op_2_size) {
            op_2 = ALU::sign_extend(data.op2, op_2_size);
            op_2_size = data.op1_size;
        }
        else {
            op_2 = data.op2;
        }

		bit carry = 0;
		ret = ALU::sub(data.op1, op_2, carry);

        flags.update_status_flags(data.op1, op_2, ret, data.op1_size, op_2_size, data.op1_size, carry, 1);
		break;
	}
	case Opcodes::TEST:
	{
		U32 result = data.op1 & data.op2;

		flags.clear(EFLAGS::OF | EFLAGS::CF); // clear the OF and CF flags
		flags.update_parity_flag(result);
		flags.update_sign_flag(result, data.op1_size);
		flags.update_zero_flag(result);
		break;
	}
	case Opcodes::XCHG:
	{
		ret = data.op2;
        ret_2 = data.op1;
		break;
	}
	case Opcodes::XLAT:
	{
		// TODO : make sure to make this instruction use 32bit addressing.
		//  In order to not have to fetch something from memory here, but before this instruction is executed,
		//  The Mod r/m and SIB bytes should: add EBX and AL together to have the offset in the segment
		//  stored in DS to get one single byte in memory.
		ret = data.op1;
		break;
	}
	case Opcodes::XOR:
	{
		ret = ALU::xor_(data.op1, data.op2);
		// TODO: flags
		break;
	}
	default:
	{
		throw UnknownInstruction("Unknown arithmetic instruction", opcode, registers.EIP);
	}
	}

	// increment EIP at the end of those instructions
	registers.write_EIP(ALU::add_no_carry(registers.EIP, 1));
}
