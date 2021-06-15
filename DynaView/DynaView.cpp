// DynaView.cpp : implementation file
//
// Craig Fitzgerald
//

#include "..\stdafx.h"
#include "G_DynaView.h"
#include <afxpriv.h>         	// needed to get CDialogTemplate

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CColorer

#define COLORMODE_NONE 0
#define COLORMODE_FADE 1
#define COLORMODE_BMP  2

CColorer::CColorer()
	{
	m_pBitmap	= NULL;
	m_clrTop		= -1	;
	m_clrBottom	= -1	;
	m_iColorMode= 0  	;
	m_iStripes	= 128;
	}

CColorer::~CColorer()
	{
	if (m_pBitmap)
		delete m_pBitmap;
	}

BOOL CColorer::IsSetup ()
	{
	return !!m_iColorMode;
	}

COLORREF CColorer::GetIncrementalColor (int iStep)
	{
	if (iStep > m_iStripes) iStep = m_iStripes;

   int r = GetRValue (m_clrTop);
   int g = GetGValue (m_clrTop);
   int b = GetBValue (m_clrTop);
   int R = GetRValue (m_clrBottom);
   int G = GetGValue (m_clrBottom);
   int B = GetBValue (m_clrBottom);
   int rx = r + ((R - r) * iStep) / m_iStripes;
   int gx = g + ((G - g) * iStep) / m_iStripes;
   int bx = b + ((B - b) * iStep) / m_iStripes;

	return RGB ((BYTE)rx, (BYTE)gx, (BYTE)bx);
	}

BOOL CColorer::FadeFill (CDC* pDC, CRect rc, CSize size, CPoint pos)
	{
   CBrush brush;
	CRect  rect;
   int    i;

	rect = rc;
	int iFullYSize = max (size.cy, rect.Height());

   for (i = 0; i <= m_iStripes; i++)
      {       
		rect.top 	= i * iFullYSize / m_iStripes - pos.y + rc.top;
      rect.bottom = rect.top + iFullYSize / m_iStripes + 1;

		if (rect.bottom < 0 || rect.top > rc.bottom)
			continue; // this stripe isn't in scrolled view

		brush.CreateSolidBrush (GetIncrementalColor (i));
      pDC->FillRect (&rect, &brush);
      brush.DeleteObject ();
      }
	return TRUE;
	}


BOOL CColorer::BmpFill (CDC* pDC, CRect cRect, CPoint pos)
	{
	CDC 	 srcDC;
	BITMAP bmp;

	if (!m_pBitmap)
		return FALSE;
	m_pBitmap->GetBitmap (&bmp);
	if (!srcDC.CreateCompatibleDC(NULL))
		return FALSE;
   CBitmap* pOldBitmap = srcDC.SelectObject(m_pBitmap);

	for (int y=-(pos.y % bmp.bmHeight); y<cRect.bottom; y += bmp.bmHeight)
		for (int x=-(pos.x % bmp.bmWidth); x<cRect.right; x+= bmp.bmWidth)
		   pDC->BitBlt(x, y, bmp.bmWidth, bmp.bmHeight, &srcDC, 0, 0, SRCCOPY); // BitBlt to pane rect

   srcDC.SelectObject(pOldBitmap);
	return TRUE;
	}


//	cClientRect - the display rectangle
//	size 			- the full display size (cClientRect or larger if there's scrollbars)
//	pos			- the offset to be the top (0,0 unless screen is scrolled)
//
BOOL CColorer::PaintIt (CDC* pDC, CRect cClientRect, CSize size, CPoint pos)
	{
	CRect cRect;

	if (m_iColorMode == COLORMODE_FADE)
		return FadeFill (pDC, cClientRect, size, pos);
	if (m_iColorMode == COLORMODE_BMP )
		return BmpFill (pDC, cClientRect, pos);
	return FALSE;
	}


BOOL CColorer::Setup (COLORREF clrTop, COLORREF clrBottom)
	{
	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap	= NULL;
	m_iColorMode= 0;

	m_clrTop		= (clrTop	 == -1 ? GetSysColor (COLOR_BTNFACE) : clrTop 	);
	m_clrBottom	= (clrBottom == -1 ? GetSysColor (COLOR_BTNFACE) : clrBottom);
	m_iColorMode= COLORMODE_FADE;
	return TRUE;
	}

BOOL CColorer::Setup (LPCTSTR lpctBitmapFile)
	{
	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap	 = NULL;
	m_iColorMode = 0;

	HBITMAP hBitmap = (HBITMAP)LoadImage (0, lpctBitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hBitmap)
		return FALSE;
	m_pBitmap = (CBitmap*) new CBitmap;
	m_pBitmap->Attach (hBitmap);

	m_iColorMode = COLORMODE_BMP;
	return TRUE;
	}

