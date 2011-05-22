/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel.h>
#include <gpuarch.h>
#include <debug.h>

/* Macros for quick access to pipe registers */
#define INIT_SCHEDULE  (COMPUTE_UNIT.init_schedule)
#define SCHEDULE_FETCH  (COMPUTE_UNIT.schedule_fetch)
#define FETCH_DECODE  (COMPUTE_UNIT.fetch_decode)
#define DECODE_READ  (COMPUTE_UNIT.decode_read)
#define READ_EXECUTE  (COMPUTE_UNIT.read_execute)
#define EXECUTE_WRITE  (COMPUTE_UNIT.execute_write)


/*
 * Public Functions
 */

void gpu_compute_unit_schedule(int compute_unit)
{
	/* Check if schedule stage is active */
	if (!INIT_SCHEDULE.do_schedule)
		return;
	
	/* Go to next subwavefront */
	INIT_SCHEDULE.subwavefront_id++;
	if (INIT_SCHEDULE.subwavefront_id >= gpu_compute_unit_time_slots) {
		INIT_SCHEDULE.subwavefront_id = 0;
		INIT_SCHEDULE.wavefront_id++;
	}
}


void gpu_compute_unit_fetch(int compute_unit)
{
	/* Check if fetch stage is active */
	if (!SCHEDULE_FETCH.do_fetch)
		return;

	/* By default, do not fetch next cycle */
	SCHEDULE_FETCH.do_fetch = 0;
}


void gpu_compute_unit_decode(int compute_unit)
{
	/* Check if decode stage is active */
	if (!FETCH_DECODE.do_decode)
		return;
	
	/* By default, do not decode next cycle */
	FETCH_DECODE.do_decode = 0;
}


void gpu_compute_unit_read(int compute_unit)
{
	/* Check if read stage is active */
	if (!DECODE_READ.do_read)
		return;
	
	/* By default, do not read next cycle */
	DECODE_READ.do_read = 0;
}


void gpu_compute_unit_execute(int compute_unit)
{
	/* Check if execute stage is active */
	if (!READ_EXECUTE.do_execute)
		return;
	
	/* By default, do not execute next cycle */
	READ_EXECUTE.do_execute = 0;
}


void gpu_compute_unit_write(int compute_unit)
{
	/* Check if write stage is active */
	if (!EXECUTE_WRITE.do_write)
		return;
	
	/* By default, do not write next cycle */
	EXECUTE_WRITE.do_write = 0;
}


void gpu_compute_unit_next_cycle(int compute_unit)
{
	gpu_compute_unit_write(compute_unit);
	gpu_compute_unit_execute(compute_unit);
	gpu_compute_unit_read(compute_unit);
	gpu_compute_unit_decode(compute_unit);
	gpu_compute_unit_fetch(compute_unit);
	gpu_compute_unit_schedule(compute_unit);
}


/* FIXME */
void gpu_run(struct opencl_kernel_t *kernel)
{
}

