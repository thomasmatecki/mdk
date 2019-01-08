/* ---------------------- mix_vm_ins_t.c :
 * Tests for the virtual machine instruction handlers.
 * ------------------------------------------------------------------
** Copyright (C) 2000, 2004 Free Software Foundation, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
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


#include <stdlib.h>
/* #define VERBOSE_TEST */
#include "test.h"
#include "mix_vm.h"
#include "mix_vm_dump.h"

typedef struct
{
  mix_word_t rA_b, rA_a;
  mix_word_t rX_b, rX_a;
  mix_short_t rJ_b, rJ_a;
  mix_short_t rI_b[6], rI_a[6];
  gboolean over_b, over_a;
  mix_cmpflag_t cmp_b, cmp_a;
  mix_address_t begin, end;
  mix_word_t  *cells_b, *cells_a;
  const mix_ins_t *ins;
} test_desc_t;

static void
set_cells_(test_desc_t *t, mix_address_t begin, mix_address_t end)
{
  g_assert(begin <= end);
  t->begin = begin;
  t->end = end;
  t->cells_b = g_new(mix_word_t,end-begin);
  t->cells_a = g_new(mix_word_t,end-begin);
}

static void
free_cells_(test_desc_t *t)
{
  g_assert(t);
  g_free(t->cells_a);
  g_free(t->cells_b);
  t->cells_a = t->cells_b = NULL;
  t->begin = t->end = 0;
}


static void
fill_test_desc_(test_desc_t *t, const mix_vm_t *vm, const mix_ins_t *ins)
{
  guint k;
  g_assert(t);
  g_assert(vm);

  t->rA_b = t->rA_a = mix_vm_get_rA(vm);
  t->rX_b = t->rX_a = mix_vm_get_rX(vm);
  t->rJ_b = t->rJ_a = mix_vm_get_rJ(vm);
  for ( k = 0; k < 6; ++k )
    t->rI_b[k] = t->rI_a[k] = mix_vm_get_rI(vm,k+1);
  t->cmp_b = t->cmp_a = mix_vm_get_cmpflag(vm);
  t->over_b = t->over_a = mix_vm_get_overflow(vm);

  for (k = 0; k < t->end-t->begin; ++k)
    t->cells_a[k] = t->cells_b[k] =
      mix_vm_get_addr_contents(vm,t->begin+k);
  t->ins = ins;
}


static void
run_test_(test_desc_t *t, mix_vm_t *vm, mix_dump_context_t *dc)
{
  guint k;
  g_assert(t);
  g_assert(vm);

  mix_vm_set_rA(vm, t->rA_b);
  mix_vm_set_rX(vm, t->rX_b);
  mix_vm_set_rJ(vm, t->rJ_b);
  for (k = 0; k < 6; ++k) mix_vm_set_rI(vm, k+1, t->rI_b[k]);
  for (k = t->begin; k < t->end; ++k)
    mix_vm_set_addr_contents(vm, k, t->cells_b[k-t->begin]);
  mix_vm_set_cmpflag(vm, t->cmp_b);
  mix_vm_set_overflow(vm, t->over_b);

  mix_ins_print(t->ins);
  if (dc) {
    mix_dump_context_range(dc, t->begin, t->end);
    mix_vm_dump(vm,dc);
  }
  k = mix_vm_exec_ins(vm, t->ins);
  if (dc) mix_vm_dump(vm, dc);
  g_assert(k == TRUE);
  g_assert(mix_vm_get_rA(vm) == t->rA_a);
  g_assert(mix_vm_get_rX(vm) == t->rX_a);
  for (k = 0; k < 6; ++k) g_assert(mix_vm_get_rI(vm, k+1) == t->rI_a[k]);
  g_assert(mix_vm_get_cmpflag(vm) == t->cmp_a);
  g_assert(mix_vm_get_overflow(vm) == t->over_a);
  for (k = t->begin; k < t->end; ++k)
    g_assert(mix_vm_get_addr_contents(vm, k) == t->cells_a[k-t->begin]);
}