BOOL CColorer::Setup (USHORT iBitmapID)
	{
	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap = NULL;
	m_iColorMode = 0;

	HINSTANCE hInst = AfxGetInstanceHandle ();
	HBITMAP hBitmap = (HBITMAP)LoadImage (hInst, MAKEINTRESOURCE(iBitmapID), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
	if (!hBitmap)
		return FALSE;
	m_pBitmap = (CBitmap*) new CBitmap;
	m_pBitmap->Attach (hBitmap);

	m_iColorMode = COLORMODE_BMP;
	return TRUE;
	}

BOOL CColorer::Setup ()
	{
	if (m_pBitmap)
		delete m_pBitmap;
	m_pBitmap = NULL;
	m_iColorMode = 0;
	return TRUE;
	}

COLORREF CColorer::ColorAtPosition (int iFullYSize, int iYPos)
	{
	if (m_iColorMode != COLORMODE_FADE)
		return 0;

	return GetIncrementalColor (iYPos * m_iStripes / iFullYSize);
	}

void CColorer::SetStripeCount (int iStripes)
   {
   m_iStripes = iStripes;
   }

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDynaStatic 
//
//BEGIN_MESSAGE_MAP(CDynaStatic, CStatic)
//	//{{AFX_MSG_MAP(CDynaStatic)
//	ON_WM_PAINT()
//	ON_MESSAGE(WM_SETTEXT, OnSetText)
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()
//
//CDynaStatic::CDynaStatic()
//	{
//	}
//
//CDynaStatic::~CDynaStatic()
//	{
//	}
//
//
//void CDynaStatic::OnPaint() 
//	{
//	CRect    cRect;
//	CString  cTxt;
//	DWORD    dwStyle, dwAlign;
//
//	CPaintDC dc(this);
//	dc.SetBkMode(TRANSPARENT);
//
//	CFont *pcFont 	  = GetFont ();
//	CFont *pcOldFont = dc.SelectObject (pcFont);
//
//	GetWindowText (cTxt);
//	GetWindowRect (cRect);
//	ScreenToClient (cRect);
//	dwStyle = GetStyle ();
//	int iID = GetDlgCtrlID (); // helps debug
//
//	if ((dwStyle & WS_DISABLED) == WS_DISABLED)
//		{
//		dc.SetTextColor (GetSysColor (COLOR_GRAYTEXT));
//		}
//	else
//		{
//	   CDynaDialog *pcParent = (CDynaDialog *)GetParent ();
//		dc.SetTextColor (m_pDH->GetColor (DCT_TEXTCOLOR));
//		}
//	if ((dwStyle & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP)
//		dwAlign = DT_LEFT;
//	else
//   	 dwAlign = (dwStyle & 0x03) | DT_WORDBREAK; // right/left alignment
//		
//
//	dc.DrawText (cTxt, &cRect, dwAlign);
//	dc.SelectObject (pcOldFont);
//	}
//
//
//LRESULT CDynaStatic::OnSetText(WPARAM, LPARAM lParam)
//	{
//	// we don't want the default SETTEXT proc to repaint the window directly!
//	ModifyStyle (WS_VISIBLE, 0);
//	CStatic::DefWindowProc (WM_SETTEXT, 0, lParam);
//	ModifyStyle (0, WS_VISIBLE);
//	Invalidate (TRUE);
//	return TRUE;
//	}



//	What in heck is he doing you ask? Well, good question.
//
//	The problem is that I need to handle the painting of some
//	of the static controls so that they look good with a bitmap
//	or a fade background.  I don't want to do a full user paint
//	because I don't want to have to worry about things like sunken, etc...
//	I just want to paint the damn strings.
//
//	You'd think that I could simply Subclass the controls and handle
//	this in the new class right? Nope, if the user has already associated
//	one of the controls with a class, AFX won't let me subclass it.  You
//	see, AFX only allows one class to be permanently associated with a
//	window handle.
//
//	OK, how about using CDynamicChain ? Well, I tried and the ATL headers
//	generate Errors - I don't know shit about ATL yet...
//
//	So, what I do is drop down to the good ole Win32 API and do the chaining
//	myself.  I trap the paint messages and paint the strings.  I also need
//	to trap the WM_SETTEXT message because it directly calls the default
//	paint proc and we don't want that.
//
//	Craig.
//

// yo - clean these up when the window is destroyed!
class CStaticControlInfo
	{
public:
	WNDPROC 			  pfnDefaultStaticHandler;
	CDynaBkgHandler* pcBH;
	};

typedef CMap<HWND,HWND,CStaticControlInfo*,CStaticControlInfo*> CMapStaticInfo;

CMapStaticInfo mapStaticInfo;

static WNDPROC pfnDefaultStaticHandler = NULL; 

static LRESULT PaintStatic (HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
	CRect    cRect;
	DWORD    dwAlign;
   PAINTSTRUCT ps;

   HDC hDC = GetDC(hWnd);
	CDC dc;
	dc.Attach (hDC);
	BeginPaint (hWnd, &ps);

	dc.SetBkMode (TRANSPARENT);

	CFont *pcFont 	  = CFont::FromHandle ((HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));
	CFont *pcOldFont = dc.SelectObject (pcFont);

	CHAR szTxt[2048];
	GetWindowText (hWnd, szTxt, sizeof (szTxt));
	GetWindowRect (hWnd, cRect);

	HWND hWndParent;
	hWndParent = GetParent (hWnd);

	POINT pt1, pt2;
	pt1.x = cRect.left;
	pt1.y = cRect.top;
	pt2.x = cRect.right;
	pt2.y = cRect.bottom;

	ScreenToClient (hWnd, &pt1);
	ScreenToClient (hWnd, &pt2);
	cRect.SetRect (pt1.x,pt1.y,pt2.x,pt2.y);

	DWORD dwStyle   = GetWindowLong (hWnd, GWL_STYLE); //	dwStyle = GetStyle ();
	DWORD dwExStyle = GetWindowLong (hWnd, GWL_EXSTYLE);
	int  iID = GetWindowLong (hWnd, GWL_ID);    //	int iID = GetDlgCtrlID (); // helps debug

	if ((dwStyle & WS_DISABLED) == WS_DISABLED)
		dc.SetTextColor (GetSysColor (COLOR_GRAYTEXT));
	else
		dc.SetTextColor (mapStaticInfo[hWnd]->pcBH->GetColor (DCT_TEXTCOLOR));

	if ((dwStyle & SS_LEFTNOWORDWRAP) == SS_LEFTNOWORDWRAP)
		dwAlign = DT_LEFT;
	else
   	dwAlign = (dwStyle & 0x03) | DT_WORDBREAK; // right/left alignment

	if ((dwStyle & SS_CENTERIMAGE) == SS_CENTERIMAGE)
      dwAlign = (dwAlign | DT_VCENTER | DT_SINGLELINE) & ~DT_WORDBREAK;
   
	if ((dwStyle & SS_SUNKEN) == SS_SUNKEN)
      cRect.DeflateRect (GetSystemMetrics (SM_CXBORDER),GetSystemMetrics (SM_CYBORDER));

	if ((dwExStyle & WS_EX_CLIENTEDGE) == WS_EX_CLIENTEDGE)
      cRect.DeflateRect (GetSystemMetrics (SM_CXEDGE),GetSystemMetrics (SM_CYEDGE));

//debug
//   sprintf (szTxt, "%lx : %lx", dwStyle, GetWindowLong (hWnd, GWL_EXSTYLE));

	dc.DrawText (szTxt, &cRect, dwAlign);
	dc.SelectObject (pcOldFont);

	EndPaint (hWnd, &ps);
	return TRUE;
	}

static LRESULT SetText (HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
	// we don't want the default SETTEXT proc to repaint the window directly!
	DWORD dwStyle = GetWindowLong (hWnd, GWL_STYLE); 

	PSZ psz = (PSZ) lParam; // for debug

	HWND hPar = GetParent(hWnd);
	RECT rect;
	GetWindowRect(hWnd, &rect); 

	POINT pt1, pt2;
	pt1.x = rect.left;
	pt1.y = rect.top;
	pt2.x = rect.right;
	pt2.y = rect.bottom;

	ScreenToClient (hPar, &pt1);
	ScreenToClient (hPar, &pt2);
	SetRect (&rect, pt1.x,pt1.y,pt2.x,pt2.y);
	InvalidateRect(hPar, &rect, TRUE);

	SetWindowLong (hWnd, GWL_STYLE, dwStyle & ~WS_VISIBLE);
	LRESULT lRet = CallWindowProc(mapStaticInfo[hWnd]->pfnDefaultStaticHandler, hWnd, WM_SETTEXT, wParam, lParam);
	SetWindowLong (hWnd, GWL_STYLE, dwStyle);
	InvalidateRect (hWnd, NULL, TRUE);
	return lRet;
	}

static LRESULT CALLBACK fnStaticHandler (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
   switch(uMsg)
   	{
   	case WM_PAINT:
			return PaintStatic (hWnd, wParam, lParam);
   	case WM_SETTEXT:
			return SetText (hWnd, wParam, lParam);
		}
	WNDPROC pfnHandler = mapStaticInfo[hWnd]->pfnDefaultStaticHandler;
	if (pfnHandler)
	   return CallWindowProc(pfnHandler, hWnd, uMsg, wParam, lParam);
	return 0;
	}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CDynaBkgHandler

#define DYNAMODE_NONE 0
#define DYNAMODE_FADE 1
#define DYNAMODE_BMP	 2

CDynaBkgHandler::CDynaBkgHandler ()
	{
	m_pWnd				  = NULL ;
	m_clrText			  = GetSysColor (COLOR_BTNTEXT);
	m_clrBackground	  = GetSysColor (COLOR_3DFACE);
	m_pBkgBrush			  = NULL ;

	m_bInitialized 	  = FALSE;
	m_iBkgMode			  = 0;
	m_bUseBkgAnyway     = FALSE;
	}


CDynaBkgHandler::~CDynaBkgHandler ()
	{
	CStaticControlInfo* pcSCI;

	POSITION pos = mapStaticInfo.GetStartPosition ();
	HWND hWnd;

	while (pos)
		{
		mapStaticInfo.GetNextAssoc(pos, hWnd, pcSCI);
		if (pcSCI->pcBH == this)
			{
			delete pcSCI;
			mapStaticInfo.RemoveKey (hWnd);
			}
		}
	if (m_pBkgBrush)
      delete m_pBkgBrush;
	}


BOOL CDynaBkgHandler::IsInitialized () // private
	{
	return m_bInitialized;
  	}

BOOL CDynaBkgHandler::RegisterControl (CWnd* pcWnd)
	{
	char	szClassName[64];

	if (!pcWnd || !pcWnd->m_hWnd)
		return FALSE;

	GetClassName (pcWnd->m_hWnd, szClassName, 63);
	int iStyle = pcWnd->GetStyle ();
	int iID    = pcWnd->GetDlgCtrlID (); // helps debug

	if (stricmp (szClassName, "Static")   ||
	    ((iStyle & SS_ICON)   == SS_ICON) ||
	    ((iStyle & SS_BITMAP) == SS_BITMAP))
		return FALSE;

//	 I Cannot use SubclassWindow.  If the user already has a class for one or
//	 more of the controls then SubclassWindow will fail.
//	 So we do this the old way
//	 See the large comment block above fnStaticHandler for more info.
//
//	CDynaStatic *pcCtl = new CDynaStatic;
//	pcCtl->SubclassWindow (pcWnd->m_hWnd);
//	pcCtl->m_pDH = this;

	CStaticControlInfo* pcSCI;

	if (mapStaticInfo.Lookup (pcWnd->m_hWnd, pcSCI)) // was already subclassed
		{
		pcSCI->pcBH = this;
		}
	else
		{
		pcSCI = new CStaticControlInfo;
		pcSCI->pfnDefaultStaticHandler = (WNDPROC)GetWindowLong(pcWnd->m_hWnd, GWL_WNDPROC);
		pcSCI->pcBH	 =	this;
	   mapStaticInfo[pcWnd->m_hWnd]   = pcSCI;
		}
   SetWindowLong(pcWnd->m_hWnd, GWL_WNDPROC, (LONG)fnStaticHandler);

	return TRUE;
	}


BOOL CDynaBkgHandler::UnregisterControl (CWnd* pcWnd)
	{
	CStaticControlInfo* pcSCI;

	if (!pcWnd || !pcWnd->m_hWnd)
		return FALSE;
	if (!mapStaticInfo.Lookup (pcWnd->m_hWnd, pcSCI))
		return FALSE; // wasn't subclassed

   SetWindowLong(pcWnd->m_hWnd, GWL_WNDPROC, (LONG)pcSCI->pfnDefaultStaticHandler);
	mapStaticInfo.RemoveKey (pcWnd->m_hWnd);
	delete pcSCI;
	return TRUE;
	}


BOOL CDynaBkgHandler::_Init (CWnd* pWnd, COLORREF clrBackground, COLORREF clrText, int iMode) // private
	{
	m_pWnd 			 = (pWnd                ? pWnd            : m_pWnd			);
	m_clrBackground = (clrBackground	== -1 ? m_clrBackground	: clrBackground);
	m_clrText		 = (clrText			== -1 ? m_clrText			: clrText		);
	m_iBkgMode 		 = (iMode			== -1 ? m_iBkgMode 		: iMode			);

	if (!pWnd || !pWnd->GetSafeHwnd())
		return FALSE;

	CDC *pDC = pWnd->GetDC();
	BOOL bUseBkg  = (GetDeviceCaps (pDC->m_hDC, BITSPIXEL) > 8 ? TRUE : m_bUseBkgAnyway);
	m_iBkgMode 	  = (bUseBkg ? m_iBkgMode : 0); 

	CWnd  *pcWnd = m_pWnd->GetWindow (GW_CHILD);

	while (pcWnd)
		{
		if (m_iBkgMode)	// subclass static ctls if were painting the bkg ourselves
			RegisterControl (pcWnd);
		else				  // clear the subclass info
			UnregisterControl (pcWnd);
		pcWnd = pcWnd->GetWindow (GW_HWNDNEXT);
		}

	if (m_iBkgMode)
		{
		if (m_pBkgBrush)
			{
			delete m_pBkgBrush;
			m_pBkgBrush = NULL;
			}
		if (m_iBkgMode == DYNAMODE_BMP  && m_clrBackground != -1)
			m_pBkgBrush = new CBrush (m_clrBackground);
		}
	return m_bInitialized = TRUE;
	}


BOOL CDynaBkgHandler::UseBkgIfNotHiColor (BOOL bUse)
	{
	return m_bUseBkgAnyway = bUse;
	}

BOOL CDynaBkgHandler::SetBkg (CWnd* pWnd, COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	_Init (pWnd, -1, clrText, DYNAMODE_FADE);
	return m_cClr.Setup (clrTop, clrBottom);
	}


BOOL CDynaBkgHandler::SetBkg (CWnd* pWnd, LPCTSTR lpctBitmapFile, COLORREF clrBackground, COLORREF clrText)
	{
	_Init (pWnd, clrBackground, clrText, DYNAMODE_BMP);
	return m_cClr.Setup (lpctBitmapFile);
	}


BOOL CDynaBkgHandler::SetBkg (CWnd* pWnd, USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText)
	{
	_Init (pWnd, clrBackground, clrText, DYNAMODE_BMP);
	return m_cClr.Setup (iBitmapID);
	}

BOOL CDynaBkgHandler::SetBkg (CWnd* pWnd)
	{
	_Init (pWnd, -1, -1, 0);
	return m_cClr.Setup ();
	}

//	If SetBkg is called before the dialog has an attached window (say, before calling DoModal)
//	Then the initialization can't complete.  This fn completes the initialization
//
BOOL CDynaBkgHandler::CompleteInit (CWnd* pWnd)
	{
	return _Init (pWnd, -1, -1, -1);
	}


BOOL CDynaBkgHandler::DrawBackground (CDC* pDC, CSize size, CPoint pos) 
	{
	CRect cRect;
	if (!m_bInitialized || !m_iBkgMode || !pDC)
		return FALSE;
	m_pWnd->GetClientRect (cRect);
	return m_cClr.PaintIt (pDC, cRect, size, pos);
	}

HBRUSH CDynaBkgHandler::GetCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor, HBRUSH hbr, int iFullYSize, int iYTop) 
	{
	if (!m_bInitialized || !m_iBkgMode || !pDC || !pWnd)
		return hbr;

	DWORD dwStyle = pWnd->GetStyle ();
	CHAR szClassName [256];
	GetClassName (pWnd->m_hWnd, szClassName, 255);

	if (stricmp (szClassName, "Button"))
		return hbr;

	if (!((dwStyle & WS_DISABLED) == WS_DISABLED))
		pDC->SetTextColor (m_clrText);

	pDC->SetBkMode(TRANSPARENT);

	COLORREF clrBkg;
	if (m_iBkgMode == DYNAMODE_BMP) // fixed bkg color if using bitmap bkg
		{
		return (HBRUSH) (m_pBkgBrush ? m_pBkgBrush->m_hObject : hbr);
		}
	else // bkg color based on vertical location in window
		{
		CRect cClientRect, cCtlRect;
		m_pWnd->GetClientRect (cClientRect);
		pWnd->GetWindowRect (cCtlRect);
		m_pWnd->ScreenToClient (cCtlRect);

		iFullYSize = max (cClientRect.bottom, iFullYSize);

		if ((dwStyle & BS_GROUPBOX) == BS_GROUPBOX)
			clrBkg = m_cClr.ColorAtPosition (iFullYSize, cCtlRect.top + iYTop);
		else
			clrBkg = m_cClr.ColorAtPosition (iFullYSize, cCtlRect.CenterPoint().y + iYTop);

		if (!m_pBkgBrush)
			m_pBkgBrush = new CBrush;
		if (m_pBkgBrush->m_hObject)
			m_pBkgBrush->DeleteObject();
		m_pBkgBrush->CreateSolidBrush (clrBkg);

		return (HBRUSH)m_pBkgBrush->m_hObject;
		}
	return hbr;
	}


