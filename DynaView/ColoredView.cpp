// ColoredTreeView1.cpp : implementation file
//
// Craig Fitzgerald
//

#include "..\stdafx.h"
#include "G_DynaView.h"
#include "G_ColoredView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColoredTreeView

IMPLEMENT_DYNCREATE(CColoredTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CColoredTreeView, CTreeView)
	//{{AFX_MSG_MAP(CColoredTreeView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CColoredTreeView::CColoredTreeView()
	{
	m_bEnabled = FALSE;
	}

CColoredTreeView::~CColoredTreeView()
	{
	}

void CColoredTreeView::OnDraw(CDC* pDC)
	{
	//CDocument* pDoc = GetDocument();
	// TODO: add draw code here
	}


#ifdef _DEBUG
void CColoredTreeView::AssertValid() const
	{
	CTreeView::AssertValid();
	}

void CColoredTreeView::Dump(CDumpContext& dc) const
	{
	CTreeView::Dump(dc);
	}
#endif //_DEBUG


BOOL CColoredTreeView::OnEraseBkgnd(CDC* pDC) 
	{
	if (m_bEnabled && m_cClr.IsSetup ())
		return TRUE;
	else
		return CTreeView::OnEraseBkgnd(pDC);
	}


void CColoredTreeView::OnPaint() 
	{
	if (!m_bEnabled || !m_cClr.IsSetup ())
		{
		CTreeView::OnPaint();
		return;
		}
	CRect crClip, crClient;

	GetUpdateRect (crClip);
   GetClientRect(crClient);
	CDC *pdc = GetDC ();

	// make an in-memory DC
	CDC cdcMem;
	cdcMem.CreateCompatibleDC(pdc);

   // make a compatible bitmap into the memory DC
	CBitmap cBmpMem;
	cBmpMem.CreateCompatibleBitmap (pdc, crClient.Width(), crClient.Height());
	CBitmap *pOldMemBitmap = cdcMem.SelectObject (&cBmpMem);


	// here we want the tree to paint itself.
	// But, rather than calling CTreeView::OnPaint(), we trick the window
	// into painting onto the memoryDC bitmap instead
	//	CTreeView::OnPaint();
	PAINTSTRUCT ps;
	BeginPaint (&ps);
	CWnd::DefWindowProc (WM_PAINT, (WPARAM)cdcMem.m_hDC, 0);
	EndPaint (&ps);

	crClip = crClient; // background/Foreground aren't bound together

	// make an in-memory Mask DC
	CDC cdcMask;
	cdcMask.CreateCompatibleDC(pdc);

   // make a bitmap for the mask
	CBitmap cBmpMask;
	cBmpMask.CreateBitmap (crClient.Width(), crClient.Height(), 1, 1, NULL);
	CBitmap *pOldMaskBitmap = cdcMask.SelectObject (&cBmpMask);
   cdcMem.SetBkColor (GetSysColor( COLOR_WINDOW )); // come back...

	// create the mask from the memory DC
   cdcMask.BitBlt (0, 0, crClient.Width(), crClient.Height (), 
                   &cdcMem, crClient.left, crClient.top, SRCCOPY);

//	// Create a DC to hold the image bitmap to use for tiling
//	CDC cdcTile;
//	cdcTile.CreateCompatibleDC(pdc);
//	CBitmap *pOldTileBitmap = cdcTile.SelectObject (m_pBitmap);
//
	// create a DC and bitmap to hold the background
	CDC cdcBkg;
	cdcBkg.CreateCompatibleDC(pdc);
	CBitmap cBmpImage;
	cBmpImage.CreateCompatibleBitmap (pdc, crClient.Width(), crClient.Height());
	CBitmap *pOldBkgBitmap = cdcBkg.SelectObject (&cBmpImage);


	// create the background

//	int x,y;
//	BITMAP m_bm;
//	m_pBitmap->GetBitmap(&m_bm);
//	for (y=0; y<crClient.bottom; y+= m_bm.bmHeight)
//		for (x=0; x<crClient.right; x+= m_bm.bmWidth)
//		   cdcBkg.BitBlt(x, y, m_bm.bmWidth, m_bm.bmHeight, &cdcTile, 0, 0, SRCCOPY);

	CSize  cSize = GetTotalSize ();
	CPoint cPos  = GetScrollPosition ();
	m_cClr.PaintIt (&cdcBkg, crClip, cSize, cPos);

	// apply mask to FG
   cdcMem.SetBkColor(RGB(0,0,0));
   cdcMem.SetTextColor(RGB(255,255,255));
	cdcMem.BitBlt(crClip.left, crClip.top, crClip.Width(), crClip.Height(), 
		            &cdcMask, crClip.left, crClip.top, SRCAND);

	// apply mask to BG
   cdcBkg.SetBkColor(RGB(255,255,255));
   cdcBkg.SetTextColor(RGB(0,0,0));
	cdcBkg.BitBlt(crClip.left, crClip.top, crClip.Width(), crClip.Height(), 
		            &cdcMask, crClip.left, crClip.top, SRCAND);

	// combine FG and BG
	cdcBkg.BitBlt(crClip.left, crClip.top, crClip.Width(), crClip.Height(), 
		            &cdcMem, crClip.left, crClip.top, SRCPAINT);

	// put final image on the screen
	pdc->BitBlt(crClip.left, crClip.top, crClip.Width(), crClip.Height(), 
	            &cdcBkg, crClip.left, crClip.top, SRCCOPY);

	cdcMask.SelectObject (pOldMaskBitmap);
//	cdcTile.SelectObject (pOldTileBitmap);
	cdcBkg.SelectObject  (pOldBkgBitmap);
	cdcMem.SelectObject  (pOldMemBitmap);
	}



