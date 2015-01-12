/* -*-c-*- ------------------ mix_ins.c :
 * Implementation of the functions declared in mix_ins.h
 * ------------------------------------------------------------------
 * Copyright (C) 1999, 2003, 2007, 2010 Free Software Foundation, Inc.
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

#include "mix_ins.h"


struct mix_ins_desc_
{
  mix_opcode_t opcode;
  mix_fspec_t  def_fspec;
  const gchar  *string_rep;
  gboolean is_ext;
};

#define IDES_(c,f,s) {c,f,#s,FALSE}
#define IDESX_(c,f,s) {c,f,#s,TRUE}

static const struct mix_ins_desc_ id_to_desc_[] =  {
  IDES_(0,0,NOP), IDES_(1,5,ADD), IDES_(2,5,SUB), IDES_(3,5,MUL),
  IDES_(4,5,DIV),
  IDESX_(5,0,NUM), IDESX_(5,1,CHAR), IDESX_(5,2,HLT),
  IDESX_(6,0,SLA), IDESX_(6,1,SRA), IDESX_(6,2,SLAX), IDESX_(6,3,SRAX),
  IDESX_(6,4,SLC), IDESX_(6,5,SRC),
  IDESX_(6,6,SLB), IDESX_(6,7,SRB),
  IDES_(7,1,MOVE), IDES_(8,5,LDA), IDES_(9,5,LD1), IDES_(10,5,LD2),
  IDES_(11,5,LD3), IDES_(12,5,LD4), IDES_(13,5,LD5),
  IDES_(14,5,LD6), IDES_(15,5,LDX), IDES_(16,5,LDAN), IDES_(17,5,LD1N),
  IDES_(18,5,LD2N), IDES_(19,5,LD3N), IDES_(20,5,LD4N),
  IDES_(21,5,LD5N), IDES_(22,5,LD6N), IDES_(23,5,LDXN),
  IDES_(24,5,STA), IDES_(25,5,ST1), IDES_(26,5,ST2), IDES_(27,5,ST3),
  IDES_(28,5,ST4), IDES_(29,5,ST5), IDES_(30,5,ST6), IDES_(31,5,STX),
  IDES_(32,2,STJ), IDES_(33,5,STZ), IDES_(34,0,JBUS), IDES_(35,0,IOC),
  IDES_(36,0,IN), IDES_(37,0,OUT), IDES_(38,0,JRED),
  IDESX_(39,0,JMP), IDESX_(39,1,JSJ), IDESX_(39,2,JOV), IDESX_(39,3,JNOV),
  IDESX_(39,4,JL), IDESX_(39,5,JE), IDESX_(39,6,JG), IDESX_(39,7,JGE),
  IDESX_(39,8,JNE), IDESX_(39,9,JLE),
  IDESX_(40,0,JAN), IDESX_(40,1,JAZ), IDESX_(40,2,JAP), IDESX_(40,3,JANN),
  IDESX_(40,4,JANZ), IDESX_(40,5,JANP),
  IDESX_(40,6,JAE), IDESX_(40,7,JAO),
  IDESX_(41,0,J1N), IDESX_(41,1,J1Z), IDESX_(41,2,J1P), IDESX_(41,3,J1NN),
  IDESX_(41,4,J1NZ), IDESX_(41,5,J1NP),
  IDESX_(42,0,J2N), IDESX_(42,1,J2Z), IDESX_(42,2,J2P), IDESX_(42,3,J2NN),
  IDESX_(42,4,J2NZ), IDESX_(42,5,J2NP),
  IDESX_(43,0,J3N), IDESX_(43,1,J3Z), IDESX_(43,2,J3P), IDESX_(43,3,J3NN),
  IDESX_(43,4,J3NZ), IDESX_(43,5,J3NP),
  IDESX_(44,0,J4N), IDESX_(44,1,J4Z), IDESX_(44,2,J4P), IDESX_(44,3,J4NN),
  IDESX_(44,4,J4NZ), IDESX_(44,5,J4NP),
  IDESX_(45,0,J5N), IDESX_(45,1,J5Z), IDESX_(45,2,J5P), IDESX_(45,3,J5NN),
  IDESX_(45,4,J5NZ), IDESX_(45,5,J5NP),
  IDESX_(46,0,J6N), IDESX_(46,1,J6Z), IDESX_(46,2,J6P), IDESX_(46,3,J6NN),
  IDESX_(46,4,J6NZ), IDESX_(46,5,J6NP),
  IDESX_(47,0,JXN), IDESX_(47,1,JXZ), IDESX_(47,2,JXP), IDESX_(47,3,JXNN),
  IDESX_(47,4,JXNZ), IDESX_(47,5,JXNP),
  IDESX_(47,6,JXE), IDESX_(47,7,JXO),
  IDESX_(48,0,INCA), IDESX_(48,1,DECA), IDESX_(48,2,ENTA), IDESX_(48,3,ENNA),
  IDESX_(49,0,INC1), IDESX_(49,1,DEC1), IDESX_(49,2,ENT1), IDESX_(49,3,ENN1),
  IDESX_(50,0,INC2), IDESX_(50,1,DEC2), IDESX_(50,2,ENT2), IDESX_(50,3,ENN2),
  IDESX_(51,0,INC3), IDESX_(51,1,DEC3), IDESX_(51,2,ENT3), IDESX_(51,3,ENN3),
  IDESX_(52,0,INC4), IDESX_(52,1,DEC4), IDESX_(52,2,ENT4), IDESX_(52,3,ENN4),
  IDESX_(53,0,INC5), IDESX_(53,1,DEC5), IDESX_(53,2,ENT5), IDESX_(53,3,ENN5),
  IDESX_(54,0,INC6), IDESX_(54,1,DEC6), IDESX_(54,2,ENT6), IDESX_(54,3,ENN6),
  IDESX_(55,0,INCX), IDESX_(55,1,DECX), IDESX_(55,2,ENTX), IDESX_(55,3,ENNX),
  IDES_(56,5,CMPA), IDES_(57,5,CMP1), IDES_(58,5,CMP2), IDES_(59,5,CMP3),
  IDES_(60,5,CMP4), IDES_(61,5,CMP5), IDES_(62,5,CMP6), IDES_(63,5,CMPX)
};

static const gsize ID_TO_DESC_SIZE_=
sizeof(id_to_desc_)/sizeof(id_to_desc_[0]);

/* To look for the mix_ins_id corresponding to a pair (fspec,opcode)
   we use an array indexed by opcode with values
   (initial_ins, final_ins - initial_ins)
*/
struct mix_opcode_desc_
{
  mix_ins_id_t init_id;
  guchar       inc; /* when inc == 0, the id does not depend on fspec */
};

