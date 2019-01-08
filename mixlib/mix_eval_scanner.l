/* -*-c-*- ------------------ mix_eval_scanner.l :
 * scanner used by mix_eval_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2004, 2006, 2007, 2008, 2019 Free Software Foundation, Inc.
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

%{
#include <string.h>
#include "mix.h"
#include "xmix_eval.h"

#define YY_DECL							\
  mix_eval_result_t  mix_eval_expr (mix_eval_data_ *data)

/* keep track of current position in buffer */
#define YY_USER_ACTION yypos += yyleng;

#define RETURN_STATE(err)			\
  do {						\
    done = TRUE;                                \
    state = err;				\
    BEGIN (INITIAL);				\
  } while (FALSE)

#define CLEAN_UP()				\
  do {						\
    yy_delete_buffer (buffer);			\
    g_free (expr_cp);				\
 } while (FALSE)

static mix_word_t eval_binop_ (const gchar *op, mix_word_t x, mix_word_t y);
static int unput_word_ (mix_word_t word);

%}

%option nomain
%option caseless
%option array
%option stack
%option noinput
%option noyywrap
%option noyy_top_state
%option outfile="lex.yy.c"

%s  EVAL
%s  WEVAL

ws	[ \t]+
digit	[0-9]
letter	[A-Z]
number	[+-]?{digit}+
mixchar [0-9A-Z .,'')(+*/=$<>@;:\-]
locsymbol   {digit}H
flocsymbol  {digit}F
blocsymbol  {digit}B
symbol	{digit}*{letter}+[A-Z0-9]*
binops	"+"|"-"|"/"|"//"|":"
binop	{binops}|"*"
atexpr	{digit}+|{symbol}|\*
expr	[+-]?{atexpr}({binop}{1}{atexpr})*
fpart   \({expr}\)
wexpr   {expr}({fpart})?(,{expr}({fpart})?)*


%%

%{
  YY_BUFFER_STATE buffer;
  mix_word_t expr_val = MIX_WORD_ZERO, wexpr_val = MIX_WORD_ZERO;
  mix_word_t wexpr_val_tmp = MIX_WORD_ZERO;
  mix_eval_result_t state = MIX_EVAL_OK;
  gchar *expr_cp;
  gint yypos = -22; /* to account for padding */
  gboolean is_fp = FALSE, done = FALSE;
  g_assert (data != NULL && data->expr != NULL);
  /* make room enough to unput computed values */
  expr_cp = g_strdup_printf ("%-20s%s"," ", data->expr);
  buffer = yy_scan_string (expr_cp);
  data->errpos = -1;
%}


<*><<EOF>> {
  CLEAN_UP ();
  return MIX_EVAL_INTERN;
}

<INITIAL>{
  {ws} /* eat whitespace */
  . {
    if (!done && state == MIX_EVAL_OK) {
       yypos -= yyleng; yyless (0);
      yy_push_state (WEVAL);
    } else {
      CLEAN_UP ();
      if (state == MIX_EVAL_OK) return (MIX_EVAL_SYNTAX);
      data->errpos = yypos;
      return state;
    }
  }
  "\n" {
    CLEAN_UP();
    if (state == MIX_EVAL_OK)
	data->value = wexpr_val;
    else
	data->errpos = yypos;
    return state;
  }
}

<WEVAL>{
  {number}"(" {
    is_fp = TRUE;
    wexpr_val_tmp = mix_word_new (atol (yytext));
  }
  {number}")"  {
    glong val  = atol (yytext);
    if ( !is_fp ) {
      RETURN_STATE (MIX_EVAL_MIS_PAREN);
    }  else if ( val < 0 || val > MIX_BYTE_MAX
         || !mix_fspec_is_valid (mix_byte_new (val)) ) {
      RETURN_STATE (MIX_EVAL_INV_FSPEC);
    } else {
      is_fp = FALSE;
      wexpr_val = mix_word_store_field (mix_byte_new (val),
					wexpr_val_tmp,
					wexpr_val);
    }
  }
  {number}/({ws}*\n)|[,()] {
    wexpr_val = mix_word_new (atol (yytext));
  }
  {expr}/({ws}*\n)|[,()] {
    if (yytext[0] != '*')
      {
	yypos -= yyleng;
	yyless (0);
      }
    else
      {
	yypos -= yyleng - 1;
	expr_val = mix_short_to_word_fast (data->loc);
	yyless (1);
      }
    yy_push_state (EVAL);
  }
  ,/{expr} /* eat comma if followed by expression */
  [\t\n ] { /* ok if not inside an f-part */
    if ( is_fp ) {
      RETURN_STATE (MIX_EVAL_MIS_PAREN);
    }
    unput (yytext[yyleng-1]); --yypos;
    done = TRUE;
    yy_pop_state ();
  }
  .  RETURN_STATE (MIX_EVAL_SYNTAX);
}

<EVAL>{
  {binop}{digit}+ {
    const gchar *s = ( yytext[1] == '/' ) ? yytext+2 : yytext+1;
    mix_word_t value = mix_word_new (atol (s));
    expr_val = eval_binop_ (yytext, expr_val, value);
  }
  {binop}{symbol} {
    const gchar *s = ( yytext[1] == '/' ) ? yytext+2 : yytext+1;
    if ( !mix_symbol_table_is_defined (data->table, s) ) {
      RETURN_STATE (MIX_EVAL_UNDEF_SYM);
    }
    expr_val = eval_binop_ (yytext, expr_val,
			    mix_symbol_table_value (data->table, s));
  }
  {binop}"*" {
    expr_val = eval_binop_ (yytext, expr_val,
			    mix_short_to_word_fast (data->loc));
  }
  "*"  yypos -= unput_word_ (mix_short_to_word_fast (data->loc));
  {number}    expr_val = mix_word_new (atol (yytext));
  {symbol} {
    if ( !mix_symbol_table_is_defined (data->table, yytext) ) {
      RETURN_STATE (MIX_EVAL_UNDEF_SYM);
    }
    expr_val = mix_symbol_table_value (data->table, yytext);
  }
  [,)(\n\t ]   {
    unput (yytext[0]); --yypos;
    yypos -= unput_word_ (expr_val);
    yy_pop_state ();
  }

  .   RETURN_STATE (MIX_EVAL_SYNTAX);
}


%%

static mix_word_t
eval_binop_ (const gchar *op, mix_word_t x, mix_word_t y)
{
  mix_word_t result = MIX_WORD_ZERO;
  switch (op[0])
    {
    case '+':
      result = mix_word_add (x,y);
      break;
    case '-':
      result = mix_word_sub (x,y);
      break;
    case '*':
      mix_word_mul (x, y, NULL, &result);
      break;
    case ':':
      {
	mix_word_t a;
	mix_word_mul (x, 8, NULL, &a);
	result = mix_word_add (a, y);
	break;
      }
    case '/':
      if ( strlen (op) > 1 && op[1] == '/' ) {
	mix_word_div (x,MIX_WORD_ZERO,y, &result, NULL);
      } else {
	mix_word_div (MIX_WORD_ZERO, x, y, &result, NULL);
      }
      break;
    default:
      g_assert_not_reached ();
    }
  return result;
}

static int
unput_word_ (mix_word_t word)
{
  gchar *value;
  gint k, result;
  value = g_strdup_printf ("%s%ld",
                           mix_word_is_negative (word)? "-":"+",
                           mix_word_magnitude (word));
  result = strlen (value);
  for (k = result - 1; k >= 0; --k)
    unput (value[k]);
  g_free (value);
  return result;
}
