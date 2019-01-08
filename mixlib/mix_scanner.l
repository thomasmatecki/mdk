/* -*-c-*- -------------- mix_scanner.l :
 * Lexical scanner used by mix_parser_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2003, 2004, 2006, 2007, 2008, 2009, 2019 Free Software Foundation, Inc.
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
#include <ctype.h>
#include <string.h>

#include "mix.h"
#include "xmix_parser.h"

#define YY_DECL mix_parser_err_t mix_flex_scan (mix_parser_t *parser)

#define RESET()					\
  do {						\
    mix_ins_fill_from_id (ins, mix_NOP);	\
    ins.address = 0;				\
    ins.index = 0;				\
    nof = FALSE;				\
    lsf = FALSE;				\
    if (symbol != NULL )			\
     {						\
       g_free (symbol);				\
       symbol = NULL;				\
     }						\
    if (lsymbol != NULL)			\
     {						\
       g_free (lsymbol);			\
       lsymbol = NULL;				\
     }                       			\
  } while (FALSE)

#define NEXT()						\
  do {							\
    if (lsymbol != NULL)				\
       mix_parser_manage_local_symbol (parser,lsymbol,  \
				       loc);    	\
    parser->loc_count++;				\
    RESET ();						\
    ++lineno;						\
    BEGIN (INITIAL);					\
  } while (FALSE)

#define ADD_INS()				\
  do {						\
    mix_parser_add_ins (parser, &ins, lineno);	\
    NEXT ();					\
  } while (FALSE)

#define ADD_RAW(value,is_con)                           \
  do {                                                  \
    mix_parser_add_raw (parser, value, lineno, is_con); \
    NEXT ();                                            \
  } while (FALSE)


#define ENTER_EVAL()                                            \
  do {                                                          \
    if (yytext[0] != '*')                                       \
      {                                                         \
        expr_val = MIX_WORD_ZERO;                               \
        yyless (0);                                             \
      }                                                         \
    else                                                        \
      {                                                         \
        expr_val = mix_short_to_word_fast (parser->loc_count);  \
	yyless (1);                                             \
      }                                                         \
    yy_push_state (EVAL);                                       \
  } while (FALSE)

#define ENTER_WEVAL(s)				\
  do {						\
    wexpr_val = MIX_WORD_ZERO;			\
    wexpr_val_tmp = MIX_WORD_ZERO;		\
    is_fp = FALSE;                              \
    yyless (s);					\
    yy_push_state (WEVAL);			\
  } while (FALSE)

#define RETURN_ERROR(error, comment)                                    \
  do {                                                                  \
    int c;                                                              \
    mix_parser_log_error (parser, error, lineno, comment, FALSE);	\
    while ( (c = input ()) != '\n' && c != EOF ) ;                      \
    if ( c == EOF ) return error; else ++lineno;                        \
    RESET ();                                                           \
    BEGIN (INITIAL);                                                    \
  } while (FALSE)


static mix_word_t eval_binop_ (const gchar *op, mix_word_t x, mix_word_t y);

static void unput_word_ (mix_word_t word);

%}

%option nomain
%option caseless
%option array
%option stack
%option noyywrap
%option noyy_top_state
%option outfile="lex.yy.c"

%s  LOC
%s  OP
%s  ADDRESS
%s  INDEX
%s  FSPEC
%s  EVAL
%s  WEVAL
%s  ORIG
%s  CON
%s  EQU
%s  END

ws	[ \t]
digit	[0-9]
letter	[A-Z]
number	[+-]?{digit}+
mixchar [0-9A-Z .,'')(+*/=$<>@;:\-]
locsymbol   {digit}H
flocsymbol  {digit}F
blocsymbol  {digit}B
symbol	{digit}*{letter}+[A-Z0-9]*
binop	"+"|"-"|"*"|"/"|"//"|":"
atexpr	{digit}+|{symbol}|\*
expr	[+-]?{atexpr}({binop}{1}{atexpr})*
fpart   \({expr}\)
wexpr   {expr}({fpart})?(,{expr}({fpart})?)*


%%

