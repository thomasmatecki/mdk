/* -*-c-*- -------------- xmix_vm_handlers.c :
 * Implementation of the functions declared in xmix_vm_handlers.h
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2010, 2014 Free Software Foundation, Inc.
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


#include "xmix_vm_handlers.h"

/* available commands (in the same order as the type enum) */
mix_vm_command_info_t commands_[] = {
  { "help", cmd_help_, N_("Display this text"), "help [COMMAND]"},
  { "load", cmd_load_, N_("Load a MIX code file"), "load FILENAME"},
  { "edit", cmd_edit_, N_("Edit a MIXAL source file"), "edit [FILENAME]"},
  { "pedit", cmd_pedit_, N_("Print the external editor command"), "pedit"},
  { "sedit", cmd_sedit_, N_("Set the external editor command"),
    "sedit COMMAND (e.g. emacs %s)"},
  { "compile", cmd_compile_, N_("Compile a MIXAL source file"),
    "compile [FILENAME]"},
  { "pasm", cmd_pasm_, N_("Print the compile command"), "pasm"},
  { "sasm", cmd_sasm_, N_("Set the compile command"),
    "sasm COMMAND (e.g. mixasm -g -l %s)"},
  { "run", cmd_run_, N_("Run loaded or given MIX code file"),
    "run [FILENAME]"},
  { "next", cmd_next_, N_("Execute next instruction(s)"),
    "next [NO_OF_INS]"},
  { "pstat", cmd_pstat_, N_("Print current vm status"), "pstat"},
  { "pc", cmd_pc_, N_("Print program counter value"), "pc" },
  { "psym", cmd_psym_, N_("Print symbol value"), "psym [SYMBOLNAME]"},
  { "preg", cmd_preg_, N_("Print register value"),
    "preg [A | X | J | I[1-6]]"},
  { "pflags", cmd_pflags_, N_("Print comparison and overflow flags"),
    "pflags"},
  { "pall", cmd_pall_, N_("Print all registers and flags"), "pall"},
  { "pmem", cmd_pmem_, N_("Print memory contents in address range"),
    "pmem FROM[-TO]"},
  { "sreg", cmd_sreg_, N_("Set register value"),
    "sreg A | X | J | I[1-6] VALUE"},
  { "scmp", cmd_scmp_, N_("Set comparison flag value"), "scmp L | E | G"},
  { "sover", cmd_sover_, N_("Set overflow flag value"), "sover T | F" },
  { "smem", cmd_smem_, N_("Set memory contents in given address"),
    "smem ADDRESS VALUE"},
  { "ssym", cmd_ssym_, N_("Set a symbol\'s value"), "ssym SYMBOL WEXPR"},
  { "sbp", cmd_sbp_, N_("Set break point at given line"), "sbp LINENO"},
  { "cbp", cmd_cbp_, N_("Clear break point at given line"), "cbp LINENO"},
  { "sbpa", cmd_sbpa_, N_("Set break point at given address"),
    "sbpa ADDRESS"},
  { "cbpa", cmd_cbpa_, N_("Clear break point at given address"),
    "cbpa ADDRESS"},
  { "sbpr", cmd_sbpr_, N_("Set conditional breakpoint on register change"),
    "sbpr A | X | J | I[1-6]"},
  { "cbpr", cmd_cbpr_, N_("Clear conditional breakpoint on register change"),
    "sbpr A | X | J | I[1-6]"},
  { "sbpm", cmd_sbpm_, N_("Set conditional breakpoint on mem cell change"),
    "sbpm ADDRESS"},
  { "cbpm", cmd_cbpm_, N_("Clear conditional breakpoint on mem cell change"),
    "cbpm ADDRESS"},
  { "sbpc", cmd_sbpc_,
    N_("Set conditional breakpoint on comparison flag change"), "sbpc"},
  { "cbpc", cmd_cbpc_,
    N_("Clear conditional breakpoint on comparison flag change"), "cbpc"},
  { "sbpo", cmd_sbpo_,
    N_("Set conditional breakpoint on overflow toggled"), "sbpo"},
  { "cbpo", cmd_cbpo_,
    N_("Clear conditional breakpoint on overflow toggled"), "cbpo"},
  { "cabp", cmd_cabp_, N_("Clear all breakpoints"), "cabp"},
  { "weval", cmd_weval_, N_("Evaluate a given W-expression"), "weval WEXPR"},
  { "w2d", cmd_w2d_, N_("Convert a MIX word to its decimal value"),
    "w2d WORD"},
  { "strace", cmd_strace_, N_("Turn on/off instruction tracing"),
    "strace on|off"},
  { "pbt", cmd_pbt_, N_("Print backtrace of executed instructions"),
    "pbt [INS_NO] (e.g pbt 5)"},
  { "stime", cmd_stime_, N_("Turn on/off timing statistics"),
    "stime on|off"},
  { "ptime", cmd_ptime_, N_("Print current time statistics"), "ptime"},
  { "sddir", cmd_sddir_, N_("Set devices directory"),"sddir NEWDIR"},
  { "pddir", cmd_pddir_, N_("Print current devices directory"),"pddir"},
  { "slog", cmd_slog_, N_("Turn on/off message logging"), "slog on|off"},
  { "pprog", cmd_pprog_, N_("Print the current program path"), "pprog"},
  { "psrc", cmd_psrc_, N_("Print the current program source path"), "psrc"},
  { "pline", cmd_pline_,
    N_("Print the current (or a given) program source line"), "pline [LINENO]"},
  { NULL, NULL, NULL, NULL},
};

