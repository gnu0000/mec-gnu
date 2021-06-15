// CSV.cpp: implementation of the CCSV class.
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "G_CSV.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// This class is only partially complete:
//
// It uses a fixed size array of parts
// It does not build CSV strings
// It could do a lot more...
//
//

CCSV::CCSV()
	{
	m_str.Empty();
	}


CCSV::CCSV(CString s, BOOL bClean)
	{
	CCSV ();
	Split (s);
	}


CCSV::~CCSV()
	{
	}


int CCSV::Split (CString s, BOOL bClean)
	{
	int i, iStr;

	m_str = s;
	m_strPart.RemoveAll ();

	BOOL bLastCharWasQuote = FALSE, bInQuotes=FALSE;
	m_strPart.Add (""); // add first part

	for (i=iStr=0; i < s.GetLength(); i++)
		{
		if (!bInQuotes && s[i] == ',')
			{
			iStr++;
			m_strPart.Add (""); // add another part
			bLastCharWasQuote = FALSE;
			continue;
			}
		if (s[i] != '"')
			{
			m_strPart[iStr] += s[i];
			bLastCharWasQuote = FALSE;
			continue;
			}
		if (bInQuotes)
			{
			bInQuotes = FALSE;
			bLastCharWasQuote = TRUE;
			continue;
			}
		if (bLastCharWasQuote)
			m_strPart[iStr] += s[i];
		bInQuotes = TRUE;
		bLastCharWasQuote = TRUE;
		}
	if (m_strPart.GetSize () == 1 && m_strPart[0].IsEmpty ())
		m_strPart.RemoveAll ();	// nothing here
	int iPartCount = m_strPart.GetSize ();

	for (i=0; bClean && i < iPartCount; i++)
		m_strPart[i].TrimLeft(), m_strPart[i].TrimRight();
	return iPartCount;
	}


CString CCSV::GetPart (int iPart)
	{
	if (iPart >= m_strPart.GetSize ())
		return "";
	return m_strPart[iPart];
	}


PSZ CCSV::GetPartPSZ (int iPart)
	{
	if (iPart >= m_strPart.GetSize ())
		return "";
	return (PSZ)(LPCTSTR)m_strPart[iPart];
	}


int CCSV::GetPartInt (int iPart)
	{
	return atoi (GetPart(iPart));
	}


CString CCSV::SetPart (CString strPart, int iPart)
	{
	if (iPart >= m_strPart.GetSize ())
		m_strPart.SetSize (iPart+1);

	return m_strPart[iPart] = strPart;
	}


int CCSV::GetPartCount ()
	{
	return m_strPart.GetSize ();
	}
