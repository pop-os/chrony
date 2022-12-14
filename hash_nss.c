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

  Routines implementing crypto hashing using NSSLOWHASH API of the NSS library.

  */

#include "config.h"

#include <nss.h>
#include <hasht.h>
#include <nsslowhash.h>

#include "hash.h"
#include "util.h"

static NSSLOWInitContext *ictx;

struct hash {
  HASH_HashType type;
  HSH_Algorithm algorithm;
  NSSLOWHASHContext *context;
};

static struct hash hashes[] = {
  { HASH_AlgMD5, HSH_MD5, NULL },
  { HASH_AlgSHA1, HSH_SHA1, NULL },
  { HASH_AlgSHA256, HSH_SHA256, NULL },
  { HASH_AlgSHA384, HSH_SHA384, NULL },
  { HASH_AlgSHA512, HSH_SHA512, NULL },
  { 0, 0, NULL }
};

int
HSH_GetHashId(HSH_Algorithm algorithm)
{
  int i;

  if (algorithm == HSH_MD5_NONCRYPTO)
    algorithm = HSH_MD5;

  for (i = 0; hashes[i].algorithm != 0; i++) {
    if (hashes[i].algorithm == algorithm)
      break;
  }

  if (hashes[i].algorithm == 0)
    return -1; /* not found */

  if (!ictx && !(ictx = NSSLOW_Init()))
    return -1; /* couldn't init NSS */

  if (!hashes[i].context &&
      !(hashes[i].context = NSSLOWHASH_NewContext(ictx, hashes[i].type)))
    return -1; /* couldn't init hash */

  return i;
}

int
HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
         unsigned char *out, int out_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  unsigned int ret = 0;

  if (in1_len < 0 || in2_len < 0 || out_len < 0)
    return 0;

  NSSLOWHASH_Begin(hashes[id].context);
  NSSLOWHASH_Update(hashes[id].context, in1, in1_len);
  if (in2)
    NSSLOWHASH_Update(hashes[id].context, in2, in2_len);
  NSSLOWHASH_End(hashes[id].context, buf, &ret, sizeof (buf));

  ret = MIN(ret, out_len);
  memcpy(out, buf, ret);

  return ret;
}

void
HSH_Finalise(void)
{
  int i;

  for (i = 0; hashes[i].algorithm != 0; i++) {
    if (hashes[i].context)
      NSSLOWHASH_Destroy(hashes[i].context);
  }

  if (ictx)
    NSSLOW_Shutdown(ictx);
}
