#include "..\stdafx.h"
#include "g_rc4.h"

/////////////////////////////////////////////////////////////////////////////
// 
// RC4.cpp
// 
// Stream encryption cypher
// Note that the algorithm is symmetric - ie decryption ð decryption
// 
// The encryption algorithm used in this module was derived from 
// 'Applied Cryptography 2nd Ed' (c) 1996 by Bruce Schneider
// and was developed by Ron Rivest for RSA Data Security Inc.
// 
/////////////////////////////////////////////////////////////////////////////
// 
// To encrypt a small buffer, simply call Crypt
// To encrypt a stream call InitStream and then call CryptStream as many
// times as needed.
/////////////////////////////////////////////////////////////////////////////


CRC4::CRC4 ()
	{
	}

CRC4::CRC4 (BYTE* ptrKey, int iKeyLen)
	{
	InitStream (ptrKey, iKeyLen);
	}

CRC4::~CRC4 ()
	{
	}

//	Initialize the stream cypher.	if iKeyLen is 0, ptrKey is assumed
//	to be a null terminated string
//
	void CRC4::InitStream (BYTE* ptrKey, int iKeyLen)
	{
	BYTE* psz = ptrKey;
	UCHAR j, tmp, k[256];
	int   i, x;

	iKeyLen = iKeyLen ? iKeyLen : strlen ((char*)psz);

	for (m_cI=m_cJ=i=x=0; i<256; i++, x++)
		{
		m_s[i] =i;
		if (x==iKeyLen) psz = ptrKey, x=0;
		k[i] = *psz++;
		}
	for (j=i=0; i<256; i++)
		{
		j   = (j + m_s[i] + k[i]);
		tmp = m_s[i], m_s[i] = m_s[j], m_s[j] = tmp;
		}
	}

//	encrypt/decrypt a stream. if iSrcLen is 0, ptrIn is assumed
//	to be a null terminated string
//
BYTE* CRC4::CryptStream (BYTE* ptrOut, BYTE* ptrIn, int iSrcLen)
	{
	int  i;
	BYTE tmp, t;
	BYTE* pHold = ptrOut;

	iSrcLen = iSrcLen ? iSrcLen : strlen ((char*)ptrIn);

	for (i=0; i< iSrcLen; i++)
		{
		m_cI += 1;
		m_cJ += m_s[m_cI];
		tmp = m_s[m_cI], m_s[m_cI] = m_s[m_cJ], m_s[m_cJ] = tmp;
		t   = m_s[m_cI] + m_s[m_cJ];
		*ptrOut++ = *ptrIn++ ^ m_s[t];
		}
	return pHold;
	}

//	encrypt/decrypt a stream. if iSrcLen is 0, ptrIn is assumed
//	to be a null terminated string
//
BYTE* CRC4::CryptStreamInPlace (BYTE* pBuff, int iSrcLen)
	{
	int  i;
	BYTE tmp, t;
	BYTE* pHold = pBuff;

	iSrcLen = iSrcLen ? iSrcLen : strlen ((char*)pBuff);

	for (i=0; i< iSrcLen; i++)
		{
		m_cI += 1;
		m_cJ += m_s[m_cI];
		tmp = m_s[m_cI], m_s[m_cI] = m_s[m_cJ], m_s[m_cJ] = tmp;
		t   = m_s[m_cI] + m_s[m_cJ];
		*pBuff++ ^= m_s[t];
		}
	return pHold;
	}


BYTE* CRC4::Crypt (BYTE* ptrOut, BYTE* ptrIn, int iSrcLen, BYTE* ptrKey, int iKeyLen)
	{
	InitStream (ptrKey, iKeyLen);
	return CryptStream (ptrOut, ptrIn, iSrcLen);
	}
