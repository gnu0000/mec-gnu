// GZFile.h: interface for the CGZFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GZFILE_H__D2BBE027_F4F1_11D3_829C_0050DA0C5DE1__INCLUDED_)
#define AFX_GZFILE_H__D2BBE027_F4F1_11D3_829C_0050DA0C5DE1__INCLUDED_
#pragma once

#include <afxwin.h>

// compression levels
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)

// Allowed flush values; see deflate() below for details
#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1 /* will be removed, use Z_SYNC_FLUSH instead */
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4

// Return codes for the compression/decompression functions. Negative
// values are errors, positive values are used for special but normal events.
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)

// The deflate compression method (the only one supported in this version)
#define Z_DEFLATED   8

typedef int (*BASEIO)(void* pBuff, int iBuffSize, void* pUserData);

//	CGZFile class
//
//	Use this to read and write files that are GZ compressed
//	Files will be compaitble with gzip program.
//	This can also read files wich are not GZ compressed
//	Input/Output stream may be redirected
//
//
class CGZFile   
   {
protected:
	int	m_iMode;
	PVOID m_file;

public:
	CGZFile ();
	virtual ~CGZFile ();

  	virtual int Open (LPCTSTR lpszFile, LPCTSTR lpszMode);
  	virtual int Open (LPCTSTR lpszFile, LPCTSTR lpszMode, BASEIO pfnIO, PVOID pUserData);
	virtual int Close ();
	virtual int Flush (int iMode);

	virtual int Seek (int iOffset, int iFrom);
	virtual int Rewind ();
	virtual int GetPosition ();

	virtual int Read (LPVOID pBuff, UINT uLen);
	virtual int ReadChar ();
	virtual LPTSTR ReadString (LPTSTR lpszStr, UINT uMaxLen);

	virtual int Write (LPVOID pBuff, UINT uLen);
	virtual int WriteChar (int c);
	virtual int WriteString (LPCTSTR lpszStr);
//	virtual int Printf (LPCTSTR lpszStr, ...);

	virtual BOOL IsEof();
	virtual int GetCurrentMode(); // -1 = not open
	virtual BOOL FileIsGZipped ();

	virtual LPCTSTR Error (int* pError);
   };


// CZipArchive class
//	This class will read and write zip files
//
class CZipArchive
 	{
protected:
	int	m_iMode; // -1=uninitialized 0=open for read 1=open for write 2=open for append
	PVOID m_zipArchive;

public:
 	CZipArchive ();
 	~CZipArchive ();
 
 	virtual int OpenArchive (LPCTSTR lpszFile, int iMode);
 	virtual int CloseArchive ();
	virtual int CloseArchive (LPCTSTR lpszComment);
	virtual int GetCurrentMode();
 
 	/*--- read zip archive operations ---*/
	//	int GetInfo ();
	virtual int GetComment (LPTSTR lpszComment, int iMaxSize);
 
 	/*--- zip directory operations ---*/
 	virtual int GotoFirstFile ();
 	virtual int GotoNextFile ();
 	virtual int LocateFile	(LPCTSTR lpszFile);
	virtual int GetCurrentFileInfo (LPTSTR lpszFileName, int iNameSize, LPTSTR lpszComment, int iCommentSize);

 	/*--- write zip archive operations ---*/
	virtual int OpenNewFile (LPCTSTR lpszName, LPCTSTR lpszComment, int iLevel, LPCTSTR lpszEncryptKey = NULL);
	virtual int OpenCurrentFile (LPCTSTR lpszEncryptKey = NULL);
 	virtual int CloseFile ();
 
 	/*--- read operations ---*/
 	virtual int Read (LPVOID pBuff, UINT uLen);
 	virtual int ReadChar ();
	virtual LPTSTR ReadString (LPTSTR lpszStr, UINT uMaxLen);
 
 	virtual int GetPosition ();
 	virtual BOOL IsEof();

 
 	/*--- write operations ---*/
 	virtual int Write (LPVOID pBuff, UINT uLen);
 	virtual int WriteChar (CHAR c);
 	virtual int WriteString (LPCTSTR lpszStr);
 
 	/*--- Statics ---*/
 	static int FileNameCompare (LPCTSTR lpszName1, LPCTSTR lpszName2, BOOL bCaseSensitive);
 	};


#endif // !defined(AFX_GZFILE_H__D2BBE027_F4F1_11D3_829C_0050DA0C5DE1__INCLUDED_)