/* trace current instruction */
static void
do_trace_ (mix_vm_cmd_dispatcher_t *dis, gboolean error)
{
  enum {BUFFER_LEN = 128};
  static gchar STRINS[BUFFER_LEN];

  const mix_src_file_t *file = mix_vm_get_src_file (dis->vm);
  const gchar *line = "\n";
  mix_address_t loc = mix_vm_get_prog_count (dis->vm);
  mix_word_t ins = mix_vm_get_addr_contents (dis->vm, loc);
  mix_ins_t fins;
  mix_word_to_ins_uncheck (ins, fins);
  mix_ins_to_string_in_buffer (&fins, STRINS, BUFFER_LEN);

  if (file != NULL)
    {
      gulong b = mix_vm_get_break_lineno (dis->vm);
      if (b > 0) line = mix_src_file_get_line (file, b);
    }

  if (!error)
    log_message_ (dis, "%d: [%-15s]\t%s", (gint)loc, STRINS, line);
  else
    log_error_ (dis, "%d: [%-15s]\t%s", (gint)loc, STRINS, line);
}

static void
trace_ (mix_vm_cmd_dispatcher_t *dis)
{
  if (wants_logs_ (dis))
    {
      do_trace_ (dis, FALSE);
    }
}

#define trace_error_(dis)  do_trace_ (dis, TRUE)

/* run a program tracing executed instructions */
static int
run_and_trace_ (mix_vm_cmd_dispatcher_t *dis)
{
  int k = MIX_VM_RUNNING;
  if (!dis->trace)
    return mix_vm_run (dis->vm);
  else while (k == MIX_VM_RUNNING)
    {
      trace_ (dis);
      k = mix_vm_exec_next (dis->vm);
    }
  return k;
}

/* print time statistics */
static void
print_time_ (mix_vm_cmd_dispatcher_t *dis)
{
  dis->laptime = mix_vm_get_uptime (dis->vm) - dis->uptime;
  dis->uptime += dis->laptime;
  dis->progtime += dis->laptime;
  if (dis->printtime)
    fprintf
      (dis->out,
       _("Elapsed time: %ld /Total program time: %ld (Total uptime: %ld)\n"),
       dis->laptime, dis->progtime, dis->uptime);
}


/* commands */
static void
print_help_ (gpointer key, gpointer val, gpointer data)
{
  mix_vm_command_info_t *info = (mix_vm_command_info_t *)val;
  if (info)
    fprintf ((FILE *)data, "%-15s%s\n", info->name, info->doc);
}

gboolean
cmd_help_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg && strlen (arg) > 0)
    {
      int i;
      mix_vm_command_info_t *info =
	(mix_vm_command_info_t *)g_hash_table_lookup
	(dis->commands, (gpointer)arg);
      for (i = 0; info == NULL && commands_[i].name; i++)
	if (!strcmp (commands_[i].name, arg)) info = commands_ + i;
      if (info)
	{
	  fprintf (dis->out , _("%-15s%s.\n%-15sUsage: %s\n"),
		   info->name, info->doc, "", info->usage);
	  return TRUE;
	}
      fprintf (dis->out, _("No commands match `%s'\n"), arg);
      return FALSE;
    }
  else
    {
      int i = 0;
      fprintf (dis->out, _("Possible commands are:\n"));
      for (i = 0; commands_[i].name; ++i)
	fprintf (dis->out, "%-15s%s\n", commands_[i].name, commands_[i].doc);
      g_hash_table_foreach (dis->commands, print_help_, (gpointer)dis->out);
      return TRUE;
    }
}

gboolean
cmd_load_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  errno = 0;
  if (arg == NULL || *arg == '\0')
    {
      log_error_ (dis, _("Missing file name"));
      return FALSE;
    }
  mix_eval_remove_symbols_from_table (dis->eval,
				      mix_vm_get_symbol_table (dis->vm));
  if (!mix_vm_load_file (dis->vm, arg) )
    {
      log_error_ (dis, _("Cannot load %s: "), arg);
      if ( errno == 0 )
	log_error_ (dis, _("Wrong file format"));
      else
	log_error_ (dis, "%s", strerror (errno));
      return FALSE;
    }

  if (dis->program != arg)
    {
      if (dis->program) g_free (dis->program);
      dis->program = mix_file_complete_name (arg, MIX_CODE_DEFEXT);
    }

  mix_eval_set_symbols_from_table (dis->eval,
				   mix_vm_get_symbol_table (dis->vm));
  if (wants_logs_ (dis))
    log_message_ (dis, _("Program loaded. Start address: %d"),
		  mix_vm_get_prog_count (dis->vm));

  dis->laptime = dis->progtime = 0;
  return TRUE;
}

gboolean
cmd_edit_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (dis->editor == NULL)
    {
      log_error_ (dis, _("Editor not specified (use sedit)"));
      return FALSE;
    }
  if (!arg || *arg == '\0') arg = mix_vm_cmd_dispatcher_get_src_file_path (dis);
  if (!arg)
    {
      log_error_ (dis, _("MIXAL source file path not found"));
      return FALSE;
    }
  else
    {
      gchar *cmd = g_strdup_printf (dis->editor, arg);
      if (wants_logs_ (dis)) log_message_ (dis, cmd);
      system (cmd);
      if (wants_logs_ (dis)) log_message_ (dis, _(" ...done"));
      g_free (cmd);
      return TRUE;
    }
}

