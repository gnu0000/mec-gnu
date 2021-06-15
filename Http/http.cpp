//
// Http.cpp
//
// Simple Http Get and post class
// supports cern proxies
// supports special header requirements
//

#include "..\stdafx.h"
#include "GnuStr.h"
#include "G_Http.h"
#include "GnuMem.h"

#define RETURN_IF_ASSERTERROR(bTest,iError) {BOOL b=(bTest); if (!b) return (SetError (iError));}

CHttp::CHttp ()
   {
   m_iError             = 0;
   m_iProxyMode         = PROXY_MODE_DIRECT;
   m_pszErrorMessage    = 
   m_pszProxyAddress    = 
   m_pszUserAgent       = NULL;
   m_bBinaryMode        = FALSE;
   m_bHeadersInResponse = FALSE;
   }

CHttp::~CHttp ()
   {
   if (m_pszProxyAddress)
      MemFreeData (m_pszProxyAddress);
   if (m_pszUserAgent)
      MemFreeData (m_pszUserAgent);
   if (m_pszErrorMessage)
      MemFreeData (m_pszErrorMessage);
   }



// Error Handling
int CHttp::IsError (PSZ* ppszErr)
   {
   if (ppszErr)
      *ppszErr = m_pszErrorMessage;
   return m_iError;
   }

PSZ CHttp::GetError (int* piError)
   {
   if (piError)
      *piError = m_iError;
   return m_pszErrorMessage;
   }

int CHttp::SetError (int iErrorVal, PSZ pszError, ...)
   {
   if (m_iError) // we are already in an error state
      return m_iError;
   m_iError = iErrorVal;

   char szErrorBuff [512];
   *szErrorBuff = '\0';

	va_list  vlst;
	va_start (vlst, pszError);
	vsprintf (szErrorBuff, pszError, vlst);
	va_end   (vlst);
   pszError = szErrorBuff;

   if (!*szErrorBuff)
      {
      switch (m_iError)
         {
         case HTTPERR_INVALID_PARAMS  : pszError = "Invalid Parameters"       ; break;
         case HTTPERR_CANNOT_OPEN_FILE: pszError = "Unable to open data file" ; break;
         case HTTPERR_BAD_URL         : pszError = "Malformed URL"            ; break;
         case HTTPERR_ACCESS_DENIED   : pszError = "Access Denied"            ; break;
         case HTTPERR_SERVER_ERROR    : pszError = "Unknown Web Server Error" ; break;
         default                      : pszError = "Unknown Error"            ; break;
         }                                       
      }
   StrReplace (&m_pszErrorMessage, pszError);
   return iErrorVal;
   }


/////////////////////////////////////////////////////////////////////////////

// CHttpException -- used if something goes wrong for us
// It will throw its own exception type to handle problems it might
// encounter while fulfilling the HTTP request.
class CHttpException : public CException
	{
	DECLARE_DYNCREATE(CHttpException)
public:
	int m_iErrorCode;
	int m_iHttpCode;
	CHttpException(int iCode = 0, int iHttpCode = 0);
	~CHttpException() { }
	};
IMPLEMENT_DYNCREATE(CHttpException, CException)

CHttpException::CHttpException(int iCode, int iHttpCode): m_iErrorCode(iCode), m_iHttpCode(iHttpCode){}

void ThrowHttpException (int iCode, int iHttpCode = 0) {CHttpException* pEx = new CHttpException(iCode, iHttpCode); throw pEx;}

/////////////////////////////////////////////////////////////////////////////

int CHttp::Post (PSZ pszURL, PSZ pszPostFile, PSZ pszResponseFile)
   {
   return Post (pszURL, pszPostFile, pszResponseFile, NULL, 0, NULL);
   }

int CHttp::Post (PSZ pszURL, PSZ pszPostFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen)
   {
   return Post (pszURL, pszPostFile, NULL, pbResponseBuffer, iBufferSize, piBufferLen);
   }


