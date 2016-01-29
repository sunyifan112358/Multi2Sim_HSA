
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

#include <cstring>

#include "Binary.h"
#include "Disassembler.h"

namespace VI
{

void Binary::ReadNotes(BinaryDictEntry *dict_entry)
{
	
}


void Binary::ReadDictionary()
{

}

void Binary::ReadSegments()
{
	
}

void Binary::ReadSections()
{

}

Binary::Binary(const char *buffer, unsigned int size, std::string name)
		: ELFReader::File(bugger, size)
{
	this->name = name;
	
	// Read encoding dictionary
	vi_dict_entry = NULL;
	ReadDictionary();
	if (!vi_dict_entry)
		// throw error

	ReadSegments();
	ReadSections();

	ReadNotes(vi_dict_entry);
}


Binary::~Binary()
{
	
}
} // namespace VI
