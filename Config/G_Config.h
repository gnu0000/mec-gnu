/*
 * -----------------------------------------------------------------------
 *          Copyright (c) 1997 by AASHTO.  All Rights Reserved.
 *  
 *  This program module is part of EBS, the Electronic Bidding System, a 
 *  proprietary product of AASHTO, no part of which may be reproduced or 
 *  transmitted in any form or by any means, electronic, mechanical, or 
 *  otherwise, including photocopying and recording or in connection with 
 *  any information storage or retrieval system, without permission in 
 *  writing from AASHTO. 
 * -----------------------------------------------------------------------
 * 
 * G_Config.h
 *
 *
 * This file provides a class for reading/writing the registry
 *
 */


#ifndef EST_REGISTRY_H
#define EST_REGISTRY_H


struct CRegStrValue
{
	CString		m_strName;
	CString		m_strData;

	CRegStrValue()	{}
	CRegStrValue(CString strName, CString strData) : m_strName(strName), m_strData(strData)	{}
};

typedef CList<CRegStrValue, CRegStrValue&>	TRegStrValueList;


// Reads configuration data in initialization file, or in registry if the file
// does not exist.
//
class CRegistry
{
	HKEY		m_hKey;			// current key, when reading in registry

public:
	CRegistry() : m_hKey(NULL)	{}
	~CRegistry();

	bool		OpenRegKey		(LPCTSTR pszBase, LPCTSTR pszRest);
	bool		CloseRegKey		();
	bool		GetValLength	(LPCTSTR pszName, ULONG& iLen);
	bool		ReadRegBin		(LPCTSTR pszName, LPBYTE pData, ULONG iLen);
	bool		ReadRegStr		(LPCTSTR pszName, CString& strData);
	INT		ReadRegInt		(LPCTSTR pszName);
	bool		WriteRegBin		(LPCTSTR pszName, LPBYTE pData, ULONG iLen);
	bool		WriteRegStr		(LPCTSTR pszName, LPCTSTR pszData);
	bool		WriteRegInt		(LPCTSTR pszName, INT iVal);
	bool		GetValues		(TRegStrValueList& valuesList);
	bool		DeleteRegKey	(LPCTSTR pszKey);

//	HKEY		GetKey			 ()	{return m_hKey;}
};


// Do not write anything after this line ----------------------------
#endif // EST_REGISTRY_H
