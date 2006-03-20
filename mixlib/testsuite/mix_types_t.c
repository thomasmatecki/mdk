/*----------------------- mix_types_t.c -----------------------------
 * Tests for mix_types.h
 * ------------------------------------------------------------------
*
** Copyright (C) 1999, 2004 Free Software Foundation, Inc.
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
#include <mix_types.h>

/* Define VERBOSE_TEST if you want to get prints of the test */
/* #define VERBOSE_TEST */
#include "test.h"

/* compare two words */
static gboolean
word_compare_(mix_word_t w1, mix_word_t w2)
{
  if ( mix_word_magnitude(w1) == 0 )
    return ( mix_word_magnitude(w2) == 0 );
  else
    return ( w1 == w2 );
}


/* create a word from an array of bytes and check the result */
static void
test_word_from_bytes_(mix_word_t *word,
		     mix_byte_t *bytes,
		     unsigned byteno,
		     const char *message)
{
  mix_byte_t r;
  unsigned int k;

  *word = mix_bytes_to_word(bytes,byteno);
  mix_word_print(*word,message);
  g_print("\n");
  for ( k = 5-byteno; k < 5; ++k ) {
    PRINT_BYTE(r = mix_word_get_byte(*word,k+1));
    g_print(" (k = %d)\n",k);
    g_assert( r  == bytes[k-5+byteno] );
  }
}


/* test field access */
static void
test_field_access_(mix_fspec_t l, mix_fspec_t r,
		   mix_word_t from, mix_word_t to)
{
  mix_fspec_t f  = mix_fspec_new(l,r);
  mix_word_t result;

  PRINT_BYTE(l); g_print(", ");
  PRINT_BYTE(r); g_print(", ");
  PRINT_BYTE(f); g_print("\n  ");
  mix_word_print(from, "from: ");
  mix_word_print(to, " to: ");
  g_assert( mix_fspec_left(f) == l );
  g_assert( mix_fspec_right(f) == r );
  result = mix_word_set_field(f,from,to);
  mix_word_print(result,"\n\tresult: ");
  g_assert( mix_word_get_field(f,from) == mix_word_get_field(f,result) );
  g_print("\n");
}

/* test word addition */
static void
test_word_add_(mix_word_t w1, mix_word_t w2)
{
  mix_word_t r;

  r = mix_word_add(w1,w2);
  mix_word_print(w1,"\n");
  mix_word_print(w2,NULL);
  mix_word_print(r," = ");
  g_assert( word_compare_(mix_word_sub(r,w1), w2) );
  g_assert( word_compare_(mix_word_sub(r,w2), w1) );
  /* while asserting the following, take into account that 0 != -0
     for mix words, although they are logically equivalent
  */
  g_assert( word_compare_(mix_word_sub(w1,r), mix_word_negative(w2)) );
  g_assert( word_compare_(mix_word_sub(w2,r), mix_word_negative(w1)) );
  g_assert( word_compare_(mix_word_add(w2,w1), r) );
}

/* test word multiplication */
static void
test_word_mul_(mix_word_t w1, mix_word_t w2)
{
  mix_word_t h, l, q, r = 0;
  mix_word_mul(w1,w2,&h,&l);
  mix_word_print(w1,"\n");
  mix_word_print(w2,"*");
  mix_word_print(h,"\n  =");
  mix_word_print(l,NULL);
  if ( w1 != 0 ) {
    g_assert( mix_word_div(h,l,w1,&q,&r) == FALSE );
    g_assert( mix_word_magnitude(r) == 0 );
    g_assert( q == w2 );
  } else {
    g_assert( mix_word_magnitude(l) == 0 && mix_word_magnitude(h) == 0 );
  }
  if ( w2 != 0 ) {
    g_assert( mix_word_div(h,l,w2,&q,&r) == FALSE );
    g_assert( mix_word_magnitude(r) == 0 );
    g_assert( q == w1 );
  } else {
    g_assert( mix_word_magnitude(l) == 0 && mix_word_magnitude(h) == 0 );
  }

}

/* test word division */
static void
test_word_div_(mix_word_t h, mix_word_t l, mix_word_t by)
{
  mix_word_t q,r;
  gboolean overflow;

  overflow = mix_word_div(h,l,by,&q,&r);

  mix_word_print(h,"\n\n");
  mix_word_print(l,NULL);
  mix_word_print(by,"\n div by ");

  if ( !overflow ) {
    mix_word_t h1, l1, h2;
    mix_word_print(q,"\n q = ");
    mix_word_print(r,"  r = ");
    mix_word_mul(by,q,&h1,&l1);
    mix_word_add_and_carry(l1,r,&h2,&l1);
    h1 = mix_word_add(h1,h2);
    g_assert( mix_word_magnitude(r) < mix_word_magnitude(by) );
    g_assert( word_compare_(h1,h) );
    g_assert( mix_word_magnitude(l1) == mix_word_magnitude(l) );
  } else
    g_print("\n\t = overflow");

}

