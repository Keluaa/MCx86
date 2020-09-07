// Minecraft Computer Breakdown.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include "pch.h"
#include <iostream>

#include "data_types.h"
#include "ALU.h"
#include "registers.h"
#include "RAM.h"
#include "ROM.h"
#include "Instructions.h"

int main()
{
	Registers registers;

	const int instructions_count = 5;
	ISA::Instruction* instructions = new ISA::Instruction[instructions_count]{
		ISA::MOV(ISA::Resigters::EAX, U8(0)),
		ISA::MOV(ISA::Resigters::EDX, U8(1)),
		ISA::ADD(ISA::Resigters::EAX, ISA::Resigters::EDX),
		ISA::XCHG(ISA::Resigters::EAX, ISA::Resigters::EDX),
		ISA::JO(2)
	};
	
	while (registers.EIP < instructions_count) {
		const ISA::Instruction* base_inst = instructions + registers.EIP * sizeof(void*);
		switch (base_inst->opcode)
		{
		case ISA::Opcodes::ADD:
		{
			const ISA::ADD* inst = static_cast<const ISA::ADD*>(inst);
			bit carry = 0;
			U32 res = ALU::add(registers.read(inst->destination), registers.read(inst->operand), carry);
			registers.write(inst->destination, res);
			break;
		}
		case ISA::Opcodes::MOV:
		{
			const ISA::MOV* inst = static_cast<const ISA::MOV*>(inst);
			registers.write(inst->destination, registers.read(inst->source));
			break;
		}
		case ISA::Opcodes::JO:
		{
			const ISA::JO* inst = static_cast<const ISA::JO*>(inst);
			if (registers.flag_read_OF()) {
				registers.EIP = inst->address;
				continue; // skip EIP increment
			}
			break;
		}
		case ISA::Opcodes::XCHG:
		{
			const ISA::XCHG* inst = static_cast<const ISA::XCHG*>(inst);
			U32 tmp = registers.read(inst->destination);
			registers.write(inst->destination, registers.read(inst->source));
			registers.write(inst->source, tmp);
			break;
		}

		default:
			break;
		}
		registers.EIP++;
	}


    std::cout << "Hello World!\n"; 
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Conseils pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
