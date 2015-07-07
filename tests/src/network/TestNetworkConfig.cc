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

#include "gtest/gtest.h"

#include <string>
#include <regex>
#include <exception>
#include <network/System.h>
#include <lib/cpp/IniFile.h>
#include <lib/cpp/Error.h>
#include <lib/esim/Engine.h>
#include <lib/esim/Trace.h>

namespace net
{

static void Cleanup()
{
	esim::Engine::Destroy();

	System::Destroy();
}

TEST(TestSystemConfiguration, section_general_frequency)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 2000000";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *network_system = System::getInstance();
	EXPECT_TRUE(network_system != nullptr);

	// Test body
	std::string message;
	try
	{
		network_system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	EXPECT_REGEX_MATCH(misc::fmt(".*%s: The value for 'Frequency' "
			"must be between 1MHz and 1000GHz.\n.*",
			ini_file.getPath().c_str()).c_str(),
			message.c_str());
}

TEST(TestSystemConfiguration, section_network_two_defaults_missing)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ General ]\n"
			"Frequency = 1000\n"
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 10";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_missing_over_negative)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_non_existant_default)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be"
					" zero/non-existent.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_default_negative)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = -5\n"
			"DefaultBandwidth = 1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_network_negative_packet_size)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: test:\nDefault values can not be negative.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_unknown_type)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 1\n"
			"\n"
			"[Network.test.Node.N1]\n"
			"Type = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Node type 'anything' is not "
					"supported.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_section_unknown)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.anything]";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(misc::fmt(
			"%s: invalid section (.Network\\.test\\.anything.)",
			ini_file.getPath().c_str()).c_str(), message.c_str());

}

TEST(TestSystemConfiguration, section_node_no_preset_net)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[Network.test.Node.N1]\n"
			"Type = Switch\n";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	// FIXME
	// This should produce an error saying network not defined
	// but currently produces an error for each variable
	// no matter they are valid or not.
	EXPECT_TRUE(system->getNetworkByName("test") == nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: section (.Network\\.test\\.Node\\.N1.), "
					"invalid variable 'Type'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_buffer_size_vs_msg_size)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.Node.N1]\n"
			"Type = EndNode\n"
			"InputBufferSize = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}
	EXPECT_TRUE(system->getNetworkByName("test") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Invalid argument for Node 'N1'\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_node_switch_bandwidth)
{
	// cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"\n"
			"[Network.test.Node.S1]\n"
			"Type = Switch\n"
			"InputBufferSize = 1\n"
			"Bandwidth = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);


	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_FALSE(network->getNodeByName("S1") != nullptr);
	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Invalid argument for Node 'S1'\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_type)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = anything";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link type 'anything' is not supported.\n.*",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_variable)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"anything = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: section (.Network\\.test\\.Link\\.S1-S2.), invalid"
					" variable 'anything'",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_source)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Source = anything\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Source node 'anything' is invalid"
					" for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_no_source)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Bidirectional\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Source node is not provided "
					"for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_wrong_destination)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Dest = anything\n"
			"Source = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Destination node 'anything' is "
					"invalid for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_no_destination)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Destination node is not provided "
					"for link 'S1-S2'.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_same_src_dst)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link 'S1-S2', source "
					"and destination cannot be the same.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_bandwidth)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = Switch\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2\n"
			"Bandwidth = -1";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("%s: Link 'S1-S2', bandwidth cannot "
					"be zero/negative.\n",
					ini_file.getPath().c_str()).c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, section_link_between_end_nodes)
{
	// Cleanup singleton instance
	Cleanup();

	// Setup configuration file
	std::string config =
			"[ Network.test ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 1\n"
			"DefaultPacketSize = 0\n"
			"[Network.test.Node.S1]\n"
			"Type = EndNode\n"
			"[Network.test.Node.S2]\n"
			"Type = EndNode\n"
			"[Network.test.Link.S1-S2]\n"
			"Type = Unidirectional\n"
			"Source = S1\n"
			"Dest = S2";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Set up network instance
	System *system = System::getInstance();
	EXPECT_TRUE(system != nullptr);

	// Test body
	std::string message;
	try
	{
		system->ParseConfiguration(&ini_file);
	}
	catch (misc::Error &error)
	{
		message = error.getMessage();
	}

	Network *network = system->getNetworkByName("test");
	EXPECT_TRUE(network != nullptr);
	EXPECT_TRUE(network->getNodeByName("S1") != nullptr);
	EXPECT_TRUE(network->getNodeByName("S2") != nullptr);

	EXPECT_REGEX_MATCH(
			misc::fmt("Network 'test': Link 'S1-S2' cannot connect "
					"two end-nodes.").c_str(),
					message.c_str());
}

TEST(TestSystemConfiguration, config_1_net_4_node_1_switch)
{
	// Cleanup singleton instance
	Cleanup();

	// Setting up the configuration file
	std::string net_config =
			"[ Network.net0 ]\n"
			"DefaultInputBufferSize = 4\n"
			"DefaultOutputBufferSize = 4\n"
			"DefaultBandwidth = 2\n"
			"\n"
			"[ Network.net0.Node.n0 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n1 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n2 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.n3 ]\n"
			"Type = EndNode\n"
			"\n"
			"[ Network.net0.Node.s0 ]\n"
			"Type = Switch\n"
			"\n"
			"[ Network.net0.Link.n0-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n0\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n1-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n1\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n2-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n2\n"
			"Dest = s0\n"
			"\n"
			"[ Network.net0.Link.n3-s0 ]\n"
			"Type = Bidirectional\n"
			"Source = n3\n"
			"Dest = s0";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(net_config);

	// Set up network instance
	System *network_system = System::getInstance();


	// Parse the configuration file
	network_system->ParseConfiguration(&ini_file);

	// Assert the system
	EXPECT_TRUE(network_system != nullptr);

	// Assert the network
	Network * net0 = network_system->getNetworkByName("net0");
	EXPECT_TRUE(net0 != nullptr);

	// Assert the nodes
	EXPECT_TRUE(net0->getNodeByName("n0") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n1") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n2") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("n3") != nullptr);
	EXPECT_TRUE(net0->getNodeByName("s0") != nullptr);
	EXPECT_TRUE(net0->getNumNodes() == 5);

	// Assert the links
	EXPECT_TRUE(net0->getConnectionByName("link_n0_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n1_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n1") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n2_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n2") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_n3_s0") != nullptr);
	EXPECT_TRUE(net0->getConnectionByName("link_s0_n3") != nullptr);
	EXPECT_TRUE(net0->getNumberConnections() == 8);
}

}
