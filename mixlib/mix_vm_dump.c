/* ---------------------- mix_vm_dump.c :
 * Implementation of the functions declared in mix_vm_dump.h
 * ------------------------------------------------------------------
** Copyright (C) 2000, 2007 Free Software Foundation, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
**
*/

#include "mix.h"
#include <unistd.h>
#include "xmix_vm.h"
#include "mix_vm_dump.h"

/* Create/destroy a dump context */
mix_dump_context_t *
mix_dump_context_new(FILE  *fd, mix_address_t begin, mix_address_t end,
		     guint32 options)
{
  mix_dump_context_t *result = NULL;

  g_return_val_if_fail (fd != NULL, NULL);

  if ( begin > end ) begin = end;
  if ( end >= MEM_CELLS_NO_ ) end = MEM_CELLS_NO_;

  result = g_new (mix_dump_context_t,1);
  result->options = options;
  result->begin = begin;
  result->end = end;
  result->channel = fd;

  return result;
}

void
mix_dump_context_delete (mix_dump_context_t *dc)
{
  g_return_if_fail (dc != NULL);
  g_free (dc);
}

/* Use the dump context */
#define WORD_FMT_   "%s %02d %02d %02d %02d %02d (%010ld)"
#define SHORT_FMT_  "%s %02d %02d (%04d)"
#define WORD_SIGN_(w)  mix_word_is_negative (w)? "-":"+"
#define SHORT_SIGN_(s) mix_short_is_negative (s)? "-":"+"
#define WORD_ABS_(w)   mix_word_magnitude (w)
#define SHORT_ABS_(s)  mix_short_magnitude (s)
#define WORD_BYTE_(w,i) mix_byte_new (mix_word_magnitude (w)>>(6*(5-i)))
#define SHORT_BYTE_(s,i) mix_byte_new (mix_short_magnitude (s)>>(6*(2-i)))
#define WORD_ARGS_(w)  WORD_SIGN_ (w), WORD_BYTE_ (w,1), WORD_BYTE_ (w,2), \
	   WORD_BYTE_ (w,3), WORD_BYTE_ (w,4), WORD_BYTE_ (w,5), WORD_ABS_ (w)
#define SHORT_ARGS_(s) SHORT_SIGN_ (s), SHORT_BYTE_ (s,1), SHORT_BYTE_ (s,2), \
		       SHORT_ABS_ (s)

void
mix_vm_dump (const mix_vm_t *vm, const mix_dump_context_t *dc)
{
  guint j, i;
  FILE *f;

  g_return_if_fail (vm != NULL);
  g_return_if_fail (dc != NULL);

  f = dc->channel;

  if ( (dc->options & MIX_DUMP_rA) == MIX_DUMP_rA )
    {
      mix_word_t rA = get_rA_ (vm);
      fprintf (f, "rA: " WORD_FMT_ "\n", WORD_ARGS_ (rA));
    }

  if ( (dc->options & MIX_DUMP_rX) == MIX_DUMP_rX )
    {
      mix_word_t rX = get_rX_ (vm);
      fprintf (f, "rX: " WORD_FMT_ "\n", WORD_ARGS_ (rX));
    }

  if ( (dc->options & MIX_DUMP_rJ) == MIX_DUMP_rJ )
    {
      mix_short_t rJ = get_rJ_ (vm);
      fprintf (f, "rJ: " SHORT_FMT_ "\n", SHORT_ARGS_ (rJ));
    }

  for (j = 0, i = 0; j < IREG_NO_; ++j)
    {
      if ( (dc->options & (MIX_DUMP_rI1<<j)) == (MIX_DUMP_rI1<<j) )
	{
	  mix_short_t rI = mix_word_to_short_fast (get_rI_ (vm, j+1));
	  fprintf (f, "rI%d: " SHORT_FMT_ "\t", j+1, SHORT_ARGS_ (rI));
	  i++;
	}
      if ( i%2 == 0 && i != 0 )	fprintf (f, "\n");
    }

  if ( i%2 == 1 ) fprintf (f, "\n");

  if ( (dc->options & MIX_DUMP_OVER) == MIX_DUMP_OVER )
    {
      fprintf (f, _("Overflow: %s\n"), get_over_ (vm)? "T":"F");
    }

  if ( (dc->options & MIX_DUMP_CMP) == MIX_DUMP_CMP )
    {
      const gchar *val = "?";
      switch (get_cmp_ (vm))
	{
	case mix_LESS:
	  val = "L";
	  break;
	case mix_GREAT:
	  val = "G";
	  break;
	case mix_EQ:
	  val = "E";
	  break;
	default:
	  g_assert_not_reached ();
	  break;
	}
      fprintf (f, _("Cmp: %s\n"), val);
    }

  if ( (dc->options & MIX_DUMP_CELLS) == MIX_DUMP_CELLS )
    {
      for (j = dc->begin; j < dc->end; ++j)
	{
	  mix_word_t cell = get_cell_ (vm,j);
	  fprintf (f, "%04d: " WORD_FMT_ "\n", j, WORD_ARGS_ (cell));
	}
    }
  fflush (f);
}