%{

  mix_ins_t ins;
  gboolean nof = FALSE, is_fp = FALSE, end = FALSE, lsf = FALSE;
  mix_word_t expr_val = MIX_WORD_ZERO, wexpr_val = MIX_WORD_ZERO,
  wexpr_val_tmp = MIX_WORD_ZERO;
  gchar *symbol = NULL, *lsymbol = NULL;
  mix_address_t loc = MIX_SHORT_ZERO;
  guint lineno = 1;
  mix_ins_fill_from_id (ins, mix_NOP);
  ins.address = 0;
  ins.index = 0;
  parser->err_line = 0;
#ifdef FLEX_DEBUG
  yy_flex_debug = getenv("FLEX_DEBUG");
#endif
  yyin = mix_file_to_FILE (parser->in_file);
  yyrestart (yyin);
%}


<*><<EOF>> {
  mix_parser_log_error (parser, MIX_PERR_UNEX_EOF, lineno, NULL, FALSE);
  return MIX_PERR_UNEX_EOF;
}

<INITIAL>{
  ^\*.*	/* eat comments */
  .	{
    if (end)
      {
        return parser->status;
      }
    yyless (0);
    BEGIN (LOC);
  }
  \n {
    ++lineno;
    if (end)
      {
        return parser->status;
      }
  }
}

<LOC>{
  {ws}+	BEGIN (OP); /* LOC field is empty */
  {locsymbol} { /* manage local symbol */
    loc = get_ploc_ (parser);
    lsymbol = g_strdup (yytext);
    if ( lsymbol == NULL ) {
      mix_parser_log_error (parser, MIX_PERR_INTERNAL, lineno, NULL, FALSE);
      return MIX_PERR_INTERNAL;
    }
    BEGIN (OP);
  }
  {locsymbol}/({ws}+EQU) {/* local symbol with value */
    loc = get_ploc_ (parser);
    symbol = g_strdup (yytext);
    lsymbol = g_strdup (yytext);
    if ( symbol == NULL || lsymbol == NULL) {
      mix_parser_log_error (parser, MIX_PERR_INTERNAL, lineno, NULL, FALSE);
      return MIX_PERR_INTERNAL;
    }
    symbol[1] = 'B'; /* this will be referred as nB afterwards */
    BEGIN (OP);
  }
  {flocsymbol}|{blocsymbol} RETURN_ERROR (MIX_PERR_UNEX_LOC, yytext);
  {symbol}/({ws}+EQU) { /* store symbol name for future definition */
    symbol = g_strdup (yytext);
    if ( symbol == NULL ) {
      mix_parser_log_error (parser, MIX_PERR_INTERNAL, lineno, NULL, FALSE);
      return MIX_PERR_INTERNAL;
    }
    BEGIN (OP);
  }
  {symbol} { /* define a new symbol */
    mix_parser_err_t err;
    if ( mix_get_id_from_string (yytext) != mix_INVALID_INS )
      mix_parser_log_error (parser, MIX_PERR_SYM_INS, lineno, yytext, TRUE);
    if ( (err = mix_parser_define_symbol_here (parser,yytext)) != MIX_PERR_OK )
      mix_parser_log_error (parser, err, lineno, yytext, FALSE);
    BEGIN (OP);
  }
  .	RETURN_ERROR (MIX_PERR_INV_LOC, yytext);
  \n 	++lineno; /* empty line */
}