static struct mix_opcode_desc_ opcode_to_id_[MIX_BYTE_MAX + 1];

/* a hash table mapping strings to mix_ins_id's */
static GHashTable *string_to_id_ = NULL;

void
mix_init_ins (void)
{
  guint k;
  g_assert (ID_TO_DESC_SIZE_ == mix_INVALID_INS);

  for ( k = 0; k < MIX_BYTE_MAX + 1; ++k )
    {
      opcode_to_id_[k].init_id = mix_INVALID_INS;
      opcode_to_id_[k].inc = 0;
    }

  for ( k = 0; k < ID_TO_DESC_SIZE_; ++k )
    {
      if ( opcode_to_id_[id_to_desc_[k].opcode].init_id == mix_INVALID_INS )
	opcode_to_id_[id_to_desc_[k].opcode].init_id = k;
      else
	++opcode_to_id_[id_to_desc_[k].opcode].inc;
    }

  if ( string_to_id_ == NULL )
    {
      string_to_id_ = g_hash_table_new (g_str_hash, g_str_equal);
      for ( k = 0; k < ID_TO_DESC_SIZE_; ++k)
	g_hash_table_insert (string_to_id_, (gpointer)id_to_desc_[k].string_rep,
                             GUINT_TO_POINTER (k));
    }

}

void
mix_release_ins (void)
{
  g_hash_table_destroy (string_to_id_);
}

/* Conversions between words and ins */
mix_word_t
mix_ins_to_word (const mix_ins_t *ins)
{
  g_return_val_if_fail (ins != NULL, MIX_WORD_ZERO);
  return  (mix_word_t)((ins->address<<18)|
		       ((ins->index)<<12)|(ins->fspec<<6)|(ins->opcode));
}

mix_ins_id_t
mix_word_to_ins (mix_word_t word, mix_ins_t *ins)
{
  mix_ins_id_t result = mix_get_ins_id (mix_get_ins_opcode (word),
                                        mix_get_ins_fspec (word));

  g_return_val_if_fail (ins != NULL, result);
  mix_word_to_ins_uncheck (word,*ins);
  return result;
}