gboolean
cmd_compile_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (dis->assembler == NULL)
    {
      log_error_ (dis, _("MIX assembler not specified (use sasm)"));
      return FALSE;
    }
  if (!arg || *arg == '\0') arg = mix_vm_cmd_dispatcher_get_src_file_path (dis);
  if (!arg)
    {
      log_error_ (dis, _("MIXAL source file path not found"));
      return FALSE;
    }
  else
    {
      gchar *cmd = g_strdup_printf (dis->assembler, arg);
      gchar *errors = NULL;
      gchar *output = NULL;
      gint exit_status;
      gboolean result;
      GError *gerr = NULL;

      if (wants_logs_ (dis)) log_message_ (dis, cmd);

      result =
        g_spawn_command_line_sync (cmd, &output, &errors, &exit_status, &gerr);

      if (output)
        {
          log_message_ (dis, output);
        }

      if (errors != NULL)
        {
          log_message_ (dis, errors);
        }
      else if ((exit_status != 0) || !result)
        {
          log_error_ (dis, _("Compilation failed"));
          if (gerr && gerr->message) log_error_ (dis, gerr->message);
        }

      if (gerr) g_free (gerr);
      if (output) g_free (output);
      if (errors) g_free (errors);

      g_free (cmd);

      return TRUE;
    }
}

gboolean
cmd_run_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg != NULL && *arg != '\0' && cmd_load_ (dis, arg) != TRUE)
    return FALSE;

  if (mix_vm_is_halted (dis->vm)) cmd_load_ (dis, dis->program);

  if (wants_logs_ (dis)) log_message_ (dis, _("Running ..."));

  switch (run_and_trace_ (dis))
    {
    case MIX_VM_HALT:
      if (wants_logs_ (dis)) log_message_ (dis, _("... done"));
      break;
    case MIX_VM_BREAK:
      if (wants_logs_ (dis))
	{
	  gulong line = mix_vm_get_break_lineno (dis->vm);
	  if (line != 0)
	    log_message_
	      (dis, _("... stopped: breakpoint at line %ld (address %d)"),
	       line, mix_vm_get_prog_count (dis->vm));
	  else
	    log_message_ (dis, _("... stopped: breakpoint at address %d"),
			  mix_vm_get_prog_count (dis->vm));
      }
      break;
    case MIX_VM_COND_BREAK:
      if (wants_logs_ (dis))
	{
	  gulong line = mix_vm_get_break_lineno (dis->vm);
	  if (line != 0)
	    log_message_ (dis, _("... stopped: %s (line %ld, address %d)"),
			  mix_vm_get_last_breakpoint_message (dis->vm),
			  line, mix_vm_get_prog_count (dis->vm));
	  else
	    log_message_ (dis, _("... stopped: %s (address %d)"),
			  mix_vm_get_last_breakpoint_message (dis->vm),
			  mix_vm_get_prog_count (dis->vm));
	}
      break;
    case MIX_VM_ERROR:
      log_error_ (dis, _("%s:"), mix_vm_get_last_error_string (dis->vm));
      trace_error_ (dis);
      break;
    default:
      g_assert_not_reached ();
      break;
    }

  if (wants_logs_ (dis)) print_time_ (dis);

  return TRUE;
}

gboolean
cmd_next_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  int ins_no = 1;
  int k;

  if ( strlen (arg) != 0 )
    {
      int k = 0;
      while (isdigit (arg[k]))
	k++;
      if (arg[k] != '\0')
	{
	  log_error_ (dis, _("Invalid argument: %s"), arg);
	  return FALSE;
	}
      ins_no = atoi (arg);
    }

  if (mix_vm_is_halted (dis->vm)) cmd_load_ (dis, dis->program);

  while ( ins_no-- > 0 )
    {
      if (dis->trace) trace_ (dis);
      k = mix_vm_exec_next (dis->vm);
      if (k == MIX_VM_HALT)
	{
	  if (wants_logs_ (dis))
	    log_message_(dis, _("End of program reached at address %d"),
			 mix_vm_get_prog_count (dis->vm));
	  break;
	}
      else if (k == MIX_VM_ERROR)
	{
	  log_error_ (dis, _("%s:"), mix_vm_get_last_error_string (dis->vm));
          trace_error_ (dis);
	  break;
	}
    }
  if (wants_logs_ (dis)) print_time_ (dis);

  return TRUE;
}

gboolean
cmd_pc_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  fprintf (dis->out, "Current address: %d\n", mix_vm_get_prog_count (dis->vm));
  return TRUE;
}

gboolean
cmd_psym_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gboolean result = FALSE;
  const mix_symbol_table_t *table = mix_eval_symbol_table (dis->eval);
  if ( table == NULL )
    log_error_ (dis, _("Symbol table not available"));
  else if (arg != NULL && *arg != '\0')
    {
      if ( mix_symbol_table_is_defined (table, arg) )
	{
	  mix_word_print_to_file (mix_symbol_table_value (table, arg),
				  NULL, dis->out);
	  putc ('\n', dis->out);
	  result = TRUE;
	}
      else
	fprintf (dis->out, _("%s: symbol not defined\n"), arg);
    }
  else
    {
      mix_symbol_table_print (table, MIX_SYM_ROWS, dis->out, TRUE);
      result = TRUE;
    }

  return result;
}