static void
test_arithmetics_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;

  g_print("\nTesting arithmetic instructions...\n");
  mix_vm_reset(vm);
  mix_ins_fill_from_id(ins,mix_ADD);
  ins.index = 0;
  ins.address = 1000;
  mix_vm_set_rA(vm,mix_word_new_b(19,18,1,2,22));
  mix_vm_set_addr_contents(vm,1000,mix_word_new_b(1,36,5,0,50));
  set_cells_(&test,1000,1001);
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(20,54,6,3,8);
  run_test_(&test, vm, dc);

  mix_vm_set_rA(vm, mix_word_new_bn(0,0,0,0,1));
  mix_vm_set_addr_contents(vm, 1000, mix_word_new_b(0,0,0,0,1));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = MIX_WORD_MINUS_ZERO;
  run_test_(&test, vm, dc);

  mix_vm_set_rA(vm, mix_word_new_b(0,0,0,0,1));
  mix_vm_set_addr_contents(vm, 1000, mix_word_new_bn(0,0,0,0,1));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = MIX_WORD_ZERO;
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins,mix_SUB);
  mix_vm_set_rA(vm,mix_word_new_bn(19,18,0,0,9));
  mix_vm_set_addr_contents(vm,1000,mix_word_new_bn(31,16,2,22,0));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(11,62,2,21,55);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins,mix_MUL);
  mix_vm_set_rA(vm,mix_word_new_b(1,1,1,1,1));
  mix_vm_set_addr_contents(vm,1000, mix_word_new_b(1,1,1,1,1));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(0,1,2,3,4);
  test.rX_a = mix_word_new_b(5,4,3,2,1);
  run_test_(&test, vm, dc);

  ins.fspec = mix_fspec_new(1,1);
  mix_vm_set_rA(vm,mix_word_new_bn(0,0,0,1,48));
  mix_vm_set_addr_contents(vm,1000,mix_word_new_bn(2,16,2,22,0));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = MIX_WORD_MINUS_ZERO;
  test.rX_a = mix_word_new_bn(0,0,0,3,32);
  run_test_(&test, vm, dc);

  mix_vm_set_rA(vm,mix_word_new_bn(0,0,0,1,48));
  mix_vm_set_addr_contents(vm,1000,mix_word_new_b(2,0,34,33,1));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = MIX_WORD_MINUS_ZERO;
  test.rX_a = mix_word_new_bn(0,0,0,3,32);
  run_test_(&test, vm, dc);

  ins.fspec = mix_fspec_new(0,5);
  mix_vm_set_rA(vm,mix_word_new_bn(50,0,1,48,4));
  mix_vm_set_addr_contents(vm,1000,mix_word_new_bn(2,0,0,0,0));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(1,36,0,3,32);
  test.rX_a = mix_word_new_b(8,0,0,0,0);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins,mix_DIV);
  mix_vm_set_rA(vm,MIX_WORD_ZERO);
  mix_vm_set_rX(vm,mix_word_new_b(0,0,0,0,17));
  mix_vm_set_addr_contents(vm,1000, mix_word_new_b(0,0,0,0,3));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(0,0,0,0,5);
  test.rX_a = mix_word_new_b(0,0,0,0,2);
  run_test_(&test, vm, dc);

  mix_vm_set_rA(vm,MIX_WORD_ZERO);
  mix_vm_set_rX(vm,mix_word_new_bn(0,0,0,0,17));
  mix_vm_set_addr_contents(vm,1000, mix_word_new_b(0,0,0,0,3));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(0,0,0,0,5);
  test.rX_a = mix_word_new_b(0,0,0,0,2);
  run_test_(&test, vm, dc);

  mix_vm_set_rA(vm, MIX_WORD_MINUS_ZERO);
  mix_vm_set_rX(vm, mix_word_new_b(19,19,0,3,1));
  mix_vm_set_addr_contents(vm,1000, mix_word_new_bn(0,0,0,2,0));
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = mix_word_new_b(0,9,41,32,1);
  test.rX_a = mix_word_new_bn(0,0,0,1,1);
  run_test_(&test, vm, dc);

  free_cells_(&test);
}

static void
test_shift_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;

  g_print("Testing shift instructions...\n");
  mix_vm_reset(vm);
  set_cells_(&test,0,0);
  fill_test_desc_(&test,vm,&ins);
  mix_ins_fill_from_id(ins,mix_SRAX);
  ins.index = 0;
  ins.address = 1;
  test.rA_b = mix_word_new_b(1,2,3,4,5);
  test.rX_b = mix_word_new_bn(6,7,8,9,10);
  test.rA_a = mix_word_new_b(0,1,2,3,4);
  test.rX_a = mix_word_new_bn(5,6,7,8,9);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SLA);
  ins.address = 2;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new_b(2,3,4,0,0);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SRC);
  ins.address = 4;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new_b(6,7,8,9,2);
  test.rX_a = mix_word_new_bn(3,4,0,0,5);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SRA);
  ins.address = 2;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new_b(0,0,6,7,8);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SLC);
  ins.address = 501;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new_b(0,6,7,8,3);
  test.rX_a = mix_word_new_bn(4,0,0,5,0);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SLB);
  ins.index = 0;
  ins.address = 3;
  fill_test_desc_(&test, vm, &ins);
  test.rA_b = mix_word_new(06543217654);
  test.rX_b = mix_word_new(03217654321);
  test.rA_a = mix_word_new(05432176543);
  test.rX_a = mix_word_new(02176543210);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_SRB);
  ins.address = 6;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new(00054321765);
  test.rX_a = mix_word_new(04321765432);
  run_test_(&test, vm, dc);
}

