/* -*-c-*- ------------------ mix_types.c :
 *  Implementation file for mix_types.h declarations.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2002, 2004, 2006, 2007, 2019 Free Software Foundation, Inc.
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

#include "mix.h"
#include <limits.h>
#include <ctype.h>
#include "mix_types.h"


/*------------------------ initialisation stuff ------------------------*/
/* flag telling whether a field spec is valid */
static gboolean is_bad_field_[MIX_BYTE_MAX + 1];
/* shift associated with a fspec */
static guint shift_[MIX_BYTE_MAX + 1];
/* mask associated with a fspec */
static glong mask_[MIX_BYTE_MAX + 1];

/* maps from gchar's to mix_char_t */
#define NONP_  (guchar)('?')
static mix_char_t ascii_to_mix_char_[UCHAR_MAX + 1];
static guchar mix_char_to_ascii_[MIX_CHAR_MAX + 1] = {
  ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', '~', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', '[', '#', 'S', 'T',
  'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6',
  '7', '8', '9', '.', ',', '(', ')', '+', '-', '*', '/', '=', '$',
  '<', '>', '@', ';', ':', '\''
};


/* initialise the above arrays */
void
mix_init_types (void)
{
  guint lt, rt;
  for (lt = 0; lt < 8; ++lt)
    for (rt = 0; rt < 8; ++rt)
      {
	guint F = 8 * lt + rt;
	is_bad_field_[F] = rt < lt || 5 < rt;
	if ( is_bad_field_[F] )
	  shift_[F] = 0, mask_[F] = 0;
	else
	  {
	    guint width = rt - (lt == 0 ? 1 : lt) + 1;
	    shift_[F] = 6 * (5 - rt);
	    mask_[F] = ((1L << (6 * width)) - 1) << shift_[F];
	  }
      }

  for ( lt = 0; lt < UCHAR_MAX + 1; ++lt )
    ascii_to_mix_char_[lt] = NONP_;
  for ( lt = 0; lt < MIX_CHAR_MAX + 1; ++lt )
    ascii_to_mix_char_[mix_char_to_ascii_[lt]] = (guchar)lt;
}

mix_char_t
mix_ascii_to_char (guchar c)
{
  return ascii_to_mix_char_[toupper (c)];
}


guchar
mix_char_to_ascii (mix_char_t c)
{
  return c > MIX_CHAR_MAX ? NONP_ : mix_char_to_ascii_[c];
}


/*---------------------------- m_word_t --------------------------------- */

/* Create mix_word_t from an array of mix_byte_t */
mix_word_t
mix_bytes_to_word (mix_byte_t *bytes, guint byteno)
{
  mix_word_t result = 0;
  guint k;

  g_return_val_if_fail (bytes != NULL, MIX_WORD_ZERO);
  g_return_val_if_fail (byteno != 0, MIX_WORD_ZERO);

  if ( byteno > 5 ) byteno = 5;
  for ( k = 0; k < byteno; k++ )
    result |= ((gulong)bytes[k]) << (6*(byteno-k-1));

  return result;
}


/* Field operations */
mix_word_t /* the specified field or 0 if f is not valid */
mix_word_get_field (mix_fspec_t f, mix_word_t word)
{
  mix_word_t result;

  g_return_val_if_fail (!is_bad_field_[f],MIX_WORD_ZERO);

  result = ( (word & mask_[f]) >> shift_[f] );
  if (f < 8)      /* if f is of the form (0:R), retain the sign of word */
    result |= mix_word_sign (word);

  return result;
}


mix_word_t
mix_word_set_field (mix_fspec_t f, mix_word_t from, mix_word_t to)
{
  mix_word_t result;
  glong m = mask_[f];

  g_return_val_if_fail (!is_bad_field_[f],to);

  if (f < 8)      /* if F is of the form (0:R), use the sign of -from- */
    result =  (to & ~m & ~MIX_WORD_SIGN_BIT)
      | ((from /*<< shift_[f]*/) & m) | mix_word_sign (from);
  else
    result = (to & ~m) | ((from /*<< shift_[f]*/) & m);

  return result;
}

