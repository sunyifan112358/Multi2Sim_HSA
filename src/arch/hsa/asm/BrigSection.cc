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

#include <lib/cpp/String.h>

#include "BrigSection.h"
#include "BrigFile.h"

namespace HSA{

BrigSection::BrigSection(ELFReader::Section *elfSection)
{
	this->elf_section = elfSection;
	std::string sectionName = elfSection->getName();	
	
}


BrigSection::~BrigSection()
{
}


void BrigSection::DumpSectionHex(std::ostream &os = std::cout) const
{
	os << misc::fmt("\n********** Section %s **********\n", 
			this->getName().c_str());
	
	const unsigned char *buf = (const unsigned char *)this->getBuffer();
	for (unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", buf[i]);
		if ((i + 1) % 4 == 0)
		{
			os << " ";
		}
		if ((i + 1) % 16 == 0)
		{
			os << "\n";
		}
	}
	os << "\n";
}

}  // namespace HSA

