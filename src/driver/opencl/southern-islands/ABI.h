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

#ifndef DRIVER_OPENCL_SI_ABI_H
#define DRIVER_OPENCL_SI_ABI_H

#include <string>

#include <src/arch/southern-islands/emu/Emu.h>
#include <src/arch/southern-islands/emu/NDRange.h>
#include <src/driver/opencl/ABI.h>
#include "Program.h"
#include "Kernel.h"

namespace SI
{

// List of OpenCL Runtime calls
enum OpenCLABICall
{
	OpenCLABIInvalid = 0,
// Shared ABIs
#define SI_ABI_CALL(space, name, code) OpenCLABI##space##name = code,
#include "../../common/SI-ABI.dat"
#undef SI_ABI_CALL

#define OPENCL_ABI_CALL(space, name, code) OpenCLABI##space##name = code,
#include "../ABI.dat"
#undef OPENCL_ABI_CALL
	OpenCLABICallCount
};

// Forward declarations of OpenCL Runtime functions
#define SI_ABI_CALL(space, name, code) \
	int SIABI##name##Impl();
#include "../../common/SI-ABI.dat"
#undef SI_ABI_CALL

#define OPENCL_ABI_CALL(space, name, code) \
	int OpenCLABI##name##Impl();
#include "../ABI.dat"
#undef OPENCL_ABI_CALL

// List of OpenCL ABI call names
std::string OpenCLABICallName[OpenCLABICallCount + 1] =
{
	nullptr,
#define SI_ABI_CALL(space, name, code) #space #name,
#include "../../common/SI-ABI.dat"
#undef SI_ABI_CALL

#define OPENCL_ABI_CALL(space, name, code) #space #name,
#include "../ABI.dat"
#undef OPENCL_ABI_CALL
	nullptr
};

}  // namespace SI

#endif
