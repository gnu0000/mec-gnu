// SecureZipArchive.cpp: implementation of the CSecureZipArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "G_SecureZipArchive.h"

//////////////////////////////////////////////////////////////////////
//
//	CSecureGZFile
//
CSecureGZFile::CSecureGZFile ()
	{
	CGZFile ();
	m_strKey = "";
	m_bEncrypted = FALSE;
	}

CSecureGZFile::~CSecureGZFile ()
	{
	}

// non class member support fn
int SecureGZReadFn (void* pBuff, int iBuffSize, void* pUserData)
	{
	CSecureGZFile* pGZ = (CSecureGZFile*)pUserData;
	int iRead = pGZ->m_cFile.Read (pBuff, iBuffSize);
	pGZ->m_cRC4.CryptStreamInPlace ((BYTE*)pBuff, iRead);
	return iRead;
	}

// non class member support fn
int SecureGZWriteFn (void* pBuff, int iBuffSize, void* pUserData)
	{
	CSecureGZFile* pGZ = (CSecureGZFile*)pUserData;
	pGZ->m_cRC4.CryptStreamInPlace ((BYTE*)pBuff, iBuffSize);
	TRY
		{
		pGZ->m_cFile.Write (pBuff, iBuffSize);
		}
   CATCH(CFileException, pEx)
   	{
		return 0;
		}
   END_CATCH

	return iBuffSize;
	}

//	This open supports encryption
//
//
int CSecureGZFile::Open (LPCTSTR lpszFile, LPCTSTR lpszMode, LPCTSTR lpszEncryptKey)
	{
	m_strKey 	 = CString(lpszEncryptKey);
	m_bEncrypted = !m_strKey.IsEmpty();

	if (!m_bEncrypted)
		return CGZFile::Open (lpszFile, lpszMode);

	m_cRC4.InitStream ((BYTE*)(LPCTSTR)m_strKey);

	BOOL bWriting = (toupper (*lpszMode) == 'W');
	UINT iFlags = (bWriting ? CFile::modeWrite | CFile::modeCreate : CFile::modeRead);

	if (!m_cFile.Open (lpszFile, iFlags))
		return Z_STREAM_ERROR;

	if (bWriting)
		return CGZFile::Open (lpszFile, lpszMode, SecureGZWriteFn, this);
	else
		return CGZFile::Open (lpszFile, lpszMode, SecureGZReadFn, this);
	}

int CSecureGZFile::Open (LPCTSTR lpszFile, LPCTSTR lpszMode)
	{
	m_bEncrypted = FALSE;
	m_strKey.Empty ();
	return CGZFile::Open (lpszFile, lpszMode);
	}

int CSecureGZFile::Open (LPCTSTR lpszFile, LPCTSTR lpszMode, BASEIO pfnIO, PVOID pUserData)
	{
	m_bEncrypted = FALSE;
	m_strKey.Empty ();
	return CGZFile::Open (lpszFile, lpszMode, pfnIO, pUserData);
	}


int CSecureGZFile::Close ()
	{
	int iRet = CGZFile::Close ();
	if (m_bEncrypted)
		m_cFile.Close ();
	return iRet;
	}


int CSecureGZFile::Flush (int iMode)
	{
	int iRet = CGZFile::Flush (iMode);
	if (m_bEncrypted)
		m_cFile.Flush ();
	return iRet;
	}


BOOL CSecureGZFile::IsEof()
	{
	if (m_bEncrypted)
		return Z_VERSION_ERROR;

	return CGZFile::IsEof ();
	}


int CSecureGZFile::Seek (int iOffset, int iFrom)
	{
	if (m_bEncrypted)
		return Z_VERSION_ERROR;

	return CGZFile::Seek (iOffset, iFrom);
	}

int CSecureGZFile::Rewind ()
	{
	if (m_bEncrypted)
		return Z_VERSION_ERROR;

//	if (!m_bEncrypted)
		return CGZFile::Rewind ();
//	m_cRC4.InitStream ((BYTE*)(LPCTSTR)m_strKey);
//	m_cFile.SeekToBegin ();
//	return 0;
	}

int CSecureGZFile::GetPosition ()
	{
	if (m_bEncrypted)
		return (int) m_cFile.GetPosition ();
	return CGZFile::GetPosition ();
	}

//////////////////////////////////////////////////////////////////////
//
//	CSecureZipArchive
//
CSecureZipArchive::CSecureZipArchive()
	{
	CZipArchive::CZipArchive ();
	m_strKey = "";
	}

CSecureZipArchive::~CSecureZipArchive()
	{
	}

int SecureReadFn (void* pBuff, int iBuffSize, void* pUserData)
	{
	CSecureZipArchive *pArc = (CSecureZipArchive *)pUserData;
	int iRet = pArc->CryptRead (pBuff, iBuffSize);
	return iRet;
	}

