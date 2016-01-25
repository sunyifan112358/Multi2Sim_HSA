/*
 *  Multi2Sim
 *  Copyright (C) 2016  Harrison Barclay (barclay.h@husky.neu.edu)
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

#ifndef ARCH_VOLCANIC_ISLANDS_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_VOLCANIC_ISLANDS_DISASSEMBLER_DISASSEMBLER_H

#include <memory>

#include <arch/common/Disassembler.h>

#include "Instruction.h"

namespace VI 
{

class Disassembler : public comm::Disassembler
{ 
 	// Unique instance of Disassembler 
	static std::unique_ptr<Disassembler> instance;
	
	// Path to binary file
	static std::string binary_file;

public:

	// Constructor
	Disassembler() : comm::Disassembler("vi") {}
   
	/// Disassemble binary file
	void DisassembleBinary(const std::string &path);

	/// Get the only instance of the Volcanic Islands disassembler.	
	static Disassembler *getInstance();

	/// Destroy the disassembler singleton	
	static void Destroy () 
	{
		instance = nullptr;
	}

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();
};

}

#endif

