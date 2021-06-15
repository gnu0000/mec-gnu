/*
 *
 * rc4.c
 * Thursday, 4/17/1997.
 *
 *
 * The encryption algorithm used in this module was derived from 
 * 'Applied Cryptography 2nd Ed' (c) 1996 by Bruce Schneider
 * and was developed by Ron Rivest for RSA Data Security Inc.
 *
 */
#include "gzrc4.h"

static UCHAR cI, cJ, s[256];

void RC4InitSBox (PSZ pszKey)
   {
   PSZ   psz = pszKey;
   UCHAR j, tmp, k[256];
   int   i;

   for (cI=cJ=i=0; i<256; i++)
      {
      s[i]=i;
      if (!*psz) psz = pszKey;
      k[i]= *psz++;
      }
   for (j=i=0; i<256; i++)
      {
      j = (j + s[i] + k[i]);
      tmp  = s[i], s[i] = s[j], s[j] = tmp;
      }
   }

PSZ RC4CryptStream (PSZ pszOut, PSZ pszIn, int iSrcLen)
   {
   int   i;
   UCHAR tmp, t;

	PSZ pszBuff = pszOut;

   for (i=0; i< iSrcLen; i++)
      {
      cI += 1;
      cJ += s[cI];
      tmp = s[cI], s[cI] = s[cJ], s[cJ] = tmp;
      t   = (s[cI] + s[cJ]);
      *pszOut++ = *pszIn++ ^ s[t];
      }
   return pszBuff;
   }

