/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cstdlib>
#include <iostream>

#include <arch/common/Runtime.h>
#include <arch/mips/asm/Asm.h>
#include <arch/mips/emu/Context.h>
#include <arch/mips/emu/Emu.h>
#include <arch/x86/asm/Asm.h>
#include <arch/x86/emu/Context.h>
#include <arch/x86/emu/Emu.h>
#include <arch/x86/emu/FileTable.h>
#include <arch/x86/emu/Signal.h>
#include <arch/hsa/asm/Asm.h>
#include <arch/hsa/emu/Emu.h>
#include <driver/opencl/OpenCLDriver.h>
#include <driver/opengl/OpenGLDriver.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Misc.h>
#include <lib/esim/ESim.h>

#include "Wrapper.h"

void mips_emulation_loop(misc::CommandLine &command_line)
{
	MIPS::Emu *emu = MIPS::Emu::getInstance();
	MIPS::Context *context = emu->newContext();
	context->Load(command_line.getArguments(),
			std::vector<std::string>(),misc::getCwd(),
			"", "");
	esim::ESim *esim = esim::ESim::getInstance();
	while (!esim->hasFinished())
	{
		bool active = emu->Run();
		if (!active)
			esim->Finish(esim::ESimFinishCtx);
		esim->ProcessEvents();
	}
}


void x86_emulation_loop(misc::CommandLine &command_line)
{
	x86::Emu *emu = x86::Emu::getInstance();
	x86::Context *context = emu->newContext();
	context->Load(command_line.getArguments(),
			std::vector<std::string>(), misc::getCwd(),
			"", "");
	esim::ESim *esim = esim::ESim::getInstance();
	while (!esim->hasFinished())
	{
		bool active = emu->Run();
		if (!active)
			esim->Finish(esim::ESimFinishCtx);
		esim->ProcessEvents();
	}
}


// Load a program from the command line
void loadCommandLineProgram(misc::CommandLine &command_line)
{
	// No program specified
	if (command_line.getNumArguments() == 0)
		return;
	
	// Get executable path
	std::string path = command_line.getArgument(0);

	// Read ELF header
	ELFReader::Header header(path);
	switch (header.getMachine())
	{
	case EM_386:
	{
		x86::Emu *emu = x86::Emu::getInstance();
		emu->loadProgram(command_line.getArguments());
		break;
	}

	case EM_ARM:
	{
		std::cout << "ARM\n";
		break;
	}

	case EM_MIPS:
	{
		MIPS::Emu *emu = MIPS::Emu::getInstance();
		emu->loadProgram(command_line.getArguments());
		break;
	}
	
	default:
		misc::fatal("%s: unsupported ELF architecture", path.c_str());
	}
}