<OP>{
  {ws}+ /* eat leading whitespace */
  \n    RETURN_ERROR (MIX_PERR_NOOP, NULL);
  ORIG{ws}+  BEGIN (ORIG);
  CON{ws}+   BEGIN (CON);
  EQU{ws}+   BEGIN (EQU);
  END{ws}+   BEGIN (END);
  ALF{ws}+\"{mixchar}{0,5}\"{ws}+.*\n |
  ALF{ws}+\"{mixchar}{0,5}\"{ws}*\n {
    mix_byte_t bytes[5];
    mix_word_t value;
    guint k, j = 4;

    while ( yytext[j++] != '\"' ) ;
    for ( k = j; k < 5+j && yytext[k] != '\"'; ++k )
      bytes[k-j] = mix_ascii_to_char (yytext[k]);
    if ( k-j < 5 )
      {
	mix_parser_log_error (parser, MIX_PERR_SHORT_ALF, lineno, NULL, TRUE);
	/* Fill with spaces */
	for (; k < 5+j; k++)
	  bytes[k-j] = mix_ascii_to_char (' ');
      }
    else if ( yytext[k] != '\"' )
      mix_parser_log_error (parser, MIX_PERR_LONG_ALF, lineno, NULL, TRUE);

    value = mix_bytes_to_word (bytes, 5);
    ADD_RAW (value, FALSE);
   }
  ALF{ws}*\n {
    mix_byte_t bytes[5];
    memset (bytes, mix_ascii_to_char (' '), 5);
    mix_word_t value = mix_bytes_to_word (bytes, 5);
    ADD_RAW (value, FALSE);
  }
  ALF{ws}+({mixchar}{1,5}) {
    mix_byte_t bytes[5];
    mix_word_t value;
    int i, n;
    for (n = 3; n < yyleng; n++)
      if (!isspace (yytext[n]))
	break;

    for (i = 0; i < 5 && n < yyleng; i++, n++)
      bytes[i] = mix_ascii_to_char (yytext[n]);

    for (; i < 5; i++)
      bytes[i] = mix_ascii_to_char (' ');

    value = mix_bytes_to_word (bytes, 5);
    ADD_RAW (value, FALSE);
  }
            /* ALF " " */
  [A-Z0-9]+{ws}*/\n |
  [A-Z0-9]+{ws}+ {
    mix_ins_id_t id = mix_get_id_from_string (g_strchomp (yytext));
    if ( id == mix_INVALID_INS )
      mix_parser_log_error (parser, MIX_PERR_INV_OP, lineno, yytext, FALSE);
    else {
      mix_ins_fill_from_id (ins, id);
      nof = mix_ins_id_is_extended (id);
    }
    BEGIN (ADDRESS);
  }
  {expr} RETURN_ERROR (MIX_PERR_INV_OP, yytext);
  .  RETURN_ERROR (MIX_PERR_INV_OP, yytext);
}


<ORIG>{
  {number}{ws}*\n   |
  {number}{ws}+.*\n {
    mix_word_t value = mix_word_new (atol (yytext));
    parser->loc_count = mix_word_to_short_fast (value);
    ++lineno;
    BEGIN (INITIAL);
  }
}

<CON>{
  {number}{ws}*\n |
  {number}{ws}+.*\n  {
    mix_word_t value = mix_word_new (atol (yytext));
    ADD_RAW (value, TRUE);
  }
}

<EQU>{
  {number}{ws}*\n   |
  {number}{ws}+.*\n {
    gint def = MIX_PERR_MIS_SYM;
    if (symbol)
      {
        mix_word_t value = mix_word_new (atol (yytext));
        def = mix_parser_define_symbol_value (parser, symbol, value);
      }
    switch (def)
      {
      case MIX_SYM_DUP:  RETURN_ERROR (MIX_PERR_DUP_SYMBOL, symbol); break;
      case MIX_SYM_LONG:  RETURN_ERROR (MIX_PERR_LONG_SYMBOL, symbol); break;
      case MIX_PERR_MIS_SYM:
        mix_parser_log_error (parser, def, lineno, NULL, TRUE);
        break;
      default: break;
      }
    ++lineno;
    BEGIN (INITIAL);
  }
}

<END>{
  {number}{ws}*\n   |
  {number}{ws}+.*\n {
    parser->start = mix_short_new (atol (yytext));
    parser->end = parser->loc_count;
    end = TRUE;
    if ( parser->status == MIX_PERR_NOCOMP ) parser->status = MIX_PERR_OK;
    RESET ();
    BEGIN (INITIAL);
    return parser->status;
  }
}

<ORIG,CON,EQU,END>{
  {wexpr} ENTER_WEVAL (0);
  . RETURN_ERROR (MIX_PERR_INV_OP, yytext);
}