gboolean
cmd_preg_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  mix_dump_context_set_opt (dis->dump, MIX_DUMP_NONE);
  if ( strlen (arg) == 0 )
    mix_dump_context_add_opt (dis->dump, MIX_DUMP_rALL);
  else switch (*arg)
    {
    case 'A':
      mix_dump_context_add_opt (dis->dump, MIX_DUMP_rA);
      break;
    case 'X':
      mix_dump_context_add_opt (dis->dump, MIX_DUMP_rX);
      break;
    case 'J':
      mix_dump_context_add_opt (dis->dump, MIX_DUMP_rJ);
      break;
    case 'I':
      {
	if ( strlen (arg) == 1 )
	  mix_dump_context_add_opt (dis->dump, MIX_DUMP_rIa);
	else
	  {
	    static gint32 opt[] = { MIX_DUMP_rI1, MIX_DUMP_rI2,
				    MIX_DUMP_rI3, MIX_DUMP_rI4,
				    MIX_DUMP_rI5, MIX_DUMP_rI6
	    };
	    int i = arg[1] - '1';
	    if ( i < 0 || i > 5 )
	      {
		log_error_ (dis, _("Invalid I index: %d"), i);
		return FALSE;
	      }
	    mix_dump_context_add_opt (dis->dump, opt[i]);
	  }
      }
      break;
    default:
      log_error_ (dis, _("Invalid argument: %s"), arg);
      return FALSE;
    }
  mix_vm_dump (dis->vm, dis->dump);
  return TRUE;
}

gboolean
cmd_pflags_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  mix_dump_context_set_opt (dis->dump, MIX_DUMP_CMP | MIX_DUMP_OVER);
  mix_vm_dump (dis->vm, dis->dump);
  return TRUE;
}

gboolean
cmd_pall_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  mix_dump_context_set_opt (dis->dump, MIX_DUMP_ALL_NOMEM);
  mix_vm_dump (dis->vm, dis->dump);
  return TRUE;
}

gboolean
cmd_pmem_ (mix_vm_cmd_dispatcher_t *dis, const gchar *carg)
{
  glong begin = MIX_SHORT_ZERO, end = MIX_SHORT_ZERO;
  int i = 0;
  gboolean error = FALSE;
  gchar *arg = NULL;

  if ( strlen (carg) == 0 )
    {
      log_error_ (dis, _("Missing memory address"));
      return FALSE;
    }
  arg = g_strdup (carg);
  while (isdigit (arg[i]))
    i++;
  while (isspace (arg[i]))
    i++;
  if (arg[i] == '\0')
    begin = end = atol (arg);
  else if (arg[i] == '-')
    {
      gchar *narg;
      arg[i++] = '\0';
      begin = atol (arg);
      narg = arg + i;
      i = 0;
      while (isdigit (narg[i]))
	i++;
      while (isspace (narg[i]))
	i++;
      if (narg[i] != '\0')
	error = TRUE;
      else
	end = atol (narg);
    }
  else
    error = TRUE;

  if (error)
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
    }
  else if ( end < begin || end > MIX_VM_CELL_NO - 1 )
    {
      log_error_ (dis, _("Invalid range: %ld-%ld"), begin, end);
      error = TRUE;
    }
  else
    {
      mix_dump_context_set_opt (dis->dump, MIX_DUMP_CELLS);
      mix_dump_context_range (dis->dump, mix_short_new (begin),
			      mix_short_new (end + 1));
      mix_vm_dump (dis->vm, dis->dump);
    }
  g_free (arg);
  return !error;
}

gboolean
cmd_sreg_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  int i = 0;
  char reg = arg[0];
  gboolean ok = TRUE;
  long value;

  i = (reg == 'I') ? 2 : 1;
  ok = strlen (arg) > 2 && isspace (arg[i]);
  if (ok)
    {
      while (isspace (arg[i])) i++;
      ok = isdigit (arg[i]) || arg[i] == '+' || arg[i] == '-';
      if (ok)
	{
	  value = atol (arg + i);
	  if (arg[i] == '+' || arg[i] == '-') i++;
	  while (isdigit (arg[i])) i++;
	  ok = (arg[i] == '\0');
	  if (ok)
	    switch (reg)
	      {
	      case 'A':
		mix_vm_set_rA (dis->vm, mix_word_new (value));
		break;
	      case 'X':
		mix_vm_set_rX (dis->vm, mix_word_new (value));
		break;
	      case 'J':
		if ( value >= 0 )
		  mix_vm_set_rJ (dis->vm, mix_short_new (value));
		else
		  ok = FALSE;
		break;
	      case 'I':
		{
		  guint k = arg[1] - '0';
		  if ( k < 7 )
		    mix_vm_set_rI (dis->vm, k, mix_short_new (value));
		  else
		    ok = FALSE;
		}
		break;
	      default:
		ok = FALSE;
	      }
	}
    }
  if (!ok)
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
    }

  return ok;
}

