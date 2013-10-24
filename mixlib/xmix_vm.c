/* ---------------------- xmix_vm.c :
 * Implementation of the functions declared in xmix_vm.h
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2003, 2004, 2007, 2010, 2013 Free Software Foundation, Inc.
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



#include "xmix_vm.h"

/* auxiliar functions */
G_INLINE_FUNC mix_address_t
get_M_ (const mix_vm_t *vm, const mix_ins_t *ins);
G_INLINE_FUNC mix_word_t
get_V_ (const mix_vm_t *vm, const mix_ins_t *ins);
G_INLINE_FUNC mix_device_t *
get_dev_ (mix_vm_t *vm, mix_fspec_t type);

G_INLINE_FUNC mix_address_t
get_M_ (const mix_vm_t *vm, const mix_ins_t *ins)
{
  if ( ins->index == 0 )
    return ins->address;
  else
    return mix_short_add (ins->address,
                          mix_word_to_short_fast (get_rI_ (vm, ins->index)));
}

G_INLINE_FUNC mix_word_t
get_V_ (const mix_vm_t *vm, const mix_ins_t *ins)
{
  return mix_word_get_field (ins->fspec, get_cell_ (vm, get_M_ (vm,ins)));
}

G_INLINE_FUNC mix_device_t *
get_dev_ (mix_vm_t *vm, mix_fspec_t type)
{
  if (type >= BD_NO_) return NULL;
  if (vm->devices[type] == NULL)
    vm->devices[type] = vm->factory (type);
  return vm->devices[type];
}

/* error macro */
#define fail_if_not_(vm,cond,error)             \
do {                                            \
  if (!(cond))                                  \
    {                                           \
      set_last_error_ (vm, error);              \
      return FALSE;                             \
    }                                           \
} while (FALSE)

#define fail_(vm,error) fail_if_not_ (vm, FALSE, error)

#define fail_unexpected_(vm) fail_ (vm, MIX_VM_ERROR_UNEXPECTED)

/* Instruction handlers */

static gboolean
nop_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opNOP);
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
add_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_word_t val = get_V_ (vm, ins);
  g_assert (ins->opcode == mix_opADD || ins->opcode == mix_opSUB);
  if ( ins->opcode == mix_opSUB ) mix_word_reverse_sign (val);
  if ( mix_word_add_and_carry (get_rA_ (vm), val, NULL, &get_rA_ (vm)) )
    set_over_ (vm,TRUE);
  inc_loc_ (vm);
  return TRUE;
}


static gboolean
mul_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opMUL);
  mix_word_mul (get_rA_ (vm), get_V_ (vm,ins), &get_rA_ (vm), &get_rX_ (vm));
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
div_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opDIV);
  if ( mix_word_div (get_rA_ (vm), get_rX_ (vm), get_V_ (vm,ins),
                     &get_rA_ (vm), &get_rX_ (vm)) )
    set_over_ (vm,TRUE);
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
spc_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opSPC);

  switch (mix_ins_id_from_ins (*ins)) {
  case mix_HLT: halt_ (vm, TRUE); break;
  case mix_CHAR:
    {
      guint32 num = mix_word_magnitude (get_rA_ (vm));
      mix_char_t z = mix_ascii_to_char ('0');
      guint i;
      for (i = 5; 0 < i; --i, num /= 10)
	mix_word_set_byte (&get_rX_ (vm), i, z + num % 10);
      for (i = 5; 0 < i; --i, num /= 10)
	mix_word_set_byte (&get_rA_ (vm), i, z + num % 10);
      break;
    }
  case mix_NUM:
    {
      guint i;
      mix_word_t num = MIX_WORD_ZERO;
      mix_word_t ten = 10;
      for (i = 1; i <= 5; ++i) {
	mix_word_mul (ten, num, NULL, &num);
	mix_word_add_and_carry (num, mix_word_get_byte (get_rA_ (vm),i)%10,
                                NULL, &num);
      }
      for (i = 1; i <= 5; ++i) {
	mix_word_mul (ten, num, NULL, &num);
	mix_word_add_and_carry (num, mix_word_get_byte (get_rX_ (vm),i)%10,
                                NULL, &num);
      }
      set_rA_ (vm, mix_word_is_negative (get_rA_ (vm)) ?
               mix_word_negative (num) : num);
      break;
    }
  default: fail_ (vm, MIX_VM_ERROR_BAD_FSPEC);
  }
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
sla_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_short_t n = get_M_ (vm,ins);

  g_assert (ins->opcode == mix_opSLx);

  fail_if_not_ (vm, mix_short_is_positive (n), MIX_VM_ERROR_BAD_M);

  switch ( mix_ins_id_from_ins (*ins) ) {
  case mix_SLA:
    mix_word_shift_left (get_rA_ (vm), MIX_WORD_ZERO, n, &get_rA_ (vm), NULL);
    break;
  case mix_SRA:
    mix_word_shift_right (get_rA_ (vm), MIX_WORD_ZERO, n, &get_rA_ (vm), NULL);
    break;
  case mix_SLAX:
    mix_word_shift_left (get_rA_ (vm), get_rX_ (vm), n,
                         &get_rA_ (vm), &get_rX_ (vm));
    break;
  case mix_SRAX:
    mix_word_shift_right (get_rA_ (vm), get_rX_ (vm), n,
                          &get_rA_ (vm), &get_rX_ (vm));
    break;
  case mix_SLC:
    mix_word_shift_left_circular (get_rA_ (vm), get_rX_ (vm), n,
                                  &get_rA_ (vm), &get_rX_ (vm));
    break;
  case mix_SRC:
    mix_word_shift_right_circular (get_rA_ (vm), get_rX_ (vm), n,
                                   &get_rA_ (vm), &get_rX_ (vm));
    break;
  default:
    fail_unexpected_ (vm);
  }

  inc_loc_ (vm);
  return TRUE;
}

