/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2skernel.h>


static struct string_map_t ctx_status_map = {
	15, {
		{ "running",      ctx_running },
		{ "specmode",     ctx_specmode },
		{ "suspended",    ctx_suspended },
		{ "finished",     ctx_finished },
		{ "exclusive",    ctx_exclusive },
		{ "locked",       ctx_locked },
		{ "handler",      ctx_handler },
		{ "sigsuspend",   ctx_sigsuspend },
		{ "nanosleep",    ctx_nanosleep },
		{ "poll",         ctx_poll },
		{ "read",         ctx_read },
		{ "write",        ctx_write },
		{ "waitpid",      ctx_waitpid },
		{ "zombie",       ctx_zombie },
		{ "futex",        ctx_futex }
	}
};


static struct ctx_t *ctx_do_create()
{
	struct ctx_t *ctx;

	/* Create context and set its status */
	ctx = calloc(1, sizeof(struct ctx_t));
	ctx->pid = ke->current_pid++;

	/* Update status so that the context is inserted in the
	 * corresponding lists. The ctx_running parameter has no
	 * effect, since it will be updated later. */
	ctx_set_status(ctx, ctx_running);
	ke_list_insert(ke_list_context, ctx);
	ke->context_max = MAX(ke->context_count, ke->context_max);

	/* Structures */
	ctx->regs = regs_create();
	ctx->signal_masks = signal_masks_create();

	/* Return context */
	return ctx;
}


struct ctx_t *ctx_create()
{
	struct ctx_t *ctx;
	
	ctx = ctx_do_create();

	/* A new parent context has a new memory map and a new
	 * set of signal handlers. */
	ld_init(ctx);
	ctx->mid = ke->current_mid++;
	ctx->mem = mem_create();
	ctx->signal_handlers = signal_handlers_create();
	
	return ctx;
}


struct ctx_t *ctx_clone(struct ctx_t *ctx)
{
        struct ctx_t *new;

	new = ctx_do_create();

	/* Register file contexts are copied from parent. */
	regs_copy(new->regs, ctx->regs);

	/* The memory image of the cloned context if the same.
	 * The memory structure must be only freed by the parent
	 * when all its children have been killed.
	 * The set of signal handlers is the same, too. */
	new->mem = ctx->mem;
	new->mem->sharing++;
	new->mid = ctx->mid;
	new->loader = ctx->loader;
	new->signal_handlers = ctx->signal_handlers;
	new->glibc_segment_base = ctx->glibc_segment_base;
	new->glibc_segment_limit = ctx->glibc_segment_limit;

	/* Update other fields. */
	new->parent = ctx;

	/* Return new context id */
	return new;

}


/* Free a context */
void ctx_free(struct ctx_t *ctx)
{
	/* If context is not finished/zombie, finish it first.
	 * This removes all references to current freed context. */
	if (!ctx_get_status(ctx, ctx_finished | ctx_zombie))
		ctx_finish(ctx, 0);
	
	/* Remove context from finished contexts list. This should
	 * be the only list the context is in right now. */
	assert(!ke_list_member(ke_list_running, ctx));
	assert(!ke_list_member(ke_list_suspended, ctx));
	assert(!ke_list_member(ke_list_zombie, ctx));
	assert(ke_list_member(ke_list_finished, ctx));
	ke_list_remove(ke_list_finished, ctx);
		
	/* Free private structures */
	regs_free(ctx->regs);
	signal_masks_free(ctx->signal_masks);

	/* Shared structures are only freed if this
	 * is the last context sharing them. */
	ctx->mem->sharing--;
	if (!ctx->mem->sharing) {
		ld_done(ctx);
		mem_free(ctx->mem);
		signal_handlers_free(ctx->signal_handlers);
	}

	/* Remove context from contexts list and free */
	ke_list_remove(ke_list_context, ctx);
	if (isa_ctx == ctx)
		isa_ctx = NULL;
	free(ctx);
}


void ctx_dump(struct ctx_t *ctx, FILE *f)
{
	char sstatus[200];
	fprintf(f, "  pid=%d\n", ctx->pid);
	map_flags(&ctx_status_map, ctx->status, sstatus, 200);
	fprintf(f, "  status=%s\n", sstatus);
	if (!ctx->parent)
		fprintf(f, "  parent=(null)\n");
	else
		fprintf(f, "  parent=%d\n", ctx->parent->pid);
	fprintf(f, "  heap break: 0x%x\n", ctx->loader->brk);

	/* Signal masks */
	fprintf(f, "  blocked signal mask: 0x%llx ",
		(long long) ctx->signal_masks->blocked);
	sim_sigset_dump(ctx->signal_masks->blocked, f);
	fprintf(f, "\n  pending signals: 0x%llx ",
		(long long) ctx->signal_masks->pending);
	sim_sigset_dump(ctx->signal_masks->pending, f);
	fprintf(f, "\n");
}


