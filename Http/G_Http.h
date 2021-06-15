//
// G_Http.h
// Simple Web Server interface class
//

#if !defined (G_HTTP_INCLUDED)
#define G_HTTP_INCLUDED
#pragma once

#include <afxinet.h>

// error values
#define HTTPERR_MIN_VALUE          50
#define HTTPERR_INVALID_PARAMS     HTTPERR_MIN_VALUE + 1
#define HTTPERR_CANNOT_OPEN_FILE   HTTPERR_MIN_VALUE + 2
#define HTTPERR_BAD_URL            HTTPERR_MIN_VALUE + 3
#define HTTPERR_ACCESS_DENIED      HTTPERR_MIN_VALUE + 4
#define HTTPERR_SERVER_ERROR       HTTPERR_MIN_VALUE + 5
#define HTTPERR_BUFFER_OVERFLOW    HTTPERR_MIN_VALUE + 6
#define HTTPERR_NET_ERROR          HTTPERR_MIN_VALUE + 7
#define HTTPERR_MAX_VALUE          HTTPERR_BUFFER_OVERFLOW

// proxy modes
#define PROXY_MODE_DIRECT          0
#define PROXY_MODE_PROXY           1
#define PROXY_MODE_AUTO            2


class CHttp
   {
public:
   CHttp ();
   ~CHttp ();
   
   // Error Handling
   int IsError (PSZ* ppszErr = NULL);
   PSZ GetError (int* piError = NULL);

   int Get (PSZ pszURL, PSZ pszResponseFile);
   int Get (PSZ pszURL, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen);

   int Post (PSZ pszURL, PSZ pszPostFile, PSZ pszResponseFile);
   int Post (PSZ pszURL, PSZ pszPostFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen);

   int SetMode (BOOL bBinary = TRUE);
   int SetProxy (PSZ pszAddress, int iPort, int iMode);
   int SetUserAgent (PSZ pszAgent = NULL);
   int IncludeHeadersInResponse (BOOL bHeadersInResponse = TRUE);
   int ClearRequestHeaders ();
   int AddRequestHeader (PSZ pszHeader);

private:
   int m_iError;
   PSZ m_pszErrorMessage;

   int m_iProxyMode;
   PSZ m_pszProxyAddress;

   PSZ m_pszUserAgent;
   CStringArray m_cExtraHeaders;

   BOOL m_bBinaryMode;
   BOOL m_bHeadersInResponse;

   int Get  (PSZ pszURL, PSZ pszResponseFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen);
   int Post (PSZ pszURL, PSZ pszPostFile, PSZ pszResponseFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen);

   int SaveToFile (CHttpFile* pHttpFile, PSZ pszOutFile);
   int SaveToBuffer (CHttpFile* pHttpFile, BYTE* pBuff, int iBuffSize, int* piReadLen);
   int SetError (int iErrorVal, PSZ pszError = NULL, ...);
   };

#endif // !defined (G_HTTP_INCLUDED)
