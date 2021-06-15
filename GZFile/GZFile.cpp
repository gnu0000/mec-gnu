// GZFile.cpp: implementation of the CGZFile class.
//
//////////////////////////////////////////////////////////////////////

#include <assert.h>
#include "G_GZFile.h"
#include "zlib.h"
#include "zip.h"
#include "unzip.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGZFile::CGZFile ()
	{
	m_iMode = -1;
	}

CGZFile::~CGZFile ()
	{
	}

int CGZFile::Open (LPCTSTR lpszFile, LPCTSTR lpszMode)
	{
	if (!(m_file = gzopen (lpszFile, lpszMode)))
		return -1;
	m_iMode = 1;
	return Z_OK;
	}

int CGZFile::Open (LPCTSTR lpszFile, LPCTSTR lpszMode, BASEIO pfnIO, PVOID pUserData)
	{
	if (!(m_file = gzopen2 (lpszFile, lpszMode, pfnIO, pUserData)))
		return -1;
	m_iMode = 1;
	return Z_OK;
	}


int CGZFile::Close ()
	{
	m_iMode = -1;
	return gzclose (m_file);
	}

int CGZFile::Flush (int iMode)
	{
	return gzflush (m_file, iMode);
	}

int CGZFile::Seek (int iOffset, int iFrom)
	{
	return gzseek (m_file, iOffset, iFrom);
	}

int CGZFile::Rewind ()
	{
	return gzrewind (m_file);
	}

int CGZFile::GetPosition ()
	{
	return gztell (m_file);
	}

int CGZFile::Read (LPVOID pBuff, UINT uLen)
	{
	return gzread (m_file, pBuff, uLen);
	}

int CGZFile::ReadChar ()
	{
	return gzgetc (m_file);
	}

LPTSTR CGZFile::ReadString (LPTSTR lpszStr, UINT uMaxLen)
	{
	return gzgets (m_file, lpszStr, uMaxLen);
	}

int CGZFile::Write (LPVOID pBuff, UINT uLen)
	{
	return gzwrite (m_file, pBuff, uLen);
	}

int CGZFile::WriteChar (int c)
	{
	return gzputc (m_file, c);
	}

int CGZFile::WriteString (LPCTSTR lpszStr)
	{
	return gzputs (m_file, lpszStr);
	}

BOOL CGZFile::IsEof()
	{
	return gzeof (m_file);
	}

LPCTSTR CGZFile::Error (int* pError)
	{
	return gzerror (m_file, pError);
	}

int CGZFile::GetCurrentMode ()
	{
	return m_iMode;
	}

BOOL CGZFile::FileIsGZipped ()
	{
	return !gzIsTransparent (m_file);
	}


///////////////////////////////////////////////////////////////// 

CZipArchive::CZipArchive ()
	{
	m_zipArchive = NULL;
	m_iMode = -1;
	}

CZipArchive::~CZipArchive ()
	{
	}

// iMode: 0=read 1=write 2=append
int CZipArchive::OpenArchive (LPCTSTR lpszFile, int iMode)
	{
	m_iMode = iMode;

	if (!m_iMode)
		m_zipArchive = unzOpen (lpszFile);
	else
		m_zipArchive = zipOpen (lpszFile, iMode-1);
	return (m_zipArchive ? 0 : -1);
	}

int CZipArchive::CloseArchive (LPCTSTR lpszComment)
	{
	int iRet;
	if (!m_iMode)
		iRet = unzClose (m_zipArchive);
	else
		iRet = zipClose (m_zipArchive, lpszComment);
	m_iMode = -1;
	return iRet;
	}

int CZipArchive::CloseArchive ()
	{
	return CloseArchive (NULL);
	}

//int CZipArchive::GetInfo ()
//	{
//	}

int CZipArchive::GetComment (LPTSTR lpszComment, int iMaxSize)
	{
	assert (m_iMode == 0);
 	return unzGetGlobalComment (m_zipArchive, lpszComment, iMaxSize);
	}

