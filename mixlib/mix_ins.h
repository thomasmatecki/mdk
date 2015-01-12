/* -*-c-*- -------------------- mix_ins.h:
 * This file declares types and functions for manipulating MIX
 * instructions
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2006, 2007, 2010 Free Software Foundation, Inc.
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


#ifndef MIX_INS_H
#define MIX_INS_H

#include "mix_types.h"


/* Initialise and free mix_ins data */
extern void
mix_init_ins(void);

extern void
mix_release_ins(void);

/* A MIX instruction is made up of address, index, fspec and op_code */

/*-- Address field: contains two bytes */
typedef mix_short_t  mix_address_t;

/*-- Index field: a value between 0 and 6 */
typedef enum {
  mix_I0, mix_I1, mix_I2, mix_I3, mix_I4, mix_I5, mix_I6 } mix_index_t;

/*-- Instruction id: enumeration of MIX instruction set */
typedef enum {
  mix_NOP,  mix_ADD, mix_SUB, mix_MUL, mix_DIV,
  mix_NUM, mix_CHAR, mix_HLT,
  mix_SLA, mix_SRA, mix_SLAX, mix_SRAX, mix_SLC, mix_SRC, mix_SLB, mix_SRB,
  mix_MOVE, mix_LDA, mix_LD1, mix_LD2, mix_LD3, mix_LD4, mix_LD5,
  mix_LD6,  mix_LDX, mix_LDAN, mix_LD1N, mix_LD2N, mix_LD3N, mix_LD4N,
  mix_LD5N, mix_LD6N, mix_LDXN, mix_STA, mix_ST1, mix_ST2, mix_ST3, mix_ST4,
  mix_ST5, mix_ST6, mix_STX, mix_STJ, mix_STZ, mix_JBUS, mix_IOC, mix_IN,
  mix_OUT, mix_JRED,
  mix_JMP, mix_JSJ, mix_JOV, mix_JNOV, mix_JL, mix_JE, mix_JG, mix_JGE,
  mix_JNE, mix_JLE,
  mix_JAN, mix_JAZ, mix_JAP, mix_JANN, mix_JANZ, mix_JANP,
  mix_JAE, mix_JAO,
  mix_J1N, mix_J1Z, mix_J1P, mix_J1NN, mix_J1NZ, mix_J1NP,
  mix_J2N, mix_J2Z, mix_J2P, mix_J2NN, mix_J2NZ, mix_J2NP,
  mix_J3N, mix_J3Z, mix_J3P, mix_J3NN, mix_J3NZ, mix_J3NP,
  mix_J4N, mix_J4Z, mix_J4P, mix_J4NN, mix_J4NZ, mix_J4NP,
  mix_J5N, mix_J5Z, mix_J5P, mix_J5NN, mix_J5NZ, mix_J5NP,
  mix_J6N, mix_J6Z, mix_J6P, mix_J6NN, mix_J6NZ, mix_J6NP,
  mix_JXN, mix_JXZ, mix_JXP, mix_JXNN, mix_JXNZ, mix_JXNP,
  mix_JXE, mix_JXO,
  mix_INCA, mix_DECA, mix_ENTA, mix_ENNA,
  mix_INC1, mix_DEC1, mix_ENT1, mix_ENN1,
  mix_INC2, mix_DEC2, mix_ENT2, mix_ENN2,
  mix_INC3, mix_DEC3, mix_ENT3, mix_ENN3,
  mix_INC4, mix_DEC4, mix_ENT4, mix_ENN4,
  mix_INC5, mix_DEC5, mix_ENT5, mix_ENN5,
  mix_INC6, mix_DEC6, mix_ENT6, mix_ENN6,
  mix_INCX, mix_DECX, mix_ENTX, mix_ENNX,
  mix_CMPA, mix_CMP1, mix_CMP2, mix_CMP3, mix_CMP4,
  mix_CMP5, mix_CMP6, mix_CMPX, mix_INVALID_INS
} mix_ins_id_t;

/* each one of the above id's has associated an opcode, a default
   fspec and a string representation */
/* the opcode fits in a byte */
typedef mix_byte_t mix_opcode_t;

