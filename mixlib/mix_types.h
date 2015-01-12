/* -*-c-*- --------------- mix_types.h:
 *  This file contains declarations for the basic types used in MIX:
 *   mix_byte_t, mix_char_t, mix_short_t and mix_word_t.
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2001, 2004, 2007 Free Software Foundation, Inc.
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


#ifndef MIX_TYPES_H
#define MIX_TYPES_H

#include <stdio.h>
#include "mix.h"

/* Initialisation function to be called before using the other
 * functions in this file
 */
extern void
mix_init_types(void);


/*----------------- m_byte_t --------------------------------------------*/
/* MIX byte type */
typedef guint8 mix_byte_t;

/* Maximum value stored in an mix_byte_t */
#define MIX_BYTE_MAX ((1L << 6) - 1)

/* Zero mix byte */
#define MIX_BYTE_ZERO  ((mix_byte_t)0)

/* Create a mix_byte_t from any native type */
#define mix_byte_new(x) ((mix_byte_t)((x) & MIX_BYTE_MAX ))

/* Operations */
/* Addition */
#define mix_byte_add(x,y) mix_byte_new((x) + (y))
/* Substraction */
#define mix_byte_sub(x,y) mix_byte_new((x) - (y))
/* Product */
#define mix_byte_mul(x,y) mix_byte_new((x) * (y))
/* Quotient */
#define mix_byte_div(x,y) mix_byte_new((x) / (y))


/*----------------- mix_char_t --------------------------------------------*/
/* MIX char type: chars are coded in MIX from 0 to MIX_CHAR_MAX */
typedef guint8 mix_char_t;

#define MIX_CHAR_MAX 55

/* Conversions for mix_char_t's */
#define mix_char_new(l) (((l) < MIX_CHAR_MAX )? (l) : MIX_CHAR_MAX)
#define mix_char_to_byte(mchar) mix_byte_new (mchar)
#define mix_byte_to_char(byte) mix_char_new (byte)

extern mix_char_t
mix_ascii_to_char(guchar c);

extern guchar
mix_char_to_ascii(mix_char_t c);


/*----------------- mix_word_t --------------------------------------------*/
/*
 * Represented as a gint32 (glib ensures that this type has 32
 * bits). Bit 30 is the sign, higher bits are 0,
 * and bits 0-29 are the magnitude.
 * Each MIX 'byte' is a 6-bit substring of the magnitude.
 */
typedef guint32  mix_word_t;

/* Maximum value stored in an mix_word_t */
#define MIX_WORD_MAX ((1L << 30) - 1)
/* Sign bit in a word */
#define MIX_WORD_SIGN_BIT    (1L << 30)
/* Zero mix word */
#define MIX_WORD_ZERO  ((mix_word_t)0)
/* Negative zero mix word */
#define MIX_WORD_MINUS_ZERO  (MIX_WORD_ZERO | MIX_WORD_SIGN_BIT)


/* Create a mix_word_t from any native type */
#define mix_word_new(x)						\
( (x) < 0							\
  ? ( MIX_WORD_SIGN_BIT | ((mix_word_t)(-(x)) & MIX_WORD_MAX) )	\
  : ( (mix_word_t)(x) & MIX_WORD_MAX )				\
)

/* Create a mix_word_t from individual bytes */
#define mix_word_new_b(b1,b2,b3,b4,b5)				\
((mix_word_t)(mix_byte_new(b5) + (mix_byte_new(b4)<<6) +	\
	      (mix_byte_new(b3)<<12) + (mix_byte_new(b2)<<18) +	\
	      (mix_byte_new(b1)<<24)))

/* Create a negative mix_word_t from individual bytes */
#define mix_word_new_bn(b1,b2,b3,b4,b5) \
   mix_word_negative(mix_word_new_b(b1,b2,b3,b4,b5))

/* Create mix_word_t from an array of mix_byte_t */
extern mix_word_t
mix_bytes_to_word(mix_byte_t *bytes, guint byteno);

/* Access byte within a word */
extern mix_byte_t /* byte -idx- or MIX_BYTE_ZERO if -idx- out of range */
mix_word_get_byte(mix_word_t word,   /* word parsed */
		  guint idx          /* byte: 1 to 5 */);

/* Set a byte  within a mix_word_t  */
extern void
mix_word_set_byte(mix_word_t *into,   /* word to be modified */
		  guint idx,          /* byte: 1 to 5 */
		  mix_byte_t value    /* byte's value */);


/* Operations */
/* Sign-related definitions */
#define mix_word_negative(word)      ( (word) ^ MIX_WORD_SIGN_BIT )
#define mix_word_reverse_sign(word)  ( word ^= MIX_WORD_SIGN_BIT )
#define mix_word_sign(word)          ( (word) & MIX_WORD_SIGN_BIT )
#define mix_word_magnitude(word)     ( (word) & (MIX_WORD_SIGN_BIT - 1) )
#define mix_word_is_positive(word)   ( mix_word_sign(word) == 0 )
#define mix_word_is_negative(word)   ( mix_word_sign(word) != 0 )
#define mix_word_is_even(word)       ( ((word) & 1) == 0 )
#define mix_word_is_odd(word)        ( ((word) & 1) == 1 )


/* Arithmetic operations */
extern mix_word_t
mix_word_add(mix_word_t x, mix_word_t y);

#define mix_word_sub(x,y)    ( mix_word_add((x),mix_word_negative(y)) )

/* Add two words filling a high word if needed.
   -high_word- and/or -low_word- can be NULL.
*/
extern gboolean /* TRUE if overflow */
mix_word_add_and_carry(mix_word_t x, mix_word_t y,
		       mix_word_t *high_word, mix_word_t *low_word);

