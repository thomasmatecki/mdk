/* -*-c-*- ------------------ xmix_io.c :
 * Implementation of the functions declared in xmix_io.h
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "xmix_io.h"

const char *io_OPENTYPE_[5] = { "r", "w", "w+", "a", "a+" };


/* initialise a mix_iochannel from a file descriptor */
gboolean
io_init_from_fdesc_(mix_iochannel_t *ioc, int fdesc)
{
  FILE *file;
  mix_fmode_t mode;
  int flags = fcntl(fdesc, F_GETFL);

  if ( (flags&O_RDONLY) == O_RDONLY )
    mode = mix_io_READ;
  else if ( (flags&O_WRONLY) == O_WRONLY )
    mode = mix_io_WRITE;
  else if ( (flags&O_RDWR) == O_RDWR )
    mode = mix_io_RDWRT;
  else
    return FALSE;

  file = fdopen(fdesc, fmode_to_type_(mode));
  g_return_val_if_fail(file != NULL, FALSE);
  if (mode == mix_io_RDWRT) rewind (file);
  io_init_from_file_(ioc, file);

  return TRUE;
}