static void
test_spc_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;

  g_print("Testing special instructions...\n");
  mix_vm_reset(vm);
  set_cells_(&test,0,0);
  fill_test_desc_(&test,vm,&ins);
  mix_ins_fill_from_id(ins,mix_NUM);
  ins.index = 0;
  ins.address = 0;
  test.rA_b = mix_word_new_bn(0,0,31,32,39);
  test.rX_b = mix_word_new_b(37,57,47,30,30);
  test.rA_a = mix_word_negative(12977700);
  test.rX_a = test.rX_b;
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_INCA);
  ins.address = 1;
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_negative(12977699);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_CHAR);
  fill_test_desc_(&test, vm, &ins);
  test.rA_a = mix_word_new_bn(30,30,31,32,39);
  test.rX_a = mix_word_new_b(37,37,36,39,39);
  run_test_(&test, vm, dc);

  mix_ins_fill_from_id(ins, mix_HLT);
  fill_test_desc_(&test, vm, &ins);
  run_test_(&test, vm, dc);
  g_assert(mix_vm_is_halted(vm));

}

static void
test_move_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;
  guint k;

  g_print("Testing move instruction...\n");
  mix_vm_reset(vm);
  set_cells_(&test,0,10);
  fill_test_desc_(&test,vm,&ins);
  mix_ins_fill_from_id(ins,mix_MOVE);

  ins.index = 0;
  ins.address = 0;
  ins.fspec = 5;
  for ( k = 0; k < 5; ++k )
    test.cells_b[k] = test.cells_a[k+5] = test.cells_a[k] =mix_word_new(100*k);

  test.rI_b[0] = 5;
  test.rI_a[0] = 10;
  run_test_(&test,vm,dc);

  free_cells_(&test);
}



static void
test_load_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;
  mix_ins_id_t ids[4] = {mix_LDA, mix_LDX, mix_LDAN, mix_LDXN};
  mix_word_t r_a[14] = {
    mix_word_new_bn(1,16,3,5,4),
    mix_word_new_b(1,16,3,5,4),
    mix_word_new_b(0,0,3,5,4),
    mix_word_new_bn(0,0,1,16,3),
    mix_word_new_b(0,0,0,0,5),
    mix_word_new_b(0,0,0,0,1),
    MIX_WORD_MINUS_ZERO,
    mix_word_new_b(1,16,3,5,4),
    mix_word_new_bn(1,16,3,5,4),
    mix_word_new_bn(0,0,3,5,4),
    mix_word_new_b(0,0,1,16,3),
    mix_word_new_bn(0,0,0,0,5),
    mix_word_new_bn(0,0,0,0,1),
    MIX_WORD_ZERO
  };
  mix_fspec_t fs[11] = {5,13,29,3,36,9,0,0,0,0,0};
  mix_address_t a_a[11] = {
    MIX_SHORT_MINUS_ZERO,
    mix_short_new_bn(0,1),
    mix_short_new_bn(1,16),
    mix_short_new_bn(16,3),
    mix_short_new_bn(3,5),
    mix_short_new_bn(5,4),
    mix_short_new_b(1,16),
    mix_short_new_b(16,3),
    mix_short_new_b(5,4),
    mix_short_new_b(5,4),
    mix_short_new_b(3,5)
  };
  mix_word_t val = mix_word_new_bn(1,16,3,5,4);
  gint j;

  g_print("Testing load instructions...\n");
  set_cells_(&test,2000,2001);
  ins.index = 1;
  ins.address = mix_short_negative(50);

  mix_vm_reset(vm);
  mix_vm_set_addr_contents(vm, 2000, val);

  for (j = 0; j < 4; ++j)
    {
      gint k;
      mix_ins_fill_from_id(ins,ids[j]);
      for ( k = 0; k < 7; ++k ) {
	fill_test_desc_(&test,vm,&ins);
	ins.fspec = fs[k];
	switch (ids[j])
	  {
	  case mix_LDA: test.rA_a = r_a[k]; break;
	  case mix_LDX: test.rX_a = r_a[k]; break;
	  case mix_LDAN: test.rA_a = r_a[k + 7]; break;
	  case mix_LDXN: test.rX_a = r_a[k + 7]; break;
	  default: g_assert_not_reached();
	  }
	test.rI_b[0] = test.rI_a[0] = 2050;
	run_test_(&test, vm, dc);
      }
    }

  ins.index = 0;
  ins.address = 2000;
  fs[0] = 0; fs[1] = 1; fs[2] = 2; fs[3] = 3; fs[4] = 4; fs[5] = 5;
  fs[6] = 10; fs[7] = 11; fs[8] = 37; fs[9] = 29; fs[10] = 12;

  mix_vm_reset(vm);
  mix_vm_set_addr_contents(vm, 2000, val);
  for ( j = 0; j < 14; j++ )
    {
      guint k;
      if (j == 6 || j == 7 ) continue; /* mix_LDX, mix_LDAN */
      mix_ins_fill_from_id(ins, mix_LD1 + j);
      for (k = 0; k < 11; ++k)
	{
	  fill_test_desc_(&test, vm, &ins);
	  ins.fspec = fs[k];
	  if ( j < 6  )
	    test.rI_a[j] = a_a[k];
	  else /* mix_LDiN */
	    test.rI_a[j-8] = mix_short_negative (a_a[k]);
	  run_test_(&test, vm, dc);
	}
    }

  free_cells_(&test);
}