mix_word_t
mix_word_store_field (mix_fspec_t f, mix_word_t from, mix_word_t to)
{
  mix_word_t result;
  glong m = mask_[f];

  g_return_val_if_fail (!is_bad_field_[f],to);

  if (f < 8)      /* if F is of the form (0:R), use the sign of -from- */
    result =  (to & ~m & ~MIX_WORD_SIGN_BIT)
      | ((from << shift_[f]) & m) | mix_word_sign (from);
  else
    result = (to & ~m) | ((from << shift_[f]) & m);

  return result;
}



gboolean
mix_fspec_is_valid (mix_fspec_t f)
{
  return !(is_bad_field_[f]);
}

mix_byte_t
mix_word_get_byte (mix_word_t word,   /* word parsed */
		   guint idx          /* byte: 1 to 5 */	  )
{
  mix_byte_t fspec = mix_fspec_new (idx,idx);

  g_return_val_if_fail ((idx > 0 && idx < 6), MIX_BYTE_ZERO);

  return mix_byte_new (mix_word_get_field (fspec,word));
}


extern void
mix_word_set_byte (mix_word_t *into, guint idx,  mix_byte_t value)
{
  mix_word_t from = value;
  mix_byte_t fspec = mix_fspec_new (idx,idx);

  g_return_if_fail (into != NULL);
  g_return_if_fail (idx > 0 && idx < 6);

  from <<= shift_[fspec];
  *into = mix_word_set_field (fspec,from,*into);
}

/* Arithmetic operations */
mix_word_t
mix_word_add (mix_word_t x, mix_word_t y)
{
  static const gulong MIX_WORD_MAX_SIM = 2*MIX_WORD_MAX + 1;

  gint32 result;
  if ( mix_word_sign (x) == mix_word_sign (y) )
    {
      result = (mix_word_magnitude (x) + mix_word_magnitude (y));
      if ( result >  MIX_WORD_MAX ) {
	/* for instance MIX_WORD_MAX + 1 = - MIX_WORD_MAX */
	result = MIX_WORD_MAX_SIM - result;
	result |= mix_word_sign (mix_word_negative (x));
      } else {
	result |= mix_word_sign (x);
      }
    }
  else
    {
      result = mix_word_magnitude (x) -  mix_word_magnitude (y);
      if (result < 0)
	result = -result|mix_word_sign (y);
      else
	result = result|mix_word_sign (x);
    }

  g_assert ( result >= 0 );

  return (mix_word_t)result;
}

gboolean /* TRUE if overflow */
mix_word_add_and_carry (mix_word_t x, mix_word_t y,
			mix_word_t *h, mix_word_t *l)
{
  gboolean result;

  if ( mix_word_sign (x) == mix_word_sign (y) )
    {
      guint32 sum = (mix_word_magnitude (x) + mix_word_magnitude (y));
      if ( sum >  MIX_WORD_MAX )
	{
	  result = TRUE;
	  if ( l != NULL )
	    {
	      *l = sum - MIX_WORD_MAX -1;
	      *l |= mix_word_sign (x);
	    }
	  if ( h != NULL )
	    {
	      *h = sum >> 30;
	      *h |= mix_word_sign (x);
	    }
	}
      else /* sum <= MIX_WORD_MAX */
	{
	  result = FALSE;
	  if ( h != NULL ) *h = 0;
	  if ( l != NULL )
	    {
	      *l = sum;
	      if ( sum != 0 )
		*l |= mix_word_sign (x);
	      /* keep positive sign for 0 so that w - w == -w + w */
	    }
	}
    }
  else /* mix_word_sign (x) != mix_word_sign (y) */
    {
      result = FALSE;
      if ( h != NULL ) *h = 0;
      if ( l != NULL )
	{
	  gint32 dif = mix_word_magnitude (x) -  mix_word_magnitude (y);
	  if ( dif < 0)
	    *l = (-dif)|mix_word_sign (y);
	  else
	    *l = dif|mix_word_sign (x);
	}
    }

  return result;
}


