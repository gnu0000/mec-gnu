#if !defined(AFX_COLOREDTREEVIEW1_H__05EF7189_9AFF_4AC4_BD11_4BA49CC4E483__INCLUDED_)
#define AFX_COLOREDTREEVIEW1_H__05EF7189_9AFF_4AC4_BD11_4BA49CC4E483__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColoredTreeView1.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CColoredTreeView view

//class CColorer; // in G_DynaView.h
#include "G_DynaView.h"

class CColoredTreeView : public CTreeView
	{
protected:
	CColoredTreeView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CColoredTreeView)

public:
	CBitmap*	  m_pBitmap;
	CColorer   m_cClr;
	BOOL		  m_bEnabled;


	BOOL SetBkg (COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1);
	BOOL SetBkg (LPCTSTR lpctBitmap, COLORREF clrText=-1);
	BOOL SetBkg (USHORT iBitmapID, COLORREF clrText=-1);
	BOOL SetBkg ();


	CSize  GetTotalSize ();
	CPoint GetScrollPosition ();

protected:
	//{{AFX_VIRTUAL(CColoredTreeView)
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:
	virtual ~CColoredTreeView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CColoredTreeView)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};


//class CColoredScrollView : public CScrollView
//	{
//protected:
//	CColoredScrollView();
//	DECLARE_DYNCREATE(CColoredScrollView)
//
//public:
//	//CColorer	m_cClr;
//	CDynaBkgHandler m_dhBkg;
//
//	BOOL SetBkg (COLORREF clrTop, COLORREF clrBottom=-1);
//	BOOL SetBkg (LPCTSTR lpctBitmap);
//	BOOL SetBkg (USHORT iBitmapID);
//
//	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
//	virtual ~CColoredScrollView();
//	//{{AFX_VIRTUAL(CColoredScrollView)
//	//}}AFX_VIRTUAL
//
//protected:
//	//{{AFX_MSG(CColoredScrollView)
//	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
//	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
//	};


class CColoredScrollView : public CScrollView
	{
protected:
	CColoredScrollView();
	DECLARE_DYNCREATE(CColoredScrollView)

public:
	//CColorer	m_cClr;
	CDynaBkgHandler m_dhBkg;
	CDynaCtlHandler m_dhCtl;

	BOOL SetBkg (COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1);
	BOOL SetBkg (LPCTSTR lpctBitmap, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg (USHORT iBitmapID, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg ();

	BOOL UseResizingControls (BOOL bUse, int iMR=-1, int iSR=-1, int iMB=-1, int iSB=-1, int iCT=-1);

	BOOL UseBkgIfNotHiColor (BOOL bUse);
	BOOL BkgEnabled ();
	virtual HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); 
	virtual ~CColoredScrollView();

	//{{AFX_VIRTUAL(CColoredScrollView)
	virtual void OnSize(UINT nType, int cx, int cy); 
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CColoredScrollView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLOREDTREEVIEW1_H__05EF7189_9AFF_4AC4_BD11_4BA49CC4E483__INCLUDED_)