int SecureWriteFn (void* pBuff, int iBuffSize, void* pUserData)
	{
	CSecureZipArchive *pArc = (CSecureZipArchive *)pUserData;
  	int iRet = pArc->CryptWrite (pBuff, iBuffSize);
	return (iRet ? 0 : iBuffSize);
	}


int CSecureZipArchive::OpenNewFile (LPCTSTR pszName, LPCTSTR lpszComment, int iLevel, LPCTSTR lpszEncryptKey)
	{
	int iRet;

	m_strKey = CString(lpszEncryptKey);
	// this chops off the leading path info
	//	PSZ p1 = strrchr (pszName, '\\');
	//	PSZ p2 = (p1 ? p1+1 : (PSZ)pszName);
	//	CString strZipName = p2;
	CString strZipName = pszName;

	//	If the caller provides a key then we'll gzip and encrypt the file before 
	//	adding it to the zip.
	if (!m_strKey.IsEmpty())
		{
		m_cRC4.InitStream ((BYTE*)(LPCTSTR)m_strKey);
		iRet = CZipArchive::OpenNewFile (strZipName+".gz.enc", NULL, Z_NO_COMPRESSION);
		m_cGZFile.Open (pszName, "wb", SecureWriteFn, this);
		}
	else
		{
		iRet = CZipArchive::OpenNewFile (strZipName, NULL, Z_DEFAULT_COMPRESSION);
		}
	return iRet;
	}

//	 0	ok
//	-1 password key needed
//	-2	incorrect password
//	-3	problem reading zip
//	-4	problem reading file in zip
//	-5 internal problem
//
int CSecureZipArchive::OpenCurrentFile (LPCTSTR lpszEncryptKey)
	{
	char szFilename [512], szComment [8];

	m_strKey.Empty();
	if (GetCurrentFileInfo (szFilename, sizeof (szFilename), szComment, sizeof (szComment)))
		return -3;
	int iLen = strlen (szFilename);
	if (iLen > 7 && !stricmp (szFilename+iLen-7, ".gz.enc"))
		{
		m_strKey = CString(lpszEncryptKey);
		if (m_strKey.IsEmpty())
			return (-1); // needs a key
		m_cRC4.InitStream ((BYTE*)(LPCTSTR)m_strKey);
		}
	if (CZipArchive::OpenCurrentFile ())
		return -4;
	if (m_strKey.IsEmpty())
		return 0;

	if (m_cGZFile.Open ("gzfile", "rb", SecureReadFn, this))
		{
		CZipArchive::CloseFile ();
		return -5;
		}
	if (!m_cGZFile.FileIsGZipped ())
		{
		m_cGZFile.Close();
		CZipArchive::CloseFile ();
		return -2;
		}
	return 0;
	}


int CSecureZipArchive::CloseFile ()
	{
	if (!m_strKey.IsEmpty())
		m_cGZFile.Close();
	return CZipArchive::CloseFile ();
	}

int CSecureZipArchive::Read (LPVOID pBuff, UINT uLen)
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.Read (pBuff, uLen);
	else 
		return CZipArchive::Read (pBuff, uLen);
	}

int CSecureZipArchive::ReadChar ()
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.ReadChar ();
	else 
		return CZipArchive::ReadChar ();
	}

LPTSTR CSecureZipArchive::ReadString (LPTSTR lpszStr, UINT uMaxLen)
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.ReadString (lpszStr, uMaxLen);
	else 
		return CZipArchive::ReadString (lpszStr, uMaxLen);
	}

int CSecureZipArchive::Write (LPVOID pBuff, UINT uLen)
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.Write (pBuff, uLen);
	else 
		return CZipArchive::Write (pBuff, uLen);
	}

int CSecureZipArchive::WriteChar (CHAR c)
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.WriteChar (c);
	else 
		return CZipArchive::WriteChar (c);
	}

int CSecureZipArchive::WriteString (LPCTSTR lpszStr)
	{
	if (!m_strKey.IsEmpty())
		return m_cGZFile.WriteString (lpszStr);
	else 
		return CZipArchive::WriteString (lpszStr);
	}

int CSecureZipArchive::CryptRead (LPVOID pBuff, UINT uLen)
	{
	int iRet = CZipArchive::Read (pBuff, uLen);
	if (!m_strKey.IsEmpty())
		m_cRC4.CryptStreamInPlace ((BYTE*)pBuff, uLen);
	return iRet;
	}

int CSecureZipArchive::CryptWrite (LPVOID pBuff, UINT uLen)
	{
	if (!m_strKey.IsEmpty())
		m_cRC4.CryptStreamInPlace ((BYTE*)pBuff, uLen);
	return CZipArchive::Write (pBuff, uLen);
	}