COLORREF CDynaBkgHandler::GetColor (int iColorType)
	{
	switch (iColorType)
		{
		case DCT_TOPCOLOR		: return m_cClr.m_clrTop;
		case DCT_BOTTOMCOLOR	: return m_cClr.m_clrBottom;
		case DCT_TEXTCOLOR	: return m_clrText;
		case DCT_BKGCOLOR		: return m_clrBackground;
		}
	return m_clrBackground;
	}

int CDynaBkgHandler::GetMode ()
	{
	return m_iBkgMode;
	}

void CDynaBkgHandler::SetStripeCount (int iStripes) // if using fade fill
   {
   m_cClr.SetStripeCount (iStripes);
   }


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CDynaCtlHandler

#define DEFAULT_META_CTLID 4000

CDynaCtlHandler::CDynaCtlHandler ()
	{
	m_pWnd				  = NULL ;
	m_bResizingControls = TRUE	;
	m_bInitialized 	  = FALSE;
	}



CDynaCtlHandler::~CDynaCtlHandler ()
	{
	HWND hWnd;
	CCtlInfo *pCtlInfo;

	POSITION pos = m_mapCtlInfo.GetStartPosition ();
	while (pos)
		{
		m_mapCtlInfo.GetNextAssoc(pos, hWnd, pCtlInfo);
		delete pCtlInfo;
		}
	m_mapCtlInfo.RemoveAll();
	}