static gboolean
mov_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_short_t from = get_M_ (vm,ins);
  mix_short_t to = mix_word_to_short_fast (get_rI_ (vm,1));
  guint k, delta = ins->fspec;

  g_assert (ins->opcode == mix_opMOVE);

  gboolean result = (delta == 0)
    || (mix_short_is_positive (from)
        && mix_short_is_positive (to)
        && MEMOK_ (from + delta -1)
        && MEMOK_ (to + delta - 1));

  if (result && delta > 0)
    {
      for (k = 0; k < delta; ++k)
        set_cell_ (vm, to+k, get_cell_ (vm, from+k));
      set_rI_ (vm, 1, to+delta);
    }

  if (result) inc_loc_ (vm);
  return result;
}

static gboolean
lda_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  gint r = 0;
  mix_word_t val;
  mix_ins_id_t id = mix_ins_id_from_ins (*ins);

  g_assert (id >= mix_LDA && id <= mix_LDXN);

  val = get_V_ (vm, ins);
  if (id > mix_LDX) mix_word_reverse_sign (val);
  if ( (id > mix_LDA && id < mix_LDX) || (id > mix_LDAN && id < mix_LDXN) )
    /* Bytes 1-3 of I regs are always null */
    val = mix_word_set_field (mix_fspec_new (1,3),MIX_WORD_ZERO,val);

  switch (id) {
  case mix_LDA: case mix_LDAN: r = A_; break;
  case mix_LDX: case mix_LDXN: r = X_; break;
  case mix_LD1: case mix_LD1N: r = I1_; break;
  case mix_LD2: case mix_LD2N: r = I2_; break;
  case mix_LD3: case mix_LD3N: r = I3_; break;
  case mix_LD4: case mix_LD4N: r = I4_; break;
  case mix_LD5: case mix_LD5N: r = I5_; break;
  case mix_LD6: case mix_LD6N: r = I6_; break;
  default: g_assert_not_reached ();
  }
  set_reg_ (vm, r, val);
  inc_loc_ (vm);
  return TRUE;
}


static gboolean
sta_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_address_t addr = get_M_ (vm, ins);
  mix_ins_id_t id = mix_ins_id_from_ins (*ins);
  mix_word_t from;

  g_assert (id >= mix_STA && id <= mix_STZ);
  switch (id) {
  case mix_STA: from = get_rA_ (vm); break;
  case mix_STX: from = get_rX_ (vm); break;
  case mix_STJ: from = get_rJ_ (vm); break;
  case mix_STZ: from = MIX_WORD_ZERO; break;
  default: from = get_rI_ (vm, id - mix_ST1 + 1); break;
  }
  set_cell_ (vm, addr,
             mix_word_store_field (ins->fspec, from,  get_cell_ (vm, addr)));
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
jbs_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opJBUS);
  fail_if_not_ (vm, ins->fspec < BD_NO_, MIX_VM_ERROR_BAD_DEVICE_NO);
  fail_if_not_ (vm, get_dev_ (vm, ins->fspec) != NULL,
                MIX_VM_ERROR_BAD_DEVICE_NO);

  if ( mix_device_busy (get_dev_ (vm, ins->fspec)) ) {
    set_rJ_ (vm, get_loc_ (vm));
    set_loc_ (vm, get_M_ (vm, ins));
  } else inc_loc_ (vm);
  return TRUE;
}

