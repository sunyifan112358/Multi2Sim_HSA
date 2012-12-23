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

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "clrt.h"
#include "debug.h"
#include "mhandle.h"
#include "platform.h"


const char *opencl_platform_full_profile = "OpenCL Multi2Sim Platform Full Profile";
const char *opencl_platform_version = "1.0";  /* FIXME - compose with runtime version numbers */
const char *opencl_platform_name = "Multi2Sim OpenCL Platform";
const char *opencl_platform_vendor = "Multi2Sim";
const char *opencl_platform_extensions = "";


/* Add new devices types here */
struct clrt_device_type_t *clcpu_create_device_type(void); /* for x86 CPU */
/* Put their constructors into this array */
clrt_device_type_create_t m2s_device_type_constructors[] = {clcpu_create_device_type};


static char *opencl_err_version =
	"\tYour OpenCL program is using a version of the Multi2Sim Runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tMulti2Sim OpenCL Runtime library ('libm2s-clrt').\n";


struct opencl_version_t
{
	int major;
	int minor;
};

struct opencl_platform_t *opencl_platform = NULL;
struct _cl_device_id *opencl_device = NULL;




/*
 * Private Functions
 */


cl_int populateString(const char *param, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	size_t size = strlen(param) + 1;
	return populateParameter(param, size, param_value_size, param_value, param_value_size_ret);
}

void visit_devices(device_visitor_t visitor, void *ctx)
{
	int i;
	for (i = 0; i < opencl_platform->num_device_types; i++)
	{
		int j;
		struct clrt_device_type_entry_t *entry = opencl_platform->entries + i;
		for (j = 0; j < entry->num_devices; j++)
			visitor(ctx, entry->devices[j], entry->device_type);
	}
}



/*
 * Public Functions
 */

struct opencl_platform_t *opencl_platform_create(void)
{
	struct opencl_platform_t *platform;

	/* Initialize */
	platform = xcalloc(1, sizeof(struct opencl_platform_t));

	/* Return */
	return platform;
}


void opencl_platform_free(struct opencl_platform_t *platform)
{
	free(platform);
}




/*
 * OpenCL API Functions
 */

cl_int clGetPlatformIDs(
	cl_uint num_entries,
	cl_platform_id *platforms,
	cl_uint *num_platforms)
{
	struct opencl_version_t version;
	int ret;

	/* Debug */
	opencl_debug("call '%s'", __FUNCTION__);
	opencl_debug("\tnum_entries = %d", num_entries);
	opencl_debug("\tplatforms = %p", platforms);
	opencl_debug("\tnum_platforms = %p", num_platforms);

	/* It can be assumed that this is the first OpenCL function called by
	 * the host program. It is checked here whether we're running in native
	 * or simulation mode. If it's simulation mode, Multi2Sim's version is
	 * checked for compatibility with the runtime library version. */
	ret = syscall(OPENCL_SYSCALL_CODE, opencl_call_init, &version);

	/* If the system call returns error, we are in native mode. */
	if (ret == -1)
		opencl_native_mode = 1;

	/* On simulation mode, check Multi2sim version and Multi2Sim OpenCL
	 * Runtime version compatibility. */
	if (!opencl_native_mode)
		if (version.major != OPENCL_VERSION_MAJOR
				|| version.minor < OPENCL_VERSION_MINOR)
			fatal("incompatible Multi2Sim Runtime version.\n"
				"\tRuntime library v. %d.%d / "
				"Host implementation v. %d.%d.\n%s",
				OPENCL_VERSION_MAJOR, OPENCL_VERSION_MINOR,
				version.major, version.minor, opencl_err_version);

	/* Create the platform object if it has not already been made */
	if (!opencl_platform)
	{
		int i;

		opencl_platform = xmalloc(sizeof (struct _cl_platform_id));
		opencl_platform->num_device_types = sizeof m2s_device_type_constructors / sizeof m2s_device_type_constructors[0];
		opencl_platform->entries = xcalloc(opencl_platform->num_device_types, sizeof opencl_platform->entries[0]);

		/* Go through all the device types and initialize them and their devices */
		for (i = 0; i < opencl_platform->num_device_types; i++)
		{
			int j;
			struct clrt_device_type_entry_t *entry = opencl_platform->entries + i; 

			/* construct the device type */
			entry->device_type = m2s_device_type_constructors[i]();

			/* query its device count */
			entry->device_type->init_devices(0, NULL, &entry->num_devices);
			
			/* allocate enough memory for those devices */
			entry->devices = xmalloc(sizeof entry->devices[0] * entry->num_devices);

			/* populate */
			entry->device_type->init_devices(entry->num_devices, entry->devices, NULL);

			/* set the device type */
			for (j = 0; j < entry->num_devices; j++)
				entry->devices[i]->device_type = entry->device_type;

		}
	}	

	/* If an array is passed in, it must have a corresponding length
	 * and the client must either want a count or a list of platforms */
	if ((!num_entries && platforms) || (num_entries && !platforms) || (!num_platforms && !platforms))
		return CL_INVALID_VALUE;

	/* If they just want to know how many platforms there are, tell them */
	if (!num_entries && num_platforms)
	{
		*num_platforms = 1;
		return CL_SUCCESS;
	}

	/* The client wants the platform itself. Also return the number of platforms inserted if they asked for it */
	if (num_platforms)
		*num_platforms = 1;
	*platforms = opencl_platform;
	
	/* Success */
	return CL_SUCCESS;
}


cl_int clGetPlatformInfo(
	cl_platform_id platform,
	cl_platform_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	if (platform != opencl_platform)
		return CL_INVALID_PLATFORM;

	switch (param_name)
	{
	case CL_PLATFORM_PROFILE:

		return populateString(opencl_platform_full_profile, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_VERSION:

		return populateString(opencl_platform_version, param_value_size,
			param_value, param_value_size_ret);
		
	case CL_PLATFORM_NAME:

		return populateString(opencl_platform_name, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_VENDOR:

		return populateString(opencl_platform_vendor, param_value_size,
			param_value, param_value_size_ret);

	case CL_PLATFORM_EXTENSIONS:

		return populateString(opencl_platform_extensions, param_value_size,
			param_value, param_value_size_ret);
	
	default:
		return CL_INVALID_VALUE;
	}
	return 0;
}

