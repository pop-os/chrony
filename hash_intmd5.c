/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2012
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Routines implementing crypto hashing using internal MD5 implementation.

  */

#include "config.h"
#include "sysincl.h"
#include "hash.h"
#include "memory.h"
#include "util.h"

#include "md5.c"

static MD5_CTX ctx;

int
HSH_GetHashId(HSH_Algorithm algorithm)
{
  /* only MD5 is supported */
  if (algorithm != HSH_MD5 && algorithm != HSH_MD5_NONCRYPTO)
    return -1;

  return 0;
}

int
HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
         unsigned char *out, int out_len)
{
  if (in1_len < 0 || in2_len < 0 || out_len < 0)
    return 0;

  MD5Init(&ctx);
  MD5Update(&ctx, in1, in1_len);
  if (in2)
    MD5Update(&ctx, in2, in2_len);
  MD5Final(&ctx);

  out_len = MIN(out_len, 16);

  memcpy(out, ctx.digest, out_len);

  return out_len;
}

void
HSH_Finalise(void)
{
}