/* Product, stored in -high_word- and -low_word-, which
   can be NULL.
*/
extern void
mix_word_mul(mix_word_t x, mix_word_t y,
	     mix_word_t *high_word, mix_word_t *low_word);

/* Division. -quotient- and/or -remainder- can be NULL. */
extern gboolean /* TRUE if overflow */
mix_word_div(mix_word_t n1, mix_word_t n0, mix_word_t d,
	     mix_word_t *quotient, mix_word_t *remainder);

/* Shift operations */
extern void
mix_word_shift_left(mix_word_t A, mix_word_t X, gulong count,
		    mix_word_t *pA, mix_word_t *pX);
extern void
mix_word_shift_right(mix_word_t A, mix_word_t X, gulong count,
		     mix_word_t *pA, mix_word_t *pX);
extern void
mix_word_shift_left_circular(mix_word_t A, mix_word_t X, gulong count,
			     mix_word_t *pA, mix_word_t *pX);
extern void
mix_word_shift_right_circular(mix_word_t A, mix_word_t X, gulong count,
                              mix_word_t *pA, mix_word_t *pX);

extern void
mix_word_shift_left_binary(mix_word_t A, mix_word_t X, gulong count,
                           mix_word_t *pA, mix_word_t *pX);

extern void
mix_word_shift_right_binary(mix_word_t A, mix_word_t X, gulong count,
                            mix_word_t *pA, mix_word_t *pX);

/*
 * Fields within a word: a word containing the (L:R)
 * bytes of the original one. L and R (with 0 <= L <= R < 6)
 * are specified by a mix_fspec_t F = 8*L + R.
 */
typedef guint8 mix_fspec_t;

#define mix_fspec_left(f) ( ((f)>>3) & 7 )
#define mix_fspec_right(f) ( (f) & 7 )
#define mix_fspec_new(L,R) ( mix_byte_new(8*(L) + (R)) )

extern gboolean
mix_fspec_is_valid(mix_fspec_t f);

extern mix_word_t /* the specified field or 0 if f is not valid */
mix_word_get_field(mix_fspec_t f, mix_word_t word);

extern mix_word_t /* -to- with the field -f- from -from- or -to-
		     if -f- is not a valid fspec  */
mix_word_set_field(mix_fspec_t f, mix_word_t from, mix_word_t to);

/* set field into a zero word */
#define mix_word_extract_field(fspec,from_word) \
    mix_word_set_field(fspec,from_word,MIX_WORD_ZERO)

/* Store operation: the no. of bytes determined by -f- is taken
 * from the right of -from- and stored into -to- in the location
 *  specified by -f-
 */
extern mix_word_t
mix_word_store_field(mix_fspec_t f, mix_word_t from, mix_word_t to);


/* Printable representation */
#define mix_word_print(word,message) \
  mix_word_print_to_file (word, message, stdout)

extern void
mix_word_print_to_file (mix_word_t word, const char *message, FILE *f);

extern void
mix_word_print_to_buffer (mix_word_t word, gchar *buf);


/*----------------- mix_short_t ------------------------------------------*/
typedef guint16 mix_short_t;

#define MIX_SHORT_MAX  ((1L << 12) - 1)
#define MIX_SHORT_SIGN_BIT  ((mix_short_t)(1L << 12))
#define MIX_SHORT_ZERO ((mix_short_t)0)
#define MIX_SHORT_MINUS_ZERO (MIX_SHORT_ZERO | MIX_SHORT_SIGN_BIT)

/* Sign-related definitions */
#define mix_short_negative(s)      ( (s) ^ MIX_SHORT_SIGN_BIT )
#define mix_short_sign(s)          ( (s) & MIX_SHORT_SIGN_BIT )
#define mix_short_magnitude(s)     	\
   ( (s) & (MIX_SHORT_SIGN_BIT - 1) )
#define mix_short_is_positive(s)   ( mix_short_sign(s) == 0 )
#define mix_short_is_negative(s)   ( mix_short_sign(s) != 0 )
#define mix_short_reverse_sign(s)  ( (s) ^= MIX_SHORT_SIGN_BIT )

/* create short from a long */
#define mix_short_new(val) \
	((val)>= 0 ? (val)&MIX_SHORT_MAX : mix_short_negative(-(val)))

/* Create shorts from individual bytes */
#define mix_short_new_b(b1,b2) \
((mix_short_t)((mix_byte_new(b1)<<6) + mix_byte_new(b2)))

#define mix_short_new_bn(b1,b2) mix_short_negative(mix_short_new_b(b1,b2))

/* Conversions between words and shorts. Arithmetic operations
   on shorts are not provided but for addition: use words instead.
*/
/* Make a short taking word's sign and its two least significant
   bytes (bytes no. 4 and 5)
*/
extern mix_short_t
mix_word_to_short(mix_word_t word);

extern  mix_word_t
mix_short_to_word(mix_short_t s);

/* fast conversion (these macros' argument are evaluated twice */
#define mix_word_to_short_fast(w)					\
( mix_word_is_negative(w) ?						\
  ((w) & MIX_SHORT_MAX)|MIX_SHORT_SIGN_BIT : (w)&MIX_SHORT_MAX )

#define mix_short_to_word_fast(s)					     \
( mix_short_is_negative(s) ?						     \
(mix_word_t) (mix_short_magnitude(s)|MIX_WORD_SIGN_BIT): (mix_word_t)(s) )


extern mix_short_t
mix_short_add(mix_short_t x, mix_short_t y);


/* printable representation */
extern void
mix_short_print(mix_short_t s, const gchar *message);

extern void
mix_short_print_to_buffer (mix_short_t s, gchar *buf);



#endif /* MIX_TYPES_H */