BOOL CColoredTreeView::SetBkg (COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	CDC* pDC = GetDC();
	if (!(m_bEnabled = (GetDeviceCaps (pDC->m_hDC, BITSPIXEL) > 8 ? TRUE : FALSE)))
		return FALSE;

	if (clrText != -1)
		GetTreeCtrl().SetTextColor (clrText);		//SendMessage (TVM_SETTEXTCOLOR, 0, clrText);
	if (clrBottom == -1)
		return !!GetTreeCtrl().SetBkColor (clrTop);
	return m_cClr.Setup (clrTop, clrBottom);
	}

BOOL CColoredTreeView::SetBkg (LPCTSTR lpctBitmap, COLORREF clrText)
	{
	CDC* pDC = GetDC();
	if (!(m_bEnabled = (GetDeviceCaps (pDC->m_hDC, BITSPIXEL) > 8 ? TRUE : FALSE)))
		return FALSE;

	if (clrText != -1)
		GetTreeCtrl().SetTextColor (clrText);		//SendMessage (TVM_SETTEXTCOLOR, 0, clrText);
	return m_cClr.Setup (lpctBitmap);
	}

BOOL CColoredTreeView::SetBkg (USHORT iBitmapID, COLORREF clrText)
	{
	CDC* pDC = GetDC();
	if (!(m_bEnabled = (GetDeviceCaps (pDC->m_hDC, BITSPIXEL) > 8 ? TRUE : FALSE)))
		return FALSE;

	if (clrText != -1)
		GetTreeCtrl().SetTextColor (clrText);		//SendMessage (TVM_SETTEXTCOLOR, 0, clrText);
	return m_cClr.Setup (iBitmapID);
	}


BOOL CColoredTreeView::SetBkg ()
	{
	return m_cClr.Setup ();
	}


CSize  CColoredTreeView::GetTotalSize ()
	{
	CRect crClient;
   GetClientRect(crClient);

	int iHeight = crClient.Height();

	// --- if there is a vert scrollbar, we must calc the display size ---
	if ((GetStyle() & WS_VSCROLL) == WS_VSCROLL)
		{
		int iMin, iMax;
		GetScrollRange (SB_VERT, &iMin, &iMax);
		int iYInc = GetTreeCtrl().GetItemHeight();
		iHeight = (iMax-iMin ? (iMax-iMin+1)*iYInc : crClient.Height());
		}
	return CSize (crClient.Width(), iHeight);
	}

CPoint CColoredTreeView::GetScrollPosition ()
	{
	int iYInc = GetTreeCtrl().GetItemHeight();
	return CPoint (GetScrollPos (SB_HORZ) * iYInc,	
		            GetScrollPos (SB_VERT) * iYInc); // this is only an approx for now...
	}



/////////////////////////////////////////////////////////////////////////////
// CColoredScrollView
//