int CZipArchive::GotoFirstFile ()
	{
	return unzGoToFirstFile (m_zipArchive);
	}

int CZipArchive::GotoNextFile ()
	{
	return unzGoToNextFile (m_zipArchive);
	}

int CZipArchive::LocateFile (LPCTSTR lpszFile)
	{
	return unzLocateFile (m_zipArchive, lpszFile, 2);
	}

// add file info
int CZipArchive::GetCurrentFileInfo	(LPTSTR lpszFileName, int iNameSize, LPTSTR lpszComment, int iCommentSize)
	{
	return unzGetCurrentFileInfo (m_zipArchive, NULL, lpszFileName, iNameSize, NULL, 0, lpszComment, iCommentSize);
	}


int CZipArchive::FileNameCompare (LPCTSTR lpszName1, LPCTSTR lpszName2, BOOL bCaseSensitive)
	{
	int iCaseSensitive = (bCaseSensitive ? 1 : 2);
	return unzStringFileNameCompare (lpszName1, lpszName2, iCaseSensitive);
	}

#define Z_DEFLATED   8

////////////////////////////////////////////////////////////////////////
int CZipArchive::OpenNewFile (LPCTSTR lpszName, LPCTSTR lpszComment, int iLevel, LPCTSTR lpszEncryptKey)
	{
	assert (m_iMode > 0);
	return zipOpenNewFileInZip2 (m_zipArchive, lpszName, NULL, NULL, 0, NULL, 0, 
	                             lpszComment, Z_DEFLATED, iLevel, lpszEncryptKey);
	}

int CZipArchive::OpenCurrentFile (LPCTSTR lpszEncryptKey)
	{
	assert (m_iMode == 0);
	return unzOpenCurrentFile2 (m_zipArchive, lpszEncryptKey);
	}

int CZipArchive::CloseFile ()
	{
	if (m_iMode == 0)
		return unzCloseCurrentFile (m_zipArchive);
	else
		return zipCloseFileInZip (m_zipArchive);
	}

int CZipArchive::Read (LPVOID pBuff, UINT uLen)
	{
	assert (m_iMode == 0);
	return unzReadCurrentFile (m_zipArchive, pBuff, uLen);
	}

int CZipArchive::ReadChar ()
	{
	assert (m_iMode == 0);
	unsigned char c;
	int iRet = unzReadCurrentFile (m_zipArchive, &c, 1);
	if (iRet)
		return (int)c;
	else
		return -1;
	}

LPTSTR CZipArchive::ReadString (LPTSTR lpszStr, UINT uMaxLen)
	{
	int c;
	UINT uLen;
	assert (m_iMode == 0);
	for (uLen=0; uLen < uMaxLen; uLen++)
		{
		if ((c = ReadChar ()) < 0)
			break;
		lpszStr[uLen] = c;
		if (c == '\x0A')
			break;
		}
	if (c < 0 && !uLen)
		return NULL;
	if (uLen < uMaxLen)
		lpszStr[uLen] = '\0';
	return lpszStr;
	}

//int CZipArchive::GetInfo (...)
//	{
//	}

int CZipArchive::GetPosition ()
	{
	assert (m_iMode == 0);
	return unztell (m_zipArchive);
	}

BOOL CZipArchive::IsEof()
	{
	assert (m_iMode == 0);
	return unzeof (m_zipArchive);
	}

int CZipArchive::Write (LPVOID pBuff, UINT uLen)
	{
	assert (m_iMode > 0);
	return zipWriteInFileInZip (m_zipArchive, pBuff, uLen);
	}

int CZipArchive::WriteChar (CHAR c)
	{
	assert (m_iMode > 0);
	return zipWriteInFileInZip (m_zipArchive, &c, 1);
	}

int CZipArchive::WriteString (LPCTSTR lpszStr)
	{
	assert (m_iMode > 0);
	return zipWriteInFileInZip (m_zipArchive, (PVOID)lpszStr, strlen (lpszStr));
	}

int CZipArchive::GetCurrentMode ()
	{
	return m_iMode;
	}