gboolean
cmd_scmp_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gboolean ok = (strlen (arg) == 1);
  if (ok) switch (arg[0])
    {
    case 'L':
      mix_vm_set_cmpflag (dis->vm, mix_LESS);
      break;
    case 'E':
      mix_vm_set_cmpflag (dis->vm, mix_EQ);
      break;
    case 'G':
      mix_vm_set_cmpflag (dis->vm, mix_GREAT);
      break;
    default:
      ok = FALSE;
    }
  if (!ok)
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
    }

  return ok;
}

gboolean
cmd_sover_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gboolean ok = (strlen (arg) == 1);
  if (ok) switch (arg[0])
    {
    case 'T':
      mix_vm_set_overflow (dis->vm, TRUE);
      break;
    case 'F':
      mix_vm_set_overflow (dis->vm, FALSE);
      break;
    default:
      ok = FALSE;
    }
  if (!ok)
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
    }

  return ok;
}

gboolean
cmd_smem_ (mix_vm_cmd_dispatcher_t *dis, const gchar *carg)
{
  gboolean ok = (strlen (carg) > 2 && isdigit (carg[0]));
  glong addr = -1;
  glong value = 0;
  int k = 0;
  gchar *arg = NULL;
  mix_word_t wval;

  if (ok)
    {
      arg = g_strdup (carg);
      while (isdigit (arg[k])) k++;
      ok = isspace (arg[k]);
      if (ok)
	{
	  arg[k++] = '\0';
	  addr = atol (arg);
	  ok = addr < MIX_VM_CELL_NO;
	}
      if (ok)
	{
	  while (isspace (arg[k])) k++;
	  value = atol (arg + k);

          if ((value == 0) && (arg[k] == '-'))
            wval = MIX_WORD_MINUS_ZERO;
          else
            wval = mix_word_new (value);

	  if ( arg[k] == '+' || arg[k] == '-' ) k++;
	  while (isdigit (arg[k])) k++;
	  ok = arg[k] == '\0';
	}
    }

  if (ok)
    {
      mix_vm_set_addr_contents (dis->vm, mix_short_new (addr), wval);
    }
  else
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
    }

  if (arg) g_free (arg);

  return ok;
}

gboolean
cmd_ssym_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gboolean result = FALSE;
  if (arg == NULL || strlen(arg) == 0)
    {
      log_error_ (dis, _("Missing arguments"));
    }
  else
    {
      gchar *a = g_strdup (arg);
      gchar *s = strtok (a, " \t");
      gchar *w = strtok (NULL, " \t");
      if (w != NULL && strtok (NULL, " \t") == NULL)
	{
	  cmd_weval_ (dis, w);
	  if (mix_eval_last_error (dis->eval) == MIX_EVAL_OK) {
	    mix_eval_set_symbol (dis->eval, s, mix_eval_value (dis->eval));
	    result = TRUE;
	  }
	}
      else
	{
	  log_error_ (dis, _("Wrong argument number"));
	}
      g_free (a);
    }
  return result;
}

gboolean
cmd_sbp_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  glong lineno;
  glong k = 0;
  while (isdigit (arg[k])) k++;
  if (arg[k] != '\0')
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
      return FALSE;
    }
  lineno = atol (arg);
  switch (k = mix_vm_set_breakpoint (dis->vm, lineno))
    {
    case MIX_VM_BP_INV_LINE:
      log_error_ (dis, _("Line number %ld too high"), lineno);
      break;
    case MIX_VM_BP_ERROR:
      log_error_ (dis, _("Could not set breakpoint: internal error"));
      break;
    case MIX_VM_BP_NDEBUG:
      log_error_ (dis, _("Could not set breakpoint: no debug info available"),
		  dis->err);
      break;
    default:
      if (wants_logs_ (dis))
	log_message_ (dis, _("Breakpoint set at line %ld"), k);
      return TRUE;
    }
  return FALSE;
}

gboolean
cmd_sbpa_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  glong address;
  glong k = 0;
  while (isdigit (arg[k])) k++;
  if (arg[k] != '\0')
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
      return FALSE;
    }
  address = atol (arg);
  switch (mix_vm_set_breakpoint_address (dis->vm, address))
    {
    case MIX_VM_BP_INV_ADDRESS:
      log_error_ (dis, _("Invalid address %ld"), address);
      break;
    case MIX_VM_BP_ERROR:
      log_error_ (dis, _("Could not set breakpoint: internal error"));
      break;
    default:
      if (wants_logs_ (dis))
	log_message_ (dis, _("Breakpoint set at address %ld"), address);
      return TRUE;
    }
  return FALSE;
}

gboolean
cmd_cbp_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  glong lineno;
  int k = 0;
  while (isdigit (arg[k])) k++;
  if (arg[k] != '\0')
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
      return FALSE;
    }
  lineno = atol (arg);
  switch (mix_vm_clear_breakpoint (dis->vm, lineno))
    {
    case MIX_VM_BP_INV_LINE:
      log_error_ (dis, _("No breakpoint set at line %ld"), lineno);
      break;
    case MIX_VM_BP_ERROR:
      log_error_ (dis, _("Could not set breakpoint: internal error"));
      break;
    case MIX_VM_BP_NDEBUG:
      log_error_ (dis, _("No debug info available"));
      break;
    case MIX_VM_BP_OK:
      if (wants_logs_ (dis))
	log_message_ (dis, _("Breakpoint cleared at line %ld"), lineno);
      return TRUE;
    default:
      g_assert_not_reached ();
      break;
    }
  return FALSE;
}