void
mix_word_mul (mix_word_t x, mix_word_t y,
	      mix_word_t *high_word, mix_word_t *low_word)
{
  guint32 sign = mix_word_sign (x) ^ mix_word_sign (y);

  /*
    x = x0 + x1 * 2 ^ 10 + x2 * 2 ^ 20
    y = y0 + y1 * 2 ^ 10 + y2 * 2 ^ 20
    x0, x1, x2, y0, y1, y2 are < 2 ^ 10
  */
  guint32 x0 = (x & 0x000003FF);
  guint32 x1 = (x & 0x000FFC00) >> 10;
  guint32 x2 = (x & 0x3FF00000) >> 20;
  guint32 y0 = (y & 0x000003FF);
  guint32 y1 = (y & 0x000FFC00) >> 10;
  guint32 y2 = (y & 0x3FF00000) >> 20;

  /*
    x * y = partial0 +
    partial1 * 2 ^ 10 +
    partial2 * 2 ^ 20 +
    partial3 * 2 ^ 30 +
    partial4 * 2 ^ 40
    partial0 and partial4 are <     2 ^ 20
    partial1 and partial3 are <     2 ^ 21
    partial2 is               < 3 * 2 ^ 20
  */
  guint32 partial0 = x0 * y0;
  guint32 partial1 = x0 * y1 + x1 * y0;
  guint32 partial2 = x0 * y2 + x1 * y1 + x2 * y0;
  guint32 partial3 = x1 * y2 + x2 * y1;
  guint32 partial4 = x2 * y2;

  /*  sum1 has a place value of 1 and is < 2 ^ 32 */
  guint32 sum1   = partial0 + (partial1 << 10);
  guint32 carry1 = (sum1 & 0xFFF00000) >> 20;

  /* sum2 has a place value of 2 ^ 20 and is < 2 ^ 32 */
  guint32 sum2   = partial2 + (partial3 << 10) + carry1;
  guint32 carry2 = (sum2 & 0xFFF00000) >> 20;

  /* sum3 has a place value of 2 ^ 40 and is < 2 ^ 20 */
  guint32 sum3   = partial4 + carry2;

  sum1 &= ~0xFFF00000;
  sum2 &= ~0xFFF00000;

  /*
    Now paste the three values back into two.
  */
  if ( low_word != NULL ) {
    *low_word   = sum1 | ((sum2 & 0x000003FF) << 20);
    *low_word  |= sign;
  }
  if ( high_word != NULL ) {
    *high_word  = ((sum2 & 0x000FFC00) >> 10) | (sum3 << 10);
    *high_word |= sign;
  }
}


gboolean
mix_word_div (mix_word_t n1, mix_word_t n0, mix_word_t d,
	      mix_word_t *quotient, mix_word_t *remainder)
{
  gboolean overflow = FALSE;
  long magn1 = mix_word_magnitude (n1);
  long magd = mix_word_magnitude (d);

  if (magd == 0)
    {
      overflow = TRUE;
      /* just so they have -some- valid value */
      if ( quotient != NULL ) *quotient = 0;
      if ( remainder != NULL )  *remainder = 0;
    }
  else if (magn1 == 0)
    {    /* special-cased for speed */
      if ( quotient != NULL )
	*quotient = (mix_word_sign (n1) ^ mix_word_sign (d))
	  | (mix_word_magnitude (n0) / magd);
      if ( remainder != NULL )
	*remainder = mix_word_sign (n1) | (mix_word_magnitude (n0) % magd);
    }
  else if (magd <= magn1)
    {
      overflow = TRUE;
      if ( quotient != NULL ) *quotient = 0;
      if ( remainder != NULL )  *remainder = 0;
    }
  else
    {
      long q = mix_word_magnitude (n0);
      long r = magn1;
      unsigned i;
      for (i = 30; i != 0; --i) {
	r <<= 1;
	if ( (q & (1L << 29)) != 0 )
	  ++r;
	q = (q << 1) & ((1L << 30) - 1);
	if (magd <= r)
	  ++q, r -= magd;
      }
      if ( quotient != NULL )
	*quotient = (mix_word_sign (n1) ^ mix_word_sign (d)) | q;
      if ( remainder != NULL )
	*remainder = mix_word_sign(n1) | r;
    }
  return overflow;
}