/* labels for each opcode */
enum {
  mix_opNOP = 0, mix_opADD, mix_opSUB, mix_opMUL, mix_opDIV,
  mix_opSPC, mix_opSLx, mix_opMOVE,
  mix_opLDA, mix_opLD1, mix_opLD2, mix_opLD3, mix_opLD4, mix_opLD5,
  mix_opLD6, mix_opLDX, mix_opLDAN, mix_opLD1N, mix_opLD2N, mix_opLD3N,
  mix_opLD4N, mix_opLD5N, mix_opLD6N, mix_opLDXN,
  mix_opSTA, mix_opST1, mix_opST2, mix_opST3, mix_opST4,
  mix_opST5, mix_opST6, mix_opSTX, mix_opSTJ, mix_opSTZ,
  mix_opJBUS, mix_opIOC, mix_opIN, mix_opOUT, mix_opJRED,
  mix_opJMP, mix_opJAx, mix_opJ1x, mix_opJ2x, mix_opJ3x,
  mix_opJ4x, mix_opJ5x, mix_opJ6x, mix_opJXx,
  mix_opINCA, mix_opINC1, mix_opINC2, mix_opINC3,
  mix_opINC4, mix_opINC5, mix_opINC6, mix_opINCX,
  mix_opCMPA, mix_opCMP1, mix_opCMP2, mix_opCMP3, mix_opCMP4,
  mix_opCMP5, mix_opCMP6, mix_opCMPX
};

extern mix_opcode_t
mix_get_opcode_from_id(mix_ins_id_t id);

extern mix_fspec_t
mix_get_fspec_from_id(mix_ins_id_t id);

/* For extended instructions, both the opcode and fspec determine
   the id (i.e., an explicit fspec cannot be used)
*/
extern gboolean
mix_ins_id_is_extended(mix_ins_id_t id);

extern const gchar *
mix_get_string_from_id(mix_ins_id_t id);

extern mix_ins_id_t
mix_get_id_from_string(const gchar *name);

extern mix_ins_id_t
mix_get_ins_id(mix_opcode_t code, mix_fspec_t fspec);


/*--  MIX instruction type */
typedef struct mix_ins_t mix_ins_t;
struct mix_ins_t
{
  mix_address_t address;
  mix_index_t   index;
  mix_fspec_t   fspec;
  mix_opcode_t  opcode;
};

#define mix_ins_fill_from_id(ins,id)		\
   do {						\
     (ins).opcode = mix_get_opcode_from_id(id);	\
     (ins).fspec = mix_get_fspec_from_id(id);	\
   } while(FALSE)


/* A mix ins can be codified into a word */
extern mix_word_t
mix_ins_to_word(const mix_ins_t *ins);

extern mix_ins_id_t
mix_word_to_ins(mix_word_t w, mix_ins_t *ins);

#define mix_word_add_address(word,addr) (word) |= ((addr)<<18)

/* decompose an instruction codified in a word into its parts */
#define mix_get_ins_address(word) ((mix_address_t)((word)>>18))
#define mix_get_ins_index(word)   ((mix_index_t)(((word)>>12)&7))
#define mix_get_ins_fspec(word)   ((mix_fspec_t)(mix_byte_new((word)>>6)))
#define mix_get_ins_opcode(word)  ((mix_opcode_t)(mix_byte_new(word)))

/* unchecked versions for speed */
#define mix_ins_to_word_uncheck(ins)					\
 (mix_word_t)(((ins).address<<18)|					\
	      (((ins).index)<<12)|((ins).fspec<<6)|((ins).opcode))

#define mix_word_to_ins_uncheck(word,ins)	\
  do {						\
    (ins).address = mix_get_ins_address(word);	\
    (ins).index = mix_get_ins_index(word);	\
    (ins).fspec = mix_get_ins_fspec(word);	\
    (ins).opcode = mix_get_ins_opcode(word);	\
  } while(FALSE)

#define mix_ins_id_from_ins(ins)  mix_get_ins_id((ins).opcode,(ins).fspec)


/* Printable representation */
extern gchar * /* this pointer must be freed by caller */
mix_ins_to_string(const mix_ins_t *ins);

extern void
mix_ins_to_string_in_buffer (const mix_ins_t *ins, gchar *buf, guint len);

extern void
mix_ins_print(const mix_ins_t *ins);


#endif /* MIX_INS_H */