gboolean
cmd_cbpa_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  glong address;
  glong k = 0;
  while (isdigit (arg[k])) k++;
  if (arg[k] != '\0')
    {
      log_error_ (dis, _("Invalid argument: %s"), arg);
      return FALSE;
    }
  address = atol (arg);
  switch (mix_vm_clear_breakpoint_address (dis->vm, address))
    {
    case MIX_VM_BP_INV_ADDRESS:
      log_error_ (dis, _("Invalid address %ld"), address);
      break;
    case MIX_VM_BP_ERROR:
      log_error_ (dis, _("Could not clear breakpoint: internal error"));
      break;
    default:
      if (wants_logs_ (dis))
	log_message_ (dis, _("Breakpoint cleared at address %ld"), address);
      return TRUE;
    }
  return FALSE;
}


gboolean
cmd_cabp_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (strlen (arg) != 0)
    {
      log_error_ (dis, _("Unexpected argument: %s"), arg);
      return FALSE;
    }
  mix_vm_clear_all_breakpoints (dis->vm);
  return TRUE;
}

gboolean
cmd_weval_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if ( strlen (arg) == 0 )
    {
      log_error_ (dis, _("Missing expression"));
      return FALSE;
    }

  if (mix_eval_expression_with_loc (dis->eval, arg,
				    mix_vm_get_prog_count (dis->vm)) ==
      MIX_EVAL_OK)
    {
      mix_word_print_to_file (mix_eval_value (dis->eval), NULL, dis->out);
      putc ('\n', dis->out);
      return TRUE;
    }
  else
    {
      gint pos = mix_eval_last_error_pos (dis->eval);
      gint k, len = strlen (arg);
      g_assert(pos > -1 && pos <= len);
      for (k = 0; k<pos; ++k) fputc (arg[k], dis->err);
      fputc ('\n', dis->err);
      for (k = 0; k<pos; ++k) fputc (' ', dis->err);
      for (k = pos; k < len; ++k) fputc (arg[k], dis->err);
      fprintf (dis->err, _("\nEvaluation error: %s\n"),
	       mix_eval_last_error_string (dis->eval));
      return FALSE;
    }
}

gboolean
cmd_w2d_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if ( strlen (arg) == 0 )
    {
      log_error_ (dis, _("Missing expression"));
      return FALSE;
    }
  else
    {
      gchar *cp = g_strdup (arg), *a = cp;
      mix_byte_t bytes[5] = {0, 0, 0, 0, 0};
      gchar *b;
      guint k = 0;
      gboolean is_n = (a[0] == '-'), success = TRUE;
      if (a[0] == '+' || a[0] == '-') ++a;
      b = strtok (a, " \t");
      while (b != NULL && k < 5)
	{
	  if (strlen (b) != 2 || !isdigit(b[0]) || !isdigit(b[1]))
	    {
	      log_error_ (dis, _("Incorrect byte specification: %s"), b);
	      success = FALSE;
	      b = NULL;
	    }
	  else
	    {
	      bytes[k++] = mix_byte_new (atoi (b));
	      b = strtok (NULL, " \t");
	    }
	}
      if (success)
	{
	  if (strtok (NULL, "\t") != NULL)
	    {
	      log_error_ (dis,
			  _("The expression %s does not fit in a word"), arg);
	      success = FALSE;
	    }
	  else
	    {
	      mix_word_t w = mix_bytes_to_word (bytes, k);
	      fprintf (dis->out, "%s%ld\n", is_n? "-":"+",
		       mix_word_magnitude (w));
	    }
	}
      g_free (cp);

      return success;
    }
}

gboolean
cmd_strace_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  static const gchar *ON = "on";
  static const gchar *OFF = "off";
  if (!arg || !strlen (arg))
    {
      log_error_ (dis, _("Missing argument"));
    }
  else if (!strcmp (arg, ON))
    {
      dis->trace = TRUE;
      if (dis->config) mix_config_update (dis->config, TRACING_KEY_, ON);
    }
  else if (!strcmp (arg, OFF))
    {
      dis->trace = FALSE;
      if (dis->config) mix_config_update (dis->config, TRACING_KEY_, OFF);
    }
  else
    log_error_ (dis, _("Wrong argument: "), arg);
  return TRUE;
}

gboolean
cmd_stime_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  static const gchar *ON = "on";
  static const gchar *OFF = "off";
  if (!arg || !strlen (arg))
    {
      log_error_ (dis, _("Missing argument"));
    }
  else if (!strcmp (arg, ON))
    {
      dis->printtime = TRUE;
      if (dis->config) mix_config_update (dis->config, TIMING_KEY_, ON);
    }
  else if (!strcmp (arg, OFF))
    {
      dis->printtime = FALSE;
      if (dis->config) mix_config_update (dis->config, TIMING_KEY_, OFF);
    }
  else
    log_error_ (dis, _("Wrong argument: "), arg);
  return TRUE;
}

gboolean
cmd_ptime_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  print_time_ (dis);
  return TRUE;
}

gboolean
cmd_pedit_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  const gchar *ed = mix_vm_cmd_dispatcher_get_editor (dis);
  if (dis)
    fprintf (dis->out, _("Edit command: %s\n"), ed);
  else
    fprintf (dis->out, _("Edit command not set (use sedit)\n"));
  return TRUE;
}

