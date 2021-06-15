// Registry.cpp



#include "..\stdafx.h"
#include "G_Config.h"


/***************************************************************************/
/*                                                                         */
/* CRegistry functions                                                     */
/*                                                                         */
/***************************************************************************/

CRegistry::~CRegistry()
{
	CloseRegKey();
}


// --- OpenRegKey() -------------------------------------------------
// 
bool CRegistry::OpenRegKey (LPCTSTR pszBase, LPCTSTR pszRest)
{
	CloseRegKey();

	CHAR  szKey [256];
	strcpy (szKey, pszBase);
	if (pszRest && *pszRest)
		strcat (strcat (szKey, "\\"), pszRest);

	DWORD dwDisposition = 0;
	RegCreateKeyEx (HKEY_CURRENT_USER, szKey, 0, "",
					REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
					NULL, &m_hKey, &dwDisposition);
	return true;
}


// --- CloseRegKey() ------------------------------------------------
// 
bool CRegistry::CloseRegKey ()
{
	if (m_hKey)
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
	return true;
}


// --- GetValLength() -----------------------------------------------
// 
bool CRegistry::GetValLength(LPCTSTR pszName, ULONG& iLen)
	{
	ULONG iType=0;

	// Get data length ---
	ULONG iRet = RegQueryValueEx (m_hKey, pszName, 0, &iType, NULL, &iLen);
	if (iRet != ERROR_SUCCESS)
		return false;

	return true;
	}


// --- ReadRegBin() -------------------------------------------------
// 
bool CRegistry::ReadRegBin(LPCTSTR pszName, LPBYTE pData, ULONG iLen)
	{
	if (m_hKey == NULL)
		return false;

	ULONG iType=0;
	ULONG iOldLen = iLen;
	ULONG iRet = RegQueryValueEx (m_hKey, pszName, 0, &iType, (LPBYTE)pData, &iLen);

	return (iRet == ERROR_SUCCESS && iOldLen == iLen);
	}

		
// --- ReadRegStr() -------------------------------------------------
// 
bool CRegistry::ReadRegStr (LPCTSTR pszName, CString& strData)
{
	if (m_hKey == NULL)
		return false;

	ULONG iLen=0, iType=0;

	// Get data length ---
	ULONG iRet = RegQueryValueEx (m_hKey, pszName, 0, &iType, NULL, &iLen);
	if (iRet != ERROR_SUCCESS)
		return false;

	char* pszBuff = strData.GetBuffer(iLen+1);	// +1 for null-terminator
	iRet = RegQueryValueEx (m_hKey, pszName, 0, &iType, (LPBYTE)pszBuff, &iLen);
	strData.ReleaseBuffer();

	return (iRet == ERROR_SUCCESS);
}



// --- ReadRegInt() -------------------------------------------------
// Returns -1 if the value cannot be found.
//
INT CRegistry::ReadRegInt (LPCTSTR pszName)
{
	if (m_hKey == NULL)
		return -1;

	DWORD dw;
	ULONG iLen = sizeof(dw), iType, iRet;

	iRet = RegQueryValueEx (m_hKey, pszName, 0, &iType, (LPBYTE)&dw, &iLen);
	if (iRet != ERROR_SUCCESS)
		return -1;
	return (INT)dw;
}


// --- WriteRegBin() ------------------------------------------------
// 
bool CRegistry::WriteRegBin(LPCTSTR pszName, LPBYTE pData, ULONG iLen)
	{
	if (m_hKey == NULL)
		return false;

	INT  iRet;
	iRet = RegSetValueEx (m_hKey, pszName, 0, REG_BINARY, (const LPBYTE)pData, iLen);
	return (iRet == ERROR_SUCCESS);
	}

	
// --- WriteRegStr() ------------------------------------------------
// 
bool CRegistry::WriteRegStr (LPCTSTR pszName, LPCTSTR pszData)
{
	if (m_hKey == NULL)
		return false;

	INT  iRet;
	iRet = RegSetValueEx (m_hKey, pszName, 0, REG_SZ, (PUCHAR)pszData, strlen(pszData)+1);
	return (iRet == ERROR_SUCCESS);
}


// --- WriteRegInt() ------------------------------------------------
// 
bool CRegistry::WriteRegInt (LPCTSTR pszName, INT iVal)
{
	if (m_hKey == NULL)
		return false;

	DWORD dw = iVal;
	INT   iRet;

	iRet = RegSetValueEx (m_hKey, pszName, 0, REG_DWORD, (LPBYTE)&dw, sizeof (DWORD));
	return (iRet == ERROR_SUCCESS);
}


// --- GetValuesList() ----------------------------------------------
// 
bool CRegistry::GetValues(TRegStrValueList& valuesList)
{
	if (m_hKey == NULL)
		return false;

	CString  strName, strData;
	DWORD dwMaxNameSize = 1024, dwMaxDataSize = 1024;
	RegQueryInfoKey(m_hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
					&dwMaxNameSize, &dwMaxDataSize, 
					NULL, NULL);

	dwMaxNameSize++;	// Take null terminator into account
	dwMaxDataSize++;	// Not needed for this one, I guess, but just in case ...
	PCHAR pszName = strName.GetBuffer(dwMaxNameSize);
	PCHAR pszData = strData.GetBuffer(dwMaxDataSize);
								
	for (int index=0; ; index++)
	{
		DWORD dwNameSize = dwMaxNameSize;
		DWORD dwDataSize = dwMaxDataSize;
		if (RegEnumValue (m_hKey, index, pszName, &dwNameSize, NULL, NULL, 
		   (LPBYTE)pszData, &dwDataSize) != ERROR_SUCCESS)
			break;
		valuesList.AddTail(CRegStrValue(pszName, pszData));
	}

	return true;
}


// --- DeleteRegKey() -----------------------------------------------
// 
bool CRegistry::DeleteRegKey(LPCTSTR pszKey)
{
	if (m_hKey == NULL)
		return false;

	LONG lRet = RegDeleteKey(m_hKey, pszKey);
	return (lRet == ERROR_SUCCESS);
}