void ctx_execute_inst(struct ctx_t *ctx)
{
	unsigned char fixed[20];
	void *buf;

	/* The isa_xxx functions work on these global
	 * variables. */
	isa_ctx = ctx;
	isa_regs = ctx->regs;
	isa_mem = ctx->mem;
	isa_eip = isa_regs->eip;
	isa_inst_count++;

	/* Read instruction from memory */
	ctx->mem->safe = mem_safe_mode;
	if (ctx_get_status(ctx, ctx_specmode))
		ctx->mem->safe = 0;
	buf = mem_get_buffer(ctx->mem, ctx->regs->eip, 20, mem_access_exec);
	if (!buf) {
		buf = &fixed;
		mem_access(ctx->mem, ctx->regs->eip, 20, buf, mem_access_exec);
	}
	ctx->mem->safe = mem_safe_mode;

	/* Disassemble */
	x86_disasm(buf, isa_eip, &isa_inst);

	/* Call the isa module to execute one machine instruction,
	 * only if we are not in speculative mode. */
	if (!ctx_get_status(ctx, ctx_specmode))
		isa_execute_inst(buf);
}


void ctx_set_eip(struct ctx_t *ctx, uint32_t eip)
{
	/* Entering specmode */
	if (ctx->regs->eip != eip && !ctx_get_status(ctx, ctx_specmode)) {
		ctx_set_status(ctx, ctx_specmode);
		ctx->backup_eip = ctx->regs->eip;
	}
	
	/* Set it */
	ctx->regs->eip = eip;
}


void ctx_recover(struct ctx_t *ctx)
{
	assert(ctx_get_status(ctx, ctx_specmode));
	ctx_clear_status(ctx, ctx_specmode);
	ctx->regs->eip = ctx->backup_eip;
}


int ctx_get_status(struct ctx_t *ctx, enum ctx_status_enum status)
{
	return (ctx->status & status) > 0;
}


static void ctx_update_status(struct ctx_t *ctx, enum ctx_status_enum status)
{
	/* Remove contexts from the following lists:
	 *   running, suspended, zombie */
	if (ke_list_member(ke_list_running, ctx))
		ke_list_remove(ke_list_running, ctx);
	if (ke_list_member(ke_list_suspended, ctx))
		ke_list_remove(ke_list_suspended, ctx);
	if (ke_list_member(ke_list_zombie, ctx))
		ke_list_remove(ke_list_zombie, ctx);
	if (ke_list_member(ke_list_finished, ctx))
		ke_list_remove(ke_list_finished, ctx);
	
	/* Update status */
	ctx->status = status;
	if (ctx->status & ctx_finished)
		ctx->status = ctx_finished;
	if (ctx->status & ctx_zombie)
		ctx->status = ctx_zombie;
	if (!(ctx->status & ctx_suspended) &&
		!(ctx->status & ctx_finished) &&
		!(ctx->status & ctx_zombie) &&
		!(ctx->status & ctx_locked))
		ctx->status |= ctx_running;
	else
		ctx->status &= ~ctx_running;
	
	/* Insert context into the corresponding lists. */
	if (ctx->status & ctx_running)
		ke_list_insert(ke_list_running, ctx);
	if (ctx->status & ctx_zombie)
		ke_list_insert(ke_list_zombie, ctx);
	if (ctx->status & ctx_finished)
		ke_list_insert(ke_list_finished, ctx);
	if (ctx->status & ctx_suspended)
		ke_list_insert(ke_list_suspended, ctx);
	
	/* Dump new status */
	if (debug_status(syscall_debug_category)) {
		char sstatus[200];
		map_flags(&ctx_status_map, ctx->status, sstatus, 200);
		syscall_debug("context %d changed status to %s\n",
			ctx->pid, sstatus);
	}
}


void ctx_set_status(struct ctx_t *ctx, enum ctx_status_enum status)
{
	ctx_update_status(ctx, ctx->status | status);
}


void ctx_clear_status(struct ctx_t *ctx, enum ctx_status_enum status)
{
	ctx_update_status(ctx, ctx->status & ~status);
}


/* Look for a context matching pid in the list of existing
 * contexts of the kernel. */
struct ctx_t *ctx_get(int pid)
{
	struct ctx_t *ctx;

	ctx = ke->context_list;
	while (ctx && ctx->pid != pid)
		ctx = ctx->context_next;
	return ctx;
}


/* Look for zombie child. If 'pid' is -1, the first finished child
 * in the zombie contexts list is return. Otherwise, 'pid' is the
 * pid of the child process. If no child has finished, return NULL. */
struct ctx_t *ctx_get_zombie(struct ctx_t *parent, int pid)
{
	struct ctx_t *ctx;

