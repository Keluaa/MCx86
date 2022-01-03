#pragma once

#include "../data_types.h"


namespace Interrupts
{
    inline U8 error_code(U8 code, bit idt, bit ext)
    {
        // See 'Operation' section of the INT instruction of the SDM
        if (idt) {
            return (code << 3) | 2 | ext;
        }
        else {
            // We don't care about FCH
            return code | ext;
        }
    }

	enum class Type : U8 { None, Fault, Trap, Abort, User };

	struct Interrupt 
	{
		U8 vector; // or interrupt code, this is an index into the interrupt descriptor table
		Type type;
		const char* mnemonic;
	};

	// Common interrupts which are triggered by some instructions
	// Some interrupts are unused, simply because they have no purpose in this implementation

	const Interrupt DivideError       { 0, Type::Fault,  "#DE" }; // Divide Error
	const Interrupt DebugException    { 1, Type::Fault,  "#DB" }; // Debug Exception
//	const Interrupt NMInterrupt       { 2, Type::None,   ""    }; // Non-maskable external Interrupt
	const Interrupt Breakpoint        { 3, Type::Trap,   "#BP" }; // Breakpoint
	const Interrupt OverflowException { 4, Type::Trap,   "#OF" }; // Overflow
	const Interrupt BoundException    { 5, Type::Fault,  "#BR" }; // BOUND Range Exceeded
	const Interrupt OpcodeException   { 6, Type::Fault,  "#UD" }; // Invalid Opcode
//	const Interrupt NoMathException   { 7, Type::Fault,  "#NM" }; // Math Coprocessor Not Available
	const Interrupt DoubleFault       { 8, Type::Abort,  "#DF" }; // Double Fault
//	const Interrupt FPCoprocessorError{ 9, Type::Fault,  ""    }; // Floating point Coprocessor Error
	const Interrupt InvalidTaskSwitch { 10, Type::Fault, "#TS" }; // Invalid Task Switch
	const Interrupt SegmentNotPresent { 11, Type::Fault, "#NP" }; // Segment Not Present
	const Interrupt StackFault        { 12, Type::Fault, "#SS" }; // Stack Segment Error
	const Interrupt GeneralProtection { 13, Type::Fault, "#GP" }; // General Protection
	const Interrupt PageFault         { 14, Type::Fault, "#PF" }; // Page Fault
//	const Interrupt _                 { 15, Type::None,  ""    }; // Reserved
	const Interrupt FloatingPointError{ 16, Type::Fault, "#MF" }; // FPU Error
	const Interrupt AlignmentError    { 17, Type::Fault, "#AC" }; // Memory alignment
//	const Interrupt MachineError      { 18, Type::Abort, "#MC" }; // Machine check
    // other interrupts from 19 to 31 are reserved


	/**
	 * Descriptor stored in the IDT, describing the privilege level of the interrupt and where to find its handler
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

		void as_none()
		{
			present = 0;
			DPL = 0;
			gate_size = 0;
			type = Type::None;
			segment_selector = 0;
			offset = 0;
		}

		void as_task(U16 TSS_segment)
		{
			present = 1;
			// TODO : DPL?
			gate_size = 0;
			type = Type::Task;
			segment_selector = TSS_segment;
			offset = 0;
		}

		void as_interrupt(bit _32bit_gate, U16 handler_segment, U32 handler_offset)
		{
			present = 1;
			// TODO : DPL?
			gate_size = _32bit_gate;
			type = Type::Interrupt;
			segment_selector = handler_segment;
			offset = handler_offset;
		}

		void as_trap(bit _32bit_gate, U16 handler_segment, U32 handler_offset)
		{
			present = 1;
			// TODO : DPL?
			gate_size = _32bit_gate;
			type = Type::Trap;
			segment_selector = handler_segment;
			offset = handler_offset;
		}
	};


    class GlobalDescriptorTable
    {
        // TODO ?
    };


	/**
	 * IDT, who lists all the interrupts which can be encountered during execution, as well as their handlers
	 */
	template<U8 length = 255>
	class InterruptDescriptorTable
	{
		static_assert(length >= 32); // all descriptors from 0 to 31 are reserved and mandatory for basic exception handling

		const InterruptDescriptor* table;

    public:
        const U32 base_address;
        const U32 limit = sizeof(InterruptDescriptor) * length;

        /**
         * 'base_address' is the address of the IDT, while 'mem_pointer' is the real pointer to the IDT, useful only in
         * the implementation.
         */
        InterruptDescriptorTable(U32 base_address, const U32* mem_pointer)
            : table(reinterpret_cast<const InterruptDescriptor*>(mem_pointer))
            , base_address(base_address)
        { }

        [[nodiscard]] const InterruptDescriptor& get_descriptor(U8 vector) const { return table[vector]; }
	};
}
