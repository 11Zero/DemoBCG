//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
// BCGPOleServerDoc.cpp : implementation file
//

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPOleServerDoc.h"
#include "BCGPOleDocIPFrameWnd.h"
#include "BCGPOleIPFrameWnd.h"
#include "BCGPOleCntrFrameWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleServerDoc

IMPLEMENT_DYNCREATE(CBCGPOleServerDoc, COleServerDoc)

CBCGPOleServerDoc::CBCGPOleServerDoc()
{
}

BOOL CBCGPOleServerDoc::OnNewDocument()
{
	if (!COleServerDoc::OnNewDocument())
		return FALSE;
	return TRUE;
}

CBCGPOleServerDoc::~CBCGPOleServerDoc()
{
}

COleServerItem* CBCGPOleServerDoc::OnGetEmbeddedItem()
{
	// OnGetEmbeddedItem is called by the framework to get the COleServerItem
	//  that is associated with the document.  It is only called when necessary.

	// Instead of returning NULL, return a pointer to a new COleServerItem
	//  derived class that is used in conjunction with this document, then
	//  remove the ASSERT(FALSE) below.
	//  (i.e., return new CMyServerItem.)
	ASSERT(FALSE);			// remove this after completing the TODO
	return NULL;
}


BEGIN_MESSAGE_MAP(CBCGPOleServerDoc, COleServerDoc)
	//{{AFX_MSG_MAP(CBCGPOleServerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleServerDoc diagnostics

#ifdef _DEBUG
void CBCGPOleServerDoc::AssertValid() const
{
	COleServerDoc::AssertValid();
}

void CBCGPOleServerDoc::Dump(CDumpContext& dc) const
{
	COleServerDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleServerDoc serialization

void CBCGPOleServerDoc::Serialize(CArchive& ar)
{
	COleServerDoc::Serialize (ar);
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPOleServerDoc commands
void CBCGPOleServerDoc::OnResizeBorder( LPCRECT lpRectBorder, LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame )
{
	ASSERT_VALID (this);
	ASSERT (lpUIWindow != NULL);
	
	if (!bFrame)
	{
		COleServerDoc::OnResizeBorder (lpRectBorder, lpUIWindow, bFrame);
		return;
	}

	CBCGPOleCntrFrameWnd* pMainFrame = NULL;

	CBCGPOleDocIPFrameWnd* p_IpDocFrame = 
						DYNAMIC_DOWNCAST(CBCGPOleDocIPFrameWnd, m_pInPlaceFrame);

	if (p_IpDocFrame != NULL)
	{
		pMainFrame =  p_IpDocFrame->GetContainerFrameWindow ();
	}
	else
	{
		CBCGPOleIPFrameWnd* p_IpFrame = 
						DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, m_pInPlaceFrame);

		if (p_IpFrame != NULL)
		{
			pMainFrame = p_IpFrame->GetContainerFrameWindow ();
		}
		else
		{
			return;
		}
	}

	if (pMainFrame == NULL)
	{
		return;
	}

	CBCGPDockManager* pDockManager = pMainFrame->GetDockManager ();
	ASSERT_VALID (pDockManager);

	CRect rcCurBorders;

	if (lpRectBorder == NULL)
	{
		if (lpUIWindow->GetBorder(&rcCurBorders) != S_OK)
		{
			lpUIWindow->SetBorderSpace (NULL);
			return;
		}
	}
	else
	{
		rcCurBorders = *lpRectBorder;
	}

	if (AfxGetThread()->m_pActiveWnd == m_pInPlaceFrame)
		OnShowControlBars(pMainFrame, TRUE);


	pDockManager->m_rectInPlace = rcCurBorders; 
	pDockManager->AdjustDockingLayout ();

	CRect rectClient = pDockManager->GetClientAreaBounds ();

	CRect rectRequest (
		abs (rectClient.left - rcCurBorders.left),
		abs (rectClient.top - rcCurBorders.top),
		abs (rectClient.right - rcCurBorders.right),
		abs (rectClient.bottom  - rcCurBorders.bottom)); // v.8.6 removed -1 to make it work in Excel

	CRect rectTemp;
	rectTemp = rectRequest;
	
	if (!rectRequest.IsRectNull() ||
		lpUIWindow->RequestBorderSpace(&rectTemp) == S_OK)
	{
		lpUIWindow->SetBorderSpace (&rectRequest);
		pDockManager->AdjustDockingLayout ();
	}
	else
	{
		OnShowControlBars (pMainFrame, FALSE);
		CRect rect (0,0,0,0);
		lpUIWindow->SetBorderSpace(&rect);
	}
}

void CBCGPOleServerDoc::OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow)
{
	COleServerDoc::OnShowControlBars (pFrameWnd, bShow);
	CBCGPOleCntrFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGPOleCntrFrameWnd, pFrameWnd);

	if (pMainFrame != NULL)
	{
		ASSERT_VALID (pMainFrame);
		CBCGPDockManager* pDockManager = pMainFrame->GetDockManager ();

		if (pDockManager != NULL)
		{
			ASSERT_VALID (pDockManager);
			pDockManager->ShowControlBars (bShow);
		}
	}
}

void CBCGPOleServerDoc::OnDocWindowActivate( BOOL bActivate )
{
	if (bActivate)
	{
		COleServerDoc::OnDocWindowActivate (bActivate);
		return;
	}

	CBCGPOleCntrFrameWnd* pMainFrame = NULL;

	CBCGPOleDocIPFrameWnd* p_IpDocFrame = 
						DYNAMIC_DOWNCAST (CBCGPOleDocIPFrameWnd, m_pInPlaceFrame);

	if (p_IpDocFrame != NULL)
	{
		pMainFrame =  p_IpDocFrame->GetContainerFrameWindow ();	
	}
	else
	{
		CBCGPOleIPFrameWnd* p_IpFrame = 
						DYNAMIC_DOWNCAST (CBCGPOleIPFrameWnd, m_pInPlaceFrame);

		if (p_IpFrame != NULL)
		{
			pMainFrame =  p_IpFrame->GetContainerFrameWindow ();
		}
	}

	if (pMainFrame == NULL)
	{
		return;
	}

	CBCGPDockManager* pDockManager = pMainFrame->GetDockManager ();
	ASSERT_VALID (pDockManager);

	pDockManager->ShowControlBars(bActivate);

	COleServerDoc::OnDocWindowActivate (bActivate);
}



