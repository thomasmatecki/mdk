/* -*-c-*- ---------------- mix_vm_command.h :
 * declarations for mix_vm_command_t, describing commands issued to a vm
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2006, 2007, 2014 Free Software Foundation, Inc.
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


#ifndef MIX_VM_COMMAND_H
#define MIX_VM_COMMAND_H

#include <stdio.h>
#include "mix.h"
#include "mix_vm.h"
#include "mix_config.h"

/* mix_vm_cmd_dispatcher encapsulates a virtual machine and helper
   objects, providing a command driven interface with output to
   provided files (in the posix sense).
*/
typedef struct mix_vm_cmd_dispatcher_t mix_vm_cmd_dispatcher_t;

/* mix_vm_cmd_dispatcher understands the commands of this type */
typedef enum {
  MIX_CMD_HELP = 0,		/* echo help message */
  MIX_CMD_LOAD,			/* load a mix program */
  MIX_CMD_EDIT,			/* edit mixal source */
  MIX_CMD_PEDIT,		/* print editor command */
  MIX_CMD_SEDIT,		/* set editor command*/
  MIX_CMD_COMPILE,		/* compile mixal source */
  MIX_CMD_PASM,			/* print compiler command */
  MIX_CMD_SASM,			/* set assembler command */
  MIX_CMD_RUN,			/* run a loaded program */
  MIX_CMD_NEXT,			/* run next instruction */
  MIX_CMD_PSTAT,		/* print current vm status */
  MIX_CMD_LOC,			/* print location pointer */
  MIX_CMD_PSYM,			/* print symbol */
  MIX_CMD_PREG,			/* print registry */
  MIX_CMD_PFLAGS,		/* print comp and overf flags */
  MIX_CMD_PALL,			/* print all registers and flags */
  MIX_CMD_PMEM,			/* print memory cells */
  MIX_CMD_SREG,			/* set register value */
  MIX_CMD_SCMP,			/* set comparison flag value */
  MIX_CMD_SOVER,		/* set overflow toggle value */
  MIX_CMD_SMEM,			/* set memory cell value */
  MIX_CMD_SSYM,			/* set symbol value */
  MIX_CMD_SBP,			/* set breakpoint at lineno */
  MIX_CMD_CBP,			/* clear breakpoint at lineno */
  MIX_CMD_SBPA,			/* set breakpoint at address */
  MIX_CMD_CBPA,			/* clear breakpoint at address */
  MIX_CMD_SBPR,			/* set breakpoint on register changed */
  MIX_CMD_CBPR,			/* clear breakpoint on register changed */
  MIX_CMD_SBPM,			/* set breakpoint on mem cell changed */
  MIX_CMD_CBPM,			/* clear breakpoint on mem cell changed */
  MIX_CMD_SBPC,			/* set breakpoint on comp flag changed */
  MIX_CMD_CBPC,			/* clear breakpoint on comp flag changed */
  MIX_CMD_SBPO,			/* set breakpoint on overflow toggled */
  MIX_CMD_CBPO,			/* clear breakpoint on overflow toggled */
  MIX_CMD_CABP,			/* clear all breakpoints */
  MIX_CMD_WEVAL,		/* evaluate a w-expression */
  MIX_CMD_W2D,			/* print word in decimal notation */
  MIX_CMD_STRACE,		/* enable/disable instruction traces */
  MIX_CMD_PBT,			/* print backtrace */
  MIX_CMD_STIME,		/* enable/disable timing statistics */
  MIX_CMD_PTIME,		/* print current time statistics */
  MIX_CMD_SDDIR,		/* set device directory */
  MIX_CMD_PDDIR,		/* print current device directory */
  MIX_CMD_SLOG,			/* set on/off message logging */
  MIX_CMD_PPROG,		/* print the current program path */
  MIX_CMD_PSRC,			/* print the current program source path */
  MIx_CMD_PLINE,		/* print the current line no */
  MIX_CMD_INVALID,		/* invalid command identifier */
  MIX_CMD_LOCAL			/* locally defined command */
} mix_vm_command_t;

/* new commands definition */
typedef gboolean (*mix_vm_cmd_function_t) (mix_vm_cmd_dispatcher_t *,
					   const gchar *);
typedef struct {
  const gchar *name;		/* User printable name of the function. */
  mix_vm_cmd_function_t  func;	/* Function to call to do the job. */
  const char *doc;		/* Documentation for this function.  */
  const char *usage;		/* Usage */
} mix_vm_command_info_t;

/* hook functions, to be invoked before and/or after command execution */
typedef void (*mix_vm_cmd_hook_t)(mix_vm_cmd_dispatcher_t *dis,
				  const gchar *arg, gpointer data);

/* global hook functions */
typedef void (*mix_vm_cmd_global_hook_t)(mix_vm_cmd_dispatcher_t *dis,
					 mix_vm_command_t cmd,
					 const gchar *arg, gpointer data);

/* conversion from/to commands to strings */
extern const gchar *
mix_vm_command_to_string (mix_vm_command_t cmd);

extern mix_vm_command_t
mix_vm_command_from_string (const gchar *name);

/* get help string about a command */
extern const gchar *
mix_vm_command_help (mix_vm_command_t cmd);

extern const gchar *
mix_vm_command_usage (mix_vm_command_t cmd);

