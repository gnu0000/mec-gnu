// G_SecureZipArchive.h: interface for the CSecureZipArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_G_SECUREZIPARCHIVE_H__7CCE137D_1001_434B_B035_E67BBF11FC0B__INCLUDED_)
#define AFX_G_SECUREZIPARCHIVE_H__7CCE137D_1001_434B_B035_E67BBF11FC0B__INCLUDED_
#pragma once

#include "G_GZFile.h"
#include "../RC4/G_RC4.h"


class CSecureGZFile : public CGZFile
   {
public:
	CRC4    m_cRC4;
	CString m_strKey;
	CFile   m_cFile;
	BOOL	  m_bEncrypted;

public:
	CSecureGZFile ();
	virtual ~CSecureGZFile ();

//	virtual LPCTSTR Error (int* pError);

	// this is currently the only open that supports encryption
  	virtual int Open (LPCTSTR lpszFile, LPCTSTR lpszMode, LPCTSTR lpszEncryptKey = NULL);

  	virtual int Open (LPCTSTR lpszFile, LPCTSTR lpszMode);
  	virtual int Open (LPCTSTR lpszFile, LPCTSTR lpszMode, BASEIO pfnIO, PVOID pUserData);

	virtual int Close ();
	virtual int Flush (int iMode);
	virtual BOOL IsEof();

	// now disallowed if encrypted
	virtual int Seek (int iOffset, int iFrom);
	virtual int Rewind ();
	virtual int GetPosition ();

   };




class CSecureZipArchive : public CZipArchive  
	{
protected:

public:
	CGZFile m_cGZFile;
	CRC4    m_cRC4;
	CString m_strKey;

	CSecureZipArchive();
	virtual ~CSecureZipArchive();

// 	int OpenArchive (LPCTSTR lpszFile, int iMode);
// 	int CloseArchive ();
//	int CloseArchive (LPCTSTR lpszComment);
//	int GetCurrentMode();
 
 	/*--- read zip archive operations ---*/
	//	int GetInfo ();
//	int GetComment (LPTSTR lpszComment, int iMaxSize);
 
 	/*--- zip directory operations ---*/
// 	int GotoFirstFile ();
// 	int GotoNextFile ();
// 	int LocateFile	(LPCTSTR lpszFile);
//	int GetCurrentFileInfo (LPTSTR lpszFileName, int iNameSize, LPTSTR lpszComment, int iCommentSize);

 	/*--- write zip archive operations ---*/
	int OpenNewFile (LPCTSTR lpszName, LPCTSTR lpszComment, int iLevel, LPCTSTR lpszEncryptKey = NULL);
	int OpenCurrentFile (LPCTSTR lpszEncryptKey = NULL);
 	int CloseFile ();
 
 	/*--- read operations ---*/
 	int Read (LPVOID pBuff, UINT uLen);
 	int ReadChar ();
	LPTSTR ReadString (LPTSTR lpszStr, UINT uMaxLen);
 
 	/*--- write operations ---*/
 	int Write (LPVOID pBuff, UINT uLen);
 	int WriteChar (CHAR c);
 	int WriteString (LPCTSTR lpszStr);

public:
 	int CryptRead (LPVOID pBuff, UINT uLen);
 	int CryptWrite (LPVOID pBuff, UINT uLen);
	};

#endif // !defined(AFX_G_SECUREZIPARCHIVE_H__7CCE137D_1001_434B_B035_E67BBF11FC0B__INCLUDED_)
