#pragma once

#include "../data_types.h"


namespace Interrupts
{
	enum class Type : U8 { None, Fault, Trap, Abort, User };

	struct Interrupt 
	{
		U8 vector; // or interrupt code, this is an index into the interrupt descriptor table
		Type type;
		const char* mnemonic;
	};

	// Common interrupts which are triggered by some instructions
	// Some interrupts are unused, simply because they have no purpose in this implementation

	const Interrupt DivideError{ 0, Type::Fault, "#DE" };           // Divide Error
	const Interrupt DebugException{ 1, Type::Fault, "#DB" };        // Debug Exception
//	const Interrupt NMInterrupt{ 2, Type::None, "" };                                     // Non-maskable external Interrupt
	const Interrupt Breakpoint{ 3, Type::Trap, "#BP" };             // Breakpoint
	const Interrupt OverflowException{ 4, Type::Trap, "#OF" };      // Overflow
	const Interrupt BoundException{ 5, Type::Fault, "#BR" };        // BOUND Range Exceeded
	const Interrupt OpcodeException{ 6, Type::Fault, "#UD" };       // Invalid Opcode
//	const Interrupt NoMathException{ 7, Type::Fault, "#NM" };           // Math Coprocessor Not Available
	const Interrupt DoubleFault{ 8, Type::Abort, "#DF" };           // Double Fault
//	const Interrupt FloatingPointCoprocessorError{ 9, Type::Fault, "" };                  // FP Cop. Error
	const Interrupt InvalidTaskSwitch{ 10, Type::Fault, "#TS" };    // Invalid Task Switch
	const Interrupt SegmentNotPresent{ 11, Type::Fault, "#NP" };    // Segment Not Present
	const Interrupt StackFault{ 12, Type::Fault, "#SS" };           // Stack Segment Error
	const Interrupt GeneralProtection{ 13, Type::Fault, "#GP" };    // General Protection
	const Interrupt PageFault{ 14, Type::Fault, "#PF" };            // Page Fault
//	const Interrupt _{ 15, Type::None, "" };                                              // Reserved
	const Interrupt FloatingPointError{ 16, Type::Fault, "#MF" };   // FPU Error
	const Interrupt AlignmentError{ 17, Type::Fault, "#AC" };       // Memory alignment
//	const Interrupt MachineError{ 18, Type::Abort, "#MC" };                               // Machine check

	/**
	 * @brief Descriptor stored in the IDT, describing the privilege level of the interrupt and where to find its handler
	 */
	struct InterruptDescriptor
	{
		enum class Type : U8 {
			None = 0,
			Task = 1,
			Interrupt = 2,
			Trap = 3
		};

		bit present : 1 = 0;
		bit DPL : 1 = 0;
		bit gate_size : 1 = 0;
		Type type : 2 = Type::None;
		U8 : 0;

		U16 segment_selector = 0;
		U32 offset = 0;

		void asNone()
		{
			present = 0;
			DPL = 0;
			gate_size = 0;
			type = Type::None;
			segment_selector = 0;
			offset = 0;
		}

		void asTask(U16 TSS_segment)
		{
			present = 1;
			// TODO : DPL?
			gate_size = 0;
			type = Type::Task;
			segment_selector = TSS_segment;
			offset = 0;
		}

		void asInterrupt(bit _32bit_gate, U16 handler_segment, U32 handler_offset)
		{
			present = 1;
			// TODO : DPL?
			gate_size = _32bit_gate;
			type = Type::Interrupt;
			segment_selector = handler_segment;
			offset = handler_offset;
		}

		void asTrap(bit _32bit_gate, U16 handler_segment, U32 handler_offset)
		{
			present = 1;
			// TODO : DPL?
			gate_size = _32bit_gate;
			type = Type::Trap;
			segment_selector = handler_segment;
			offset = handler_offset;
		}
	};

	/**
	 * @brief IDT, who lists all of the interrupts which can be encountered during execution, as well as their handlers
	 */
	template<U8 length = 255>
	class InterruptDescriptorTable
	{
		static_assert(length >= 32); // all descriptors from 0 to 31 are reserved and mandatory for basic exception handling

		InterruptDescriptor table[length];
	};
}