/* create a new command dispatcher */
extern mix_vm_cmd_dispatcher_t *
mix_vm_cmd_dispatcher_new (FILE *out, /* output messages file */
			   FILE *err /* error messages file */);

extern mix_vm_cmd_dispatcher_t *
mix_vm_cmd_dispatcher_new_with_config (FILE *out, FILE *err,
				       mix_config_t *config);

/* delete (does not close the fds in the constructor) */
extern void
mix_vm_cmd_dispatcher_delete (mix_vm_cmd_dispatcher_t *dis);

/* register new commands for a dispatcher */
extern void
mix_vm_cmd_dispatcher_register_new (mix_vm_cmd_dispatcher_t *dis,
				    mix_vm_command_info_t *cmd);

/* get command completion list */
const GList *
mix_vm_cmd_dispatcher_complete (const mix_vm_cmd_dispatcher_t *dis,
				const gchar *cmd, gchar **prefix);

/* set/get out/error streams */
extern FILE *
mix_vm_cmd_dispatcher_get_out_stream (const mix_vm_cmd_dispatcher_t *dis);

extern FILE *
mix_vm_cmd_dispatcher_get_err_stream (const mix_vm_cmd_dispatcher_t *dis);

extern FILE * /* old output stream */
mix_vm_cmd_dispatcher_set_out_stream (mix_vm_cmd_dispatcher_t *dis,
				      FILE *out);

extern FILE * /* old error stream */
mix_vm_cmd_dispatcher_set_error_stream (mix_vm_cmd_dispatcher_t *dis,
					FILE *err);

/* set editor and compiler templates */
extern void
mix_vm_cmd_dispatcher_set_editor (mix_vm_cmd_dispatcher_t *dis,
				  const gchar *edit_tplt);

extern void
mix_vm_cmd_dispatcher_set_assembler (mix_vm_cmd_dispatcher_t *dis,
				     const gchar *asm_tplt);

extern const gchar *
mix_vm_cmd_dispatcher_get_src_file_path (const mix_vm_cmd_dispatcher_t *dis);

extern const gchar *
mix_vm_cmd_dispatcher_get_program_path (const mix_vm_cmd_dispatcher_t *dis);

extern const gchar *
mix_vm_cmd_dispatcher_get_editor (const mix_vm_cmd_dispatcher_t *dis);

extern const gchar *
mix_vm_cmd_dispatcher_get_assembler (const mix_vm_cmd_dispatcher_t *dis);

/* dispatch a command */
extern gboolean /* TRUE if success, FALSE otherwise */
mix_vm_cmd_dispatcher_dispatch (mix_vm_cmd_dispatcher_t *dis,
				mix_vm_command_t cmd, const gchar *arg);

/* dispatch a command in text format */
extern gboolean
mix_vm_cmd_dispatcher_dispatch_text (mix_vm_cmd_dispatcher_t *dis,
				     const gchar *text);

/* dispatch a command in text format, with command and arg split */
extern gboolean
mix_vm_cmd_dispatcher_dispatch_split_text (mix_vm_cmd_dispatcher_t *dis,
					   const gchar *command,
					   const gchar *arg);

/* get the last dispatch's result */
extern gboolean
mix_vm_cmd_dispatcher_get_last_result (const mix_vm_cmd_dispatcher_t *dis);

/* get total uptime */
extern mix_time_t
mix_vm_cmd_dispatcher_get_uptime (const mix_vm_cmd_dispatcher_t *dis);

/* get program total time */
extern mix_time_t
mix_vm_cmd_dispatcher_get_progtime (const mix_vm_cmd_dispatcher_t *dis);

/* get time lapse */
extern mix_time_t
mix_vm_cmd_dispatcher_get_laptime (const mix_vm_cmd_dispatcher_t *dis);

/* src file info */
extern gulong
mix_vm_cmd_dispatcher_get_src_file_lineno (const mix_vm_cmd_dispatcher_t *dis);

extern const gchar *
mix_vm_cmd_dispatcher_get_src_file_line (const mix_vm_cmd_dispatcher_t *dis,
					 gulong line, gboolean cr);

/* toggle time printing */
extern void
mix_vm_cmd_dispatcher_print_time (mix_vm_cmd_dispatcher_t * dis,
				  gboolean print);

/* install hooks */
extern void
mix_vm_cmd_dispatcher_pre_hook (mix_vm_cmd_dispatcher_t *dis,
				mix_vm_command_t cmd,
				mix_vm_cmd_hook_t hook, gpointer data);

extern void
mix_vm_cmd_dispatcher_post_hook (mix_vm_cmd_dispatcher_t *dis,
				 mix_vm_command_t cmd,
				 mix_vm_cmd_hook_t hook, gpointer data);

extern void
mix_vm_cmd_dispatcher_global_pre_hook (mix_vm_cmd_dispatcher_t *dis,
				       mix_vm_cmd_global_hook_t hook,
				       gpointer data);

extern void
mix_vm_cmd_dispatcher_global_post_hook (mix_vm_cmd_dispatcher_t *dis,
					mix_vm_cmd_global_hook_t hook,
					gpointer data);

/* get the mix vm */
extern const mix_vm_t *
mix_vm_cmd_dispatcher_get_vm (const mix_vm_cmd_dispatcher_t *dis);


#endif /* MIX_VM_COMMAND_H */
