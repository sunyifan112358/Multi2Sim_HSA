/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cassert>
#include <cstring>

#include <lib/cpp/ELFReader.h>
#include <lib/cpp/String.h>

#include "Asm.h"
#include "Inst.h"

using namespace misc;


namespace x86
{

// List of possible prefixes
static const unsigned char asm_prefixes[] =
{
	0xf0,  // lock
	0xf2,  // repnz
	0xf3,  // rep
	0x66,  // op
	0x67,  // addr
	0x2e,  // use cs
	0x36,  // use ss
	0x3e,  // use ds
	0x26,  // use es
	0x64,  // use fs
	0x65   // use gs
};


void Asm::InsertInstInfo(InstDecodeInfo **table, InstDecodeInfo *elem, int at)
{
	// First entry
	if (!table[at])
	{
		table[at] = elem;
		return;
	}

	// Go to the end of the list
	InstDecodeInfo *prev = table[at];
	while (prev->next)
		prev = prev->next;
	prev->next = elem;
}


void Asm::InsertInstInfo(InstInfo *info)
{
	// Obtain the table where to insert, the initial index, and
	// the number of times we must insert the instruction.
	int index;
	int count;
	InstDecodeInfo **table;
	if ((info->op1 & 0xff) == 0x0f)
	{
		table = dec_table_0f;
		index = info->op2 & 0xff;
		count = info->op2 & INDEX ? 8 : 1;
	}
	else
	{
		table = dec_table;
		index = info->op1 & 0xff;
		count = info->op1 & INDEX ? 8 : 1;
	}

	// Insert
	for (int i = 0; i < count; i++)
	{
		InstDecodeInfo *elem = new InstDecodeInfo();
		elem->info = info;
		InsertInstInfo(table, elem, index + i);
	}
}


void Asm::FreeInstDecodeInfo(InstDecodeInfo *elem)
{
	while (elem)
	{
		InstDecodeInfo *next = elem->next;
		delete elem;
		elem = next;
	}
}


Asm::Asm()
{
	// Initialize instruction information list
	memset(inst_info, 0, sizeof inst_info);
	inst_info[InstOpcodeInvalid].fmt = "";
	struct InstInfo *info;
#define DEFINST(__name, __op1, __op2, __op3, __modrm, __imm, __prefixes) \
	info = &inst_info[INST_##__name]; \
	info->opcode = INST_##__name; \
	info->op1 = __op1; \
	info->op2 = __op2; \
	info->op3 = __op3; \
	info->modrm = __modrm; \
	info->imm = __imm; \
	info->prefixes = __prefixes; \
	info->fmt = #__name;
#include "Asm.dat"
#undef DEFINST

	// Initialize table of prefixes
	memset(is_prefix, 0, sizeof is_prefix);
	for (unsigned i = 0; i < sizeof asm_prefixes; i++)
		is_prefix[asm_prefixes[i]] = true;

	// Initialize decoding table. This table contains lists of information
	// structures for x86 instructions. To find an instruction in the table,
	// the table can be indexed by the first byte of its opcode.
	memset(dec_table, 0, sizeof dec_table);
	memset(dec_table_0f, 0, sizeof dec_table_0f);
	for (int op = 1; op < InstOpcodeCount; op++)
	{
		// Insert into table
		info = &inst_info[op];
		InsertInstInfo(info);

		// Compute 'match_mask' and 'mach_result' fields. Start with
		// the 'modrm' field in the instruction format definition.
		if (!(info->modrm & SKIP))
		{
			info->modrm_size = 1;

			// If part of the offset is in the 'reg' field of the ModR/M byte,
			// it must be matched.
			if (!(info->modrm & REG))
			{
				info->match_mask = 0x38;
				info->match_result = (info->modrm & 0x7) << 3;
			}

			// If instruction expects a memory operand, the 'mod' field of 
			// the ModR/M byte cannot be 11.
			if (info->modrm & MEM)
			{
				info->nomatch_mask = 0xc0;
				info->nomatch_result = 0xc0;
			}
		}

		// Third opcode byte
		if (!(info->op3 & SKIP))
		{
			info->opcode_size++;
			info->match_mask <<= 8;
			info->match_result <<= 8;
			info->nomatch_mask <<= 8;
			info->nomatch_result <<= 8;
			info->match_mask |= 0xff;
			info->match_result |= info->op3 & 0xff;
			assert(!(info->op3 & INDEX));
		}

		// Second opcode byte
		if (!(info->op2 & SKIP))
		{
			info->opcode_size++;
			info->match_mask <<= 8;
			info->match_result <<= 8;
			info->nomatch_mask <<= 8;
			info->nomatch_result <<= 8;
			info->match_mask |= 0xff;
			info->match_result |= info->op2 & 0xff;

			// The opcode has an index
			if (info->op2 & INDEX) {
				info->match_mask &= 0xfffffff8;
				info->opindex_shift = 8;
			}
		}

		// First opcode byte (always there)
		info->opcode_size++;
		info->match_mask <<= 8;
		info->match_result <<= 8;
		info->nomatch_mask <<= 8;
		info->nomatch_result <<= 8;
		info->match_mask |= 0xff;
		info->match_result |= info->op1 & 0xff;
		if (info->op1 & INDEX)
		{
			info->match_mask &= 0xfffffff8;
			info->opindex_shift = 0;
		}

		// Immediate size
		if (info->imm & IB)
			info->imm_size = 1;
		if (info->imm & IW)
			info->imm_size = 2;
		if (info->imm & ID)
			info->imm_size = 4;
	}
}


Asm::~Asm()
{
	// Free instruction info tables
	for (int i = 0; i < 0x100; i++)
	{
		FreeInstDecodeInfo(dec_table[i]);
		FreeInstDecodeInfo(dec_table_0f[i]);
	}
}


void Asm::DisassembleBinary(const std::string &path, std::ostream &os) const
{
	// Traverse sections of ELF file
	ELFReader::File file(path);
	for (int idx = 0; idx < file.getNumSections(); idx++)
	{
		// Get section and skip if it does not contain code
		ELFReader::Section *section = file.getSection(idx);
		if ((section->getFlags() & SHF_EXECINSTR) == 0)
			continue;

		// Title
		os << "Disassembly of section " << section->getName()
				<< ":\n";

		// Keep track of current symbol
		int current_symbol = 0;
		ELFReader::Symbol *symbol = file.getSymbol(current_symbol);

		// Disassemble
		Inst inst(this);
		unsigned offset = 0;
		while (offset < section->getSize())
		{
			// Get position in section
			unsigned eip = section->getAddr() + offset;
			const char *buffer = section->getBuffer() + offset;

			// Decode instruction
			inst.Decode(buffer, eip);
			if (inst.getOpcode())
			{
				assert(inst.getSize());
				offset += inst.getSize();
			}
			else
				offset++;

			// Symbol
			while (symbol && symbol->getValue() < eip)
			{
				current_symbol++;
				symbol = file.getSymbol(current_symbol);
			}
			if (symbol && symbol->getValue() == eip)
				os << StringFmt("\n%08x <%s>:\n", eip,
						symbol->getName().c_str());

			// Address
			os << StringFmt("%8x:\t", eip);

			// Hex dump of bytes 0..6
			for (int i = 0; i < 7; i++)
			{
				if (i < inst.getSize())
					os << StringFmt("%02x ",
							(unsigned char)
							buffer[i]);
				else
					os << "   ";
			}

			// Instruction
			os << '\t';
			if (inst.getOpcode())
				os << inst;
			else
				os << "???";
			os << '\n';

			// Hex dump of bytes 7..13
			if (inst.getSize() > 7)
			{
				os << StringFmt("%8x:\t", eip + 7);
				for (int i = 7; i < 14 && i < inst.getSize(); i++)
					os << StringFmt("%02x ",
							(unsigned char)
							buffer[i]);
				os << '\n';
			}
		}

		// Pad
		os << '\n';
	}
}


}  // namespace x86