void
mix_word_shift_right (mix_word_t A, mix_word_t X, gulong count,
		      mix_word_t *pA, mix_word_t *pX)
{
  if ( pX != NULL ) *pX = mix_word_sign (X);
  if ( pA != NULL ) *pA = mix_word_sign (A);
  if (count < 5) {
    if ( pA != NULL )
      *pA |= mix_word_magnitude (A) >> (6 * count);
    if ( pX != NULL )
      *pX |= MIX_WORD_MAX & ( (mix_word_magnitude (X) >> (6 * count))
			      | (A << (30 - 6 * count)) );
  } else if (count < 10 && pX != NULL)
    *pX |= mix_word_magnitude (A) >> (6 * count - 30);
  else
    ;
}


void
mix_word_shift_left (mix_word_t A, mix_word_t X, gulong count,
		     mix_word_t *pA, mix_word_t *pX)
{
  if ( pX != NULL ) *pX = mix_word_sign (X);
  if ( pA != NULL ) *pA = mix_word_sign (A);
  if (count < 5) {
    if ( pX != NULL ) *pX |= MIX_WORD_MAX & (X << (6 * count));
    if ( pA != NULL )
      *pA |= MIX_WORD_MAX & ( (A << (6 * count)) |
			      (mix_word_magnitude (X) >> (30 - 6 * count)) );
  } else if (count < 10 && pA != NULL)
    *pA |= MIX_WORD_MAX & (X << (6 * count - 30));
  else
    ;
}

void
mix_word_shift_left_circular (mix_word_t A, mix_word_t X, gulong count,
			      mix_word_t *pA, mix_word_t *pX)
{
  mix_word_t A1, X1;
  guint c;

  count %= 10;
  A1 = count < 5 ? A : X;
  X1 = count < 5 ? X : A;
  c = 6 * (count < 5 ? count : count - 5);
  if ( pX != NULL )
    *pX = mix_word_sign (X)
      | ( MIX_WORD_MAX & (X1 << c) ) | ( mix_word_magnitude (A1) >> (30 - c) );
  if ( pA != NULL )
    *pA = mix_word_sign (A)
      | ( MIX_WORD_MAX & (A1 << c) ) | ( mix_word_magnitude (X1) >> (30 - c) );
}

void
mix_word_shift_right_circular (mix_word_t A, mix_word_t X, gulong count,
			       mix_word_t *pA, mix_word_t *pX)
{
  mix_word_t A1, X1;
  guint c;

  count %= 10;
  A1 = count < 5 ? A : X;
  X1 = count < 5 ? X : A;
  c = 6 * (count < 5 ? count : count - 5);
  if ( pX != NULL )
    *pX = mix_word_sign (X)
      | ( mix_word_magnitude (X1) >> c ) | ( MIX_WORD_MAX & (A1 << (30 - c)) );
  if ( pA != NULL )
    *pA = mix_word_sign (A)
      | ( mix_word_magnitude (A1) >> c ) | ( MIX_WORD_MAX & (X1 << (30 - c)) );
}

void
mix_word_shift_left_binary (mix_word_t A, mix_word_t X, gulong count,
                            mix_word_t *pA, mix_word_t *pX)
{
  if ( pX != NULL ) *pX = mix_word_sign (X);
  if ( pA != NULL ) *pA = mix_word_sign (A);
  if (count < 30) {
    if ( pX != NULL ) *pX |= MIX_WORD_MAX & (X << count);
    if ( pA != NULL )
      *pA |= MIX_WORD_MAX & ( (A << count) |
			      (mix_word_magnitude (X) >> (30 - count)) );
  } else if (count < 60 && pA != NULL)
    *pA |= MIX_WORD_MAX & (X << (count - 30));
  else
    ;
}