static gboolean
ioc_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_address_t addr;
  mix_device_t *dev;

  g_assert (ins->opcode == mix_opIOC);

  addr = get_M_ (vm, ins);
  fail_if_not_ (vm, ins->fspec < BD_NO_, MIX_VM_ERROR_BAD_DEVICE_NO);

  dev = get_dev_ (vm, ins->fspec);
  fail_if_not_ (vm, dev != NULL, MIX_VM_ERROR_BAD_DEVICE_NO);

  fail_if_not_ (vm, mix_device_ioc (dev, addr), MIX_VM_ERROR_DEV_CTL);

  inc_loc_ (vm);
  return TRUE;
}

static gboolean
inp_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_address_t addr;
  mix_device_t *dev;

  g_assert (ins->opcode == mix_opIN);
  fail_if_not_ (vm, ins->fspec < BD_NO_, MIX_VM_ERROR_BAD_DEVICE_NO);

  addr = get_M_ (vm, ins);
  fail_if_not_ (vm, MEMOK_ (addr), MIX_VM_ERROR_BAD_ACCESS);

  dev = get_dev_ (vm, ins->fspec);
  fail_if_not_ (vm, dev != NULL, MIX_VM_ERROR_BAD_DEVICE_NO);

  fail_if_not_ (vm, MEM_CELLS_NO_ - addr > mix_device_block_size (dev),
                MIX_VM_ERROR_BAD_ACCESS);

  fail_if_not_ (vm, mix_device_read (dev, get_cell_ptr_ (vm, addr)),
                MIX_VM_ERROR_DEV_READ);

  inc_loc_ (vm);
  return TRUE;
}

static gboolean
out_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_address_t addr;
  mix_device_t *dev;

  g_assert (ins->opcode == mix_opOUT);

  fail_if_not_ (vm, ins->fspec < BD_NO_, MIX_VM_ERROR_BAD_DEVICE_NO);

  addr = get_M_ (vm, ins);
  fail_if_not_ (vm, MEMOK_ (addr), MIX_VM_ERROR_BAD_ACCESS);

  dev = get_dev_ (vm, ins->fspec);
  fail_if_not_ (vm, dev != NULL, MIX_VM_ERROR_BAD_DEVICE_NO);

  fail_if_not_ (vm, MEM_CELLS_NO_ - addr > mix_device_block_size (dev),
                MIX_VM_ERROR_BAD_ACCESS);

  fail_if_not_ (vm, mix_device_write (dev, get_cell_ptr_ (vm, addr)),
                MIX_VM_ERROR_DEV_WRITE);

  inc_loc_ (vm);
  return TRUE;
}

static gboolean
jrd_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode == mix_opJRED);
  fail_if_not_ (vm, ins->fspec < BD_NO_, MIX_VM_ERROR_BAD_DEVICE_NO);
  fail_if_not_ (vm, get_dev_ (vm, ins->fspec) != NULL,
                MIX_VM_ERROR_BAD_DEVICE_NO);

  inc_loc_ (vm);
  if ( !mix_device_busy (get_dev_ (vm, ins->fspec)) )
    {
      set_rJ_ (vm, get_loc_ (vm));
      set_loc_ (vm, get_M_ (vm, ins));
    }
  return TRUE;
}