BOOL CDynaCtlHandler::_Init () // private!
	{
	CWnd  *pDynaAreas[5];
	CRect cDynaRect[5], rAll, rCtl, rTmp;
	CCtlInfo *pCtlInfo;

	if (!m_pWnd || !m_pWnd->GetSafeHwnd())
		return FALSE;
	if (!m_bResizingControls)
		return m_bInitialized = TRUE;

	/*--- store location of metadata rectangles ---*/
	for (int i=0; i<5; i++)
		{
		cDynaRect[i] = CRect(0,0,0,0);
		int iID = m_iMetaCtlID[i];
		if (pDynaAreas[i] = m_pWnd->GetDlgItem (iID))
			pDynaAreas[i]->GetWindowRect (cDynaRect[i]);
		}

	/*--- define boundry of all the controls ---*/
	CWnd *pChild;
	rAll = CRect (0, 0, 0, 0);
	for (pChild=m_pWnd->GetWindow (GW_CHILD); pChild; pChild=pChild->GetWindow (GW_HWNDNEXT))
		{
		pChild->GetWindowRect (rCtl);
		rAll |= rCtl;
		}
	m_pWnd->ScreenToClient (rAll);
	rAll.InflateRect (0, 0, rAll.left, rAll.top);


	/*--- save the original positions of moveable ctls ---*/
	int iDynaType;
	for (pChild=m_pWnd->GetWindow (GW_CHILD); pChild; pChild=pChild->GetWindow (GW_HWNDNEXT))
		{
		int iID = pChild->GetDlgCtrlID ();

		if (m_mapCtlInfo.Lookup (pChild->m_hWnd, pCtlInfo)) // already done previously
			continue;

		if(iID==m_iMetaCtlID[0] ||
			iID==m_iMetaCtlID[1] ||
			iID==m_iMetaCtlID[2] ||
			iID==m_iMetaCtlID[3] )
			continue;

		pChild->GetWindowRect (rCtl);
		iDynaType = 0;
		int iTmpDynaType = 0;

		if (m_mapAddCtlList.Lookup (iID, iTmpDynaType)) // explicitly added controls
			{
			iDynaType = iTmpDynaType;
			}
		else
			{
		 	for (int i=0; i<5; i++)
				if (rTmp.IntersectRect (rCtl, cDynaRect[i]))
					iDynaType |= (i==0 ? DT_MOVER :(i==1 ? DT_SIZER :(i==2 ? DT_MOVEB : (i==3 ? DT_SIZEB : DT_CTR))));
			}
		if (!iDynaType)
			continue;

		m_pWnd->ScreenToClient (rCtl);
		CCtlInfo *pCtlInfo = new CCtlInfo ();
		m_mapCtlInfo [pChild->m_hWnd] = pCtlInfo;
		pCtlInfo->iDynaType = iDynaType;
		pCtlInfo->iXMinPos  = rCtl.right;
		pCtlInfo->iYMinPos  = rCtl.bottom;

		if (iDynaType & DT_CTR) // centering is incompatible with the other types
			{
			pCtlInfo->iXGap  = rCtl.left - (rAll.left + rAll.right )/2;
			pCtlInfo->iYGap  = rCtl.top  - (rAll.top  + rAll.bottom)/2;
			}
		else
			{
			pCtlInfo->iXGap  = rAll.right - rCtl.right;
			pCtlInfo->iYGap  = rAll.bottom - rCtl.bottom;
			}
		}
	return m_bInitialized = TRUE;
	}