gboolean
cmd_sedit_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (!arg || !strlen (arg))
    {
      log_error_ (dis, _("Missing argument"));
      return FALSE;
    }
  mix_vm_cmd_dispatcher_set_editor (dis, arg);
  return TRUE;
}

gboolean
cmd_pasm_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  const gchar *ed = mix_vm_cmd_dispatcher_get_assembler (dis);
  if (dis)
    fprintf (dis->out, _("Compile command: %s\n"), ed);
  else
    fprintf (dis->out, _("Compile command not set (use sasm)\n"));
  return TRUE;
}

gboolean
cmd_sasm_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (!arg || !strlen (arg))
    {
      log_error_ (dis, _("Missing argument"));
      return FALSE;
    }
  mix_vm_cmd_dispatcher_set_assembler (dis, arg);
  return TRUE;
}

gboolean
cmd_sddir_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (!arg || !strlen (arg)) log_error_ (dis, _("Missing argument"));
  else if (mix_device_set_dir (arg) && dis->config)
    mix_config_set_devices_dir (dis->config, arg);
  return TRUE;
}

gboolean
cmd_pddir_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  fprintf (dis->out, _("Device directory: %s\n"), mix_device_get_dir ());
  return TRUE;
}

static const gint INVALID_REG_ = -2;

static mix_predicate_type_t
get_reg_pred_ (const gchar *arg)
{
  mix_predicate_type_t pred = INVALID_REG_;

  switch (*arg)
    {
    case 'A':
      pred = MIX_PRED_REG_A;
      break;
    case 'X':
      pred = MIX_PRED_REG_X;
      break;
    case 'J':
      pred = MIX_PRED_REG_J;
      break;
    case 'I':
      {
	if ( strlen (arg) == 2 )
	  {
	    int i = arg[1] - '1';
	    if (i >= 0 && i < 6)
	      pred = MIX_PRED_REG_I1 + i;
	  }
      }
      break;
    default:
      break;
    }
  return pred;
}

gboolean
cmd_sbpr_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  mix_predicate_type_t pred = get_reg_pred_ (arg);
  if (pred != INVALID_REG_)
    {
      mix_vm_set_conditional_breakpoint (dis->vm, dis->preds[pred]);
      if (wants_logs_ (dis))
	log_message_ (dis,
		      _("Conditional breakpoint on r%s change set"), arg);
      return TRUE;
    }
  else
    {
      log_error_ (dis, _("Invalid argument %s"), arg);
      return FALSE;
    }
}

gboolean
cmd_cbpr_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  mix_predicate_type_t pred = get_reg_pred_ (arg);
  if (pred != INVALID_REG_)
    {
      if (mix_vm_clear_conditional_breakpoint (dis->vm, dis->preds[pred]))
	{
	  if (wants_logs_(dis))
	    log_message_ (dis,
			  _("Conditional breakpoint on r%s change removed"),
			  arg);
	}
      else
	log_error_ (dis, _("No breakpoint set on r%s change"), arg);
      return TRUE;
    }
  else
    {
      log_error_ (dis, _("Invalid argument %s"), arg);
      return FALSE;
    }
}

gboolean
cmd_sbpm_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gint add = atoi (arg);
  gpointer key, value;
  if (add < 0 || add > MIX_VM_CELL_NO)
    {
      log_error_ (dis, _("Invalid memory address: %s"), arg);
      return FALSE;
    }
  if (!g_hash_table_lookup_extended (dis->mem_preds, GINT_TO_POINTER (add),
				     &key, &value))
    {
      mix_predicate_t *new_pred = mix_predicate_new (MIX_PRED_MEM);
      mix_predicate_set_mem_address (new_pred, add);
      g_hash_table_insert (dis->mem_preds,
			   GINT_TO_POINTER (add), (gpointer)new_pred);
      mix_vm_set_conditional_breakpoint (dis->vm, new_pred);
    }
  if (wants_logs_ (dis))
    log_message_ (dis, _("Conditional breakpoint on mem cell no. %d set"),
		  add);
  return TRUE;
}

gboolean
cmd_cbpm_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  gint add = atoi (arg);
  gpointer key, value;
  if (add < 0 || add > MIX_VM_CELL_NO)
    {
      log_error_ (dis, _("Invalid memory address: %s"), arg);
      return FALSE;
    }
  if (g_hash_table_lookup_extended (dis->mem_preds, GINT_TO_POINTER (add),
				    &key, &value))
    {
      g_hash_table_remove (dis->mem_preds, key);
      mix_vm_clear_conditional_breakpoint (dis->vm, (mix_predicate_t *)value);
      mix_predicate_delete ((mix_predicate_t *)value);
      if (wants_logs_ (dis))
	log_message_ (dis,
		      _("Conditional breakpoint on mem cell no. %d removed"),
		      add);
    }
  else
    {
      log_error_ (dis, _("No conditional breakpoint set at address %d"),
		  add);
    }
  return TRUE;
}

gboolean
cmd_sbpo_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg && strlen (arg))
    log_error_ (dis, _("Unexpected argument: %s"), arg);
  else
    {
      mix_vm_set_conditional_breakpoint (dis->vm, dis->preds[MIX_PRED_OVER]);
      if (wants_logs_ (dis))
	log_message_ (dis,
		      _("Conditional breakpoint on overflow toggled set"));
    }
  return TRUE;
}