//IMPLEMENT_DYNCREATE(CColoredScrollView, CScrollView)
//
//BEGIN_MESSAGE_MAP(CColoredScrollView, CScrollView)
//	//{{AFX_MSG_MAP(CColoredScrollView)
//	ON_WM_ERASEBKGND()
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()
//
//CColoredScrollView::CColoredScrollView()
//	{
//	}
//
//CColoredScrollView::~CColoredScrollView()
//	{
//	}
//
//void CColoredScrollView::OnDraw(CDC* pDC)
//	{
//	return;
//	}
//
////BOOL CColoredScrollView::PreCreateWindow(CREATESTRUCT& cs)
////	{
////	return CScrollView::PreCreateWindow(cs);
////	}
//
//
//BOOL CColoredScrollView::OnEraseBkgnd(CDC* pDC) 
//	{
//	CRect cClientRect;
//	GetClientRect(cClientRect);
//	return m_cClr.PaintIt (pDC, cClientRect, GetTotalSize (), GetScrollPosition ());
//	}
//
//BOOL CColoredScrollView::SetBkg (COLORREF clrTop, COLORREF clrBottom)
//	{
//	return m_cClr.Setup (clrTop, clrBottom);
//	}
//
//BOOL CColoredScrollView::SetBkg (LPCTSTR lpctBitmap)
//	{
//	return m_cClr.Setup (lpctBitmap);
//	}
//
//BOOL CColoredScrollView::SetBkg (USHORT iBitmapID)
//	{
//	return m_cClr.Setup (iBitmapID);
//	}
//
//
//


IMPLEMENT_DYNCREATE(CColoredScrollView, CScrollView)

BEGIN_MESSAGE_MAP(CColoredScrollView, CScrollView)
	//{{AFX_MSG_MAP(CColoredScrollView)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CColoredScrollView::CColoredScrollView()
	{
	}

CColoredScrollView::~CColoredScrollView()
	{
	}

void CColoredScrollView::OnDraw(CDC* pDC)
	{
	return;
	}

//BOOL CColoredScrollView::PreCreateWindow(CREATESTRUCT& cs)
//	{
//	return CScrollView::PreCreateWindow(cs);
//	}


BOOL CColoredScrollView::OnEraseBkgnd(CDC* pDC) 
	{
//	CRect cClientRect;
//	GetClientRect(cClientRect);
//	return m_cClr.PaintIt (pDC, cClientRect, GetTotalSize (), GetScrollPosition ());
//
	if (!m_dhBkg.DrawBackground (pDC, GetTotalSize (), GetScrollPosition ()))
		return CScrollView::OnEraseBkgnd(pDC);
	return TRUE;

	}

BOOL CColoredScrollView::SetBkg (COLORREF clrTop, COLORREF clrBottom, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, clrTop, clrBottom, clrText);
	}

BOOL CColoredScrollView::SetBkg (LPCTSTR lpctBitmap, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, lpctBitmap, clrBackground, clrText);
	}

BOOL CColoredScrollView::SetBkg (USHORT iBitmapID, COLORREF clrBackground, COLORREF clrText)
	{
	return m_dhBkg.SetBkg (this, iBitmapID, clrBackground, clrText);
	}

BOOL CColoredScrollView::SetBkg ()
	{
	return m_dhBkg.SetBkg (this);
	}

BOOL CColoredScrollView::UseBkgIfNotHiColor (BOOL bUse)
	{
	return m_dhBkg.UseBkgIfNotHiColor (bUse);
	}

BOOL CColoredScrollView::BkgEnabled ()
	{
	return !!m_dhBkg.GetMode ();
	}


BOOL CColoredScrollView::UseResizingControls (BOOL bUse, int iMR, int iSR, int iMB, int iSB, int iCT)
	{
	return m_dhCtl.UseResizingControls (this, bUse, iMR, iSR, iMB, iSB, iCT);
	}


HBRUSH CColoredScrollView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
	{
   HBRUSH hbr = CScrollView::OnCtlColor(pDC, pWnd, nCtlColor);

	return m_dhBkg.GetCtlColor(pDC, pWnd, nCtlColor, hbr, GetTotalSize().cy, GetScrollPosition().y);
	}

void CColoredScrollView::OnSize(UINT nType, int cx, int cy) 
	{
	m_dhCtl.Size ();
	CScrollView::OnSize(nType, cx, cy);

	if (m_dhBkg.GetMode() == DYNAMODE_FADE)
		Invalidate ();
	}

