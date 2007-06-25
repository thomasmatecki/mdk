/* -*-c-*- ------------------ xmix_io.h :
 * Implementation of mix_iochannel_t and mix_file_t
 * ------------------------------------------------------------------
 * Copyright (C) 2000, 2007 Free Software Foundation, Inc.
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


#ifndef XMIX_IOCHANNEL_H
#define XMIX_IOCHANNEL_H

#include <stdio.h>
#include "mix_io.h"

/* the actual definition of mix_iochannel_t */
struct mix_iochannel_t
{
  FILE *file;
};


extern const char * io_OPENTYPE_[5];

#define fmode_to_type_(mode) ( (mode) < 6 ? io_OPENTYPE_[(mode)]:NULL )

/* initialisation */
extern gboolean
io_init_from_fdesc_(mix_iochannel_t *ioc, int fdesc);

#define io_init_from_file_(ioc,f) (ioc)->file = f

#define io_close_(ioc)  fclose((ioc)->file)

/* write/read data */
#define write_data_(ioc,data,no)  \
 ( (no) == fwrite((const void*)(data), sizeof(*(data)), (no), (ioc)->file) )

#define read_data_(ioc,data,no)  \
 ( (no) == fread((void*)(data), sizeof(*(data)), (no), (ioc)->file) )

/* state */
#define is_eof_(ioc)  ( feof((ioc)->file) != 0 )
#define is_ready_(ioc) ( ferror((ioc)->file) == 0 )

/* conversions */
#define io_get_FILE_(ioc)  (MIX_IOCHANNEL(ioc))->file


#endif /* XMIX_IOCHANNEL_H */

