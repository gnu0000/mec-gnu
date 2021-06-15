// CSV.h: interface for the CCSV class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CSV_H__6FEEB3DC_F6E3_11D3_91D9_E5A7CEB81F25__INCLUDED_)
#define AFX_CSV_H__6FEEB3DC_F6E3_11D3_91D9_E5A7CEB81F25__INCLUDED_
#pragma once

//#define MAXPARTS 20

// This class is only partially complete:
//
// It uses a fixed size array of parts
// It does not build CSV strings
// It could do a lot more...
//


typedef CArray<CString,LPCTSTR> strArray;

class CCSV
	{
public:
	CString  m_str;
	strArray m_strPart;

	CCSV();
	CCSV(CString s, BOOL bClean=TRUE);
	virtual ~CCSV();

	int     Split (CString s, BOOL bClean=TRUE);

	CString GetPart    (int iPart);
   PSZ     GetPartPSZ (int iPart);
	int     GetPartInt (int iPart);

	CString SetPart (CString strPart, int iPart);
	int     GetPartCount ();
	};

#endif // !defined(AFX_CSV_H__6FEEB3DC_F6E3_11D3_91D9_E5A7CEB81F25__INCLUDED_)
