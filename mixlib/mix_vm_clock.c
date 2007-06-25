/* -*-c-*- -------------- mix_vm_clock.c :
 * Implementation of the functions declared in mix_vm_clock.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2006, 2007 Free Software Foundation, Inc.
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


#include "mix_vm_clock.h"

static const mix_time_t exec_times_[] = {
  1, 2, 2, 10, 12, 10, 2, 1,
  2, 2, 2, 2 ,  2,  2, 2, 2,
  2, 2, 2, 2 ,  2,  2, 2, 2,
  2, 2, 2, 2 ,  2,  2, 2, 2,
  2, 2, 1, 1,   1,  1, 1, 1,
  1, 1, 1, 1,   1,  1, 1, 1,
  1, 1, 1, 1,   1,  1, 1, 1,
  2, 2, 2, 2 ,  2,  2, 2, 2
};


/* Create/delete a clock */
mix_vm_clock_t *
mix_vm_clock_new ()
{
  mix_vm_clock_t *result = g_new (mix_vm_clock_t, 1);
  result->time = 0;
  return result;
}

void
mix_vm_clock_delete (mix_vm_clock_t *clock)
{
  g_return_if_fail (clock != NULL);
}

/* Increase time with the units needed to execute ins */
mix_time_t
mix_vm_clock_add_lapse (mix_vm_clock_t *clock, const mix_ins_t *ins)
{
  mix_time_t t = 0;
  g_return_val_if_fail (clock != NULL, 0);
  if (ins != NULL)
    {
      t = exec_times_[ins->opcode];
      if (ins->opcode == mix_opMOVE) {
	t += 2 * (ins->fspec);
      }
    }
  clock->time += t;
  return t;
}