void CDynaCtlHandler::Size ()
	{
	CCtlInfo *pCtlInfo;
	CRect rCtl, rClient;

	if (!m_pWnd || !m_bResizingControls || !m_bInitialized)
		return;

// promote this!
//	if (m_pWnd && m_pWnd->m_hWnd && m_iBkgMode == DYNAMODE_FADE)
//		m_pWnd->Invalidate ();

	m_pWnd->GetClientRect (rClient);
	CWnd *pChild;
	for (pChild=m_pWnd->GetWindow (GW_CHILD); pChild; pChild=pChild->GetWindow (GW_HWNDNEXT))
		{
		if (!m_mapCtlInfo.Lookup (pChild->m_hWnd, pCtlInfo))
			continue;
		pChild->GetWindowRect (rCtl);
		m_pWnd->ScreenToClient (rCtl);
		m_pWnd->InvalidateRect (rCtl);

		if (pCtlInfo->iDynaType & DT_CTR) // centering is incompatible with the other types
			{
			rCtl += CPoint (rClient.right/2  + pCtlInfo->iXGap - rCtl.left,
								 rClient.bottom/2 + pCtlInfo->iYGap - rCtl.top);
			}
		else
			{ // Sizing taking precedence over moving
			if (pCtlInfo->iDynaType & DT_SIZER)
				rCtl.right = max (pCtlInfo->iXMinPos, rClient.right - pCtlInfo->iXGap);
			if (pCtlInfo->iDynaType & DT_MOVER)
				rCtl += CPoint (max (pCtlInfo->iXMinPos, rClient.right - pCtlInfo->iXGap) - rCtl.right, 0);
			if (pCtlInfo->iDynaType & DT_SIZEB)
				rCtl.bottom = max (pCtlInfo->iYMinPos, rClient.bottom - pCtlInfo->iYGap);
			if (pCtlInfo->iDynaType & DT_MOVEB)
				rCtl += CPoint (0, max (pCtlInfo->iYMinPos, rClient.bottom - pCtlInfo->iYGap) - rCtl.bottom);
			}
		pChild->MoveWindow (rCtl);
		}
	}