void
mix_word_shift_right_binary (mix_word_t A, mix_word_t X, gulong count,
                             mix_word_t *pA, mix_word_t *pX)
{
  if ( pX != NULL ) *pX = mix_word_sign (X);
  if ( pA != NULL ) *pA = mix_word_sign (A);
  if (count < 30) {
    if ( pA != NULL )
      *pA |= mix_word_magnitude (A) >> count;
    if ( pX != NULL )
      *pX |= MIX_WORD_MAX & ( (mix_word_magnitude (X) >> count)
			      | (A << (30 - count)) );
  } else if (count < 60 && pX != NULL)
    *pX |= mix_word_magnitude (A) >> (count - 30);
  else
    ;
}

/* Printable representation */
void
mix_word_print_to_file (mix_word_t word, const char *message, FILE *f)
{
  guint k;
  if ( message ) fprintf (f, "%s ", message);
  fprintf (f, "%s ", mix_word_sign (word) == 0 ? "+" : "-");
  for ( k = 1; k < 6; ++k ) {
    fprintf (f, "%02d ", mix_word_get_byte (word,k));
  }
  fprintf (f, "(%010ld)", mix_word_magnitude (word));
}

void
mix_word_print_to_buffer (mix_word_t word, gchar *buf)
{
  g_return_if_fail (buf != NULL);
  sprintf (buf, "%s %02d %02d %02d %02d %02d",
	   mix_word_sign (word) == 0 ? "+" : "-",
	   mix_word_get_byte (word, 1),
	   mix_word_get_byte (word, 2),
	   mix_word_get_byte (word, 3),
	   mix_word_get_byte (word, 4),
	   mix_word_get_byte (word, 5));
}

/* Conversions between words and shorts */
mix_short_t
mix_word_to_short (mix_word_t word)
{
  if  ( mix_word_is_negative (word) )
    return ( (word & MIX_SHORT_MAX) | MIX_SHORT_SIGN_BIT );
  else
    return (word & MIX_SHORT_MAX);
}

mix_word_t
mix_short_to_word (mix_short_t s)
{
  if ( mix_short_is_negative (s) )
    return ((mix_word_t) (mix_short_magnitude (s) | MIX_WORD_SIGN_BIT));
  else
    return ((mix_word_t)s);
}

mix_short_t
mix_short_add (mix_short_t x, mix_short_t y)
{
  static const guint16 MIX_SHORT_MAX_SIM = 2*MIX_SHORT_MAX + 1;

  gint16 result;
  if ( mix_short_sign (x) == mix_short_sign (y) )
    {
      result = (mix_short_magnitude (x) + mix_short_magnitude (y));
      if ( result >  MIX_SHORT_MAX ) {
	/* for instance MIX_SHORT_MAX + 1 = - MIX_SHORT_MAX */
	result = MIX_SHORT_MAX_SIM - result;
	result |= mix_short_sign (mix_short_negative (x));
      } else if ( result != 0 )
	result |= mix_short_sign (x);
      /* keep positive sign for 0 so that w - w == -w + w */
    }
  else
    {
      result = mix_short_magnitude (x) -  mix_short_magnitude (y);
      if (result < 0)
	result = -result|mix_short_sign (y);
      else if (result > 0)
	result = result|mix_short_sign (x);
      /* keep positive sign for 0 so that w - w == -w + w */
    }

  g_assert ( result >= 0 );

  return (mix_short_t)result;
}


/* Printable representation */
void
mix_short_print (mix_short_t s, const gchar *message)
{
  if ( message ) g_print ("%s ", message);
  g_print ("%s ", mix_short_sign (s) == 0 ? "+" : "-");
  g_print ("%02d %02d ", mix_byte_new (s>>6), mix_byte_new (s));
  g_print ("(%04d)", mix_short_magnitude (s));
}

void
mix_short_print_to_buffer (mix_short_t s, gchar *buf)
{
  g_return_if_fail (buf != NULL);
  sprintf (buf, "%s %02d %02d",
	   mix_short_sign (s) == 0 ? "+" : "-",
	   mix_byte_new (s>>6), mix_byte_new (s));
  /* g_print ("(%04d)", mix_short_magnitude (s));*/
}