static gboolean
jmp_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  gboolean jump = FALSE;
  mix_address_t addr = get_M_ (vm, ins);
  mix_ins_id_t id = mix_ins_id_from_ins (*ins);

  g_assert (ins->opcode == mix_opJMP);
  fail_if_not_ (vm, MEMOK_ (addr), MIX_VM_ERROR_BAD_ACCESS);

  switch ( id ) {
  case mix_JMP:
  case mix_JSJ:
    jump = TRUE;
    break;
  case mix_JOV:
    jump = get_over_ (vm);
    if (jump) set_over_ (vm, FALSE);
    break;
  case mix_JNOV:
    jump = !get_over_ (vm);
    set_over_ (vm, FALSE);
    break;
  case mix_JL:
    jump = ( get_cmp_ (vm) == mix_LESS );
    break;
  case mix_JE:
    jump = ( get_cmp_ (vm) == mix_EQ );
    break;
  case mix_JG:
    jump = ( get_cmp_ (vm) == mix_GREAT );
    break;
  case mix_JGE:
    jump = ( get_cmp_ (vm) != mix_LESS );
    break;
  case mix_JNE:
    jump = ( get_cmp_ (vm) != mix_EQ );
    break;
  case mix_JLE:
    jump = ( get_cmp_ (vm) != mix_GREAT );
    break;
  default:
    fail_unexpected_ (vm);
  }

  inc_loc_ (vm);
  if ( jump ) {
    if ( id != mix_JSJ ) set_rJ_ (vm, get_loc_ (vm));
    set_loc_ (vm, addr);
  }
  return TRUE;
}

static gboolean
jpx_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  gboolean jump = FALSE;
  mix_address_t addr = get_M_ (vm, ins);
  mix_ins_id_t id = mix_ins_id_from_ins (*ins);
  mix_word_t val;

  g_assert (ins->opcode >= mix_opJAx && ins->opcode <= mix_opJXx);
  fail_if_not_ (vm, MEMOK_ (addr), MIX_VM_ERROR_BAD_ACCESS);

  switch (ins->opcode) {
  case mix_opJAx: val = get_rA_ (vm); break;
  case mix_opJXx: val = get_rX_ (vm); break;
  default: val = get_rI_ (vm, ins->opcode - mix_opJAx);
  }

  switch (id) {
  case mix_JAN: case mix_JXN:
  case mix_J1N: case mix_J2N: case mix_J3N:
  case mix_J4N: case mix_J5N: case mix_J6N:
    jump = mix_word_is_negative (val) && val != MIX_WORD_MINUS_ZERO;
    break;
  case mix_JAZ: case mix_JXZ:
  case mix_J1Z: case mix_J2Z: case mix_J3Z:
  case mix_J4Z: case mix_J5Z: case mix_J6Z:
    jump = mix_word_magnitude (val) == MIX_WORD_ZERO;
    break;
  case mix_JAP: case mix_JXP:
  case mix_J1P: case mix_J2P: case mix_J3P:
  case mix_J4P: case mix_J5P: case mix_J6P:
    jump = mix_word_is_positive (val) && val != MIX_WORD_ZERO;
    break;
  case mix_JANN: case mix_JXNN:
  case mix_J1NN: case mix_J2NN: case mix_J3NN:
  case mix_J4NN: case mix_J5NN: case mix_J6NN:
    jump = mix_word_magnitude (val) == MIX_WORD_ZERO
      || mix_word_is_positive (val);
    break;
  case mix_JANZ: case mix_JXNZ:
  case mix_J1NZ: case mix_J2NZ: case mix_J3NZ:
  case mix_J4NZ: case mix_J5NZ: case mix_J6NZ:
    jump = mix_word_magnitude (val) != MIX_WORD_ZERO;
    break;
  case mix_JANP: case mix_JXNP:
  case mix_J1NP: case mix_J2NP: case mix_J3NP:
  case mix_J4NP: case mix_J5NP: case mix_J6NP:
    jump = mix_word_magnitude (val) == MIX_WORD_ZERO
      || mix_word_is_negative (val);
    break;
  default:
    fail_unexpected_ (vm);
  }

  inc_loc_ (vm);
  if ( jump ) {
    set_rJ_ (vm, get_loc_ (vm));
    set_loc_ (vm, addr);
  }
  return TRUE;
}