void CDynaCtlHandler::AddDynaControl (int iDialogID, int iDynaFlags)
	{
	m_mapAddCtlList [iDialogID] = iDynaFlags;
	}


BOOL CDynaCtlHandler::UseResizingControls (CWnd* pWnd, BOOL bUse, int iMR, int iSR, int iMB, int iSB, int iCT)
	{
	m_bResizingControls 	= bUse;
	m_pWnd 					= pWnd;
	m_iMetaCtlID[0] =	(iMR == -1 ? DEFAULT_META_CTLID   : iMR);
	m_iMetaCtlID[1] =	(iSR == -1 ? DEFAULT_META_CTLID+1 : iSR);
	m_iMetaCtlID[2] =	(iMB == -1 ? DEFAULT_META_CTLID+2 : iMB);
	m_iMetaCtlID[3] =	(iSB == -1 ? DEFAULT_META_CTLID+3 : iSB);
	m_iMetaCtlID[4] =	(iCT == -1 ? DEFAULT_META_CTLID+4 : iCT);

	return _Init ();
	}

BOOL CDynaCtlHandler::CompleteInit (CWnd* pWnd)
	{
	m_pWnd = pWnd;
	return _Init ();
	}

BOOL CDynaCtlHandler::IsInitialized ()
	{
	return m_bInitialized;
  	}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// class CDynaFontHandler
CDynaFontHandler::CDynaFontHandler ()
	{
	m_strFont.Empty();
	m_pDT			= NULL ;
	m_iFontSize	= 0		;
	}


CDynaFontHandler::~CDynaFontHandler ()
	{
	}


BOOL CDynaFontHandler::SetFont (CString strFontName, int iFontSize)
	{
	m_strFont   = strFontName;
	m_iFontSize	= iFontSize;
	return TRUE;
	}


CString& CDynaFontHandler::GetFont (CString &strFontName, int &iFontSize)
	{
	strFontName = m_strFont  ;
	iFontSize   = m_iFontSize;

	return strFontName;
	}


LPCDLGTEMPLATE CDynaFontHandler::GetResourceTemplate (LPCTSTR lpszTemplateName)
	{
	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);

	m_pDT = (PVOID)(CDialogTemplate*)new CDialogTemplate ((HGLOBAL)NULL);
	((CDialogTemplate*)m_pDT)->Load(lpszTemplateName);
	if (m_strFont)
		((CDialogTemplate*)m_pDT)->SetFont (m_strFont, m_iFontSize);
	return (LPCDLGTEMPLATE)LockResource(((CDialogTemplate*)m_pDT)->m_hTemplate);
	}


void CDynaFontHandler::FreeResourceTemplate ()
	{
	if (!m_pDT)
		return;
	if (((CDialogTemplate*)m_pDT)->m_hTemplate != NULL)
		{
		UnlockResource(((CDialogTemplate*)m_pDT)->m_hTemplate);
		FreeResource(((CDialogTemplate*)m_pDT)->m_hTemplate);
		}
	delete (CDialogTemplate*)m_pDT;
	m_pDT = NULL;
	}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


// CDynaDialog dialog
BEGIN_MESSAGE_MAP(CDynaDialog, CDialog)
	//{{AFX_MSG_MAP(CDynaDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CDynaDialog::CDynaDialog(UINT uID, CWnd* pParent /*=NULL*/): CDialog(uID, pParent)
	{
	m_uID = uID;
//	m_dhBkg.Init (this);
//	m_dhCtl.Init (this);
	}

BOOL CDynaDialog::SetFont (CString strFontName, int iFontSize)
	{
	return m_dhFont.SetFont (strFontName, iFontSize);
	}

BOOL CDynaDialog::SetBkg  (COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, clrTop, clrBottom, clrText);
	}

BOOL CDynaDialog::SetBkg  (USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, iBitmapID, clrBackground, clrText);
	}

BOOL CDynaDialog::SetBkg  (LPCTSTR lpctBitmapFile, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, lpctBitmapFile, clrBackground, clrText);
	}

BOOL CDynaDialog::SetBkg  ()
	{
	return m_dhBkg.SetBkg (this);
	}

BOOL CDynaDialog::UseBkgIfNotHiColor (BOOL bUse)
	{
	return m_dhBkg.UseBkgIfNotHiColor (bUse);
	}

BOOL CDynaDialog::UseResizingControls (BOOL bUse, int iMR, int iSR, int iMB, int iSB, int iCT)
	{
	return m_dhCtl.UseResizingControls (this, bUse, iMR, iSR, iMB, iSB, iCT);
	}

void CDynaDialog::AddDynaControl (int iDialogID, int iDynaFlags)
	{
	m_dhCtl.AddDynaControl (iDialogID, iDynaFlags);
	}



//todo...
BOOL CDynaDialog::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
	{
	return CDialog::Create(m_uID, pParentWnd);
	}

BOOL CDynaDialog::OnEraseBkgnd(CDC* pDC) 
	{
	if (!m_dhBkg.DrawBackground (pDC))
		return CDialog::OnEraseBkgnd(pDC);
	return TRUE;
	}


BOOL CDynaDialog::OnInitDialog() 
	{
	CDialog::OnInitDialog();

	m_dhBkg.CompleteInit (this);
	m_dhCtl.CompleteInit (this);
	return TRUE;
	}


HBRUSH CDynaDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{
   HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_dhBkg.GetCtlColor(pDC, pWnd, nCtlColor, hbr);
	}


void CDynaDialog::OnSize(UINT nType, int cx, int cy) 
	{
	m_dhCtl.Size ();
	CDialog::OnSize(nType, cx, cy);

	if (m_dhBkg.GetMode() == DYNAMODE_FADE)
		Invalidate ();
	}


void CDynaDialog::OnDestroy() 
	{
	CDialog::OnDestroy();
	}

COLORREF CDynaDialog::GetColor (int iColorType)
	{
	return m_dhBkg.GetColor (iColorType);
	}

