/* -*-c-*- -------------- mix_eval_t.c :
 * Test of mix_eval_t
 * ------------------------------------------------------------------
 *  Last change: Time-stamp: "01/02/20 00:26:15 jose"
 * ------------------------------------------------------------------
 * Copyright (C) 2000 Free Software Foundation, Inc.
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


#include <stdlib.h>
#include <mix_eval.h>
/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"

typedef struct test_case_t
{
  const gchar *expr;
  gint value;
  mix_eval_result_t result;
} test_case_t;

#define ok_case(exp,val) {exp, val, MIX_EVAL_OK}
#define err_case(exp,err) {exp, 0, err}

typedef struct sym_val_t
{
  const gchar *sym;
  mix_word_t value;
} sym_val_t;

#define new_symbol(sym,val) {sym, mix_word_new (val)}

int 
main(int argc, char **argv)
{
  size_t k;
  mix_eval_t *eval;
  mix_short_t loc = mix_short_new (30);
  sym_val_t symbols[] = {
    new_symbol ("s0", 43),
    new_symbol ("s1", -1234),
    new_symbol ("s2", 0),
    new_symbol ("s3", -20),
    new_symbol (NULL, 0)
  };
  
  test_case_t cases[] = {
    ok_case ("2343", 2343),
    ok_case ("-890", -890),
    ok_case ("15+1015", 1030),
    ok_case ("1-481", -480),
    ok_case ("2300/10", 230),
    ok_case ("24*3", 72),
    ok_case ("2:5", 21),
    ok_case ("1//3", 357913941),
    ok_case ("12+*", 42),
    ok_case ("***", 900),
    ok_case ("1:3*2-4", 18),
    ok_case ("-1+5*20/6", 13),
    ok_case ("-1000(0,2),1", 1),
    ok_case ("s0-s2*3", 129),
    ok_case ("s3**", -600),
    ok_case ("s3(3:5)", 20),
    ok_case ("-s1", 1234),
    ok_case ("s1/10+s0", 166),
    err_case ("foo", MIX_EVAL_UNDEF_SYM),
    err_case ("11--2", MIX_EVAL_SYNTAX),
    err_case ("s2*foo*3", MIX_EVAL_UNDEF_SYM),
    err_case ("12/32),1", MIX_EVAL_MIS_PAREN),
    err_case ("2000(88)", MIX_EVAL_INV_FSPEC),
    ok_case (NULL, 0)
  };
  
  INIT_TEST;
  g_print ("Entering mix_eval test...");
  eval = mix_eval_new ();

  for (k = 0; symbols[k].sym; ++k)
    mix_eval_set_symbol (eval, symbols[k].sym, symbols[k].value);
  
  for (k = 0; cases[k].expr; ++k) {
    mix_eval_result_t r = cases[k].result, s;
    g_print ("Evaluating \"%s\" = %d ...",
	       cases[k].expr, cases[k].value);
    
    s = mix_eval_expression_with_loc (eval, cases[k].expr, loc);
    g_assert (s == r);
    if ( s == MIX_EVAL_OK ) {
      mix_word_print (mix_eval_value (eval), "... ");
      g_print ("\n");
      g_assert (mix_eval_value (eval) == mix_word_new(cases[k].value));
    } else {
      g_print ("\n------->%s, at pos %d\n",
	       mix_eval_last_error_string (eval),
	       mix_eval_last_error_pos (eval));
    }
  }
  mix_eval_delete (eval);

  return EXIT_SUCCESS;
}


