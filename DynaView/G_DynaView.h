#if !defined(AFX_DYNAVIEW_H__6121D9AE_F04E_11D3_91D9_FB8189031921__INCLUDED_)
#define AFX_DYNAVIEW_H__6121D9AE_F04E_11D3_91D9_FB8189031921__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DynaView.h : header file
//

#define IDC_DYNA_MR	0x7ff0
#define IDC_DYNA_SR	(IDC_DYNAUNUSED+1)
#define IDC_DYNA_MB	(IDC_DYNAUNUSED+2)
#define IDC_DYNA_SB	(IDC_DYNAUNUSED+3)
#define IDC_DYNA_CT	(IDC_DYNAUNUSED+4)


#define DYNAMODE_NONE 0
#define DYNAMODE_FADE 1
#define DYNAMODE_BMP	 2




/////////////////////////////////////////////////////////////////////////////
//
// CCtlInfo class 
//	Used by CDynaHandler to manage dialog ctl sizes and positions
//	Used only if using Resizing Controls
/////////////////////////////////////////////////////////////////////////////

class CCtlInfo: public CObject
	{
public:
	int iDynaType;
	int iXGap;
	int iXMinPos;
	int iYGap;
	int iYMinPos;
	};

typedef CMap<HWND,HWND,CCtlInfo*,CCtlInfo*&> CMapCtlInfo;
typedef CMap<int,int,int,int> CMapIntInt;

/////////////////////////////////////////////////////////////////////////////
//
// CDynaHandler class
//	Does most of the work used by all Dyna View classes
/////////////////////////////////////////////////////////////////////////////

// constants for GetColor
//
#define DCT_TOPCOLOR		 0
#define DCT_BOTTOMCOLOR	 1
#define DCT_TEXTCOLOR	 2
#define DCT_BKGCOLOR		 3

class CColorer
	{
public:
	CBitmap*		m_pBitmap;
	COLORREF		m_clrTop;
	COLORREF		m_clrBottom;
	int			m_iStripes;
	int			m_iColorMode;

	CColorer();
	~CColorer();

	BOOL Setup (COLORREF clrTop, COLORREF clrBottom);
	BOOL Setup (LPCTSTR lpctBitmapFile);
	BOOL Setup (USHORT iBitmapID);
	BOOL Setup ();

	BOOL PaintIt (CDC* pDC, CRect cClientRect, CSize size, CPoint pos);
	BOOL IsSetup ();
	COLORREF ColorAtPosition (int iFullYSize, int iYPos);
   void SetStripeCount (int iStripes); // if using fade fill

protected:
	COLORREF GetIncrementalColor (int iStep);
	BOOL FadeFill (CDC* pDC, CRect rc, CSize size, CPoint pos);
	BOOL BmpFill (CDC* pDC, CRect cRect, CPoint pos);
	};