int CHttp::Post (PSZ pszURL, PSZ pszPostFile, PSZ pszResponseFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen)
   {
   CInternetSession* pSession    = NULL;
   CHttpConnection*  pServer     = NULL;
   CHttpFile*        pHttpFile   = NULL;
   int               iReturnCode = 0;

   DWORD dwAccessType = (m_iProxyMode == PROXY_MODE_DIRECT ? INTERNET_OPEN_TYPE_DIRECT :
                         m_iProxyMode == PROXY_MODE_PROXY  ? INTERNET_OPEN_TYPE_PROXY  :
                                                             INTERNET_OPEN_TYPE_PRECONFIG);
   PSZ pszProxyURL    = (m_iProxyMode == PROXY_MODE_PROXY  ? m_pszProxyAddress : NULL);

   RETURN_IF_ASSERTERROR (pszURL && *pszURL && pszPostFile && *pszPostFile && pszResponseFile && *pszResponseFile, HTTPERR_INVALID_PARAMS);
   try
      {
      // Open Internet Session
   	pSession = new CInternetSession (m_pszUserAgent, 1, dwAccessType, pszProxyURL);

      INTERNET_PORT nPort;
      DWORD   dwServiceType;
      CString strServerName;
      CString strObject;

      // decompose URL
      if (!AfxParseURL (pszURL, dwServiceType, strServerName, strObject, nPort) || dwServiceType != INTERNET_SERVICE_HTTP)
         ThrowHttpException (HTTPERR_BAD_URL); // only use http:// URLs

      // Establish connection to server
      pServer = pSession->GetHttpConnection (strServerName, nPort);

      // init post request
      DWORD dwOpenRequestFlags = INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE; // INTERNET_FLAG_EXISTING_CONNECT not valid aparently
      pHttpFile = pServer->OpenRequest (CHttpConnection::HTTP_VERB_POST, strObject, NULL, 1, NULL, NULL, dwOpenRequestFlags);

      // Add http headers
      if (m_pszUserAgent)
         pHttpFile->AddRequestHeaders(CString ("User-Agent: ") + m_pszUserAgent + "\r\n");
      for (int i=0; i< m_cExtraHeaders.GetSize (); i++)
         pHttpFile->AddRequestHeaders(m_cExtraHeaders[i]);

      // read post data into a buffer
      CString strPostData;
      CString strLine;
      CStdioFile cPostFile (pszPostFile, CFile::modeRead);
      while (cPostFile.ReadString (strLine))
         strPostData += strLine + "\x0D\x0A";
      cPostFile.Close ();

      // ok, now do the post request
      pHttpFile->SendRequest (NULL, 0, (PSZ)(LPCTSTR)strPostData, strPostData.GetLength());

      DWORD dwRet;
      pHttpFile->QueryInfoStatusCode (dwRet);

      // if access was denied, prompt the user for the password
      if (dwRet == HTTP_STATUS_DENIED || dwRet == HTTP_STATUS_PROXY_AUTH_REQ)
         {
         if (pHttpFile->ErrorDlg () != ERROR_INTERNET_FORCE_RETRY) // if the user cancelled the dialog, bail out
            ThrowHttpException (HTTPERR_ACCESS_DENIED); // Access Denied

         pHttpFile->SendRequest (NULL, 0, (PSZ)(LPCTSTR)strPostData, strPostData.GetLength());
         pHttpFile->QueryInfoStatusCode (dwRet);
         }
     
      // Read In Response
      iReturnCode =  pszResponseFile ? SaveToFile (pHttpFile, pszResponseFile) :
                                       SaveToBuffer (pHttpFile, pbResponseBuffer, iBufferSize, piBufferLen);

      if (dwRet != HTTP_STATUS_OK)
         ThrowHttpException (HTTPERR_SERVER_ERROR, dwRet); // who knows
      }

   catch (CInternetException* pEx) // catch errors from WinINet
      {
      TCHAR szErr[1024];
      pEx->GetErrorMessage(szErr, 1024);
      iReturnCode = SetError (HTTPERR_NET_ERROR, szErr);
      pEx->Delete();
      }
   catch (CFileException* pEx)
      {
      iReturnCode = SetError (HTTPERR_NET_ERROR, "Could not open file %s", pEx->m_strFileName);
      pEx->Delete();
      }
   catch (CHttpException* pEx) // catch things wrong with parameters, etc
      {
      if (pEx->m_iErrorCode == HTTPERR_SERVER_ERROR) // http error - lets keep the HTTP return value
         {
         PSZ pszErr;
         switch (pEx->m_iHttpCode)
            {
            case 403: pszErr = "Access Forbidden [%d]";                                   break;
            case 407: pszErr = "Proxy Name/Password Incorrect or Unsupported Proxy [%d]"; break;
            case 408: pszErr = "Request Timeout [%d]";                                    break;
            case 500: pszErr = "HTTP Server Error [%d]";                                  break;
            case 502: pszErr = "Bad Gateway [%d]";                                        break;
            case 504: pszErr = "Gateway Timeout [%d]";                                    break;
            default : pszErr = "Web Server Error Return [%d]";
            }
         iReturnCode = SetError (pEx->m_iErrorCode, pszErr, pEx->m_iHttpCode);
         }
      else
         iReturnCode = SetError (pEx->m_iErrorCode, NULL);
      pEx->Delete();
      }

   //  Cleanup
   if (pHttpFile) pHttpFile->Close ();
   if (pHttpFile) delete pHttpFile;
   if (pServer  ) delete pServer;
   if (pSession ) pSession->Close();
   if (pSession ) delete pSession;
   return iReturnCode;
   }


