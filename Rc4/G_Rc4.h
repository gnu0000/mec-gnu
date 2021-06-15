#if !defined(AFX_G_RC4_H__INCLUDED_)
#define AFX_G_RC4_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
//
// G_RC4.h
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
//	Whenever a iSrcLen, or iKeyLen is zero, the data in question is assumed
//	to be a null terminated string.
/////////////////////////////////////////////////////////////////////////////


class CRC4
	{
public:
	CRC4 ();
	CRC4 (BYTE* ptrKey, int iKeyLen=0);
	~CRC4 ();
	void InitStream (BYTE* ptrKey, int iKeyLen=0);
	BYTE* CryptStream (BYTE* ptrOut, BYTE* ptrIn, int iSrcLen=0);
	BYTE* CryptStreamInPlace (BYTE* pBuff, int iSrcLen=0);
	BYTE* Crypt (BYTE* ptrOut, BYTE* ptrIn, int iSrcLen, BYTE* ptrKey, int iKeyLen=0);

private:
	BYTE m_cI; 
	BYTE m_cJ; 
	BYTE m_s[256];
	};


#endif // !defined(AFX_G_RC4_H__INCLUDED_)