static void
test_store_(mix_vm_t *vm, mix_dump_context_t *dc)
{
  test_desc_t test;
  mix_ins_t ins;
  mix_word_t reg = mix_word_new_b(6,7,8,9,0);
  mix_word_t add = mix_word_new_bn(1,2,3,4,5);
  mix_word_t addr[6] = { mix_word_new_b(6,7,8,9,0),
			  mix_word_new_bn(6,7,8,9,0),
			  mix_word_new_bn(1,2,3,4,0),
			  mix_word_new_bn(1,0,3,4,5),
			  mix_word_new_bn(1,9,0,4,5),
			  mix_word_new_b(0,2,3,4,5)};
  mix_word_t addri[6] = { mix_word_new_b(0,0,0,9,0),
			  mix_word_new_bn(0,0,0,9,0),
			  mix_word_new_bn(1,2,3,4,0),
			  mix_word_new_bn(1,0,3,4,5),
			  mix_word_new_bn(1,9,0,4,5),
			  mix_word_new_b(0,2,3,4,5)};
  mix_word_t addrz[6] = { mix_word_new_b(0,0,0,0,0),
			  mix_word_new_bn(0,0,0,0,0),
			  mix_word_new_bn(1,2,3,4,0),
			  mix_word_new_bn(1,0,3,4,5),
			  mix_word_new_bn(1,0,0,4,5),
			  mix_word_new_b(0,2,3,4,5)};
  mix_fspec_t fs[6] = {5,13,45,18,19,1};
  gint i,j;

  g_print("Testing store instructions...\n");

  set_cells_(&test,2000,2001);
  ins.index = 0;
  ins.address = 2000;

  mix_vm_reset(vm);
  fill_test_desc_(&test,vm,&ins);
  test.rA_a = test.rA_b = test.rX_a = test.rX_b = reg;
  test.rJ_a = test.rJ_b = mix_word_to_short(reg);
  for (j = 0; j < 6; ++j)
    test.rI_a[j] = test.rI_b[j] = test.rJ_a;
  test.cells_b[0] = add;

  for (i = 0; i < 10; ++i)
    {
      mix_ins_fill_from_id(ins,mix_STA+i);
      for (j = 0; j < 6; ++j)
	{
	  ins.fspec = fs[j];
	  if (i == 0 || i == 7 ) /* mix_STA, mix_STX */
	    test.cells_a[0] = addr[j];
	  else if ( i < 9 ) /* mix_STi, mix_STJ */
	    test.cells_a[0] = addri[j];
	  else /* mix_STZ */
	    test.cells_a[0] = addrz[j];
	  run_test_(&test,vm,dc);
	}
    }

  free_cells_(&test);
}



int
main(int argc, const char **argv)
{
  mix_vm_t *vm;
  mix_dump_context_t *dc;

  INIT_TEST;

  vm = mix_vm_new();

#ifdef VERBOSE_TEST
  dc = mix_dump_context_new(MIX_DUMP_DEF_CHANNEL, 0, 0, MIX_DUMP_ALL);
#else
  dc = NULL;
#endif

  test_arithmetics_(vm, dc);
  test_shift_(vm, dc);
  test_spc_(vm,dc);
  test_move_(vm,dc);
  test_load_(vm,dc);
  test_store_(vm,dc);

  mix_vm_delete(vm);

#ifdef VERBOSE_TEST
  mix_dump_context_delete(dc);
#endif

  return EXIT_SUCCESS;
}