class CDynaBkgHandler
	{
public:
	CWnd*		   m_pWnd;
	COLORREF		m_clrText;
	COLORREF		m_clrBackground;
	CBrush*     m_pBkgBrush;
	BOOL			m_bUseBkgAnyway;
	BOOL			m_bInitialized;
	int			m_iBkgMode;

	CColorer    m_cClr;

	CDynaBkgHandler ();
	~CDynaBkgHandler ();

	BOOL SetBkg (CWnd* pWnd, COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1);
	BOOL SetBkg (CWnd* pWnd, LPCTSTR lpctBitmapFile, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg (CWnd* pWnd, USHORT  iBitmapID, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg (CWnd* pWnd); // turn off
	BOOL UseBkgIfNotHiColor (BOOL bUse=TRUE);

	BOOL DrawBackground (CDC* pDC, CSize size=CSize(0,0), CPoint pos=CPoint(0,0));
	HBRUSH GetCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor, HBRUSH hbr, int iFullYSize=0, int iYTop=0);
	COLORREF GetColor (int iColorType);
	int GetMode ();

	BOOL CompleteInit (CWnd *pWnd);
	BOOL RegisterControl (CWnd* pcWnd);
   BOOL UnregisterControl (CWnd* pcWnd);
   void SetStripeCount (int iStripes); // if using fade fill

private:
	COLORREF GetIncrementalColor (int iStep);
	BOOL FadeFill (CDC* pDC, CRect rc, CSize size, CPoint pos);
	BOOL BmpFill (CDC* pDC, CRect cRect, CPoint pos);
	BOOL CDynaBkgHandler::_Init (CWnd *pWnd, COLORREF clrBackground, COLORREF clrText, int iMode);
//	BOOL _Init ();
	BOOL IsInitialized ();
	};


#define DT_MOVER 0x01 // Move relative to rt edge
#define DT_SIZER 0x02 // Size relative to rt edge
#define DT_MOVEB 0x04 // Move relative to bottom edge
#define DT_SIZEB 0x08 // Size relative to bottom edge
#define DT_CTR   0x10 // Keep position relative to center

class CDynaCtlHandler
	{
public:
	CWnd*		   m_pWnd;
	BOOL			m_bResizingControls;
	CMapCtlInfo m_mapCtlInfo;
	CMapIntInt	m_mapAddCtlList;
	BOOL 			m_bInitialized;
	PVOID			m_pDT;
	int			m_iMetaCtlID[5]; // MR,SR,MB,SB,CT ctl id's

	CDynaCtlHandler ();
	~CDynaCtlHandler ();
	void Size ();
	void MapMetaControlID (int iControlID, int iControlType);
	void AddDynaControl (int iDialogID, int iDynaFlags);
	BOOL UseResizingControls (CWnd* pWnd, BOOL bUse=TRUE, int iMR=-1, int iSR=-1, int iMB=-1, int iSB=-1, int iCT=-1);
	BOOL CompleteInit (CWnd* pWnd);

private:
	BOOL _Init ();
	BOOL IsInitialized ();
	};


class CDynaFontHandler
	{
public:
	CString		m_strFont;
	int			m_iFontSize;
	PVOID			m_pDT;

	CDynaFontHandler ();
	~CDynaFontHandler ();

	BOOL SetFont (CString strFontName, int iFontSize = 12);
	CString& GetFont (CString &strFontName, int &iFontSize);
	LPCDLGTEMPLATE GetResourceTemplate (LPCTSTR lpszTemplateName);
	void FreeResourceTemplate ();
	};


/////////////////////////////////////////////////////////////////////////////
// CDynaStatic window
//
//class CDynaStatic : public CStatic
//	{
//public:
//	CDynaBkgHandler* m_pDH;
//	CDynaStatic();
//	virtual ~CDynaStatic();
//protected:
//	//{{AFX_MSG(CDynaStatic)
//	afx_msg void OnPaint();
//	afx_msg LRESULT OnSetText(WPARAM, LPARAM lParam);
//	//}}AFX_MSG
//	DECLARE_MESSAGE_MAP()
//	};
//
/////////////////////////////////////////////////////////////////////////////
// CDynaDialog dialog

class CDynaDialog : public CDialog
	{
public:
	UINT			     m_uID;
	CDynaBkgHandler  m_dhBkg;
	CDynaCtlHandler  m_dhCtl;
	CDynaFontHandler m_dhFont;

public:
	CDynaDialog(UINT uID, CWnd* pParent = NULL);   // standard constructor

	//	DynaView settings
	//	Call these between construction and DoModal
	//
	BOOL SetFont (CString strFontName, int iFontSize=12);
	BOOL SetBkg  (COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1);
	BOOL SetBkg  (LPCTSTR lpctBitmap, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg  (USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText);
	BOOL SetBkg  (); // turn off
	BOOL UseBkgIfNotHiColor (BOOL bUse=TRUE);
	BOOL UseResizingControls (BOOL bUse=TRUE, int iMR=-1, int iSR=-1, int iMB=-1, int iSB=-1, int iCT=-1);
	void AddDynaControl (int iDialogID, int iDynaFlags);
	void MapMetaControlID (int iControlID, int iControlType);
	COLORREF GetColor (int iColorType);
//	void RegisterControl (CWnd* pcWnd, DWORD dwFlags=0);
   void SetStripeCount (int iStripes); 

	//{{AFX_VIRTUAL(CDynaDialog)
	public:
	virtual int DoModal();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CDynaDialog)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////
// CDynaPropertyPage dialog

class CDynaPropertyPage : public CPropertyPage
	{
	DECLARE_DYNCREATE(CDynaPropertyPage)

public:
	UINT			     m_uID;
	CDynaBkgHandler  m_dhBkg;
	CDynaCtlHandler  m_dhCtl;
	CDynaFontHandler m_dhFont;

public:
	CDynaPropertyPage(UINT uID);
	~CDynaPropertyPage();

	//	DynaView settings
	//	Call these between construction and DoModal
	//
	BOOL SetFont (CString strFontName, int iFontSize=12);
	BOOL SetBkg  (COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1);
	BOOL SetBkg  (LPCTSTR lpctBitmap, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg  (USHORT iBitmapID, COLORREF clrBackground=-1, COLORREF clrText=-1);
	BOOL SetBkg  (); // turn off
	BOOL UseBkgIfNotHiColor (BOOL bUse=TRUE);
	BOOL UseResizingControls (BOOL bUse=TRUE, int iMR=-1, int iSR=-1, int iMB=-1, int iSB=-1, int iCT=-1);
	void AddDynaControl (int iDialogID, int iDynaFlags);
	COLORREF GetColor (int iColorType);
//	void RegisterControl (CWnd* pcWnd, DWORD dwFlags=0);
   void SetStripeCount (int iStripes); 

	//{{AFX_VIRTUAL(CDynaPropertyPage)
	public:
	virtual int DoModal();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDynaPropertyPage)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////
// CDynaFormView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif


class CDynaFormView : public CFormView
	{
protected:
	DECLARE_DYNCREATE(CDynaFormView)

	CDynaFormView(UINT uID); 
	virtual ~CDynaFormView();

public:
	UINT			     m_uID;
	CDynaBkgHandler  m_dhBkg;
	CDynaCtlHandler  m_dhCtl;
	CDynaFontHandler m_dhFont;

public:
	//	DynaView settings
	//	Call these between construction and DoModal
	//
	BOOL SetFont (CString strFontName, int iFontSize=12);
	BOOL SetBkg  (COLORREF clrTop, COLORREF clrBottom=-1, COLORREF clrText=-1); // fade
	BOOL SetBkg  (LPCTSTR lpctBitmap, COLORREF clrBackground=-1, COLORREF clrText=-1); // bmp
	BOOL SetBkg  (USHORT iBitmapID, COLORREF clrBackground=-1, COLORREF clrText=-1);	// bmp
	BOOL SetBkg  (); // turn off
	BOOL UseBkgIfNotHiColor (BOOL bUse=TRUE);
	BOOL UseResizingControls (BOOL bUse=TRUE, int iMR=-1, int iSR=-1, int iMB=-1, int iSB=-1, int iCT=-1);
	void AddDynaControl (int iDialogID, int iDynaFlags);
	COLORREF GetColor (int iColorType);
//	void RegisterControl (CWnd* pcWnd, DWORD dwFlags=0);
   void SetStripeCount (int iStripes); 

	//{{AFX_DATA(CDynaFormView)
	//}}AFX_DATA
	//{{AFX_VIRTUAL(CDynaFormView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, 
	                    const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	//{{AFX_MSG(CDynaFormView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	};

/////////////////////////////////////////////////////////////////////////////


//{{AFX_INSERT_LOCATION}}

#endif // !defined(AFX_DYNAVIEW_H__6121D9AE_F04E_11D3_91D9_FB8189031921__INCLUDED_)



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