int CHttp::Get (PSZ pszURL, PSZ pszResponseFile)
   {
   return Get (pszURL, pszResponseFile, NULL, 0, NULL);
   }

int CHttp::Get (PSZ pszURL, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen)
   {
   return Get (pszURL, NULL, pbResponseBuffer, iBufferSize, piBufferLen);
   }

int CHttp::Get (PSZ pszURL, PSZ pszResponseFile, BYTE* pbResponseBuffer, int iBufferSize, int* piBufferLen)
   {
   CInternetSession* pSession    = NULL;
   CHttpFile*        pHttpFile   = NULL;
   int               iReturnCode = 0;

   DWORD dwAccessType = (!m_iProxyMode == PROXY_MODE_DIRECT ? INTERNET_OPEN_TYPE_DIRECT :
                          m_iProxyMode == PROXY_MODE_PROXY  ? INTERNET_OPEN_TYPE_PROXY  :
                                                              INTERNET_OPEN_TYPE_PRECONFIG);
   PSZ pszProxyURL    = ( m_iProxyMode  == PROXY_MODE_PROXY  ? m_pszProxyAddress : NULL);

   RETURN_IF_ASSERTERROR (pszURL && *pszURL, HTTPERR_BAD_URL);
   try
      {
   	CInternetSession *pSession = new CInternetSession (m_pszUserAgent, 1, dwAccessType, pszProxyURL);
      DWORD dwMode = m_bBinaryMode ? INTERNET_FLAG_TRANSFER_BINARY : INTERNET_FLAG_TRANSFER_ASCII;

      CString strExtraHeaders;
      for (int i=0; i< m_cExtraHeaders.GetSize (); i++)
         strExtraHeaders += (m_cExtraHeaders[i] + "\r\n");

		CHttpFile* pHttpFile = (CHttpFile*)pSession->OpenURL (pszURL, 1, dwMode, (LPCTSTR)strExtraHeaders, -1);

      iReturnCode =  pszResponseFile ? SaveToFile (pHttpFile, pszResponseFile) :
                                       SaveToBuffer (pHttpFile, pbResponseBuffer, iBufferSize, piBufferLen);
      }
	catch (CInternetException* pEx)
		{
      TCHAR szErr[1024];
      pEx->GetErrorMessage(szErr, 1024);
      iReturnCode = SetError (HTTPERR_NET_ERROR, szErr);
      pEx->Delete();
		}
   if (pHttpFile) delete pHttpFile;
   if (pSession ) pSession->Close();
   if (pSession ) delete pSession;
   return iReturnCode;
   }


