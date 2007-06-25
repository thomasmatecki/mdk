/* -*-c-*- ---------------- mixgtk_mixvm.h :
 * Functions dealing with the mixvm widgets
 * ------------------------------------------------------------------
 * Copyright (C) 2001, 2004, 2006, 2007 Free Software Foundation, Inc.
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


#ifndef MIXGTK_MIXVM_H
#define MIXGTK_MIXVM_H

#include <mixlib/mix_vm.h>
#include "mixgtk.h"
#include "mixgtk_widgets.h"

/* initialise the mixvm widgets */
extern gboolean
mixgtk_mixvm_init (mix_vm_t *vm);

extern void
mixgtk_mixvm_update_fonts (void);

/* update register widgets */
extern void
mixgtk_mixvm_update_registers (void);

/* update the overflow toggle */
extern void
mixgtk_mixvm_update_over_toggle (void);

/* update the comparison flag */
extern void
mixgtk_mixvm_update_cmp (void);

/* update the memory cells */
extern void
mixgtk_mixvm_update_cells (void);

extern void
mixgtk_mixvm_update_cells_to_address (gint address);

/* update the loc pointer */
extern void
mixgtk_mixvm_update_loc (void);

/* update times */
extern void
mixgtk_mixvm_update_times (void);

/* update all mixvm widgets */
extern void
mixgtk_mixvm_update_vm_widgets (void);


#endif /* MIXGTK_MIXVM_H */