int CDynaDialog::DoModal() 
	{
	CString strFont;
	int		iFontSize;

	m_dhFont.GetFont (strFont, iFontSize);
	if (strFont.IsEmpty())
		return CDialog::DoModal();

	LPCDLGTEMPLATE lpDialogTemplate = m_dhFont.GetResourceTemplate (m_lpszTemplateName);

//	if (!m_strFont)
//		return CDialog::DoModal();
//--
	HINSTANCE hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
//
//	CDialogTemplate cdt((HGLOBAL)NULL);
//	cdt.Load(m_lpszTemplateName);
//	cdt.SetFont (m_strFont, m_iFontSize);
//
//	LPCDLGTEMPLATE	lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(cdt.m_hTemplate);

	if (lpDialogTemplate == NULL)
		return -1;
//--
	HWND hWndParent = PreModal();
	BOOL bEnableParent = FALSE;
	if (hWndParent != NULL && ::IsWindowEnabled(hWndParent))
		{
		::EnableWindow(hWndParent, FALSE);
		bEnableParent = TRUE;
		}

	if (CreateDlgIndirect(lpDialogTemplate, CWnd::FromHandle(hWndParent), hInst))
		{
		if (m_nFlags & WF_CONTINUEMODAL)
			{
			// enter modal loop
			DWORD dwFlags = MLF_SHOWONIDLE;
			if (GetStyle() & DS_NOIDLEMSG) dwFlags |= MLF_NOIDLEMSG;
			VERIFY(RunModalLoop(dwFlags) == m_nModalResult);
			}

		// hide the window before enabling the parent, etc.
		if (m_hWnd != NULL)
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
		}
	if (bEnableParent)
		::EnableWindow(hWndParent, TRUE);
	if (hWndParent != NULL && ::GetActiveWindow() == m_hWnd)
		::SetActiveWindow(hWndParent);

	// destroy modal window
	DestroyWindow();
	PostModal();

	m_dhFont.FreeResourceTemplate ();

	return m_nModalResult;
	}


//void CDynaDialog::RegisterControl (CWnd* pcWnd, DWORD dwFlags) 
//	{
//	m_dhBkg.RegisterControl (pcWnd); 			 // background painting
////	m_dhCtl.RegisterControl (pcWnd, dwFlags); // control sizing
//	}


void CDynaDialog::SetStripeCount (int iStripes)
   {
   m_dhBkg.SetStripeCount (iStripes);
   }


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDynaPropertyPage property page

IMPLEMENT_DYNAMIC(CDynaPropertyPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CDynaPropertyPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDynaPropertyPage)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDynaPropertyPage::CDynaPropertyPage(UINT uID): CPropertyPage(uID)
	{
	m_uID = uID;
//	m_dhBkg.Init (this);
//	m_dhCtl.Init (this);
	}

CDynaPropertyPage::~CDynaPropertyPage()
	{
	}


BOOL CDynaPropertyPage::SetFont (CString strFontName, int iFontSize)
	{
	return m_dhFont.SetFont (strFontName, iFontSize);
	}

BOOL CDynaPropertyPage::SetBkg (COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, clrTop, clrBottom, clrText);
	}

BOOL CDynaPropertyPage::SetBkg (LPCTSTR lpctBitmapFile, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, lpctBitmapFile, clrBackground, clrText);
	}

BOOL CDynaPropertyPage::SetBkg (USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, iBitmapID, clrBackground, clrText);
	}

BOOL CDynaPropertyPage::SetBkg ()
	{
	return m_dhBkg.SetBkg (this);
	}

BOOL CDynaPropertyPage::UseBkgIfNotHiColor (BOOL bUse)
	{
	return m_dhBkg.UseBkgIfNotHiColor (bUse);
	}

BOOL CDynaPropertyPage::UseResizingControls (BOOL bUse, int iMR, int iSR, int iMB, int iSB, int iCT)
	{
	return m_dhCtl.UseResizingControls (this, bUse, iMR, iSR, iMB, iSB, iCT);
	}

void CDynaPropertyPage::AddDynaControl (int iDialogID, int iDynaFlags)
	{
	m_dhCtl.AddDynaControl (iDialogID, iDynaFlags);
	}

BOOL CDynaPropertyPage::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
	{
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
	}

BOOL CDynaPropertyPage::OnEraseBkgnd(CDC* pDC) 
	{
	if (!m_dhBkg.DrawBackground (pDC))
		return CPropertyPage::OnEraseBkgnd(pDC);
	return TRUE;
	}


BOOL CDynaPropertyPage::OnInitDialog() 
	{
	CPropertyPage::OnInitDialog();

	m_dhBkg.CompleteInit (this);
	m_dhCtl.CompleteInit (this);
	return TRUE;
	}


HBRUSH CDynaPropertyPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{
	HBRUSH hbr = CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_dhBkg.GetCtlColor(pDC, pWnd, nCtlColor, hbr);
	}


void CDynaPropertyPage::OnSize(UINT nType, int cx, int cy) 
	{
	m_dhCtl.Size ();
	CPropertyPage::OnSize(nType, cx, cy);

	if (m_dhBkg.GetMode() == DYNAMODE_FADE)
		Invalidate ();
	}


void CDynaPropertyPage::OnDestroy() 
	{
	CPropertyPage::OnDestroy();
	}


COLORREF CDynaPropertyPage::GetColor (int iColorType)
	{
	return m_dhBkg.GetColor (iColorType);
	}


int CDynaPropertyPage::DoModal() 
	{
	// TODO: Add your specialized code here and/or call the base class
	return CPropertyPage::DoModal();
	}

//void CDynaPropertyPage::RegisterControl (CWnd* pcWnd, DWORD dwFlags) 
//	{
//	m_dhBkg.RegisterControl (pcWnd); 			 // background painting
////	m_dhCtl.RegisterControl (pcWnd, dwFlags); // control sizing
//	}


void CDynaPropertyPage::SetStripeCount (int iStripes)
   {
   m_dhBkg.SetStripeCount (iStripes);
   }



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CDynaFormView

IMPLEMENT_DYNAMIC(CDynaFormView, CFormView)

