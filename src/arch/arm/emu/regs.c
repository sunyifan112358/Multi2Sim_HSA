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

#include <stdlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "regs.h"


struct arm_regs_t *arm_regs_create()
{
	struct arm_regs_t *regs;

	regs = calloc(1, sizeof(struct arm_regs_t));
	if(!regs)
		fatal("%s: out of memory", __FUNCTION__);

	regs->cpsr.mode = ARM_MODE_USER;
	return regs;
}


void arm_regs_free(struct arm_regs_t *regs)
{
	free(regs);
}


void arm_regs_copy(struct arm_regs_t *dst, struct arm_regs_t *src)
{
	memcpy(dst, src, sizeof(struct arm_regs_t));
}
