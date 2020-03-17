/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019
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

  Support for AES128 and AES256 CMAC in Nettle.

  */

#include "config.h"

#include "sysincl.h"

#include <nettle/cmac.h>

#include "cmac.h"
#include "memory.h"

struct CMC_Instance_Record {
  int key_length;
  union {
    struct cmac_aes128_ctx aes128;
    struct cmac_aes256_ctx aes256;
  } context;
};

/* ================================================== */

unsigned int
CMC_GetKeyLength(const char *cipher)
{
  if (strcmp(cipher, "AES128") == 0)
    return AES128_KEY_SIZE;
  else if (strcmp(cipher, "AES256") == 0)
    return AES256_KEY_SIZE;
  return 0;
}

/* ================================================== */

CMC_Instance
CMC_CreateInstance(const char *cipher, const unsigned char *key, unsigned int length)
{
  CMC_Instance inst;

  if (length == 0 || length != CMC_GetKeyLength(cipher))
    return NULL;

  inst = MallocNew(struct CMC_Instance_Record);
  inst->key_length = length;

  switch (length) {
    case AES128_KEY_SIZE:
      cmac_aes128_set_key(&inst->context.aes128, key);
      break;
    case AES256_KEY_SIZE:
      cmac_aes256_set_key(&inst->context.aes256, key);
      break;
    default:
      assert(0);
  }

  return inst;
}

/* ================================================== */

unsigned int
CMC_Hash(CMC_Instance inst, const unsigned char *in, unsigned int in_len,
         unsigned char *out, unsigned int out_len)
{
  if (out_len > CMAC128_DIGEST_SIZE)
    out_len = CMAC128_DIGEST_SIZE;

  switch (inst->key_length) {
    case AES128_KEY_SIZE:
      cmac_aes128_update(&inst->context.aes128, in_len, in);
      cmac_aes128_digest(&inst->context.aes128, out_len, out);
      break;
    case AES256_KEY_SIZE:
      cmac_aes256_update(&inst->context.aes256, in_len, in);
      cmac_aes256_digest(&inst->context.aes256, out_len, out);
      break;
    default:
      assert(0);
  }

  return out_len;
}

/* ================================================== */

void
CMC_DestroyInstance(CMC_Instance inst)
{
  Free(inst);
}