BEGIN_MESSAGE_MAP(CDynaFormView, CFormView)
	//{{AFX_MSG_MAP(CDynaFormView)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CDynaFormView::CDynaFormView(UINT uID): CFormView(uID)
	{
	m_uID = uID;
//	m_dhBkg.Init ((CDialog*)this);
//	m_dhCtl.Init ((CDialog*)this);
	}

CDynaFormView::~CDynaFormView()
	{
	}

#ifdef _DEBUG
void CDynaFormView::AssertValid() const
	{
	CFormView::AssertValid();
	}

void CDynaFormView::Dump(CDumpContext& dc) const
	{
	CFormView::Dump(dc);
	}
#endif //_DEBUG



BOOL CDynaFormView::SetFont (CString strFontName, int iFontSize)
	{
	return m_dhFont.SetFont (strFontName, iFontSize);
	}

BOOL CDynaFormView::SetBkg (COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	return m_dhBkg.SetBkg ((CWnd*)this, clrTop, clrBottom, clrText);
	}

BOOL CDynaFormView::SetBkg  (LPCTSTR lpctBitmapFile, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg ((CWnd*)this, lpctBitmapFile, clrBackground, clrText);
	}

BOOL CDynaFormView::SetBkg  (USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg ((CWnd*)this, iBitmapID, clrBackground, clrText);
	}

BOOL CDynaFormView::SetBkg  ()
	{
	return m_dhBkg.SetBkg ((CWnd*)this);
	}

BOOL CDynaFormView::UseBkgIfNotHiColor (BOOL bUse)
	{
	return m_dhBkg.UseBkgIfNotHiColor (bUse);
	}

BOOL CDynaFormView::UseResizingControls (BOOL bUse, int iMR, int iSR, int iMB, int iSB, int iCT)
	{
	return m_dhCtl.UseResizingControls ((CWnd*)this, bUse, iMR, iSR, iMB, iSB, iCT);
	}


void CDynaFormView::AddDynaControl (int iDialogID, int iDynaFlags)
	{
	m_dhCtl.AddDynaControl (iDialogID, iDynaFlags);
	}

//BOOL CDynaFormView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
//	{
//	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
//	}

BOOL CDynaFormView::OnEraseBkgnd(CDC* pDC) 
	{
	if (!m_dhBkg.DrawBackground (pDC, GetTotalSize(), GetScrollPosition()))
		return CFormView::OnEraseBkgnd(pDC);
	return TRUE;
	}



//BOOL CDynaFormView::OnInitDialog() 
//	{
//	CFormView::OnInitDialog();
//
////	m_dh.Init (this);
//	return TRUE;
//	}


HBRUSH CDynaFormView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_dhBkg.GetCtlColor(pDC, pWnd, nCtlColor, hbr, GetTotalSize().cy, GetScrollPosition().y);
	}

void CDynaFormView::OnSize(UINT nType, int cx, int cy) 
	{
	m_dhCtl.Size ();
	CFormView::OnSize(nType, cx, cy);

	if (m_dhBkg.GetMode() == DYNAMODE_FADE)
		Invalidate ();
	}


void CDynaFormView::OnDestroy() 
	{
	CFormView::OnDestroy();
	}


COLORREF CDynaFormView::GetColor (int iColorType)
	{
	return m_dhBkg.GetColor (iColorType);
	}



//	todo: call void CDynaHandler::FreeResourceTemplate () at some point!
//
//
BOOL CDynaFormView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
	DWORD dwRequestedStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
	{
	ASSERT(pParentWnd != NULL);
	ASSERT(m_lpszTemplateName != NULL);

	/*--- just call the default if we don't need to set the font ---*/
	CString strFont;
	int	  iFontSize;
	m_dhFont.GetFont (strFont, iFontSize);
	if (strFont.IsEmpty())
		return CFormView::Create (lpszClassName, lpszWindowName, dwRequestedStyle, rect, pParentWnd, nID, pContext);

	m_pCreateContext = pContext;    // save state for later OnCreate

	// call PreCreateWindow to get prefered extended style
	CREATESTRUCT cs; memset(&cs, 0, sizeof(CREATESTRUCT));
	if (dwRequestedStyle == 0)
		dwRequestedStyle = AFX_WS_DEFAULT_VIEW;
	cs.style = dwRequestedStyle;
	if (!PreCreateWindow(cs))
		return FALSE;

	HINSTANCE hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
	LPCDLGTEMPLATE lpDialogTemplate = m_dhFont.GetResourceTemplate (m_lpszTemplateName);
	if (lpDialogTemplate == NULL)
		return FALSE;
	if (!CreateDlgIndirect(lpDialogTemplate, pParentWnd, hInst))
		return FALSE;

	m_pCreateContext = NULL;

	// we use the style from the template - but make sure that
	//  the WS_BORDER bit is correct
	// the WS_BORDER bit will be whatever is in dwRequestedStyle
	ModifyStyle(WS_BORDER|WS_CAPTION, cs.style & (WS_BORDER|WS_CAPTION));
	ModifyStyleEx(WS_EX_CLIENTEDGE, cs.dwExStyle & WS_EX_CLIENTEDGE);

	SetDlgCtrlID(nID);

	CRect rectTemplate;
	GetWindowRect(rectTemplate);
	SetScrollSizes(MM_TEXT, rectTemplate.Size());

	// initialize controls etc
	if (!ExecuteDlgInit(m_lpszTemplateName))
		return FALSE;

	// force the size requested
	SetWindowPos(NULL, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOZORDER|SWP_NOACTIVATE);

	// make visible if requested
	if (dwRequestedStyle & WS_VISIBLE)
		ShowWindow(SW_NORMAL);

	return TRUE;
	}



//void CDynaFormView::RegisterControl (CWnd* pcWnd, DWORD dwFlags) 
//	{
//	m_dhBkg.RegisterControl (pcWnd); 			 // background painting
////	m_dhCtl.RegisterControl (pcWnd, dwFlags); // control sizing
//	}


void CDynaFormView::SetStripeCount (int iStripes)
   {
   m_dhBkg.SetStripeCount (iStripes);
   }