#define XFER_BUFF_SIZE 8192

int CHttp::SaveToFile (CHttpFile* pHttpFile, PSZ pszOutFile)
   {
   BYTE pBuff [XFER_BUFF_SIZE];
   UINT uReadLen;

   CFile cOutFile;
   if (!cOutFile.Open (pszOutFile, CFile::modeWrite | CFile::modeCreate))
      return SetError (HTTPERR_CANNOT_OPEN_FILE);

   if (m_bHeadersInResponse)
      {
      DWORD dwLen = sizeof (pBuff);
      pHttpFile->QueryInfo (HTTP_QUERY_RAW_HEADERS_CRLF, &pBuff, &dwLen);
      cOutFile.Write (pBuff, dwLen);
      cOutFile.Write ("\n", 1);
      }
   while (uReadLen = pHttpFile->Read (pBuff, XFER_BUFF_SIZE))
      cOutFile.Write (pBuff, uReadLen);
   cOutFile.Close ();
   return 0;
   }


// todo: properly check and return HTTPERR_BUFFER_OVERFLOW as needed
//
int CHttp::SaveToBuffer (CHttpFile* pHttpFile, BYTE* pBuff, int iBuffSize, int* piReadLen)
   {
   DWORD dwIdx = 0;

   if (m_bHeadersInResponse)
      {
      dwIdx = iBuffSize;
      pHttpFile->QueryInfo (HTTP_QUERY_RAW_HEADERS_CRLF, &pBuff, &dwIdx);
      pBuff[dwIdx++] = '\n';
      pBuff[dwIdx] = '\0';
      }
   int iLen = pHttpFile->Read (pBuff + dwIdx, iBuffSize - dwIdx);
   if (iLen + (int)dwIdx < iBuffSize)
      pBuff[iLen + dwIdx] = '\0';
   if (piReadLen)
      *piReadLen = iLen + dwIdx;
   return 0;
   }


int CHttp::SetProxy (PSZ pszAddress, int iPort, int iMode)
   {
   char szBuff[256];

   if (pszAddress && *pszAddress)
      {
      sprintf (szBuff, "%s:%u", pszAddress, iPort);
      pszAddress = szBuff;
      }
   StrReplace (&m_pszProxyAddress, pszAddress);
   m_iProxyMode = iMode;
   return 0;
   }


// "User-Agent: Expedite %s", CUtil::GetVersionString()
int CHttp::SetUserAgent (PSZ pszAgent)
   {
   StrReplace (&m_pszUserAgent, pszAgent);
   return 0;
   }

int CHttp::ClearRequestHeaders ()
   {
   m_cExtraHeaders.RemoveAll ();
   return 0;
   }


//   _T("Accept: text/*\r\n"
//   "User-Agent: Expedite Version " + (CString)CUtil::GetVersionString() + "\r\n"
//   "Random-ID: " + (CString)iRANDOM + "\r\n"
//   "Content-type: multipart/form-data; boundary=-----------------------------000\r\n"
//   ));
//
int CHttp::AddRequestHeader (PSZ pszHeader)
   {
   m_cExtraHeaders.Add (pszHeader);
   return 0;
   }

int CHttp::SetMode (BOOL bBinary)
   {
   m_bBinaryMode = bBinary;
   return 0;
   }

int CHttp::IncludeHeadersInResponse (BOOL bHeadersInResponse)
   {
   m_bHeadersInResponse = bHeadersInResponse;
   return 0;
   }