// Load programs from context configuration file
void loadPrograms(misc::CommandLine &command_line)
{
#if 0
	struct config_t *config;

	char section[MAX_STRING_SIZE];
	char exe_full_path[MAX_STRING_SIZE];
	char *exe_file_name;
	char *cwd_path;

	Elf32_Ehdr ehdr;

	int id;

	/* Load guest program specified in the command line */
	if (argc > 1)
	{
		/* Load program depending on architecture */
		elf_file_read_header(argv[1], &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			X86EmuLoadContextFromCommandLine(x86_emu, argc - 1, argv + 1);
			break;

		case EM_ARM:
			arm_ctx_load_from_command_line(argc - 1, argv + 1);
			break;

		case EM_MIPS:
			MIPSEmuLoadContextFromCommandLine(mips_emu, argc - 1, argv + 1);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	/* Continue processing the context configuration file, if specified. */
	if (!*ctx_config_file_name)
		return;

	/* Open file */
	config = config_create(ctx_config_file_name);
	if (*ctx_config_file_name)
		config_load(config);

	/* Iterate through consecutive contexts */
	for (id = 0; ; id++)
	{
		/* Read section */
		snprintf(section, sizeof section, "Context %d", id);
		if (!config_section_exists(config, section))
			break;

		/* Read executable full path */
		exe_file_name = config_read_string(config, section, "Exe", "");
		cwd_path = config_read_string(config, section, "Cwd", "");
		file_full_path(exe_file_name, cwd_path, exe_full_path, sizeof exe_full_path);

		/* Load context depending on architecture */
		elf_file_read_header(exe_full_path, &ehdr);
		switch (ehdr.e_machine)
		{
		case EM_386:
			X86EmuLoadContextsFromConfig(x86_emu, config, section);
			break;

		case EM_ARM:
			arm_ctx_load_from_ctx_config(config, section);
			break;

		default:
			fatal("%s: unsupported ELF architecture", argv[1]);
		}
	}

	/* Close file */
	config_check(config);
	config_free(config);
#endif
}


void mainLoop()
{
	esim::ESim *esim = esim::ESim::getInstance();
	x86::Emu *x86_emu = x86::Emu::getInstance();
	MIPS::Emu *mips_emu = MIPS::Emu::getInstance();
	while (!esim->hasFinished())
	{
		bool active = false;
		
		// Run for all architectures
		active |= x86_emu->Run();
		active |= mips_emu->Run();

		// Check if finished
		if (!active)
			esim->Finish(esim::ESimFinishCtx);

		// Next cycle
		esim->ProcessEvents();
	}
}


void main_cpp(int argc, char **argv)
{
	// Read command line
	misc::CommandLine command_line(argc, argv);
	command_line.setErrorMessage("Please type 'm2s --help' for a list of "
			"valid Multi2Sim command-line options.\n");
	command_line.setHelp("Syntax:"
			"\n\n"
			"$ m2s [<options>] [<exe>] [<args>]"
			"\n\n"
			"Multi2Sim's command line can take a program "
			"executable <exe> as an argument, given as a binary "
			"file in any of the supported CPU architectures, and "
			"optionally followed by its arguments <args>. The "
			"following list of command-line options can be used "
			"for <options>:");

	// Register three sample command-line options
	long long m2s_max_time = 0;
	command_line.RegisterInt64("--max-time", m2s_max_time,
			"Maximum simulation time in seconds. The simulator "
			"will stop once this time is exceeded. A value of 0 "
			"(default) means no time limit.");
	
	std::string m2s_trace_file;
	command_line.RegisterString("--trace", m2s_trace_file,
			"Generate a trace file with debug information on the "
			"configuration of the modeled CPUs, GPUs, and memory "
			"system, as well as their dynamic simulation. The "
			"trace is a compressed plain-text file in format. The "
			"user should watch the size of the generated trace as "
			"simulation runs, since the trace file can quickly "
			"become extremely large.");
	
	std::string m2s_visual_file;
	command_line.RegisterString("--visual", m2s_visual_file,
			"Run the Multi2Sim Visualization Tool. This option "
			"consumes a file generated with the '--trace' option "
			"in a previous simulation. This option is only "
			"available on systems with support for GTK 3.0 or "
			"higher.");

	// Register module configurations
	command_line.AddConfig(x86::Asm::config);
	command_line.AddConfig(x86::Emu::config);
	command_line.AddConfig(MIPS::Emu::config);
	command_line.AddConfig(HSA::Asm::config);
	command_line.AddConfig(HSA::Emu::config);

	// Process command line. Return to C version of Multi2Sim if a
	// command-line option was not recognized.
	if (!command_line.Process(false))
		return;

	// Finish if C++ version of Multi2Sim is not activated
	if (!command_line.getUseCpp())
		return;

	//
	// Register architectures
	//

	// Get architecture pool
	comm::ArchPool *arch_pool = comm::ArchPool::getInstance();

	// x86
	arch_pool->Register("x86",
			x86::Asm::getInstance(),
			x86::Emu::getInstance());
	
	// Southern Islands
	arch_pool->Register("SouthernIslands");

	// HSA
	arch_pool->Register("HSA");

	// MIPS
	arch_pool->Register("MIPS",
			MIPS::Asm::getInstance(),
			MIPS::Emu::getInstance());



	//
	// Register runtimes
	//

	comm::RuntimePool *runtime_pool = comm::RuntimePool::getInstance();
	runtime_pool->Register("OpenCL", "OpenCL", "m2s-opencl", "/dev/m2s-si-cl",
			Driver::OpenCLSIDriver::getInstance());

#ifdef HAVE_OPENGL
	runtime_pool->Register("OpenGL", "OpenGL", "m2s-opengl", "/dev/m2s-si-gl",
			Driver::OpenGLSIDriver::getInstance());
#endif


	// Load programs
	loadCommandLineProgram(command_line);
	loadPrograms(command_line);

	// Main simulation loop
	mainLoop();

	// End
	exit(0);
}