static void
test_mix_char_(void)
{
  mix_char_t mchar;
  guchar achar;
  guint k;
  g_print("\nTesting mix_char_t. Table of mix_chars:\n");

  for (k = 0; k < MIX_CHAR_MAX + 1; ++k) {
    mchar = k;
    achar = mix_char_to_ascii(mchar);
    g_print("%02d: %c, ", k, achar);
    if ( (k+1)%5 == 0 ) g_print("\n");
    g_assert( mchar == mix_ascii_to_char(achar) );
  }

  g_print("\nchar <-> byte conversions...\n");

  for (k = 0; k < MIX_CHAR_MAX + 1; ++k) {
    mix_byte_t c = mix_byte_new (k);
    g_assert (c == mix_byte_to_char (mix_char_to_byte (c)));
  }

  g_print("\n");
}


/* main test driver for mix_types.h/c */

int main(int argc, char **argv)
{
  unsigned int j,k;
  mix_byte_t bytes[5] = { 0, 3, 20, 30, 40 };
  mix_byte_t r;
  mix_short_t ss[6];
  mix_word_t words[6];

  INIT_TEST;

  g_print("Testing mix_byte_t arithmetics...\n");
  PRINT_BYTE(bytes[0]);  g_print(", ");
  PRINT_BYTE(bytes[1]);  g_print(", ");
  PRINT_BYTE(bytes[2]);  g_print(", ");
  PRINT_BYTE(bytes[3]);  g_print(", ");
  PRINT_BYTE(bytes[4]);  g_print("\n");
  PRINT_BYTE(r = mix_byte_add(bytes[1],bytes[2]));
  g_print("\n"); g_assert(r == 23);
  PRINT_BYTE(r = mix_byte_add(bytes[3],bytes[4]));
  g_print("\n"); g_assert(r == 6);
  PRINT_BYTE(r = mix_byte_sub(bytes[0],bytes[1]));
  g_print("\n"); g_assert(r == 61);
  PRINT_BYTE(r = mix_byte_sub(bytes[4],bytes[3]));
  g_print("\n"); g_assert(r == 10);
  PRINT_BYTE(r = mix_byte_sub(bytes[1],bytes[4]));
  g_print("\n"); g_assert(r == 27);
  PRINT_BYTE(r = mix_byte_mul(bytes[0],bytes[1]));
  g_print("\n"); g_assert(r == 0);
  PRINT_BYTE(r = mix_byte_mul(bytes[1],bytes[2]));
  g_print("\n"); g_assert(r == 60);
  PRINT_BYTE(r = mix_byte_mul(bytes[1],bytes[4]));
  g_print("\n"); g_assert(r == 56);
  PRINT_BYTE(r = mix_byte_mul(bytes[4],bytes[1]));
  g_print("\n"); g_assert(r == 56);
  PRINT_BYTE(r = mix_byte_div(bytes[4],bytes[2]));
  g_print("\n"); g_assert(r == 2);
  PRINT_BYTE(r = mix_byte_div(bytes[3],bytes[2]));
  g_print("\n"); g_assert(r == 1);

  test_mix_char_();

  g_print("\nTesting word<->short conversions...");
  words[0] = mix_bytes_to_word(bytes+1,5);
  words[1] = mix_word_negative(words[0]);
  ss[0] = mix_word_to_short(words[0]);
  ss[1] = mix_word_to_short(words[1]);
  mix_word_print(words[0],"\nwords[0]=");
  mix_word_print(words[1],"\nwords[1]=");
  mix_short_print(ss[0],"\nss[0]=");
  mix_short_print(ss[1],"\nss[1]=");
  g_assert(mix_short_is_positive(ss[0]));
  g_assert(mix_short_is_negative(ss[1]));
  words[2] = mix_short_to_word(ss[0]);
  words[3] = mix_short_to_word(ss[1]);
  mix_word_print(words[2],"\nwords[2]=");
  mix_word_print(words[3],"\nwords[3]=");
  g_assert(mix_word_sign(words[0]) == mix_word_sign(words[2]));
  g_assert(mix_word_sign(words[1]) == mix_word_sign(words[3]));
  g_assert(mix_short_magnitude(ss[0]) == mix_word_magnitude(words[2]));
  g_assert(mix_short_magnitude(ss[1]) == mix_word_magnitude(words[3]));
  g_assert(mix_word_get_byte(words[0],4) == mix_word_get_byte(words[2],4));
  g_assert(mix_word_get_byte(words[0],5) == mix_word_get_byte(words[2],5));
  g_assert(mix_word_get_byte(words[1],4) == mix_word_get_byte(words[3],4));
  g_assert(mix_word_get_byte(words[1],5) == mix_word_get_byte(words[3],5));
  words[4] = mix_word_extract_field(mix_fspec_new(4,5),words[0]);
  words[5] = mix_word_extract_field(mix_fspec_new(4,5),words[1]);
  mix_word_reverse_sign(words[5]);
  g_assert(words[4] == words[2]);
  g_assert(words[5] == words[3]);

  g_print("Testing mix_word_t creation and byte access...\n");
  test_word_from_bytes_(words,bytes,5,"word[0] created from bytes[0-4]");
  test_word_from_bytes_(words+1,bytes,4,"\nword[1] created from bytes[0-3]");
  test_word_from_bytes_(words+2,bytes,3,"\nword[2] created from bytes[0-2]");
  words[3] = mix_word_negative(words[2]);
  g_assert( mix_word_negative(words[3]) == words[2] );
  g_assert( mix_word_is_negative(words[3]) && !mix_word_is_negative(words[2]));
  mix_word_print(words[3],"\nword[3] created from -word[2]");
  test_word_from_bytes_(words+4,bytes+2,2,"\nword[2] created from bytes[2-3]");

  g_print("\nTesting mix_word_t field access...\n");
  mix_word_set_byte(words+3,1,12);
  mix_word_set_byte(words+3,2,58);
  g_assert( mix_word_get_byte(words[3],1) == 12 );
  g_assert( mix_word_get_byte(words[3],2) == 58 );
  test_field_access_(0,5,words[3],words[4]);
  test_field_access_(1,5,words[3],words[4]);
  test_field_access_(2,5,words[3],words[4]);
  test_field_access_(3,5,words[3],words[4]);
  test_field_access_(4,5,words[3],words[4]);
  test_field_access_(5,5,words[3],words[4]);
  test_field_access_(0,0,words[3],words[4]);

  g_print("\n\nTesting mix_word_t arithmetics...\n");
  words[0] = MIX_WORD_MAX;
  words[1] = mix_word_negative(words[0]);
  for ( k = 1; k < 6; ++k ) {
    mix_word_set_byte(words+2,k,5*k);
    mix_word_set_byte(words+4,k,10*(5-k));
    mix_word_set_byte(words+3,k,21 + 3*k);
  }
  words[5] = 0;

  g_print("\n***addition***");
  for ( k = 0; k < 6; ++k )
    for ( j = 0; j <= k; ++j ) {
      test_word_add_(words[k],mix_word_negative(words[j]));
      test_word_add_(words[k],words[j]);
    }
  g_print("\n***product***");
  for ( k = 0; k < 6; ++k )
    for ( j = 0; j <= k; ++j ) {
      test_word_mul_(words[k],words[j]);
    }
  g_print("\n***division***");
  for ( k = 0; k < 6; ++k ) {
    test_word_div_(words[k],0,words[0]);
    for ( j = 0; j <= k; ++j ) {
      test_word_div_(k,words[j],words[j]);
      test_word_div_(0,mix_word_add(mix_word_magnitude(words[j]),j),words[j]);
      test_word_div_(mix_word_negative(k),words[j],words[j]);
    }
  }

  g_print("\nTesting shift operations...\n");
  for ( k = 0; k < 10; ++k )
    mix_word_set_byte(words+(k/5),1+(k%5),k+1);

  mix_word_print(words[0],"A = ");
  mix_word_print(words[1],"X = ");
  for ( k = 0; k < 11; ++k ) {
    mix_word_t A, X;
    unsigned int m;

    mix_word_shift_left(words[0],words[1],k,&A,&X);
    g_print("\nShift left %d:\n",k);
    mix_word_print(A,"A ");
    mix_word_print(X,"X ");
    for ( m = 0; m < 10 - k; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) ==
	      mix_word_get_byte(words[(m+k)/5], ((m+k)%5)+1) );
    for ( ; m < 10; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) == 0 );

    mix_word_shift_right(words[0],words[1],k,&A,&X);
    g_print("\nShift right %d:\n",k);
    mix_word_print(A,"A ");
    mix_word_print(X,"X ");
    for ( m = 0; m < k; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) == 0 );
    for ( ; m < 10; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) ==
	      mix_word_get_byte(words[(m-k)/5], ((m-k)%5)+1) );

    mix_word_shift_left_circular(words[0],words[1],k,&A,&X);
    g_print("\nShift left circular %d:\n",k);
    mix_word_print(A,"A ");
    mix_word_print(X,"X ");
    for ( m = 0; m < 10 - k; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) ==
	      mix_word_get_byte(words[(m+k)/5], ((m+k)%5)+1) );
    for ( ; m < 10; ++m )
      g_assert( mix_word_get_byte( m < 5 ? A:X, (m%5)+1 ) ==
	      mix_word_get_byte(words[(m-10+k)/5], 1+((m-10+k)%5)) );
    mix_word_shift_right_circular(A, X, k, &A, &X);
    g_print("\nRe-shiftting right...\n");
    mix_word_print(A, "A ");
    mix_word_print(X, "X ");
    g_assert(A == words[0]);
    g_assert(X == words[1]);
  }


  g_print("\n");
  return EXIT_SUCCESS;
}






