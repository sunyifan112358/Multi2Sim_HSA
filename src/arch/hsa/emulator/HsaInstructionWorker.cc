/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <arch/hsa/disassembler/BrigCodeEntry.h>

#include "HsaInstructionWorker.h"
#include "WorkItem.h"
#include "StackFrame.h"

namespace HSA
{
HsaInstructionWorker::HsaInstructionWorker(WorkItem *work_item,
		StackFrame *stack_frame) :
		work_item(work_item),
		stack_frame(stack_frame)
{
}


void HsaInstructionWorker::getOperandValue(unsigned int index, void *buffer)
{
	// Get the operand entry
	BrigCodeEntry *inst = stack_frame->getPc();
	auto operand = inst->getOperand(index);

	// Do corresponding action according to the type of operand
	switch (operand->getKind())
	{
	case BRIG_KIND_OPERAND_CONSTANT_BYTES:

	{
		BrigImmed immed(operand->getBytes(),
				inst->getOperandType(index));

		immed.getImmedValue(buffer);
		return;
	}

	case BRIG_KIND_OPERAND_WAVESIZE:

		*(unsigned int *)buffer = 1;
		return;

	case BRIG_KIND_OPERAND_REGISTER:

	{
		std::string register_name = operand->getRegisterName();
		stack_frame->getRegisterValue(register_name, buffer);
		return;
	}

	case BRIG_KIND_OPERAND_ADDRESS:

	{
		unsigned address;
		unsigned long long offset = operand->getOffset();
		if (operand->getSymbol().get())
		{
			auto symbol = operand->getSymbol();
			std::string name = symbol->getName();

			// Get the variable
			Variable *variable =
				stack_frame->getSymbol(name);

			// If the variable is not found in stack frame
			// try kernel argument
			if (!variable)
				variable = work_item->getGrid()->
						getKernelArgument(name);

			// If the variable is still not found
			if (!variable)
				throw misc::Error(misc::fmt(
						"Symbol %s is not"
						" defined",
						name.c_str()));

			// Variable not in stack frame, try kernel
			// argument
			address = variable->getAddress();
		}
		else
		{
			std::string register_name = operand->getReg()
						->getRegisterName();
			stack_frame->getRegisterValue(register_name,
					&address);
		}
		address += offset;
		*(unsigned *)buffer = address;
		return;
	}

	case BRIG_KIND_OPERAND_OPERAND_LIST:

	{
		// Get the vector modifier
		unsigned vector_size = inst->getVectorModifier();
		for (unsigned int i = 0; i < vector_size; i++)
		{
			auto op_item = operand->getOperandElement(i);
			switch (op_item->getKind())
			{
			case BRIG_KIND_OPERAND_REGISTER:

			{
				std::string register_name =
						op_item->
						getRegisterName();
				unsigned size = AsmService::getSizeInByteByRegisterName(
								register_name);
				stack_frame->getRegisterValue
						(register_name,
						(unsigned char *)buffer
						+ i * size);
				break;
			}

			default:
				throw misc::Panic(misc::fmt(
						"Unsupported operand "
						"type in operand list")
						);
			}
		}
		break;
	}

	default:

		throw misc::Panic("Unsupported operand type "
				"for getOperandValue");
		break;

	}
}


void HsaInstructionWorker::setOperandValue(unsigned int index, void *value)
{
	// Get the operand entry
	BrigCodeEntry *inst = stack_frame->getPc();
	auto operand = inst->getOperand(index);

	// Do corresponding action according to the type of operand
	switch (operand->getKind())
	{
	case BRIG_KIND_OPERAND_REGISTER:

	{
		std::string register_name = operand->getRegisterName();
		stack_frame->setRegisterValue(register_name, value);
		break;
	}

	case BRIG_KIND_OPERAND_OPERAND_LIST:

	{
		// Get the vector modifier
		unsigned vector_size = inst->getVectorModifier();
		for (unsigned int i = 0; i < vector_size; i++)
		{
			auto op_item = operand->getOperandElement(i);
			switch (op_item->getKind())
			{
			case BRIG_KIND_OPERAND_REGISTER:

			{
				std::string register_name =
						op_item->
						getRegisterName();
				unsigned size = AsmService::getSizeInByteByRegisterName(
						register_name);
				stack_frame->setRegisterValue
						(register_name,
						 (unsigned char *)value + i * size);
				break;
			}

			default:
				throw misc::Panic(misc::fmt(
						"Unsupported operand "
						"type in operand list")
						);
			}
		}
		break;
	}

	default:

		throw misc::Panic("Unsupported operand type "
				"for storeOperandValue");
	}
}
}