	for (ctx = ke->zombie_list; ctx; ctx = ctx->zombie_next) {
		if (ctx->parent != parent)
			continue;
		if (ctx->pid == pid || pid == -1)
			return ctx;
	}
	return NULL;
}


/* Finish a context. If the context has no parent, its status will be set
 * to 'ctx_finished'. If it has, its status is set to 'ctx_zombie', waiting for
 * a call to 'waitpid'.
 * The children of the finished context will set their 'parent' attribute to NULL.
 * The zombie children will be finished. */
void ctx_finish(struct ctx_t *ctx, int status)
{
	struct ctx_t *aux;
	
	/* Context already finished */
	if (ctx_get_status(ctx, ctx_finished | ctx_zombie))
		return;

	/* From now on, all children have lost their parent. If a children is
	 * already zombie, finish it, since its parent won't be able to waitpid it
	 * anymore. */
	for (aux = ke->context_list; aux; aux = aux->context_next) {
		if (aux->parent == ctx) {
			aux->parent = NULL;
			if (ctx_get_status(aux, ctx_zombie))
				ctx_set_status(aux, ctx_finished);
		}
	}

	/* Send finish signal to parent */
	if (ctx->exit_signal && ctx->parent) {
		syscall_debug("  sending signal %d to pid %d\n",
			ctx->exit_signal, ctx->parent->pid);
		sim_sigset_add(&ctx->parent->signal_masks->pending,
			ctx->exit_signal);
		signal_process(ctx->parent);
	}

	/* If clear_child_tid was set, a futex() call must be performed on
	 * that pointer. Also wake up futexes in the robust list. */
	if (ctx->clear_child_tid) {
		uint32_t zero = 0;
		mem_write(ctx->mem, ctx->clear_child_tid, 4, &zero);
		ctx_futex_wake(ctx, ctx->clear_child_tid, 1);
	}
	ctx_exit_robust_list(ctx);

	/* If we are in a signal handler, stop it. */
	if (ctx_get_status(ctx, ctx_handler))
		signal_handler_return(ctx);

	/* Finish context */
	ctx_set_status(ctx, ctx->parent ? ctx_zombie : ctx_finished);
	ctx->exit_code = status;
	ke_event_finish();
}


int ctx_futex_wake(struct ctx_t *ctx, uint32_t futex, uint32_t count)
{
	int wakeup_count = 0;
	struct ctx_t *wakeup_ctx;

	/* Look for threads suspended in this futex */
	while (count) {
		wakeup_ctx = NULL;
		for (ctx = ke->suspended_list; ctx; ctx = ctx->suspended_next) {
			if (!ctx_get_status(ctx, ctx_futex) || ctx->wakeup_futex != futex)
				continue;
			if (!wakeup_ctx || ctx->wakeup_futex_sleep < wakeup_ctx->wakeup_futex_sleep)
				wakeup_ctx = ctx;
		}
		if (wakeup_ctx) {
			ctx_clear_status(wakeup_ctx, ctx_suspended | ctx_futex);
			syscall_debug("  futex 0x%x: thread %d woken up\n", futex, wakeup_ctx->pid);
			wakeup_count++;
			count--;
		} else {
			break;
		}
	}
	return wakeup_count;
}


void ctx_exit_robust_list(struct ctx_t *ctx)
{
	uint32_t next, lock_entry, offset, lock_word;

	/* Read the offset from the list head. This is how the structure is
	 * represented in the kernel:
	 * struct robust_list {
	 *      struct robust_list __user *next;
	 * }
	 * struct robust_list_head {
	 *	struct robust_list list;
	 *	long futex_offset;
	 *	struct robust_list __user *list_op_pending;
	 * }
	 * See linux/Documentation/robust-futex-ABI.txt for details
	 * about robust futex wake up at thread exit.
	 */

	lock_entry = ctx->robust_list_head;
	if (!lock_entry)
		return;
	
	syscall_debug("ctx %d: processing robust futex list\n",
		ctx->pid);
	for (;;) {
		mem_read(ctx->mem, lock_entry, 4, &next);
		mem_read(ctx->mem, lock_entry + 4, 4, &offset);
		mem_read(ctx->mem, lock_entry + offset, 4, &lock_word);

		syscall_debug("  lock_entry=0x%x: offset=%d, lock_word=0x%x\n",
			lock_entry, offset, lock_word);

		/* Stop processing list if 'next' points to robust list */
		if (!next || next == ctx->robust_list_head)
			break;
		lock_entry = next;
	}

/*	robust_list_elem = ctx->robust_list_head;
	for (;;) {
		mem_read(ctx->mem, robust_list_elem, 4, &next);
		printf("robust_list_elem at 0x%x\n", robust_list_elem);
		if (next == robust_list_elem)
			break;
	}*/
	//exit(1);
}