/* Getting ins parameters */
mix_opcode_t
mix_get_opcode_from_id (mix_ins_id_t id)
{
  g_return_val_if_fail (id < ID_TO_DESC_SIZE_, MIX_BYTE_ZERO);
  return id_to_desc_[id].opcode;
}

gboolean
mix_ins_id_is_extended (mix_ins_id_t id)
{
  g_return_val_if_fail (id < ID_TO_DESC_SIZE_, FALSE);
  return id_to_desc_[id].is_ext;
}


mix_fspec_t
mix_get_fspec_from_id (mix_ins_id_t id)
{
  g_return_val_if_fail (id < ID_TO_DESC_SIZE_, MIX_BYTE_ZERO);
  return id_to_desc_[id].def_fspec;
}


const gchar *
mix_get_string_from_id (mix_ins_id_t id)
{
  g_return_val_if_fail (id < ID_TO_DESC_SIZE_, NULL);
  return id_to_desc_[id].string_rep;
}

mix_ins_id_t
mix_get_id_from_string (const gchar *name)
{
  gpointer key, value;
  if ( !g_hash_table_lookup_extended (string_to_id_, (gpointer)name,
                                      &key, &value) )
    return mix_INVALID_INS;
  return (mix_ins_id_t)GPOINTER_TO_UINT (value);
}


mix_ins_id_t
mix_get_ins_id (mix_opcode_t code, mix_fspec_t fspec)
{
  if ( opcode_to_id_[code].inc == 0 )
    return opcode_to_id_[code].init_id;
  else if ( opcode_to_id_[code].inc < fspec )
    return mix_INVALID_INS;
  else
    return (opcode_to_id_[code].init_id + fspec);
}

/* Printable representation */
extern gchar * /* this pointer must be freed by caller */
mix_ins_to_string (const mix_ins_t *ins)
{
  gboolean needs_f;
  gchar *result;
  mix_ins_id_t id;

  g_return_val_if_fail (ins != NULL, NULL);
  id = mix_ins_id_from_ins (*ins);
  needs_f = ins->fspec != id_to_desc_[id].def_fspec;
  if ( needs_f )
    result =  g_strdup_printf ("%s\t%s%d,%d(%d:%d)",
                               mix_get_string_from_id (id),
                               mix_short_is_negative (ins->address) ? "-" : "",
                               mix_short_magnitude (ins->address), ins->index,
                               mix_fspec_left (ins->fspec),
                               mix_fspec_right (ins->fspec));
  else
    result =  g_strdup_printf ("%s\t%s%d,%d", mix_get_string_from_id (id),
                               mix_short_is_negative (ins->address) ? "-" : "",
                               mix_short_magnitude (ins->address), ins->index);
  return result;
}

extern void
mix_ins_to_string_in_buffer (const mix_ins_t *ins, gchar *buf, guint len)
{
  gboolean needs_f;
  mix_ins_id_t id;

  g_return_if_fail (ins != NULL);
  g_return_if_fail (buf != NULL);
  id = mix_ins_id_from_ins (*ins);
  needs_f = ins->fspec != id_to_desc_[id].def_fspec;
  if ( needs_f )
    g_snprintf (buf, len, "%s\t%s%d,%d(%d:%d)",
		mix_get_string_from_id (id),
		mix_short_is_negative (ins->address) ? "-" : "",
		mix_short_magnitude (ins->address), ins->index,
		mix_fspec_left (ins->fspec),
		mix_fspec_right (ins->fspec));
  else
    g_snprintf (buf, len, "%s\t%s%d,%d",
                mix_get_string_from_id (id),
                mix_short_is_negative (ins->address) ? "-" : "",
                mix_short_magnitude (ins->address), ins->index);
}



void
mix_ins_print (const mix_ins_t *ins)
{
  g_return_if_fail (ins != NULL);
  g_print ("%s", mix_get_string_from_id (mix_ins_id_from_ins (*ins)));
  g_print (" %s%d,%d(%d:%d)", mix_short_is_negative (ins->address) ? "-" : "+",
           mix_short_magnitude (ins->address), ins->index,
           mix_fspec_left (ins->fspec), mix_fspec_right (ins->fspec));
}