<ADDRESS,INDEX,FSPEC>{locsymbol}  RETURN_ERROR (MIX_PERR_UNEX_LOC, yytext);

<ADDRESS>{
  =/[+-]?{number}=  lsf = TRUE;
  =/{expr}=     lsf = TRUE;
  ={wexpr}=  { lsf = TRUE; ENTER_WEVAL (1); }
  [+-]?{number}={ws}*\n   |
  [+-]?{number}={ws}+.*\n {
    if (!lsf) RETURN_ERROR (MIX_PERR_INV_ADDRESS, yytext);
    mix_parser_define_ls (parser, mix_word_new (atol (yytext)));
    lsf = FALSE;
    ADD_INS ();
  }
  [+-]?{number}=[(,] {
    int pos = yyleng - 3;
    if (!lsf) RETURN_ERROR (MIX_PERR_INV_ADDRESS, yytext);
    unput (yytext[yyleng - 1]);
    while (pos >= 0) {
      unput (yytext[pos]);
      --pos;
    }
  }
  [+-]?{number}{ws}+.*\n |
  [+-]?{number}[(,\n] {
    ins.address = mix_short_new (atol (yytext));
    switch ( yytext[yyleng-1] ) {
    case '(' : BEGIN (FSPEC); break;
    case ',' : BEGIN (INDEX); break;
    case '\n' : ADD_INS (); break;
    default: g_assert_not_reached ();
    }
  }
  ([+-]?{symbol})/[(,\n\t ] {
    gboolean neg = (yytext[0] == '-');
    const gchar *s = (neg || yytext[0] == '+')? yytext+1 : yytext;
    if ( !mix_symbol_table_is_defined (parser->symbol_table, s) )
      {
	mix_parser_set_future_ref (parser, s);
	if (neg)
	  mix_parser_log_error (parser, MIX_PERR_UNDEF_SYM, lineno, s, TRUE);
	unput (neg? '1':'0');
      }
    else
      {
	mix_word_t v = mix_symbol_table_value (parser->symbol_table, s);
	if ( neg ) mix_word_reverse_sign (v);
	unput_word_ (v);
      }
  }
  {expr}/[(,=\n\t ] ENTER_EVAL ();
  \n    ADD_INS ();
  .	RETURN_ERROR (MIX_PERR_INV_ADDRESS, yytext);
}


<INDEX>{
  {number}[\n(\t ] {
    int end = yytext[yyleng-1];
    ins.index = mix_byte_new (atol (yytext));
    if ( end == '\n' )
      ADD_INS ();
    else if ( end == '(' )
      BEGIN (FSPEC);
    else
      { /* eat rest of line (comment) */
	while ( (end = input()) != '\n' && end != EOF ) ;
	if ( end == '\n' ) ADD_INS ();
	else RETURN_ERROR (MIX_PERR_UNEX_EOF, NULL);
      }
  }
  {expr}/[\n(\t ]  ENTER_EVAL ();
  \n	{
    mix_parser_log_error (parser, MIX_PERR_INV_IDX, lineno++, NULL, FALSE);
    RESET ();
    BEGIN (INITIAL);
  }
  .	RETURN_ERROR (MIX_PERR_INV_IDX, yytext);
}

<FSPEC>{
  {number}")"(({ws}+.*\n)|\n) {
    glong val  = atol (yytext);

    if (val < 0 || val > MIX_BYTE_MAX)
      RETURN_ERROR (MIX_PERR_INV_FSPEC, NULL);

    if (ins.opcode != mix_opMOVE
        && ins.opcode != mix_opNOP
        && ( ins.opcode < mix_opJBUS || ins.opcode > mix_opJXx )
        && !mix_fspec_is_valid (mix_byte_new (val)) )
      {
        gchar *spec = g_strdup_printf ("%d", (int)val);
        mix_parser_log_error (parser, MIX_PERR_INV_FSPEC, lineno, spec, TRUE);
        g_free (spec);
      }

    if (nof)
      {
        mix_parser_log_error (parser, MIX_PERR_INV_FSPEC,
                              lineno, "ignored", TRUE);
      }
    else
      {
	ins.fspec = mix_byte_new (val);
        if (lsf)
          {
            mix_parser_define_ls (parser,
                                  mix_short_to_word_fast (ins.address));
            ins.address = MIX_WORD_ZERO;
            lsf = FALSE;
          }
	ADD_INS ();
      }
  }

  {expr}/")"(({ws}+.*\n)|\n)  {
    ENTER_EVAL ();
  }

  {expr}")".+ {
    RETURN_ERROR (MIX_PERR_INV_FSPEC, g_strdup_printf ("(%s", yytext));
  }

  .	RETURN_ERROR (MIX_PERR_INV_FSPEC, yytext);
}


<EVAL>{
  {binop}{digit}+ {
    const gchar *s = ( yytext[1] == '/' ) ? yytext+2 : yytext+1;
    mix_word_t value = mix_word_new (atol (s));
    expr_val = eval_binop_ (yytext, expr_val, value);
  }
  {binop}{symbol} {
    const gchar *s = ( yytext[1] == '/' ) ? yytext+2 : yytext+1;
    if ( !mix_symbol_table_is_defined (parser->symbol_table, s) ) {
      mix_parser_log_error (parser, MIX_PERR_UNDEF_SYM, lineno, s, FALSE);
      yy_pop_state ();
    }
    expr_val = eval_binop_ (yytext, expr_val,
			    mix_symbol_table_value (parser->symbol_table, s));
  }
  {binop}"*" {
    expr_val = eval_binop_ (yytext, expr_val,
			    mix_short_to_word_fast (parser->loc_count));
  }
  "*"         unput_word_ (mix_short_to_word_fast (parser->loc_count));
  {number}    expr_val = mix_word_new (atol (yytext));
  {symbol} {
    if ( !mix_symbol_table_is_defined (parser->symbol_table, yytext) ) {
      mix_parser_log_error (parser, MIX_PERR_UNDEF_SYM, lineno, yytext, FALSE);
      yy_pop_state ();
    }
    expr_val = mix_symbol_table_value (parser->symbol_table, yytext);
  }
  [,)(=\n\t ]   unput (yytext[0]); unput_word_ (expr_val); yy_pop_state ();
  .   RETURN_ERROR (MIX_PERR_INV_EXPR, yytext);
}

<WEVAL>{
  {number}"(" {
    is_fp = TRUE;
    wexpr_val_tmp = mix_word_new (atol (yytext));
  }
  {number}")"  {
    glong val  = atol (yytext);
    if ( !is_fp ) {
      mix_parser_log_error (parser, MIX_PERR_MIS_PAREN, lineno, NULL, FALSE);
      yy_pop_state ();
    }
    if ( val < 0 || val > MIX_BYTE_MAX
         || !mix_fspec_is_valid (mix_byte_new (val)) ) {
      mix_parser_log_error (parser, MIX_PERR_INV_FSPEC, lineno, NULL, FALSE);
      yy_pop_state ();
    }
    is_fp = FALSE;
    wexpr_val = mix_word_store_field (mix_byte_new (val), wexpr_val_tmp,
				      wexpr_val);
  }
  {number}/[,()\n\t ] wexpr_val = mix_word_new (atol (yytext));
  {expr}/[,()\n\t ]  ENTER_EVAL ();
  ,/{expr} /* eat comma if followed by expression */
  [=\n\t ] { /* ok if not inside an f-part */
    if ( is_fp ) {
      mix_parser_log_error (parser, MIX_PERR_MIS_PAREN, lineno, NULL, FALSE);
      yy_pop_state ();
     }
    unput (yytext[yyleng-1]);
    unput_word_ (wexpr_val);
    yy_pop_state ();
  }
  .  RETURN_ERROR (MIX_PERR_INV_EXPR, NULL);
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

static void
unput_word_ (mix_word_t word)
{
  gchar *value;
  gint k;
  value = g_strdup_printf ("%s%ld",
                           mix_word_is_negative (word)? "-":"+",
                           mix_word_magnitude (word));
  for (k = strlen (value) - 1; k >= 0; --k)
    unput (value[k]);
  g_free (value);
}
