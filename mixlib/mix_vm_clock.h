/* -*-c-*- ---------------- mix_vm_clock.h :
 * Declaration of mix_vm_clock_t, a clock for the MIX virtual machine.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_VM_CLOCK_H
#define MIX_VM_CLOCK_H

#include "mix.h"
#include "mix_ins.h"

/* the type of time unit */
typedef gulong mix_time_t;

/* the clock type */
typedef struct mix_vm_clock_t
{
  mix_time_t time;
} mix_vm_clock_t;

/* Create/delete a clock */
extern mix_vm_clock_t *
mix_vm_clock_new ();

extern void
mix_vm_clock_delete (mix_vm_clock_t *clock);

/* Increase time with the units needed to execute ins */
extern mix_time_t /* the added lapse */
mix_vm_clock_add_lapse (mix_vm_clock_t *clock, const mix_ins_t *ins);

/* Get the time since creation */
#define mix_vm_clock_get_time(clock) (clock? clock->time:0)

#endif /* MIX_VM_CLOCK_H */

