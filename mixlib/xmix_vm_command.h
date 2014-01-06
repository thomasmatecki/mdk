/* -*-c-*- ---------------- xmix_vm_command.h :
 * Private type declarations form mix_vm_command
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2007, 2014 Free Software Foundation, Inc.
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


#ifndef XMIX_VM_COMMAND_H
#define XMIX_VM_COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "mix.h"
#include "mix_device.h"
#include "mix_vm.h"
#include "mix_vm_dump.h"
#include "mix_eval.h"
#include "mix_predicate.h"
#include "mix_vm_command.h"
#include "completion.h"


/* configuration keys */
extern const gchar *TRACING_KEY_;
extern const gchar *TIMING_KEY_;
extern const gchar *EDITOR_KEY_;
extern const gchar *ASM_KEY_;
extern const gchar *LOGGING_KEY_;

/* hooks */
typedef struct
{
  mix_vm_cmd_hook_t func;
  gpointer data;
} hook_;

typedef struct
{
  mix_vm_cmd_global_hook_t func;
  gpointer data;
} global_hook_;

enum {PRNO_ = MIX_PRED_MEM, HOOKNO_ = MIX_CMD_INVALID};

struct mix_vm_cmd_dispatcher_t
{
  mix_vm_t *vm;			/* the virtual machine */
  gboolean result;		/* last command's outcome */
  gchar *program;		/* the name of the last loaded program */
  gchar *editor;		/* edit command line template */
  gchar *assembler;		/* compile command line template */
  FILE *out;			/* message output file */
  FILE *err;			/* error output file */
  mix_dump_context_t *dump;	/* dump context for output */
  mix_eval_t *eval;		/* evaluator for w-expressions */
  gboolean trace;		/* tracing flag */
  gboolean printtime;		/* printing times flag */
  mix_time_t uptime;		/* total running time */
  mix_time_t laptime;		/* last run time */
  mix_time_t progtime;		/* current program running time */
  GHashTable *commands;		/* local commands */
  Completion *completions;	/* command completion list */
  GSList *pre_hooks[HOOKNO_];	/* Pre-command hooks */
  GSList *post_hooks[HOOKNO_];	/* Post-command hooks */
  GSList *global_pre;		/* global pre-command hook */
  GSList *global_post;		/* global post-command hook */
  mix_config_t *config;		/* externally provided configuration */
  mix_predicate_t *preds[PRNO_]; /* predicates for conditional breakpoints */
  GHashTable *mem_preds;	/* predicates for memory conditional bps */
  gboolean log_msg;		/* message logging activation flag*/
};

extern void
log_message_ (mix_vm_cmd_dispatcher_t *dis, const gchar *fmt, ...);

extern void
log_error_ (mix_vm_cmd_dispatcher_t *dis, const gchar *fmt, ...);

#define wants_logs_(dis) (dis)->log_msg

#endif /* XMIX_VM_COMMAND_H */