gboolean
cmd_cbpo_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg && strlen (arg))
    log_error_ (dis, _("Unexpected argument: %s"), arg);
  else
    {
      if (mix_vm_clear_conditional_breakpoint
	  (dis->vm, dis->preds[MIX_PRED_OVER]))
	{
	  if (wants_logs_ (dis))
	    log_message_
	      (dis, _("Conditional breakpoint on overflow toggled removed."));
	}
      else
	log_error_ (dis, _("No breakpoint set on overflow toggle"));
    }
  return TRUE;
}

gboolean
cmd_sbpc_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg && strlen (arg))
    log_error_ (dis, _("Unexpected argument: %s"), arg);
  else
    {
      mix_vm_set_conditional_breakpoint (dis->vm, dis->preds[MIX_PRED_CMP]);
      if (wants_logs_ (dis))
	log_message_
	  (dis, _("Conditional breakpoint on comparison flag changed set"));
    }
  return TRUE;
}

gboolean
cmd_cbpc_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  if (arg && strlen (arg))
    log_error_ (dis, _("Unexpected argument: %s"), arg);
  else
    {
      if (mix_vm_clear_conditional_breakpoint
	  (dis->vm, dis->preds[MIX_PRED_CMP]))
	{
	  if (wants_logs_ (dis))
	    log_message_
	      (dis,
	       _("Conditional breakpoint on comparison flag changed removed."));
	}
      else
	log_error_ (dis, _("No breakpoint set on comparison flag change"));
    }
  return TRUE;
}

gboolean
cmd_pbt_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  enum {SIZE = 256};
  static gchar BUFFER[SIZE];
  gint no = atoi (arg);
  gint k = 0, address;
  guint line;
  const mix_src_file_t *file = mix_vm_get_src_file (dis->vm);
  char *name =
    file ? g_path_get_basename (mix_src_file_get_path (file)) : NULL;

  const GSList *add = mix_vm_get_backtrace (dis->vm);
  while (add && (no == 0 || k < no))
    {
      BUFFER[0] = '\0';
      address = GPOINTER_TO_INT (add->data);
      line = mix_vm_get_address_lineno (dis->vm, address);
      if (line && file)
	{
	  int j = 0;
	  g_snprintf (BUFFER, SIZE, "%s", mix_src_file_get_line (file, line));
	  while (!isspace (BUFFER[j])) j++;
	  BUFFER[j] = '\0';
	}
      if (strlen (BUFFER) == 0)	g_snprintf (BUFFER, SIZE, "%d", address);
      fprintf (dis->out, "#%d\t%s\tin %s%s:%d\n", k, BUFFER, name,
	       MIX_SRC_DEFEXT, line);
      ++k;
      add = add->next;
    }
  return TRUE;
}

gboolean
cmd_slog_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  static const gchar *ON = "on";
  static const gchar *OFF = "off";
  if (arg && !strcmp (arg, ON))
    {
      dis->log_msg = TRUE;
      if (dis->config) mix_config_update (dis->config, LOGGING_KEY_, ON);
    }
  else if (arg && !strcmp (arg, OFF))
    {
      dis->log_msg = FALSE;
      if (dis->config) mix_config_update (dis->config, LOGGING_KEY_, OFF);
    }
  else
    log_error_ (dis, _("Wrong argument: "), arg);
  return TRUE;
}

gboolean
cmd_pprog_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_program_path (dis);
  fprintf (dis->out, "%s", path? path : _("No program currently loaded"));
  fprintf (dis->out, "%s", "\n");
  return (path != NULL);
}

gboolean
cmd_psrc_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  const gchar *path = mix_vm_cmd_dispatcher_get_src_file_path (dis);
  fprintf (dis->out, "%s", path? path : _("No program currently loaded\n"));
  fprintf (dis->out, "%s", "\n");
  return (path != NULL);
}

gboolean
cmd_pline_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  glong line = 0;
  const gchar *txt;

  if (arg && strlen (arg)) line = atoi (arg);

  if (line < 0)
    {
      log_error_ (dis, _("Invalid argument"));
      return FALSE;
    }

  if (line == 0)
    line = mix_vm_cmd_dispatcher_get_src_file_lineno (dis);

  if (line == 0)
    txt = "No such line (debug info not available)\n";
  else
    txt = mix_vm_cmd_dispatcher_get_src_file_line (dis, line, FALSE);

  if (txt == NULL || strlen (txt) == 0) txt = "No such line\n";

  fprintf (dis->out, "Line %ld: %s\n", line, txt);

  return TRUE;
}

gboolean
cmd_pstat_ (mix_vm_cmd_dispatcher_t *dis, const gchar *arg)
{
  static const gchar *MSGS[MIX_VM_EMPTY + 1] = {
    N_("Error loading or executing file"),
    N_("Execution stopped: breakpoint encountered"),
    N_("Execution stopped: conditional breakpoint encountered"),
    N_("Program successfully terminated"),
    N_("Execution stopped"),
    N_("Program successfully loaded"),
    N_("No program loaded")
  };
  mix_vm_status_t status =
    mix_vm_get_run_status (mix_vm_cmd_dispatcher_get_vm (dis));
  fprintf (dis->out, "VM status: %s\n", MSGS[status]);
  return TRUE;
}