static gboolean
ina_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  mix_word_t val = mix_short_to_word_fast (get_M_ (vm, ins));
  mix_ins_id_t id = mix_ins_id_from_ins (*ins);
  gint r;

  g_assert (id >= mix_INCA && id <= mix_ENNX);

  switch (ins->opcode) {
  case mix_opINCA: r = A_; break;
  case mix_opINCX: r = X_; break;
  default: r = I1_ + ins->opcode - mix_opINC1;
  }

  switch (id) {
  case mix_ENTA: case mix_ENTX:
    break;
  case mix_ENT1: case mix_ENT2: case mix_ENT3:
  case mix_ENT4: case mix_ENT5: case mix_ENT6:
    val = mix_word_set_field (mix_fspec_new (1,3), MIX_WORD_ZERO, val);
    break;
  case mix_INCA: case mix_INCX:
    if ( mix_word_add_and_carry (val, get_reg_ (vm, r), NULL, &val) )
      set_over_ (vm, TRUE);
    break;
  case mix_INC1: case mix_INC2: case mix_INC3:
  case mix_INC4: case mix_INC5: case mix_INC6:
    mix_word_add_and_carry (val, get_reg_ (vm,r), NULL, &val);
    val = mix_word_set_field (mix_fspec_new (1,3), MIX_WORD_ZERO, val);
    break;
  case mix_DECA: case mix_DECX:
    if ( mix_word_add_and_carry (mix_word_negative (val), get_reg_ (vm, r),
                                 NULL, &val) )
      set_over_ (vm, TRUE);
    break;
  case mix_DEC1: case mix_DEC2: case mix_DEC3:
  case mix_DEC4: case mix_DEC5: case mix_DEC6:
    mix_word_add_and_carry (mix_word_negative (val), get_reg_ (vm,r),
                            NULL, &val);
    val = mix_word_set_field (mix_fspec_new (1,3), MIX_WORD_ZERO, val);
    break;
  case mix_ENN1: case mix_ENN2: case mix_ENN3:
  case mix_ENN4: case mix_ENN5: case mix_ENN6:
    val = mix_word_set_field (mix_fspec_new (1,3), MIX_WORD_ZERO, val);
    /* fallthrough */
  case mix_ENNA: case mix_ENNX:
    mix_word_reverse_sign (val);
    break;
  default:
    fail_unexpected_ (vm);
  }
  set_reg_ (vm, r, val);
  inc_loc_ (vm);
  return TRUE;
}

static gboolean
cmp_handler_ (mix_vm_t *vm, const mix_ins_t *ins)
{
  g_assert (ins->opcode >= mix_opCMPA && ins->opcode <= mix_opCMPX);

  if ( ins->fspec == 0 ) { /* shortcut: +0 == -0 */
    set_cmp_ (vm, mix_EQ);
  } else {
    mix_word_t v = get_V_ (vm, ins);
    mix_word_t reg;
    mix_cmpflag_t flag;

    switch (ins->opcode) {
    case mix_opCMPA:
      reg = get_rA_ (vm);
      break;
    case mix_opCMPX:
      reg = get_rX_ (vm);
      break;
    default:
      reg = get_rI_ (vm, ins->opcode - mix_opCMPA);
      break;
    }
    reg = mix_word_get_field (ins->fspec, reg);
    mix_word_add_and_carry (reg, mix_word_negative (v), NULL, &reg);
    if ( mix_word_magnitude (reg) == MIX_WORD_ZERO ) flag = mix_EQ;
    else if ( mix_word_is_positive (reg) ) flag = mix_GREAT;
    else flag = mix_LESS;
    set_cmp_ (vm, flag);
  }
  inc_loc_ (vm);
  return TRUE;
}

ins_handler_t_ ins_handlers_[MIX_BYTE_MAX + 1] = {
  nop_handler_, add_handler_, add_handler_, mul_handler_, div_handler_,
  spc_handler_, sla_handler_, mov_handler_, lda_handler_, lda_handler_,
  lda_handler_, lda_handler_, lda_handler_, lda_handler_, lda_handler_,
  lda_handler_, lda_handler_, lda_handler_, lda_handler_, lda_handler_,
  lda_handler_, lda_handler_, lda_handler_, lda_handler_, sta_handler_,
  sta_handler_, sta_handler_, sta_handler_, sta_handler_, sta_handler_,
  sta_handler_, sta_handler_, sta_handler_, sta_handler_, jbs_handler_,
  ioc_handler_, inp_handler_, out_handler_, jrd_handler_, jmp_handler_,
  jpx_handler_, jpx_handler_, jpx_handler_, jpx_handler_, jpx_handler_,
  jpx_handler_, jpx_handler_, jpx_handler_, ina_handler_, ina_handler_,
  ina_handler_, ina_handler_, ina_handler_, ina_handler_, ina_handler_,
  ina_handler_, cmp_handler_, cmp_handler_, cmp_handler_, cmp_handler_,
  cmp_handler_, cmp_handler_, cmp_handler_, cmp_handler_,
};
