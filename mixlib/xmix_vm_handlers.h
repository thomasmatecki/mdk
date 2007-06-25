/* -*-c-*- ---------------- xmix_vm_handlers.h :
 * mix_vm_cmd_dispatcher command handlers
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2006, 2007 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#ifndef XMIX_VM_HANDLERS_H
#define XMIX_VM_HANDLERS_H

#include "xmix_vm_command.h"

/* command handlers */
#define DEC_FUN(name) \
  extern gboolean cmd_##name (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)

DEC_FUN (help_);
DEC_FUN (load_);
DEC_FUN (run_);
DEC_FUN (next_);
DEC_FUN (pc_);
DEC_FUN (psym_);
DEC_FUN (preg_);
DEC_FUN (pflags_);
DEC_FUN (pall_);
DEC_FUN (pmem_);
DEC_FUN (sreg_);
DEC_FUN (scmp_);
DEC_FUN (sover_);
DEC_FUN (smem_);
DEC_FUN (ssym_);
DEC_FUN (sbp_);
DEC_FUN (sbpa_);
DEC_FUN (cbp_);
DEC_FUN (cbpa_);
DEC_FUN (cabp_);
DEC_FUN (weval_);
DEC_FUN (w2d_);
DEC_FUN (strace_);
DEC_FUN (stime_);
DEC_FUN (ptime_);
DEC_FUN (edit_);
DEC_FUN (compile_);
DEC_FUN (pedit_);
DEC_FUN (sedit_);
DEC_FUN (pasm_);
DEC_FUN (sasm_);
DEC_FUN (pddir_);
DEC_FUN (sddir_);
DEC_FUN (sbpr_);
DEC_FUN (sbpm_);
DEC_FUN (sbpc_);
DEC_FUN (sbpo_);
DEC_FUN (cbpr_);
DEC_FUN (cbpm_);
DEC_FUN (cbpc_);
DEC_FUN (cbpo_);
DEC_FUN (pbt_);
DEC_FUN (slog_);
DEC_FUN (pprog_);
DEC_FUN (psrc_);
DEC_FUN (pline_);
DEC_FUN (pstat_);

/* default command structure */
extern mix_vm_command_info_t commands_[];


#endif /* XMIX_VM_HANDLERS_H */

