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
//
// BCGPVisualContainer.cpp: implementation of the CBCGPVisualContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPMath.h"
#include "BCGPVisualContainer.h"
#include "BCGPVisualCtrl.h"

#ifndef _BCGPCHART_STANDALONE
#include "BCGPVisualCollector.h"
#include "BCGPVisualConstructor.h"
#endif

#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
#include "BCGPPngImage.h"
#endif

#include "BCGPDrawManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

UINT BCGM_POSTREDRAW = ::RegisterWindowMessage (_T("BCGM_POSTREDRAW"));
UINT BCGM_CONTAINER_SELCHANGED = ::RegisterWindowMessage (_T("BCGM_CONTAINER_SELCHANGED"));

IMPLEMENT_DYNCREATE(CBCGPVisualContainer, CBCGPBaseAccessibleObject)
IMPLEMENT_DYNAMIC(CBCGPVisualDataObject, CObject)

CMap<UINT,UINT,CBCGPBaseVisualObject*,CBCGPBaseVisualObject*> CBCGPBaseVisualObject::m_mapAnimations;
CCriticalSection CBCGPBaseVisualObject::g_cs;

IMPLEMENT_DYNAMIC(CBCGPBaseVisualObject, CBCGPBaseAccessibleObject)

#define SEL_MARKER_SIZE 4

#ifdef _BCGSUITE_
class CBCGPToolBarImagesForSave : public CBCGPToolBarImages
{
public:
	CBCGPToolBarImagesForSave()
	{
		m_bUserImagesList = TRUE;
	}
};
#endif

static BOOL SaveBitmapToFile(HBITMAP hbmp, const CString& strFilePath)
{
	if (hbmp == NULL)
	{
		return FALSE;
	}

	TCHAR ext[_MAX_EXT];

#if _MSC_VER < 1400
	_tsplitpath (strFilePath, NULL, NULL, NULL, ext);
#else
	_tsplitpath_s (strFilePath, NULL, 0, NULL, 0, NULL, 0, ext, _MAX_EXT);
#endif

	CString strExt = ext;
	strExt.MakeUpper();

#if (!defined _BCGSUITE_) && (!defined _BCGPCHART_STANDALONE)
	if (strExt == _T(".PNG"))
	{
		CBCGPPngImage pngImage;
		pngImage.Attach(hbmp);

		return pngImage.SaveToFile(strFilePath);
	}
	else
#endif
	{
#ifdef _BCGSUITE_
		CBCGPToolBarImagesForSave img;
#else
		CBCGPToolBarImages img;
#endif
		img.AddImage(hbmp, TRUE);
		img.SetSingleImage();

		return img.Save(strFilePath);
	}
}

//////////////////////////////////////////////////////////////////////
// CBCGPBaseVisualObject

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPBaseVisualObject::CBCGPBaseVisualObject(CBCGPVisualContainer* pContainer)
{
	m_pParentContainer = NULL;

	if (pContainer != NULL)
	{
		ASSERT_VALID(pContainer);
		pContainer->Add(this);
	}
	else
	{
		m_pWndOwner = NULL;
	}

	m_nID = 0;
	m_dwUserData = 0;
	m_bIsVisible = TRUE;
	m_rect.SetRectEmpty();
	m_bIsAutoDestroy = TRUE;
	m_bIsDirty = TRUE;
	m_bCacheImage = FALSE;
	m_bIsSelected = FALSE;
	m_uiEditFlags = 0;
	m_bIsTracked = FALSE;
	m_sizeScaleRatio = CBCGPSize(1., 1.);
	m_rectOriginal = CBCGPRect(-1., -1., 0., 0.);
	m_nClickAndHoldID = 0;
	m_bDontResetRect = FALSE;
	m_bIsInteractiveMode = FALSE;

#if _MSC_VER < 1300
	m_pStdObject = NULL;
#endif
}
//*******************************************************************************
CBCGPBaseVisualObject::CBCGPBaseVisualObject(const CBCGPBaseVisualObject& src)
{
	m_pParentContainer = NULL;
	m_pWndOwner = NULL;

#if _MSC_VER < 1300
	m_pStdObject = NULL;
#endif

	CopyFrom(src);
}
//*******************************************************************************
CBCGPBaseVisualObject::~CBCGPBaseVisualObject()
{
	StopAllAnimations();
	RemoveAllData();

#if _MSC_VER < 1300
	if (m_pStdObject != NULL)
	{
		//force disconnect accessibility clients
		::CoDisconnectObject ((IAccessible*)m_pStdObject, NULL);
		m_pStdObject = NULL;
	}
#endif
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText = GetName();
		*pszName = strText.AllocSysString();

		return S_OK;
	}

	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		if (m_arData.GetSize() == 0)
		{
			return S_FALSE;
		}
		
		double dblVal = GetValue();
		if (dblVal == 0.0)
		{
			return S_FALSE;
		}
		
		CString str;
		str.Format(_T("%f"), dblVal);
		*pszValue = str.AllocSysString();
		
		return S_OK;
	}
	
	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accDescription(VARIANT /*varChild*/, BSTR* /*pszDescription*/)
{
	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (pvarRole == NULL)
	{
		return E_INVALIDARG;
	}
	
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = m_bIsInteractiveMode ? ROLE_SYSTEM_PUSHBUTTON : ROLE_SYSTEM_PANE;
		
		
		return S_OK;
	}
	
	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (pvarState == NULL)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		UINT nState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;

		if (!m_bIsInteractiveMode)
		{
			nState |= STATE_SYSTEM_READONLY;
		}

		if (IsSelected())
		{
			nState |= (STATE_SYSTEM_FOCUSED | STATE_SYSTEM_SELECTED);
		}
		
		pvarState->vt = VT_I4;
		pvarState->lVal = nState;

		return S_OK;
	}
	
	return S_FALSE;
}
//*******************************************************************************
HRESULT CBCGPBaseVisualObject::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
	if (pszDefaultAction == NULL)
	{
		return E_INVALIDARG;
	}
	
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF) && m_bIsInteractiveMode)
	{
		CString str = _T("Open");
		*pszDefaultAction = str.AllocSysString();
		return S_OK;
	}
	
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPBaseVisualObject::accHitTest(long  /*xLeft*/, long /*yTop*/, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}
	
	pvarChild->vt = VT_I4;
	pvarChild->lVal = CHILDID_SELF;
	
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPBaseVisualObject::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight || varChild.vt != VT_I4)
	{
		return E_INVALIDARG;
	}
	
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}
	
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rcProp = GetRect();
		m_pWndOwner->ClientToScreen(&rcProp);
		
		*pxLeft = rcProp.left;
		*pyTop = rcProp.top;
		*pcxWidth = rcProp.Width();
		*pcyHeight = rcProp.Height();
		
		return S_OK;
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPBaseVisualObject::get_accParent(IDispatch **ppdispParent)
{
    if (ppdispParent == NULL)
	{
		return E_INVALIDARG;
	}
	
	*ppdispParent = NULL;
	
	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		CWnd* pWndParent = m_pWndOwner->GetParent();
		if (pWndParent->GetSafeHwnd() != NULL)
		{
			return AccessibleObjectFromWindow(pWndParent->GetSafeHwnd(), (DWORD)OBJID_CLIENT,
				IID_IAccessible, (void**)ppdispParent);
		}
	}
	
	return S_FALSE;
}

#if _MSC_VER < 1300
IMPLEMENT_OLECREATE(CBCGPBaseVisualObject, "BCGPOleAcc.CBCGPBaseVisualObject", 0xf0345314, 0x6b7d, 0x4f03, 0xbf, 0xb8, 0xb5, 0xaa, 0x36, 0x81, 0x33, 0x8d);
#endif

void CBCGPBaseVisualObject::CopyFrom(const CBCGPBaseVisualObject& src)
{
	ASSERT_VALID(this);

	m_bIsDirty = TRUE;
	m_bIsSelected = FALSE;
	m_bIsTracked = FALSE;
	
	m_nID = src.m_nID;
	m_bIsInteractiveMode = src.m_bIsInteractiveMode;
	m_dwUserData = src.m_dwUserData;
	m_bIsVisible = src.m_bIsVisible;
	m_rect = src.m_rect;
	m_bIsAutoDestroy = src.m_bIsAutoDestroy;
	m_bCacheImage = src.m_bCacheImage;
	m_uiEditFlags = src.m_uiEditFlags;
	m_sizeScaleRatio = src.m_sizeScaleRatio;
	m_rectOriginal = src.m_rectOriginal;
	m_bDontResetRect = FALSE;
	
	RemoveAllData();

	for (int i = 0; i < src.m_arData.GetSize(); i++)
	{
		CBCGPVisualDataObject* pData = src.m_arData[i];
		ASSERT_VALID (pData);
		
		CBCGPVisualDataObject* pCopyData = pData->CreateCopy();
		ASSERT_VALID(pCopyData);
		
		AddData (pCopyData);
	}
}
//*******************************************************************************
int CBCGPBaseVisualObject::AddData(CBCGPVisualDataObject* pObject)
{
	if (pObject == NULL)
	{
		return -1;
	}

	pObject->SetParentVisual (this);
	return (int)m_arData.Add(pObject);
}
//*******************************************************************************
void CBCGPBaseVisualObject::RemoveAllData()
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPVisualDataObject* pData = m_arData[i];
		ASSERT_VALID(pData);
		
		delete pData;
	}

	m_arData.RemoveAll();
}
//*******************************************************************************
CWnd* CBCGPBaseVisualObject::SetOwner(CWnd* pWndOwner, BOOL bRedraw/* = TRUE*/)
{
	CWnd* pOldOwner = m_pWndOwner;

	SetDirty();

	if (m_pWndOwner != pWndOwner)
	{
		if (pWndOwner->GetSafeHwnd () == NULL)
		{
			bRedraw = FALSE;
		}

		m_pWndOwner = pWndOwner;

		StopAllAnimations(bRedraw);

		if (bRedraw)
		{
			Redraw ();
		}
	}

	return pOldOwner;
}
//*******************************************************************************
void CBCGPBaseVisualObject::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	CBCGPRect rectOld;

	if (m_rect != rect)
	{
		rectOld = m_rect;
		m_rect = rect;

		if (!m_bDontResetRect && m_sizeScaleRatio != CBCGPSize(1., 1.))
		{
			m_rectOriginal = CBCGPRect(-1., -1., 0., 0.);
		}

		SetDirty();
	}

	if (m_bIsSelected)
	{
		SetSelected();
	}

	if (bRedraw)
	{
		if (!rectOld.IsRectEmpty())
		{
			RedrawRect(rectOld);
		}

		Redraw();
	}
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::ResetRect()
{
	if (m_rectOriginal != CBCGPRect(-1., -1., 0., 0.))
	{
		SetRect(m_rectOriginal);
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::ResetOriginalRect()
{
	if (m_sizeScaleRatio == CBCGPSize(1., 1.))
	{
		return FALSE;
	}

	m_rectOriginal = CBCGPRect(-1., -1., 0., 0.);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::SetValue(double dblVal, int nIndex, UINT uiAnimationTime, BOOL bRedraw)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CBCGPVisualDataObject* pData = m_arData[nIndex];
	ASSERT_VALID(pData);

	const double dblRange = GetAnimationRange(nIndex);

	SetDirty();

	if (uiAnimationTime == 0 || dblRange == 0.)
	{
		pData->SetValue(dblVal);

		if (bRedraw)
		{
			Redraw();
		}

		return TRUE;
	}

	pData->SetAnimatedValue(pData->GetValue());
	pData->SetValue(dblVal);

	if (pData->GetAnimationID() > 0)
	{
		::KillTimer(NULL, pData->GetAnimationID());
	}

	pData->SetAnimationStep(fabs((pData->GetValue() - pData->GetAnimatedValue()) / 10));

	double dblSteps = fabs((dblRange) / (pData->GetValue() - pData->GetAnimatedValue()));

	pData->SetAnimationID ((UINT) ::SetTimer (NULL, 0, 
		(int)(.5 + (double)uiAnimationTime / dblSteps), AnimTimerProc));

	g_cs.Lock ();
	m_mapAnimations.SetAt (pData->GetAnimationID(), this);
	g_cs.Unlock ();

	return TRUE;
}
//*******************************************************************************
double CBCGPBaseVisualObject::GetValue(int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		ASSERT(FALSE);
		return 0.;
	}

	return m_arData[nIndex]->GetValue();
}
//*******************************************************************************
VOID CALLBACK CBCGPBaseVisualObject::AnimTimerProc (HWND /*hwnd*/, UINT /*uMsg*/,
													   UINT_PTR idEvent, DWORD /*dwTime*/)
{
	CBCGPBaseVisualObject* pObject = NULL;

	g_cs.Lock ();

	if (!m_mapAnimations.Lookup ((UINT) idEvent, pObject))
	{
		g_cs.Unlock ();
		return;
	}

	ASSERT_VALID(pObject);

	g_cs.Unlock ();

	BOOL bStopAnimation = pObject->OnAnimation((UINT)idEvent);

	if (bStopAnimation)
	{
		pObject->StopAnimation((UINT)idEvent, TRUE);
	}
	else
	{
		pObject->Redraw();
	}
}
//*******************************************************************************
CBCGPVisualDataObject* CBCGPBaseVisualObject::GetAnimated(UINT uiID) const
{
	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPVisualDataObject* pData = m_arData[i];
		ASSERT_VALID(pData);

		if (pData->GetAnimationID() == uiID)
		{
			return pData;
		}
	}

	return NULL;
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::OnAnimation(UINT idEvent)
{
	CBCGPVisualDataObject* pData = GetAnimated(idEvent);
	if (pData == NULL)
	{
		ASSERT(FALSE);
		return TRUE;
	}

	BOOL bStopAnimation = FALSE;

	if (pData->GetAnimatedValue() < pData->GetValue())
	{
		pData->SetAnimatedValue(pData->GetAnimatedValue() + pData->GetAnimationStep());
		bStopAnimation = pData->GetAnimatedValue() >= pData->GetValue();
	}
	else
	{
		pData->SetAnimatedValue(pData->GetAnimatedValue() - pData->GetAnimationStep());
		bStopAnimation = pData->GetAnimatedValue() <= pData->GetValue();
	}

	return bStopAnimation;
}
//*******************************************************************************
void CBCGPBaseVisualObject::StopAnimation(UINT id, BOOL bRedraw)
{
	ASSERT_VALID (this);

	if (id == 0)
	{
		return;
	}

	::KillTimer (NULL, id);

	g_cs.Lock ();
	m_mapAnimations.RemoveKey (id);
	g_cs.Unlock ();

	CBCGPVisualDataObject* pData = GetAnimated(id);
	if (pData == NULL)
	{
		ASSERT(FALSE);
	}
	else
	{
		pData->SetAnimationID(0);
		pData->SetAnimatedValue(0.);
		pData->SetAnimationStep(0.);
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::StopAllAnimations(BOOL bRedraw)
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arData.GetSize(); i++)
	{
		CBCGPVisualDataObject* pData = m_arData[i];
		ASSERT_VALID(pData);

		StopAnimation(pData->GetAnimationID(), bRedraw);
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::Redraw()
{
	ASSERT_VALID (this);

	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		if (m_pWndOwner->GetExStyle() & WS_EX_LAYERED)
		{
			CBCGPBaseVisualCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPBaseVisualCtrl, m_pWndOwner);
			if (pCtrl != NULL)
			{
				pCtrl->OnDrawLayeredPopup();
				return;
			}
		}

		m_pWndOwner->RedrawWindow();
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::RedrawRect(const CBCGPRect& rect)
{
	ASSERT_VALID (this);

	if (rect.IsRectEmpty() || m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	if (m_pWndOwner->GetExStyle() & WS_EX_LAYERED)
	{
		CBCGPBaseVisualCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPBaseVisualCtrl, m_pWndOwner);
		if (pCtrl != NULL)
		{
			pCtrl->OnDrawLayeredPopup();
			return;
		}
	}

	CRect rectGDI = rect;
	m_pWndOwner->RedrawWindow(rectGDI);
}
//*******************************************************************************
void CBCGPBaseVisualObject::Invalidate()
{
	ASSERT_VALID (this);
	InvalidateRect(m_rect);
}
//*******************************************************************************
void CBCGPBaseVisualObject::InvalidateRect(const CBCGPRect& rect)
{
	ASSERT_VALID (this);

	if (rect.IsRectEmpty() || m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	m_pWndOwner->InvalidateRect((CRect&)rect);
}
//*******************************************************************************
void CBCGPBaseVisualObject::UpdateWindow()
{
	ASSERT_VALID (this);
	
	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->UpdateWindow();
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::PostRedraw(BOOL bSetDirty)
{
	ASSERT_VALID (this);

	if (m_pWndOwner->GetSafeHwnd() != NULL && !m_rect.IsRectEmpty())
	{
		m_pWndOwner->PostMessage(BCGM_POSTREDRAW, (WPARAM)bSetDirty);
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::ResetGestureOptions()
{
	ASSERT_VALID (this);

	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		CBCGPGestureConfig gestureConfig;
		if (GetGestureConfig(gestureConfig))
		{
			bcgpGestureManager.SetGestureConfig(m_pWndOwner->GetSafeHwnd(), gestureConfig);
		}
	}
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::OnSetMouseCursor(const CBCGPPoint& pt)
{
	int nHitTest = HitTest(pt);

	switch (nHitTest)
	{
	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZENWSE));
		return TRUE;

	case HTTOP:
	case HTBOTTOM:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZENS));
		return TRUE;

	case HTRIGHT:
	case HTLEFT:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZEWE));
		return TRUE;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZENESW));
		return TRUE;

	case HTCAPTION:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZEALL));
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
HBITMAP CBCGPBaseVisualObject::ExportToBitmap(CBCGPGraphicsManager* pGM)
{
	ASSERT_VALID(pGM);

	const CBCGPSize size = m_rect.Size();

	HBITMAP hmbpDib = CBCGPDrawManager::CreateBitmap_32(size, NULL);
	if (hmbpDib == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPRect rect(CBCGPPoint(), size);

	CDC dcMem;
	dcMem.CreateCompatibleDC (NULL);

	HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject (hmbpDib);

	pGM->BindDC(&dcMem, rect);
	pGM->BeginDraw();

	CBCGPRect rectSaved = m_rect;

	SetRect(m_rect - rectSaved.TopLeft());
	SetDirty();

	BOOL bCacheImage = m_bCacheImage;
	m_bCacheImage = FALSE;

	pGM->FillRectangle(m_rect, CBCGPBrush(CBCGPColor::White));

	OnDraw(pGM, m_rect);

	pGM->EndDraw();

	dcMem.SelectObject (hbmpOld);

	CBCGPDrawManager::FillAlpha (rect, hmbpDib, 255);

	pGM->BindDC(NULL);

	SetRect(rectSaved);
	m_bCacheImage = bCacheImage;

	SetDirty();

	return hmbpDib;
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::ExportToImage(CBCGPGraphicsManager* pGMSrc, CBCGPImage& image, const CBCGPBrush& brFill)
{
	ASSERT_VALID(pGMSrc);

	CBCGPGraphicsManager* pGM = pGMSrc->CreateOffScreenManager(m_rect, &image);

	if (pGM == NULL)
	{
		return FALSE;
	}

	CBCGPRect rectSaved = m_rect;
	m_rect = m_rect - rectSaved.TopLeft();

	if (!brFill.IsEmpty())
	{
		pGM->FillRectangle(m_rect, brFill);
	}

	SetDirty();
	OnDraw(pGM, CBCGPRect());

	m_rect = rectSaved;
	delete pGM;

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::CopyToClipboard(CBCGPGraphicsManager* pGM)
{
	if (pGM == NULL)
	{
		return FALSE;
	}

	HBITMAP hbmp = ExportToBitmap(pGM);

	CBCGPToolBarImages img;
	img.AddImage(hbmp, TRUE);
	img.SetSingleImage();

	return img.CopyImageToClipboard(0);
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::ExportToFile(const CString& strFilePath, CBCGPGraphicsManager* pGM)
{
	if (pGM == NULL)
	{
		return FALSE;
	}

	return SaveBitmapToFile(ExportToBitmap(pGM), strFilePath);
}
//*******************************************************************************
void CBCGPBaseVisualObject::EnableImageCache(BOOL bEnable)
{
	m_bCacheImage = bEnable;
}
//*******************************************************************************
CBCGPRect CBCGPBaseVisualObject::MakeTrackMarker(double x, double y) const
{
	CBCGPRect rect(x, y, x, y);
	rect.InflateRect(SEL_MARKER_SIZE * m_sizeScaleRatio.cx, SEL_MARKER_SIZE * m_sizeScaleRatio.cy);

	return rect;
}
//*******************************************************************************
CBCGPRect CBCGPBaseVisualObject::MakeTrackMarker(const CBCGPPoint& pt) const
{
	return MakeTrackMarker(pt.x, pt.y);
}
//*******************************************************************************
void CBCGPBaseVisualObject::SetSelected(BOOL bSet)
{
	if (!IsEditMode())
	{
		ASSERT(FALSE);
		return;
	}

	if (!bSet)
	{
		m_rectTrack.SetRectEmpty();
		m_bIsSelected = FALSE;
		m_mapTrackRects.RemoveAll();
		return;
	}

	SetTrackingRect(m_rect);
	m_bIsSelected = TRUE;
}
//*******************************************************************************
void CBCGPBaseVisualObject::SetTrackingRect(const CBCGPRect& rect)
{
	const CBCGPSize sizeMin = GetMinSize();

	if (!sizeMin.IsEmpty())
	{
		if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
		{
			return;
		}
	}

	m_rectTrack = rect;

	if ((m_uiEditFlags & BCGP_EDIT_NOSIZE) == BCGP_EDIT_NOSIZE)
	{
		return;
	}

	m_mapTrackRects[HTTOPLEFT] = MakeTrackMarker(m_rectTrack.TopLeft());
	m_mapTrackRects[HTBOTTOMRIGHT] = MakeTrackMarker(m_rectTrack.BottomRight());
	m_mapTrackRects[HTTOPRIGHT] = MakeTrackMarker(m_rectTrack.right, m_rectTrack.top);
	m_mapTrackRects[HTBOTTOMLEFT] = MakeTrackMarker(m_rectTrack.left, m_rectTrack.bottom);

	m_mapTrackRects[HTLEFT] = MakeTrackMarker(m_rectTrack.left, m_rectTrack.CenterPoint().y);
	m_mapTrackRects[HTRIGHT] = MakeTrackMarker(m_rectTrack.right, m_rectTrack.CenterPoint().y);
	m_mapTrackRects[HTTOP] = MakeTrackMarker(m_rectTrack.CenterPoint().x, m_rectTrack.top);
	m_mapTrackRects[HTBOTTOM] = MakeTrackMarker(m_rectTrack.CenterPoint().x, m_rectTrack.bottom);
}
//*******************************************************************************
BOOL CBCGPBaseVisualObject::IsEditMode() const
{
	return m_pParentContainer != NULL && m_pParentContainer->IsEditMode();
}
//*******************************************************************************
int CBCGPBaseVisualObject::HitTest(const CBCGPPoint& pt) const
{
	if (m_bIsSelected)
	{
		for (POSITION pos = m_mapTrackRects.GetStartPosition (); pos != NULL;)
		{
			UINT uiHitTest = 0;
			CBCGPRect rect;

			m_mapTrackRects.GetNextAssoc (pos, uiHitTest, rect);
			rect.OffsetRect(-m_ptScrollOffset);

			if (rect.PtInRect(pt))
			{
				return uiHitTest;
			}
		}
	}

	CBCGPRect rect = m_rect;
	rect.Normalize();

	rect.OffsetRect(-m_ptScrollOffset);

	if (!rect.PtInRect(pt))
	{
		return HTNOWHERE;
	}

	if (IsEditMode() && (m_uiEditFlags & BCGP_EDIT_NOMOVE) == 0)
	{
		return HTCAPTION;
	}

	return  HTCLIENT;
}
//*******************************************************************************
void CBCGPBaseVisualObject::DrawTrackingRect(CBCGPGraphicsManager* pGM,
											 const CBCGPBrush& brOutline, const CBCGPBrush& brFill)
{
	if (m_rectTrack.IsRectEmpty())
	{
		return;
	}

	CBCGPRect rectTrack = m_rectTrack;
	rectTrack.OffsetRect(-m_ptScrollOffset);

	double scaleRatio = GetScaleRatioMid();
	pGM->DrawRectangle(rectTrack, brOutline, scaleRatio);

	for (POSITION pos = m_mapTrackRects.GetStartPosition (); pos != NULL;)
	{
		UINT uiHitTest = 0;
		CBCGPRect rect;

		m_mapTrackRects.GetNextAssoc (pos, uiHitTest, rect);

		rect.OffsetRect(-m_ptScrollOffset);

		switch (uiHitTest)
		{
		case HTTOPLEFT:
		case HTBOTTOMRIGHT:
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
			pGM->FillEllipse(rect, brFill);
			pGM->DrawEllipse(rect, brOutline, scaleRatio);
			break;

		default:
			pGM->FillRectangle(rect, brFill);
			pGM->DrawRectangle(rect, brOutline, scaleRatio);
		}
	}
}
//*******************************************************************************
void CBCGPBaseVisualObject::OnBeforeDrawScrolled (const CBCGPPoint& ptDrawOffset)
{
	m_ptDrawOffset = ptDrawOffset;
}
//*******************************************************************************
void CBCGPBaseVisualObject::OnAfterDrawScrolled ()
{
	m_ptDrawOffset = CBCGPPoint(0, 0);
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPBaseVisualObject::CreateCopy () const
{
	return NULL;
}
//*******************************************************************************
void CBCGPBaseVisualObject::SetClickAndHoldEvent(const CBCGPRect& rectBounds, UINT nEventID)
{
	m_rectClickAndHold = rectBounds;
	m_nClickAndHoldID = nEventID;
}

#ifndef _BCGPCHART_STANDALONE

//////////////////////////////////////////////////////////////////////
// CBCGPVisualLayout

void CBCGPVisualLayout::AdjustLayout ()
{
	const int count = (int)m_listWnd.GetCount ();
	if (count == 0)
	{
		return;
	}

	POSITION pos = m_listWnd.GetHeadPosition ();
	while (pos != NULL)
	{
		XWndItem& item = m_listWnd.GetNext (pos);

		int nIndex = m_container.FindIndex((const CBCGPBaseVisualObject*)item.m_Handle);
		if (nIndex >= 0)
		{
			CRect rectItem;
			UINT uiFlags = CalculateItem (item, rectItem);

			if ((uiFlags & (SWP_NOMOVE | SWP_NOSIZE)) != (SWP_NOMOVE | SWP_NOSIZE))
			{
				CBCGPBaseVisualObject* pObject = (CBCGPBaseVisualObject*)m_container[nIndex];
				pObject->SetRect(rectItem);
			}
		}
	}
}
//*******************************************************************************
CRect CBCGPVisualLayout::GetItemRect(XWndItem& item) const
{
	int nIndex = m_container.FindIndex((const CBCGPBaseVisualObject*)item.m_Handle);
	if (nIndex >= 0)
	{
		return m_container[nIndex]->GetRect();
	}

	return CRect (0, 0, 0, 0);
}
//********************************************************************************
BOOL CBCGPVisualLayout::AddAnchor(CBCGPBaseVisualObject* pObject, XMoveType typeMove, XSizeType typeSize, 
								  const CPoint& percMove, const CPoint& percSize)
{
	if (pObject == NULL || m_container.FindIndex(pObject) < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return CBCGPStaticLayout::AddAnchor ((LPVOID)pObject, typeMove, typeSize, percMove, percSize);
}
//********************************************************************************
void CBCGPVisualLayout::GetHostWndRect(CRect& rect) const
{
	rect = m_container.GetRect();
}

#endif

//////////////////////////////////////////////////////////////////////
// CBCGPVisualContainer

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGPVisualContainer::CBCGPVisualContainer(CWnd* pWndOwner)
{
#ifndef _BCGPCHART_STANDALONE
	m_pLayout = NULL;
#endif

	m_bDrawDynamicObjectsOnTop = TRUE;
	m_pWndOwner = pWndOwner;
	m_rect.SetRectEmpty();
	m_bCacheImage = FALSE;
	m_bIsEditMode = FALSE;
	m_bScrollBars = FALSE;
	m_dwEditFlags = 0;
	m_nDragMode = HTNOWHERE;
	m_bDrawScrollBars = TRUE;

	m_bHitTestSelected = FALSE;
	m_ptDragStart = m_ptDragFinish = CBCGPPoint(-1, -1);
	m_szDrag = CBCGPSize(::GetSystemMetrics(SM_CXDRAG), ::GetSystemMetrics(SM_CYDRAG));

	m_brSelFill = CBCGPBrush(CBCGPColor::LightBlue, .5);
	m_brSelOutline = CBCGPBrush(CBCGPColor::SteelBlue);
	m_brGrid = CBCGPBrush(CBCGPColor::WhiteSmoke);

	m_sizeGrid = CBCGPSize(10., 10.);

	m_pNewObject = NULL;
	m_bAddNewObjectMode = FALSE;

	m_GMType = (CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER)-1;

	m_sizeScaleRatio = CBCGPSize(1., 1.);

	m_nClickAndHoldID = 0;

	m_ScrollBarVert.SetHorizontal(FALSE);
	m_ScrollBarHorz.SetHorizontal(TRUE);

#if _MSC_VER < 1300
	m_pStdObject = NULL;
#endif
}
//*******************************************************************************
CBCGPVisualContainer::~CBCGPVisualContainer()
{
#ifndef _BCGPCHART_STANDALONE
	if (m_pLayout != NULL)
	{
		delete m_pLayout;
		m_pLayout = NULL;
	}
#endif

	RemoveAll();

#if _MSC_VER < 1300
	if (m_pStdObject != NULL)
	{
		//force disconnect accessibility clients
		::CoDisconnectObject ((IAccessible*)m_pStdObject, NULL);
		m_pStdObject = NULL;
	}
#endif
}

#if _MSC_VER < 1300
IMPLEMENT_OLECREATE(CBCGPVisualContainer, "BCGPOleAcc.CBCGPVisualContainer", 0xea74083, 0x4c72, 0x4a41, 0x86, 0xcc, 0x8b, 0x5a, 0xa7, 0xb8, 0x89, 0x8e);
#endif

CWnd* CBCGPVisualContainer::SetOwner(CWnd* pWndOwner, BOOL bRedraw/* = TRUE*/)
{
	CWnd* pOldOwner = m_pWndOwner;

	if (m_pWndOwner != pWndOwner)
	{
		m_pWndOwner = pWndOwner;

		for (int i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject != NULL)
			{
				ASSERT_VALID(pObject);		
				pObject->SetOwner(m_pWndOwner, FALSE);
			}
		}

		if (bRedraw && m_pWndOwner->GetSafeHwnd () != NULL)
		{
			Redraw ();
		}
	}

	return pOldOwner;
}
//*******************************************************************************
void CBCGPVisualContainer::EnableImageCache(BOOL bEnable)
{
	m_bCacheImage = bEnable;

	if (!m_bCacheImage)
	{
		m_ImageCache.Destroy();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::EnableScrollBars(BOOL bEnable, CBCGPVisualScrollBarColorTheme* pColorTheme)
{
	m_bScrollBars = bEnable;

	if (bEnable && pColorTheme != NULL)
	{
		m_ScrollBarVert.SetColorTheme(*pColorTheme);
		m_ScrollBarHorz.SetColorTheme(*pColorTheme);
	}

	AdjustLayout();
}
//*******************************************************************************
BOOL CBCGPVisualContainer::Add(CBCGPBaseVisualObject* pObject, BOOL bAutoDestroy)
{
	ASSERT_VALID(pObject);
	return InsertAt(pObject, -1, bAutoDestroy);
}
//*******************************************************************************
BOOL CBCGPVisualContainer::InsertAt(CBCGPBaseVisualObject* pObject, int nIndex, BOOL bAutoDestroy)
{
	ASSERT_VALID(pObject);
	ASSERT(pObject->m_pParentContainer == NULL);

	if (nIndex == -1)
	{
#ifndef _BCGPCHART_STANDALONE
		if (pObject->IsKindOf (RUNTIME_CLASS(CBCGPDiagramConnector)))
		{
			nIndex = 0;
		}
		else
#endif
		{
			nIndex = (int)m_arObjects.GetSize();
		}
	}

	if (nIndex < 0 || nIndex > m_arObjects.GetSize())
	{
		return FALSE;
	}

	if (FindIndex(pObject) < 0)
	{
		ASSERT(pObject->m_pParentContainer == NULL);
		m_arObjects.InsertAt(nIndex, pObject);
	}

	pObject->m_pParentContainer = this;
	pObject->m_pWndOwner = m_pWndOwner;
	pObject->m_bIsAutoDestroy = bAutoDestroy;
	pObject->SetScrollOffset(m_ptScrollOffset);

	OnAdd (pObject);

	AdjustLayout();
	return TRUE;
}
//*******************************************************************************
void CBCGPVisualContainer::OnAdd (CBCGPBaseVisualObject* /*pObject*/)
{
}
//*******************************************************************************
BOOL CBCGPVisualContainer::Remove(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arObjects.GetSize())
	{
		return FALSE;
	}

	CBCGPBaseVisualObject* pObject = m_arObjects[nIndex];
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		
		OnRemove (pObject);
		
		if (pObject->m_bIsAutoDestroy)
		{
			delete pObject;
		}
	}

	m_arObjects.RemoveAt(nIndex);
	AdjustLayout();
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnCopyObject(CBCGPBaseVisualObject* pObject, const CBCGPRect& rectNew)
{
	ASSERT_VALID(pObject);

	CBCGPBaseVisualObject* pNewVisualObject = pObject->CreateCopy();
	if (pNewVisualObject == NULL)
	{
		pNewVisualObject = DYNAMIC_DOWNCAST(CBCGPBaseVisualObject, pObject->GetRuntimeClass()->CreateObject());
		
		if (pNewVisualObject == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}

	pNewVisualObject->SetRect(rectNew);

	Add(pNewVisualObject, TRUE);
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::Remove(CBCGPBaseVisualObject* pObject)
{
	int nIndex = FindIndex(pObject);
	if (nIndex == -1)
	{
		return FALSE;
	}

	return Remove(nIndex);
}
//*******************************************************************************
void CBCGPVisualContainer::OnRemove (CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);
	
	POSITION pos = m_lstSel.Find(pObject);
	if (pos != NULL)
	{
		m_lstSel.RemoveAt(pos);
	}
}
//*******************************************************************************
void CBCGPVisualContainer::RemoveAll()
{
	m_lstSel.RemoveAll();

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->m_bIsAutoDestroy)
			{
				delete pObject;
			}
			else
			{
				pObject->m_pParentContainer = NULL;
			}

			m_arObjects[i] = NULL;
		}
	}

	m_arObjects.RemoveAll();
	AdjustLayout();
}
//*******************************************************************************
int CBCGPVisualContainer::GetCount() const
{
	return (int)m_arObjects.GetSize();
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPVisualContainer::GetAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arObjects.GetSize())
	{
		return NULL;
	}

	CBCGPBaseVisualObject* pObject = m_arObjects.GetAt(nIndex);

	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
	}

	return pObject;
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPVisualContainer::GetByID(UINT nID)
{
	for (int i = 0; i < (int)m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects.GetAt(i);
		ASSERT_VALID(pObject);

		if (pObject->m_nID == nID)
		{
			return pObject;
		}
	}

	return NULL;
}
//*******************************************************************************
const CBCGPBaseVisualObject* CBCGPVisualContainer::operator[](int nIndex) const
{
	if (nIndex < 0 || nIndex >= m_arObjects.GetSize())
	{
		return NULL;
	}

	const CBCGPBaseVisualObject* pObject = m_arObjects[nIndex];

	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
	}

	return pObject;
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPVisualContainer::GetFromPoint(const CPoint& point)
{
	if (m_ScrollBarVert.GetRect().PtInRect(point) ||
		m_ScrollBarHorz.GetRect().PtInRect(point) ||
		m_rectBottomRight.PtInRect(point))
	{
		return NULL;
	}

	if (m_bHitTestSelected)
	{
		for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
		{
			CBCGPBaseVisualObject* pObject = m_lstSel.GetNext(pos);
			ASSERT_VALID(pObject);

			if (pObject->HitTest(point) != HTNOWHERE)
			{
				return pObject;
			}
		}
	}

	int count = (int)m_arObjects.GetSize ();

	for (int i = 0; i < count; i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[count - i - 1];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		if (pObject->IsVisible())
		{
			CBCGPRect rect = pObject->m_rect;
			rect.Normalize();

			rect.OffsetRect(-m_ptScrollOffset);

			if (rect.PtInRect (point) && pObject->HitTest (point) != HTNOWHERE)
			{
				return pObject;
			}
		}
	}

	return NULL;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::MoveForward(CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);

	int nIndex = FindIndex(pObject);
	if (nIndex < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (nIndex == m_arObjects.GetSize() - 1)
	{
		return FALSE;
	}

	CBCGPBaseVisualObject* pObjectNext = m_arObjects[nIndex + 1];

	m_arObjects[nIndex + 1] = pObject;
	m_arObjects[nIndex] = pObjectNext;

	m_ImageCache.Destroy();
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::MoveBackward(CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);

	int nIndex = FindIndex(pObject);
	if (nIndex < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (nIndex == 0)
	{
		return FALSE;
	}

	CBCGPBaseVisualObject* pObjectPrev = m_arObjects[nIndex - 1];

	m_arObjects[nIndex - 1] = pObject;
	m_arObjects[nIndex] = pObjectPrev;

	m_ImageCache.Destroy();
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::MoveToFront(CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);

	int nIndex = FindIndex(pObject);
	if (nIndex < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (nIndex == m_arObjects.GetSize() - 1)
	{
		return FALSE;
	}

	for (int i = nIndex; i < m_arObjects.GetSize() - 1; i++)
	{
		m_arObjects[i] = m_arObjects[i + 1];
	}

	m_arObjects[m_arObjects.GetSize() - 1] = pObject;

	m_ImageCache.Destroy();
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::MoveToBack(CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);

	int nIndex = FindIndex(pObject);
	if (nIndex < 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (nIndex == 0)
	{
		return FALSE;
	}

	for (int i = nIndex; i > 0; i--)
	{
		m_arObjects[i] = m_arObjects[i - 1];
	}

	m_arObjects[0] = pObject;

	m_ImageCache.Destroy();
	return TRUE;
}
//*******************************************************************************
int CBCGPVisualContainer::FindIndex(const CBCGPBaseVisualObject* pObject) const
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pArObject = m_arObjects[i];
		if (pArObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pArObject);
		
		if (pArObject == pObject)
		{
			return i;
		}
	}

	return -1;
}
//*******************************************************************************
CBCGPRect CBCGPVisualContainer::GetBoundsRect(BOOL bOnlyVisible) const
{
	CBCGPRect rect(0, 0, 0, 0);

	if (bOnlyVisible)
	{
		for (int i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject != NULL)
			{
				ASSERT_VALID(pObject);
				
				if (pObject->IsVisible ())
				{
					rect.UnionRect(rect, pObject->m_rect);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject != NULL)
			{
				ASSERT_VALID(pObject);
				
				rect.UnionRect(rect, pObject->m_rect);
			}
		}
	}

	return rect;
}
//*******************************************************************************
void CBCGPVisualContainer::SetRect(const CBCGPRect& rect, BOOL bReposObjects, BOOL bRedraw)
{
	CBCGPRect rectOld = m_rect;

	m_rect = rect;

	if (m_rect == rectOld)
	{
		return;
	}

	if (bReposObjects)
	{
		double dx = m_rect.left;
		double dy = m_rect.top;

		if (!rectOld.IsRectEmpty())
		{
			dx -= rectOld.left;
			dy -= rectOld.top;
		}

		for (int i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject != NULL)
			{
				ASSERT_VALID(pObject);
				
				pObject->m_rect.OffsetRect(dx, dy);
			}
		}
	}

	AdjustLayout();

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::EnableEditMode(BOOL bEnable, DWORD dwFlags)
{
	m_bIsEditMode = bEnable;
	m_dwEditFlags = dwFlags;
}
//*******************************************************************************
void CBCGPVisualContainer::SetDirty(BOOL bSet) const
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			pObject->SetDirty(bSet);
		}
	}
}
//*******************************************************************************
BOOL CBCGPVisualContainer::IsDirty() const
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->IsDirty())
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPVisualContainer::Redraw()
{
	ASSERT_VALID (this);

	if (m_pWndOwner->GetSafeHwnd() == NULL || m_rect.IsRectEmpty())
	{
		return;
	}

	if (m_pWndOwner->GetExStyle() & WS_EX_LAYERED)
	{
		CBCGPBaseVisualCtrl* pCtrl = DYNAMIC_DOWNCAST(CBCGPBaseVisualCtrl, m_pWndOwner);
		if (pCtrl != NULL)
		{
			pCtrl->OnDrawLayeredPopup();
			return;
		}
	}

	m_pWndOwner->RedrawWindow();
}
//*******************************************************************************
void CBCGPVisualContainer::DrawObjects(CBCGPGraphicsManager* pGM, CBCGPRect rectClip, DWORD dwFlags, const CBCGPPoint& ptOffset)
{
	CBCGPRect rectInter;

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		BOOL bCacheImage = pObject->m_bCacheImage;
		if (m_bCacheImage || !m_bDrawDynamicObjectsOnTop)
		{
			pObject->m_bCacheImage = FALSE;
		}

		CBCGPRect rectSaved = pObject->GetRect();

		if (ptOffset != CBCGPPoint(0, 0))
		{
			pObject->m_rect.OffsetRect(-ptOffset);
		}

		CBCGPRect rectObj = pObject->GetRect().NormalizedRect();

		if (!rectObj.IsRectEmpty() && pObject->IsVisible() &&
			rectInter.IntersectRect (rectObj, rectClip))
		{
			pObject->OnBeforeDrawScrolled (-ptOffset);
			pObject->OnDraw(pGM, rectClip, dwFlags);
			pObject->OnAfterDrawScrolled ();
			pObject->SetDirty(FALSE);
		}

		pObject->m_bCacheImage = bCacheImage;

		if (ptOffset != CBCGPPoint(0, 0))
		{
			pObject->m_rect = rectSaved;
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::DrawTrackingRects(CBCGPGraphicsManager* pGM)
{
	if (!m_bIsEditMode)
	{
		return;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			pObject->DrawTrackingRect(pGM, m_brSelOutline, m_brSelFill);
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnDraw(CBCGPGraphicsManager* pGM, const CBCGPRect& rectClip)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pGM);

	if (m_bIsEditMode)
	{
		m_ImageCache.Destroy();
	}

	DWORD dwDrawFlags = m_bDrawDynamicObjectsOnTop ? BCGP_DRAW_STATIC : (BCGP_DRAW_STATIC | BCGP_DRAW_DYNAMIC);

	if (m_bCacheImage && !pGM->IsOffscreen() && !m_bIsEditMode)
	{	
		if (m_ImageCache.GetHandle() == NULL || IsDirty())
		{
			m_ImageCache.Destroy();

			CBCGPRect rectBounds = GetBoundsRect(TRUE);
			
			rectBounds.InflateRect(1., 1.);
			rectBounds.left = m_rect.left;
			rectBounds.top = m_rect.top;
			rectBounds.right = max(m_rect.right, rectBounds.right);
			rectBounds.bottom = max(m_rect.bottom, rectBounds.bottom);

			CBCGPGraphicsManager* pGMMem = pGM->CreateOffScreenManager(rectBounds, &m_ImageCache);
			if (pGMMem != NULL)
			{
				CBCGPRect rectSaved = m_rect;
				m_rect = m_rect - m_rect.TopLeft();

				OnFillBackground(pGMMem);
				DrawObjects(pGMMem, m_rect, dwDrawFlags, rectSaved.TopLeft());
				OnDrawBorder(pGMMem);

				delete pGMMem;

				m_rect = rectSaved;
			}
		}
	}

	BOOL bCached = FALSE;
	CBCGPRect rectSel;
	BOOL bIsSemitransp = pGM->IsSupported(BCGP_GRAPHICS_MANAGER_COLOR_OPACITY);

	if (m_ImageCache.GetHandle() != NULL)
	{
		pGM->DrawImage(m_ImageCache, m_rect.TopLeft());
		bCached = TRUE;
	}
	else
	{
		if (m_nDragMode == HTNOWHERE && m_ptDragStart != CBCGPPoint(-1, -1) && m_pNewObject == NULL)
		{
			rectSel = CBCGPRect(m_ptDragStart, CBCGPSize(m_ptDragFinish.x - m_ptDragStart.x, m_ptDragFinish.y - m_ptDragStart.y));
		}

		OnFillBackground(pGM);

		if (!bIsSemitransp && !rectSel.IsRectEmpty())
		{
			DrawSelectedArea(pGM, rectSel);
		}

		DrawObjects(pGM, rectClip, dwDrawFlags, m_ptScrollOffset);
	}

	if (m_bDrawDynamicObjectsOnTop)
	{
		DrawObjects(pGM, rectClip, BCGP_DRAW_DYNAMIC, m_ptScrollOffset);
	}

	if (m_bDrawScrollBars)
	{
		if (!bCached)
		{
			DrawTrackingRects(pGM);

			if (bIsSemitransp && !rectSel.IsRectEmpty())
			{
				DrawSelectedArea(pGM, rectSel);
			}

			if (m_pNewObject != NULL)
			{
				ASSERT_VALID(m_pNewObject);
				m_pNewObject->OnDraw(pGM, rectClip);
			}

			OnDrawBorder(pGM);
		}

		CBCGPRect rectInter;

		if (!m_rectBottomRight.IsRectEmpty() && !rectClip.IsRectEmpty() && rectInter.IntersectRect (m_rectBottomRight, rectClip))
		{
			pGM->FillRectangle(m_rectBottomRight, m_ScrollBarVert.GetColorTheme().m_brFace);
		}

		if (!rectClip.IsRectEmpty() && rectInter.IntersectRect (m_ScrollBarVert.GetRect(), rectClip))
		{
			m_ScrollBarVert.DoDraw(pGM);
		}

		if (!rectClip.IsRectEmpty() && rectInter.IntersectRect (m_ScrollBarHorz.GetRect(), rectClip))
		{
			m_ScrollBarHorz.DoDraw(pGM);
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnFillBackground(CBCGPGraphicsManager* pGM)
{
	if (!m_brFill.IsEmpty())
	{
		pGM->FillRectangle(m_rect, m_brFill);
	}

	if (m_bIsEditMode && (m_dwEditFlags & BCGP_CONTAINER_GRID) == BCGP_CONTAINER_GRID)
	{
		OnDrawGrid(pGM);
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnDrawBorder(CBCGPGraphicsManager* pGM)
{
	if (!m_brOutline.IsEmpty())
	{
		CBCGPRect rect(m_rect);
		if (pGM->GetType() != CBCGPGraphicsManager::BCGP_GRAPHICS_MANAGER_GDI)
		{
			rect.right -= 1;
			rect.bottom -= 1;
		}

		pGM->DrawRectangle(rect, m_brOutline, 1);
	}
}
//*******************************************************************************
void CBCGPVisualContainer::DrawSelectedArea(CBCGPGraphicsManager* pGM, const CBCGPRect& rectSel)
{
	pGM->FillRectangle(rectSel, m_brSelFill);
	pGM->DrawRectangle(rectSel, m_brSelOutline, 1);
}
//*******************************************************************************
void CBCGPVisualContainer::OnDrawGrid(CBCGPGraphicsManager* pGM)
{
	if (m_brGrid.IsEmpty())
	{
		return;
	}

	double scaleRatio = GetScaleRatioMid();
	CBCGPSize sizeGrid(m_sizeGrid.cx * m_sizeScaleRatio.cx, m_sizeGrid.cy * m_sizeScaleRatio.cy);

	for (double x = m_rect.left + sizeGrid.cx; x < m_rect.right; x += sizeGrid.cx)
	{
		pGM->DrawLine(x, m_rect.top, x, m_rect.bottom, m_brGrid, scaleRatio);
	}

	for (double y = m_rect.top + sizeGrid.cy; y < m_rect.bottom; y += sizeGrid.cy)
	{
		pGM->DrawLine(m_rect.left, y, m_rect.right, y, m_brGrid, scaleRatio);
	}
}
//*******************************************************************************
#ifndef _BCGPCHART_STANDALONE

BOOL CBCGPVisualContainer::LoadFromXML (UINT uiXMLResID)
{
	ASSERT_VALID (this);
	return LoadFromXML (MAKEINTRESOURCE (uiXMLResID));
}
//*******************************************************************************
BOOL CBCGPVisualContainer::LoadFromXML (LPCTSTR lpszXMLResID)
{
	CBCGPVisualInfo info;
	CBCGPVisualInfoLoader loader (info);

	if (!loader.Load (lpszXMLResID))
	{
		TRACE0("Cannot load dashboard from buffer\n");
		return FALSE;
	}

	CBCGPVisualConstructor constr (info);
	constr.Construct (*this);

	m_ImageCache.Destroy();

	SetDirty (TRUE);
	AdjustLayout ();
	Redraw ();

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::LoadFromFile (LPCTSTR lpszFileName)
{
	CBCGPVisualInfo info;
	CBCGPVisualInfoLoader loader (info);

	if (!loader.LoadFromFile (lpszFileName))
	{
		TRACE0("Cannot load dashboard from buffer\n");
		return FALSE;
	}

	CBCGPVisualConstructor constr (info);
	constr.Construct (*this);

	m_ImageCache.Destroy();

	SetDirty (TRUE);
	AdjustLayout ();
	Redraw ();

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::LoadFromBuffer (LPCTSTR lpszXMLBuffer)
{
	ASSERT_VALID (this);
	ASSERT (lpszXMLBuffer != NULL);

	CBCGPVisualInfo info;
	CBCGPVisualInfoLoader loader (info);

	if (!loader.LoadFromBuffer (lpszXMLBuffer))
	{
		TRACE0("Cannot load ribbon from buffer\n");
		return FALSE;
	}

	CBCGPVisualConstructor constr (info);
	constr.Construct (*this);

	m_ImageCache.Destroy();

	SetDirty (TRUE);
	AdjustLayout ();
	Redraw ();

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::SaveToXML (const CString& strFileName)
{
	CBCGPVisualInfo info;

	CBCGPVisualCollector collector (info);
	collector.Collect (*this);

	CBCGPVisualInfoWriter writer (info);
	return writer.Save (strFileName);
}
//*********************************************************************************
void CBCGPVisualContainer::EnableLayout(BOOL bEnable)
{
	if (m_pLayout != NULL)
	{
		delete m_pLayout;
		m_pLayout = NULL;
	}

	if (!bEnable)
	{
		return;
	}

	m_pLayout = new CBCGPVisualLayout(*this);
}

#endif

void CBCGPVisualContainer::AdjustLayout()
{
#ifndef _BCGPCHART_STANDALONE
	m_ImageCache.Destroy();

	if (m_pLayout == NULL)
	{
		AdjustScrollBars();
		return;
	}

	ASSERT_VALID(m_pLayout);
	m_pLayout->AdjustLayout();
#endif
	
	AdjustScrollBars();
}
//*******************************************************************************
void CBCGPVisualContainer::AdjustScrollBars()
{
	m_ScrollBarVert.SetParentVisualContainer(this);
	m_ScrollBarHorz.SetParentVisualContainer(this);

	double xOffset = m_ScrollBarHorz.GetOffset();
	double yOffset = m_ScrollBarVert.GetOffset();

	m_rectBottomRight.SetRectEmpty();

	m_ScrollBarVert.Reset();
	m_ScrollBarHorz.Reset();

	m_sizeScrollTotal = CBCGPSize(0, 0);

	CBCGPPoint ptScrollOffsetOld = m_ptScrollOffset;

	double dblMaxX = 0.;
	double dblMaxY = 0.;

	CBCGPRect rectScrollVert;
	CBCGPRect rectScrollHorz;

	int i = 0;

	if (m_bScrollBars)
	{
		for (i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject == NULL)
			{
				continue;
			}

			ASSERT_VALID(pObject);

			dblMaxX = max(dblMaxX, pObject->GetRect().right);
			dblMaxY = max(dblMaxY, pObject->GetRect().bottom);
		}
	}

	const int cxScrollBar = ::GetSystemMetrics(SM_CXVSCROLL);
	const int cyScrollBar = ::GetSystemMetrics(SM_CYHSCROLL);

	if (dblMaxX >= m_rect.right - cxScrollBar)
	{
		m_sizeScrollTotal.cx = dblMaxX;

		rectScrollHorz = m_rect;
		rectScrollHorz.DeflateRect(1, 1);

		rectScrollHorz.top = rectScrollHorz.bottom - cyScrollBar;

		if (dblMaxY >= m_rect.bottom - cyScrollBar)
		{
			rectScrollHorz.right -= cxScrollBar;
		}
	}

	if (dblMaxY >= m_rect.bottom - cyScrollBar)
	{
		m_sizeScrollTotal.cy = dblMaxY;

		rectScrollVert = m_rect;
		rectScrollVert.DeflateRect(1, 1);

		rectScrollVert.left = rectScrollVert.right - cxScrollBar;

		if (dblMaxX >= m_rect.right - cxScrollBar)
		{
			rectScrollVert.bottom -= cyScrollBar;
		}
	}

	m_ScrollBarHorz.SetTotal(m_sizeScrollTotal.cx);
	m_ScrollBarVert.SetTotal(m_sizeScrollTotal.cy);

	m_ptScrollOffset.x = max(0, min(m_ptScrollOffset.x, m_sizeScrollTotal.cx));
	m_ptScrollOffset.y = max(0, min(m_ptScrollOffset.y, m_sizeScrollTotal.cy));

	if (ptScrollOffsetOld != m_ptScrollOffset)
	{
		for (i = 0; i < m_arObjects.GetSize(); i++)
		{
			CBCGPBaseVisualObject* pObject = m_arObjects[i];
			if (pObject != NULL)
			{
				ASSERT_VALID(pObject);
				pObject->SetScrollOffset(m_ptScrollOffset);
			}
		}
	}

	if (m_bScrollBars)
	{
		m_ScrollBarHorz.SetOffset(bcg_clamp(xOffset, 0.0, m_ScrollBarHorz.GetTotal()));
		m_ScrollBarVert.SetOffset(bcg_clamp(yOffset, 0.0, m_ScrollBarVert.GetTotal()));

		m_ScrollBarHorz.SetRect(rectScrollHorz);
		m_ScrollBarVert.SetRect(rectScrollVert);

		if (!m_ScrollBarVert.GetRect().IsRectEmpty() && !m_ScrollBarHorz.GetRect().IsRectEmpty())
		{
			m_rectBottomRight = m_rect;
			m_rectBottomRight.DeflateRect(1, 1);

			m_rectBottomRight.top = m_rectBottomRight.bottom - m_ScrollBarHorz.GetRect().Height() - 1;
			m_rectBottomRight.left = m_rectBottomRight.right - m_ScrollBarVert.GetRect().Width() - 1;
		}

		SetDirty();
		Redraw();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::SetOutlineBrush(const CBCGPBrush& br)
{
	m_brOutline = br;
}
//*******************************************************************************
void CBCGPVisualContainer::SetFillBrush(const CBCGPBrush& br)
{
	m_brFill = br;
}
//*******************************************************************************
void CBCGPVisualContainer::SetGridBrush(const CBCGPBrush& br)
{
	m_brGrid = br;
}
//*******************************************************************************
void CBCGPVisualContainer::SetDrawDynamicObjectsOnTop(BOOL bSet)
{
	m_bDrawDynamicObjectsOnTop = bSet;
	SetDirty();
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnMouseDown(nButton, pt))
		{
			return TRUE;
		}
	}

	if (m_ScrollBarVert.OnMouseDown(pt) || m_ScrollBarHorz.OnMouseDown(pt))
	{
		return TRUE;
	}

	BOOL bSelChanged = FALSE;

	if (m_bIsEditMode)
	{
		if (nButton == 0)
		{
			m_nDragMode = HTNOWHERE;

			if (pObject != NULL && !m_bAddNewObjectMode)
			{
				const BOOL bCtrl = ::GetAsyncKeyState (VK_CONTROL) & 0x8000;

				BOOL bWasSelected = pObject->IsSelected ();

				if ((m_dwEditFlags & BCGP_CONTAINER_ENABLE_COPY) == BCGP_CONTAINER_ENABLE_COPY && bWasSelected)
				{
					// Don't unselect selected object
				}
				else
				{
					Select(pObject, bCtrl);
					bSelChanged = bWasSelected != pObject->IsSelected ();

					pObject->Redraw();
				}

				m_nDragMode = pObject->HitTest(pt);

			}
			else
			{
				ClearSelection();

				if (IsSingleSel() && !m_bAddNewObjectMode)
				{
					Redraw();
					FireSelectionChangedEvent();
					return FALSE;
				}
			}

			m_ptDragStart = m_ptDragFinish = pt;
			
			if (m_bAddNewObjectMode)
			{
				m_pNewObject = OnStartAddNewObject(pt);

				if (m_pNewObject != NULL)
				{
					m_pNewObject->SetRect(CBCGPRect(pt, CBCGPSize(1., 1.)));
				}
			}

			Redraw();

			if (bSelChanged)
			{
				FireSelectionChangedEvent();
			}

			return TRUE;
		}
		else if (nButton == 1 && pObject == NULL)
		{
			ClearSelection();
			FireSelectionChangedEvent();
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGetToolTip(const CBCGPPoint& pt, CString& strToolTip, CString& strDescr)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);

		if (pObject->OnGetToolTip(pt, strToolTip, strDescr))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPVisualContainer::Select(CBCGPBaseVisualObject* pObject, BOOL bAddToSel)
{
	ASSERT_VALID(pObject);

	if (!pObject->IsSelected())
	{
		if (!bAddToSel || IsSingleSel())
		{
			ClearSelection();
		}

		pObject->SetSelected();
		m_lstSel.AddTail(pObject);
	}
	else if (bAddToSel)
	{
		pObject->SetSelected(FALSE);

		POSITION pos = m_lstSel.Find(pObject);
		if (pos != NULL)
		{
			m_lstSel.RemoveAt(pos);
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::SelectAll()
{
	if (!m_bIsEditMode || IsSingleSel ())
	{
		TRACE(_T("Container is not in edit mode or multi selection disabled.\n"));
		ASSERT(FALSE);
		return;
	}

	BOOL bSelChanged = FALSE;
	const int count = GetCount ();
	for (int i = 0; i < count; i++)
	{
		CBCGPBaseVisualObject* pObject = GetAt(i);
		if (!pObject->IsSelected ())
		{
			Select (pObject, TRUE);
			bSelChanged = TRUE;
		}
	}

	if (bSelChanged)
	{
		Redraw ();
		FireSelectionChangedEvent();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::ClearSelection(BOOL bRedraw)
{
	if (!m_bIsEditMode)
	{
		return;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);

			BOOL bRedrawObject = pObject->IsSelected();
			
			pObject->SetSelected(FALSE);

			if (bRedrawObject)
			{
				CBCGPRect rectRedraw = pObject->GetRect();
				if (!rectRedraw.IsRectEmpty())
				{
					rectRedraw.InflateRect(SEL_MARKER_SIZE, SEL_MARKER_SIZE);
					pObject->InvalidateRect(rectRedraw);
				}
			}
		}
	}

	m_lstSel.RemoveAll();

	if (bRedraw && m_pWndOwner->GetSafeHwnd() != NULL)
	{
		m_pWndOwner->UpdateWindow();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::SetHitTestSelected(BOOL bSet)
{
	m_bHitTestSelected = bSet;
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPVisualContainer::GetSel(int nIndex) const
{
	if (m_lstSel.IsEmpty())
	{
		return NULL;
	}

	POSITION pos = m_lstSel.FindIndex(nIndex);
	if (pos == NULL)
	{
		return NULL;
	}

	return m_lstSel.GetAt(pos);
}
//*******************************************************************************
void CBCGPVisualContainer::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	m_bHitTestSelected = FALSE;

	m_ScrollBarVert.OnCancelMode();
	m_ScrollBarHorz.OnCancelMode();

	int i = 0;

	for (i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->IsCaptured())
			{
				pObject->OnMouseUp(nButton, pt);
				return;
			}
		}
	}

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		pObject->OnMouseUp(nButton, pt);
	}

	if (m_ptDragStart == CBCGPPoint(-1, -1))
	{
		return;	
	}

	BOOL bSelChanged = FALSE;

	if (m_nDragMode == HTNOWHERE)
	{
		CBCGPRect rectSel(m_ptDragStart, CBCGPSize(m_ptDragFinish.x - m_ptDragStart.x, m_ptDragFinish.y - m_ptDragStart.y));

		rectSel.OffsetRect(m_ptScrollOffset);

		if (m_pNewObject != NULL)
		{
			if ((m_pNewObject->GetEditFlags() & BCGP_EDIT_NO_NORMALIZE_RECT) == 0)
			{
				rectSel.Normalize();
			}

			if (OnFinishAddNewObject(m_pNewObject, rectSel))
			{
				m_pNewObject->SetRect(rectSel);
				Add(m_pNewObject);

				Select(m_pNewObject);
				bSelChanged = TRUE;
			}
			else
			{
				delete m_pNewObject;
			}

			m_pNewObject = NULL;
			m_bAddNewObjectMode = FALSE;
		}
		else
		{
			rectSel.Normalize();

			for (i = 0; i < m_arObjects.GetSize(); i++)
			{
				CBCGPBaseVisualObject* pObject = m_arObjects[i];
				if (pObject != NULL)
				{
					ASSERT_VALID(pObject);
					
					CBCGPRect rectInter;
					
					if (rectInter.IntersectRect(pObject->GetRect().NormalizedRect(), rectSel))
					{
						Select(pObject, TRUE);
						bSelChanged = TRUE;
					}
				}
			}
		}
	}
	else
	{
		for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
		{
			CBCGPBaseVisualObject* pObject = m_lstSel.GetNext(pos);
			ASSERT_VALID(pObject);

			if (!pObject->m_rectTrack.IsRectEmpty())
			{
				CBCGPRect rect = pObject->m_rectTrack;

				if ((pObject->GetEditFlags() & BCGP_EDIT_NO_NORMALIZE_RECT) == 0)
				{
					rect.Normalize();
				}

				if (pObject->GetRect() != rect)
				{
					CBCGPRect rectArea = m_rect;
					rectArea.Scale (m_sizeScaleRatio);

					if (rect.bottom <= rectArea.top)
					{
						rect.OffsetRect(0, rectArea.top - rect.bottom + 2);
					}

					if (rect.right <= rectArea.left)
					{
						rect.OffsetRect(rectArea.left - rect.right + 2, 0);
					}

					BOOL bCtrlIsPressed = GetAsyncKeyState (VK_CONTROL);

					if (bCtrlIsPressed && (m_dwEditFlags & BCGP_CONTAINER_ENABLE_COPY) == BCGP_CONTAINER_ENABLE_COPY)
					{
						OnCopyObject(pObject, rect);
					}
					else
					{
						pObject->SetRect(rect);
					}
				}
			}
		}

		AdjustScrollBars();
	}

	m_ptDragStart = m_ptDragFinish = CBCGPPoint(-1, -1);

	Redraw();

	if (bSelChanged)
	{
		FireSelectionChangedEvent();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnMouseMove(const CBCGPPoint& pt)
{
	if (m_ScrollBarHorz.OnDragThumb(pt) || m_ScrollBarVert.OnDragThumb(pt))
	{
		return;
	}

	if (m_bAddNewObjectMode && m_ptDragStart == CBCGPPoint(-1, -1))
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
		return;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pArObject = m_arObjects[i];
		if (pArObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pArObject);

		if (pArObject->IsCaptured())
		{
			pArObject->OnMouseMove(pt);
			return;
		}

		if (pArObject->m_bIsTracked)
		{
			CBCGPRect rect = pArObject->m_rect;
			rect.Normalize();

			if (!rect.PtInRect(pt))
			{
				pArObject->m_bIsTracked = FALSE;
				pArObject->OnMouseLeave();
			}
		}
	}

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);

		pObject->OnMouseMove(pt);
		pObject->m_bIsTracked = TRUE;
	}

	if (m_ptDragStart == CBCGPPoint(-1, -1))
	{
		return;	
	}

	if (m_nDragMode != HTNOWHERE)
	{
		MoveTrackingRects (pt);

		m_ptDragStart = pt;
	}
	else if (m_pNewObject != NULL)
	{
		ASSERT_VALID(m_pNewObject);

		CBCGPRect rect(m_ptDragStart, CBCGPSize(m_ptDragFinish.x - m_ptDragStart.x, m_ptDragFinish.y - m_ptDragStart.y));
		if ((m_pNewObject->GetEditFlags() & BCGP_EDIT_NO_NORMALIZE_RECT) == 0)
		{
			rect.Normalize();
		}

		m_pNewObject->SetRect(rect);
	}

	m_ptDragFinish = pt;

	if (m_nDragMode != HTCAPTION)
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
	}

	Redraw();
}
//*******************************************************************************
void CBCGPVisualContainer::MoveTrackingRects (CBCGPPoint pt)
{
	if (m_nDragMode == HTNOWHERE)
	{
		return;
	}

	if (m_ptDragStart == CBCGPPoint(-1, -1))
	{
		return;	
	}

	CBCGPPoint ptOffset = pt - m_ptDragStart;

	for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
	{
		CBCGPBaseVisualObject* pObject = m_lstSel.GetNext(pos);
		ASSERT_VALID(pObject);

		CBCGPRect rect = pObject->m_rectTrack;

		BOOL bLockAspRatio = (pObject->GetEditFlags() & BCGP_EDIT_SIZE_LOCK_ASPECT_RATIO) == BCGP_EDIT_SIZE_LOCK_ASPECT_RATIO;

		if (bLockAspRatio && m_nDragMode != HTCAPTION)
		{
			if (m_nDragMode == HTTOPLEFT || m_nDragMode == HTBOTTOMRIGHT)
			{
				if (fabs(ptOffset.x) > fabs(ptOffset.y))
				{
					ptOffset.y = ptOffset.x;
				}
				else
				{
					ptOffset.x = ptOffset.y;
				}
			}
			else if (m_nDragMode == HTTOPRIGHT || m_nDragMode == HTBOTTOMLEFT)
			{
				if (fabs(ptOffset.x) < fabs(ptOffset.y))
				{
					ptOffset.y = -ptOffset.x;
				}
				else
				{
					ptOffset.x = -ptOffset.y;
				}
			}
		}

		switch (m_nDragMode)
		{
		case HTCAPTION:
			rect += ptOffset;
			break;

		case HTLEFT:
			rect.left += ptOffset.x;
			if (bLockAspRatio)
			{
				rect.bottom -= ptOffset.x;
			}
			break;

		case HTRIGHT:
			rect.right += ptOffset.x;
			if (bLockAspRatio)
			{
				rect.bottom += ptOffset.x;
			}
			break;

		case HTTOP:
			rect.top += ptOffset.y;
			if (bLockAspRatio)
			{
				rect.right -= ptOffset.y;
			}
			break;

		case HTBOTTOM:
			rect.bottom += ptOffset.y;
			if (bLockAspRatio)
			{
				rect.right += ptOffset.y;
			}
			break;

		case HTTOPLEFT:
			rect.left += ptOffset.x;
			rect.top += ptOffset.y;
			break;

		case HTBOTTOMLEFT:
			rect.left += ptOffset.x;
			rect.bottom += ptOffset.y;
			break;

		case HTTOPRIGHT:
			rect.right += ptOffset.x;
			rect.top += ptOffset.y;
			break;

		case HTBOTTOMRIGHT:
			rect.right += ptOffset.x;
			rect.bottom += ptOffset.y;
			break;
		}

		pObject->SetTrackingRect(rect);
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnMouseLeave()
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->m_bIsTracked)
			{
				pObject->m_bIsTracked = FALSE;
				pObject->OnMouseLeave();
			}
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnCancelMode()
{
	m_bHitTestSelected = FALSE;

	m_ScrollBarVert.OnCancelMode();
	m_ScrollBarHorz.OnCancelMode();

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			pObject->OnCancelMode();
		}
	}
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnSetMouseCursor(const CBCGPPoint& pt)
{
	if (m_bAddNewObjectMode)
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
		return TRUE;
	}

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		return pObject->OnSetMouseCursor(pt);
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnMouseWheel(const CBCGPPoint& pt, short zDelta)
{
	if (m_bScrollBars)
	{
		if (!m_ScrollBarVert.GetRect().IsRectEmpty())
		{
			const int nSteps = abs(zDelta) / WHEEL_DELTA;
			for (int i = 0; i < nSteps; i++)
			{
				m_ScrollBarVert.OnScrollStep(zDelta < 0);
			}

			return TRUE;
		}

		if (!m_ScrollBarHorz.GetRect().IsRectEmpty())
		{
			const int nSteps = abs(zDelta) / WHEEL_DELTA;
			for (int i = 0; i < nSteps; i++)
			{
				m_ScrollBarHorz.OnScrollStep(zDelta < 0);
			}

			return TRUE;
		}
	}

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		return pObject->OnMouseWheel(pt, zDelta);
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (IsEditMode ())
	{
		if (m_ptDragStart != CBCGPPoint(-1, -1) && m_nDragMode != HTNOWHERE)
		{
			return FALSE;
		}

		if ((m_dwEditFlags & BCGP_CONTAINER_ENABLE_KEYBOARD) == BCGP_CONTAINER_ENABLE_KEYBOARD)
		{
			BOOL bResize = ::GetAsyncKeyState(VK_SHIFT) & 0x8000;
			const double delta = 1.0;
			CBCGPPoint ptOffset;

			switch (nChar)
			{
			case VK_LEFT:
				ptOffset.x -= delta;
				break;
			case VK_RIGHT:
				ptOffset.x += delta;
				break;
			case VK_UP:
				ptOffset.y -= delta;
				break;
			case VK_DOWN:
				ptOffset.y += delta;
				break;
			}

			BOOL bChanged = FALSE;

			if (ptOffset != CBCGPPoint())
			{
				POSITION pos = m_lstSel.GetHeadPosition ();
				while (pos != NULL)
				{
					CBCGPBaseVisualObject* pObject = m_lstSel.GetNext (pos);
					if (pObject == NULL || !pObject->IsVisible ())
					{
						continue;
					}

					UINT uiEditFlags = pObject->GetEditFlags();

					if ((uiEditFlags & BCGP_EDIT_NO_KEYBOARD) == BCGP_EDIT_NO_KEYBOARD)
					{
						continue;
					}

					CBCGPRect rect (pObject->GetRect ());
					if (bResize)
					{
						if ((uiEditFlags & BCGP_EDIT_NOSIZE) == BCGP_EDIT_NOSIZE)
						{
							continue;
						}

						CBCGPPoint ptSize (ptOffset);
						if ((uiEditFlags & BCGP_EDIT_SIZE_LOCK_ASPECT_RATIO) == BCGP_EDIT_SIZE_LOCK_ASPECT_RATIO)
						{
							if (ptSize.x != 0.0)
							{
								ptSize.y = ptSize.x;
							}
							else if (ptSize.y != 0.0)
							{
								ptSize.x = ptSize.y;
							}
						}

						rect.SetSize (rect.Width () + ptSize.x, rect.Height () + ptSize.y);

						if ((uiEditFlags & BCGP_EDIT_NO_NORMALIZE_RECT) == 0)
						{
							rect.Normalize();
						}

						CBCGPSize sizeMin (pObject->GetMinSize());
						if (sizeMin.IsEmpty())
						{
							sizeMin.SetSize (1.0, 1.0);
						}

						if (fabs(rect.Width ()) < sizeMin.cx || fabs(rect.Height ()) < sizeMin.cy)
						{
							continue;
						}
					}
					else if ((uiEditFlags & BCGP_EDIT_NOMOVE) == 0)
					{
						rect.OffsetRect (ptOffset);
					}

					if (rect != pObject->GetRect())
					{
						pObject->SetTrackingRect (rect);
						pObject->SetRect (rect);
						bChanged = TRUE;
					}
				}
			}

			if (bChanged)
			{
				Redraw();
			}
		}

		if ((m_dwEditFlags & BCGP_CONTAINER_DISABLE_KEYBOARD_OBJECT) == BCGP_CONTAINER_DISABLE_KEYBOARD_OBJECT)
		{
			return TRUE;
		}
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->OnKeyboardDown(nChar, nRepCnt, nFlags))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnKeyboardUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (IsEditMode ())
	{
		if (m_ptDragStart != CBCGPPoint(-1, -1) && m_nDragMode != HTNOWHERE)
		{
			return FALSE;
		}

		if ((m_dwEditFlags & BCGP_CONTAINER_DISABLE_KEYBOARD_OBJECT) == BCGP_CONTAINER_DISABLE_KEYBOARD_OBJECT)
		{
			return TRUE;
		}
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			
			if (pObject->OnKeyboardUp(nChar, nRepCnt, nFlags))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::ExportToImage(CBCGPGraphicsManager* pGMSrc, CBCGPImage& image, BOOL bFullImage)
{
	ASSERT_VALID(pGMSrc);

	CBCGPGraphicsManager* pGM = pGMSrc->CreateOffScreenManager(m_rect, &image);

	if (pGM == NULL)
	{
		return FALSE;
	}

	CBCGPRect rectSaved = m_rect;

	if (bFullImage)
	{
		CBCGPSize size = m_rect.Size();
		m_rect = CBCGPRect(0., 0., max(m_sizeScrollTotal.cx + 10, size.cx), max(m_sizeScrollTotal.cy + 10, size.cy));
	}
	else
	{
		m_rect = m_rect - rectSaved.TopLeft();
	}

	pGM->FillRectangle(m_rect, CBCGPBrush(CBCGPColor::White));

	SetDirty();
	OnDraw(pGM, CBCGPRect());

	m_rect = rectSaved;
	delete pGM;

	return TRUE;
}
//*******************************************************************************
HBITMAP CBCGPVisualContainer::ExportToBitmap(CBCGPGraphicsManager* pGM, BOOL bFullImage)
{
	ASSERT_VALID(pGM);

	CBCGPSize size = m_rect.Size();

	if (bFullImage)
	{
		size.cx = max(m_sizeScrollTotal.cx + 10, size.cx);
		size.cy = max(m_sizeScrollTotal.cy + 10, size.cy);
	}

	HBITMAP hmbpDib = CBCGPDrawManager::CreateBitmap_32(size, NULL);
	if (hmbpDib == NULL)
	{
		ASSERT (FALSE);
		return NULL;
	}

	CBCGPPoint ptScrollOffset = m_ptScrollOffset;

	if (bFullImage)
	{
		m_bDrawScrollBars = FALSE;
		m_ptScrollOffset.SetPoint(0., 0.);
	}

	CBCGPRect rect(CBCGPPoint(), size);

	CDC dcMem;
	dcMem.CreateCompatibleDC (NULL);

	HBITMAP hbmpOld = (HBITMAP) dcMem.SelectObject (hmbpDib);

	pGM->BindDC(&dcMem, rect);
	pGM->BeginDraw();

	CBCGPRect rectSaved = m_rect;

	if (bFullImage)
	{
		SetRect(CBCGPRect(CBCGPPoint(0., 0.), size));
	}
	else
	{
		SetRect(m_rect - rectSaved.TopLeft());
	}

	SetDirty();

	BOOL bCacheImage = m_bCacheImage;
	m_bCacheImage = FALSE;

	if (m_brFill.IsEmpty())
	{
		pGM->FillRectangle(m_rect, CBCGPBrush(CBCGPColor::White));
	}

	OnDraw(pGM, m_rect);

	pGM->EndDraw();

	dcMem.SelectObject (hbmpOld);

	CBCGPDrawManager::FillAlpha (rect, hmbpDib, 255);

	pGM->BindDC(NULL);

	SetRect(rectSaved);
	m_bCacheImage = bCacheImage;

	if (bFullImage)
	{
		m_bDrawScrollBars = TRUE;
		m_ptScrollOffset = ptScrollOffset;
	}

	SetDirty(TRUE);

	if (bFullImage)
	{
		Redraw();
	}

	return hmbpDib;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::CopyToClipboard(CBCGPGraphicsManager* pGM, BOOL bFullImage)
{
	if (pGM == NULL)
	{
		return FALSE;
	}

	HBITMAP hbmp = ExportToBitmap(pGM, bFullImage);

	CBCGPToolBarImages img;
	img.AddImage(hbmp, TRUE);
	img.SetSingleImage();

	return img.CopyImageToClipboard(0);
}
//*******************************************************************************
BOOL CBCGPVisualContainer::ExportToFile(const CString& strFilePath, CBCGPGraphicsManager* pGM, BOOL bFullImage)
{
	if (pGM == NULL)
	{
		return FALSE;
	}

	return SaveBitmapToFile(ExportToBitmap(pGM, bFullImage), strFilePath);
}
//*******************************************************************************
void CBCGPVisualContainer::SetScaleRatio(const CBCGPSize& sizeScaleRatio)
{
	if (m_sizeScaleRatio == sizeScaleRatio)
	{
		return;
	}

	CBCGPSize sizeScaleRatioOld = m_sizeScaleRatio;
	m_sizeScaleRatio = sizeScaleRatio;

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);
		pObject->SetScaleRatio(m_sizeScaleRatio);
	}

	OnScaleRatioChanged(sizeScaleRatioOld);

	if (m_bScrollBars)
	{
		AdjustScrollBars();
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	if (sizeScaleRatioOld.cx == 0.)
	{
		ASSERT(FALSE);
		return;
	}

	if (sizeScaleRatioOld.cy == 0.)
	{
		ASSERT(FALSE);
		return;
	}

	const double dblAspectRatioX = m_sizeScaleRatio.cx / sizeScaleRatioOld.cx;
	const double dblAspectRatioY = m_sizeScaleRatio.cy / sizeScaleRatioOld.cy;

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		if (m_sizeScaleRatio != CBCGPSize(1., 1.) || !pObject->ResetRect())
		{
			BOOL bDontResetRectSaved = pObject->m_bDontResetRect;
			pObject->m_bDontResetRect = TRUE;

			CBCGPRect rect = pObject->GetRect();
			rect.Scale (dblAspectRatioX, dblAspectRatioY);

			pObject->SetRect(rect);
			
			pObject->m_bDontResetRect = bDontResetRectSaved;
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::FireSelectionChangedEvent()
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

#ifndef _BCGPCHART_STANDALONE
	if (globalData.IsAccessibilitySupport () && GetSelCount() > 0)
	{
		CBCGPBaseVisualObject* pSel = GetSel();
		if (pSel != NULL)
		{
#ifdef _BCGSUITE_
			::NotifyWinEvent(EVENT_OBJECT_FOCUS, m_pWndOwner->GetSafeHwnd(), OBJID_CLIENT, GetAccChildIndex(pSel));
#else
			globalData.NotifyWinEvent(EVENT_OBJECT_FOCUS, m_pWndOwner->GetSafeHwnd(), OBJID_CLIENT, GetAccChildIndex(pSel));
#endif
		}
	}
#endif

	CWnd* pWndOwner = m_pWndOwner->GetOwner();
	if (pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	pWndOwner->SendMessage(BCGM_CONTAINER_SELCHANGED, 0, (LPARAM)this);
}
//*******************************************************************************
UINT CBCGPVisualContainer::GetClickAndHoldID() const
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		if (!pObject->GetClickAndHoldRect().IsRectEmpty())
		{
			return pObject->GetClickAndHoldID();
		}
	}

	return m_nClickAndHoldID;
}
//*******************************************************************************
CBCGPRect CBCGPVisualContainer::GetClickAndHoldRect()
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		if (!pObject->GetClickAndHoldRect().IsRectEmpty())
		{
			return pObject->GetClickAndHoldRect();
		}
	}

	return m_rectClickAndHold;
}
//*******************************************************************************
void CBCGPVisualContainer::OnClickAndHoldEvent(UINT nID, const CBCGPPoint& point)
{
	if (m_ScrollBarVert.GetPrevButton().PtInRect(point))
	{
		m_ScrollBarVert.OnScrollStep(FALSE);
		return;
	}
	else if (m_ScrollBarVert.GetNextButton().PtInRect(point))
	{
		m_ScrollBarVert.OnScrollStep(TRUE);
		return;
	}
	else if (m_ScrollBarHorz.GetPrevButton().PtInRect(point))
	{
		m_ScrollBarHorz.OnScrollStep(FALSE);
		return;
	}
	else if (m_ScrollBarHorz.GetNextButton().PtInRect(point))
	{
		m_ScrollBarHorz.OnScrollStep(TRUE);
		return;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject == NULL)
		{
			continue;
		}

		ASSERT_VALID(pObject);

		if (!pObject->GetClickAndHoldRect().IsRectEmpty())
		{
			pObject->OnClickAndHoldEvent(nID, point);
		}
	}
}
//*******************************************************************************
void CBCGPVisualContainer::OnScroll(CBCGPVisualScrollBar* pScrollBar, double dblDelta)
{
	if (pScrollBar == &m_ScrollBarVert)
	{
		m_ptScrollOffset.y += dblDelta;
	}
	else if (pScrollBar == &m_ScrollBarHorz)
	{
		m_ptScrollOffset.x += dblDelta;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			pObject->SetScrollOffset(m_ptScrollOffset);
		}
	}

	SetDirty();
	Redraw();
}
//*******************************************************************************
void CBCGPVisualContainer::SetClickAndHoldEvent(const CBCGPRect& rectBounds, UINT nEventID)
{
	m_rectClickAndHold = rectBounds;
	m_nClickAndHoldID = nEventID;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGestureEventZoom(const CBCGPPoint& ptCenter, double dblZoomFactor)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(ptCenter);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnGestureEventZoom(ptCenter, dblZoomFactor))
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGestureEventPan(const CBCGPPoint& ptFrom, const CBCGPPoint& ptTo, CBCGPSize& sizeOverPan)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(ptFrom);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnGestureEventPan(ptFrom, ptTo, sizeOverPan))
		{
			return TRUE;
		}
	}

	if (!m_ScrollBarVert.GetRect().IsRectEmpty() || !m_ScrollBarHorz.GetRect().IsRectEmpty())
	{
		const BOOL bScrollVert = !m_ScrollBarVert.GetRect().IsRectEmpty();
		CBCGPVisualScrollBar& scrollBar = bScrollVert ? m_ScrollBarVert : m_ScrollBarHorz;

		double dblDelta = bScrollVert ? ptTo.y - ptFrom.y : ptTo.x - ptFrom.x;
		double dblOffsetOld = scrollBar.GetOffset();

		double dblOffsetNew = dblOffsetOld - dblDelta;
		double dblOffsetMax = scrollBar.GetTotal() - scrollBar.GetScrollSize();
		double dblOverPan = 0.;

		if (dblOffsetNew < 0.)
		{
			dblOverPan = -dblOffsetNew;
			dblOffsetNew = 0.;
			dblDelta = dblOffsetOld;
		}
		else if (dblOffsetNew > dblOffsetMax)
		{
			dblOverPan = dblOffsetMax - dblOffsetNew;
			dblOffsetNew = dblOffsetMax;
			dblDelta = dblOffsetOld - dblOffsetNew;
		}

		if (dblOverPan != 0.)
		{
			if (bScrollVert)
			{
				sizeOverPan.cy = dblOverPan;
			}
			else
			{
				sizeOverPan.cx = dblOverPan;
			}
		}

		if (dblOffsetNew != scrollBar.GetOffset())
		{
			scrollBar.SetOffset(dblOffsetNew);
			scrollBar.ReposThumb();

			OnScroll(&scrollBar, -dblDelta);
		}

		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGestureEventRotate(const CBCGPPoint& ptCenter, double dblAngle)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(ptCenter);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnGestureEventRotate(ptCenter, dblAngle))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGestureEventTwoFingerTap(const CBCGPPoint& ptCenter)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(ptCenter);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnGestureEventTwoFingerTap(ptCenter))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}
//*******************************************************************************
BOOL CBCGPVisualContainer::OnGestureEventPressAndTap(const CBCGPPoint& ptPress, long lDelta)
{
	CBCGPBaseVisualObject* pObject = GetFromPoint(ptPress);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnGestureEventPressAndTap(ptPress, lDelta))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}
//*******************************************************************************
void CBCGPVisualContainer::ResetGestureOptions()
{
	ASSERT_VALID (this);

	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		CBCGPGestureConfig gestureConfig;
		if (GetGestureConfig(gestureConfig))
		{
			bcgpGestureManager.SetGestureConfig(m_pWndOwner->GetSafeHwnd(), gestureConfig);
		}
	}
}
//*****************************************************************************
CBCGPBaseVisualObject* CBCGPVisualContainer::GetAccChild(int nIndex)
{
	ASSERT_VALID(this);

	nIndex--;

	if (nIndex < 0 || nIndex >= (int)m_arObjects.GetSize())
	{
		return NULL;
	}

	return m_arObjects[nIndex];
}
//*****************************************************************************
long CBCGPVisualContainer::GetAccChildIndex(CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pObject);

	if (!pObject->IsVisible())
	{
		return 0;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pCurrObject = m_arObjects[i];
		ASSERT_VALID(pCurrObject);

		if (pCurrObject == pObject)
		{
			return i + 1;
		}
	}

	return 0;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accChildCount(long *pcountChildren)
{
	if (!pcountChildren)
	{
		return E_INVALIDARG;
	}

	*pcountChildren = (long)m_arObjects.GetSize();
	return S_OK;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accChild(VARIANT /*varChild*/, IDispatch** /*ppdispChild*/)
{
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accName(VARIANT varChild, BSTR *pszName)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CString strText;

		if (m_pWndOwner->GetSafeHwnd() != NULL)
		{
			m_pWndOwner->GetWindowText(strText);

			if (strText.IsEmpty())
			{
				strText = _T("Visual Container");
			}
		}

		*pszName = strText.AllocSysString();
		return S_OK;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
		if (pAccObject != NULL)
		{
			ASSERT_VALID(pAccObject);

			varChild.lVal = CHILDID_SELF;
			return pAccObject->get_accName(varChild, pszName);
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accValue(VARIANT varChild, BSTR *pszValue)
{
	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		return S_FALSE;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
		if (pAccObject != NULL)
		{
			ASSERT_VALID(pAccObject);
			
			varChild.lVal = CHILDID_SELF;
			return pAccObject->get_accValue(varChild, pszValue);
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accDescription(VARIANT varChild, BSTR *pszDescription)
{
	if (((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)) || (NULL == pszDescription))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		*pszDescription = SysAllocString(L"WinUITiles Table");
		return S_OK;
	}

	CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
	if (pAccObject != NULL)
	{
		ASSERT_VALID(pAccObject);
		
		varChild.lVal = CHILDID_SELF;
		return pAccObject->get_accDescription(varChild, pszDescription);
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
	if (!pvarRole || ((varChild.vt != VT_I4) && (varChild.lVal != CHILDID_SELF)))
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarRole->vt = VT_I4;
		pvarRole->lVal = ROLE_SYSTEM_GROUPING;

		return S_OK;
	}

	CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
	if (pAccObject != NULL)
	{
		ASSERT_VALID(pAccObject);
		
		varChild.lVal = CHILDID_SELF;
		return pAccObject->get_accRole(varChild, pvarRole);
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accState(VARIANT varChild, VARIANT *pvarState)
{
	if (pvarState == NULL)
	{
		return E_INVALIDARG;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		pvarState->vt = VT_I4;
		pvarState->lVal = STATE_SYSTEM_NORMAL;
		return S_OK;
	}

	if (varChild.vt == VT_I4)
	{
		CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
		if (pAccObject != NULL)
		{
			ASSERT_VALID(pAccObject);
			
			varChild.lVal = CHILDID_SELF;
			return pAccObject->get_accState(varChild, pvarState);
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
	if (varChild.vt == VT_I4 && varChild.lVal > 0)
	{
		CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
		if (pAccObject != NULL)
		{
			ASSERT_VALID(pAccObject);
			
			varChild.lVal = CHILDID_SELF;
			return pAccObject->get_accDefaultAction(varChild, pszDefaultAction);
		}
	}
	
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::accSelect(long flagsSelect, VARIANT varChild)
{
	if (varChild.vt != VT_I4)
	{
		return E_INVALIDARG;
	}

	if (flagsSelect == SELFLAG_NONE || varChild.lVal == CHILDID_SELF)
	{
		return S_FALSE;
	}

	if (flagsSelect == SELFLAG_TAKEFOCUS || flagsSelect == SELFLAG_TAKESELECTION)
	{
		CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
		if (pAccObject != NULL)
		{
			Select(pAccObject);
			return S_OK;
		}
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varChild)
{
	if (!pxLeft || !pyTop || !pcxWidth || !pcyHeight || varChild.vt != VT_I4)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	if ((varChild.vt == VT_I4) && (varChild.lVal == CHILDID_SELF))
	{
		CRect rc;
		m_pWndOwner->GetWindowRect(rc);

		*pxLeft = rc.left;
		*pyTop = rc.top;
		*pcxWidth = rc.Width();
		*pcyHeight = rc.Height();

		return S_OK;
	}

	CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
	if (pAccObject != NULL)
	{
		ASSERT_VALID(pAccObject);
		
		varChild.lVal = CHILDID_SELF;
		return pAccObject->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
	}

	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::get_accParent(IDispatch **ppdispParent)
{
    if (ppdispParent == NULL)
	{
		return E_INVALIDARG;
	}
	
	*ppdispParent = NULL;
	
	if (m_pWndOwner->GetSafeHwnd() != NULL)
	{
		CWnd* pWndParent = m_pWndOwner->GetParent();
		if (pWndParent->GetSafeHwnd() != NULL)
		{
			return AccessibleObjectFromWindow(pWndParent->GetSafeHwnd(), (DWORD)OBJID_CLIENT,
				IID_IAccessible, (void**)ppdispParent);
		}
	}
	
	return S_FALSE;
}
//****************************************************************************
HRESULT CBCGPVisualContainer::accHitTest(long  xLeft, long yTop, VARIANT *pvarChild)
{
	if (!pvarChild)
	{
		return E_INVALIDARG;
	}

	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return S_FALSE;
	}

	pvarChild->vt = VT_I4;

	CPoint pt(xLeft, yTop);
	m_pWndOwner->ScreenToClient(&pt);

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		pvarChild->lVal = GetAccChildIndex(pObject);
	}
	else
	{
		pvarChild->lVal = CHILDID_SELF;
	}

	return S_OK;
}
//******************************************************************************
HRESULT CBCGPVisualContainer::accDoDefaultAction(VARIANT varChild)
{
    if (varChild.vt != VT_I4)
    {
        return E_INVALIDARG;
    }

	CBCGPBaseVisualObject* pAccObject = GetAccChild(varChild.lVal);
	if (pAccObject != NULL)
	{
/*		if (OnClick(pAccObject))
		{
			return S_OK;
		}*/
    }

    return S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CBCGPVisualScrollBar

static const UINT nIDScrollPrev = 1001;
static const UINT nIDScrollNext = 1002;

CBCGPVisualScrollBarColorTheme::CBCGPVisualScrollBarColorTheme(const CBCGPColor& color)
{
	if (color == CBCGPColor())
	{
		// Set default colors:
		m_brFace = CBCGPBrush(CBCGPColor::WhiteSmoke);
		m_brBorder = CBCGPBrush(CBCGPColor::Gray);
		m_brButton = CBCGPBrush(CBCGPColor::LightGray);
	}
	else
	{
		CBCGPColor colorFace = color;
		CBCGPColor colorBorder = color;
		CBCGPColor colorButton = color;

		if (color.IsPale())
		{
			colorBorder.MakeDarker(.2);
			colorButton.MakeDarker(.02);
		}
		else
		{
			colorFace.MakeLighter(.4);
			colorBorder.MakeDarker(.2);
			colorButton.MakeLighter(.2);
		}

		m_brFace = CBCGPBrush(colorFace);
		m_brBorder = CBCGPBrush(colorBorder);
		m_brButton = CBCGPBrush(colorButton);
	}
}
//*******************************************************************************
CBCGPVisualScrollBar::CBCGPVisualScrollBar()
{
	m_pOwner = NULL;
	m_pOwnerContainer = NULL;

	m_bIsHorizontal = TRUE;
	m_dblStep = 0.0;

	Reset();
}
//*******************************************************************************
void CBCGPVisualScrollBar::SetParentVisualObject(CBCGPBaseVisualObject* pOwner)
{
	m_pOwner = pOwner;
}
//*******************************************************************************
void CBCGPVisualScrollBar::SetParentVisualContainer(CBCGPVisualContainer* pOwnerContainer)
{
	m_pOwnerContainer = pOwnerContainer;
}
//*******************************************************************************
BOOL CBCGPVisualScrollBar::OnMouseDown(const CBCGPPoint& pt)
{
	if (m_dblTotal <= 0. || m_rect.IsRectEmpty())
	{
		return FALSE;
	}

	if (m_rectThumb.PtInRect(pt))
	{
		m_bIsDraggingThumb = TRUE;
		m_ptDragThumbLast = pt;

		return TRUE;
	}

	double size = m_bIsHorizontal ? m_rectScroll.Width() : m_rectScroll.Height();
	double dblOffsetOld = m_dblOffset;
	double dblOffset = m_dblOffset;

	if (m_rectPrev.PtInRect(pt))
	{
		OnScrollStep(FALSE);

		if (m_pOwner != NULL)
		{
			m_pOwner->SetClickAndHoldEvent(m_rectPrev, nIDScrollPrev);
		}
		else if (m_pOwnerContainer != NULL)
		{
			m_pOwnerContainer->SetClickAndHoldEvent(m_rectPrev, nIDScrollPrev);
		}
		return TRUE;
	}
			
	if (m_rectNext.PtInRect(pt))
	{
		OnScrollStep(TRUE);

		if (m_pOwner != NULL)
		{
			m_pOwner->SetClickAndHoldEvent(m_rectNext, nIDScrollNext);
		}
		else if (m_pOwnerContainer != NULL)
		{
			m_pOwnerContainer->SetClickAndHoldEvent(m_rectNext, nIDScrollNext);
		}
		return TRUE;
	}
	
	if (!m_rectScroll.PtInRect(pt))
	{
		return FALSE;
	}

	if (m_bIsHorizontal)
	{
		if (pt.x < m_rectThumb.left)
		{
			dblOffset -= size;
		}
		else
		{
			dblOffset += size;
		}
	}
	else
	{
		if (pt.y < m_rectThumb.top)
		{
			dblOffset -= size;
		}
		else
		{
			dblOffset += size;
		}
	}

	m_dblOffset = min(max(0, dblOffset), m_dblTotal - size); 
	ReposThumb();

	if (m_pOwner != NULL)
	{
		m_pOwner->OnScroll(this, m_dblOffset - dblOffsetOld);
	}
	else if (m_pOwnerContainer != NULL)
	{
		m_pOwnerContainer->OnScroll(this, m_dblOffset - dblOffsetOld);
	}

	return TRUE;
}
//*******************************************************************************
void CBCGPVisualScrollBar::OnCancelMode()
{
	m_bIsDraggingThumb = FALSE;
}
//*******************************************************************************
void CBCGPVisualScrollBar::ReposThumb()
{
	if (m_rect.IsRectEmpty())
	{
		Reset();
		return;
	}

	m_rectThumb = m_rectScroll;

	double size = m_bIsHorizontal ? m_rectScroll.Width() : m_rectScroll.Height();

	double dblScrollRatio = size / m_dblTotal;
	double dblThumbSize = max(2.0, dblScrollRatio * size);

	if (m_bIsHorizontal)
	{
		m_rectThumb.left += m_dblOffset * dblScrollRatio;
		m_rectThumb.right = m_rectThumb.left + dblThumbSize;
	}
	else
	{
		m_rectThumb.top += m_dblOffset * dblScrollRatio;
		m_rectThumb.bottom = m_rectThumb.top + dblThumbSize;
	}
}
//*******************************************************************************
void CBCGPVisualScrollBar::SetHorizontal(BOOL bSet)
{
	m_bIsHorizontal = bSet;
}
//*******************************************************************************
void CBCGPVisualScrollBar::SetRect(const CBCGPRect& rectIn)
{
	if (rectIn.IsRectEmpty())
	{
		Reset();
		return;
	}

	CBCGPRect rect = rectIn;
	rect.DeflateRect(0.5, 0.5);

	m_rect = m_rectScroll = m_rectNext = m_rectPrev = rect;

	if (m_bIsHorizontal)
	{
		m_rectPrev.right = m_rectPrev.left + m_rectPrev.Height();
		m_rectNext.left = m_rectNext.right - m_rectNext.Height();

		m_rectScroll.DeflateRect(m_rectScroll.Height(), 0);
	}
	else
	{
		m_rectPrev.bottom = m_rectPrev.top + m_rectPrev.Width();
		m_rectNext.top = m_rectNext.bottom - m_rectNext.Width();

		m_rectScroll.DeflateRect(0, m_rectScroll.Width());
	}

	ReposThumb();
}
//*******************************************************************************
void CBCGPVisualScrollBar::Reset()
{
	m_bIsDraggingThumb = FALSE;
	m_dblTotal = 0.;
	m_dblOffset = 0.;

	m_rect.SetRectEmpty();
	m_rectScroll.SetRectEmpty();
	m_rectNext.SetRectEmpty();
	m_rectPrev.SetRectEmpty();
	m_rectThumb.SetRectEmpty();
}
//*******************************************************************************
void CBCGPVisualScrollBar::DoDraw(CBCGPGraphicsManager* pGM)
{
	if (m_rect.IsRectEmpty())
	{
		return;
	}

	pGM->FillRectangle(m_rect, m_ColorTheme.m_brFace);
	pGM->DrawRectangle(m_rect, m_ColorTheme.m_brBorder);

	pGM->FillRectangle(m_rectThumb, m_ColorTheme.m_brButton);
	pGM->DrawRectangle(m_rectThumb, m_ColorTheme.m_brBorder);

	pGM->FillRectangle(m_rectPrev, m_ColorTheme.m_brButton);
	pGM->DrawRectangle(m_rectPrev, m_ColorTheme.m_brBorder);

	CBCGPRect rectArrow = m_rectPrev;
	CBCGPPointsArray arPrevPoints;

	if (m_bIsHorizontal)
	{
		rectArrow.DeflateRect(5, 3);

		arPrevPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.CenterPoint().y));
		arPrevPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.top));
		arPrevPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.bottom));
	}
	else
	{
		rectArrow.DeflateRect(3, 5);

		arPrevPoints.Add(CBCGPPoint(rectArrow.CenterPoint().x, rectArrow.top));
		arPrevPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.bottom));
		arPrevPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.bottom));
	}

	pGM->FillGeometry(CBCGPPolygonGeometry(arPrevPoints), m_ColorTheme.m_brBorder);

	pGM->FillRectangle(m_rectNext, m_ColorTheme.m_brButton);
	pGM->DrawRectangle(m_rectNext, m_ColorTheme.m_brBorder);

	rectArrow = m_rectNext;
	CBCGPPointsArray arNextPoints;

	if (m_bIsHorizontal)
	{
		rectArrow.DeflateRect(5, 3);

		arNextPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.CenterPoint().y));
		arNextPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.top));
		arNextPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.bottom));
	}
	else
	{
		rectArrow.DeflateRect(3, 5);

		arNextPoints.Add(CBCGPPoint(rectArrow.CenterPoint().x, rectArrow.bottom));
		arNextPoints.Add(CBCGPPoint(rectArrow.left, rectArrow.top));
		arNextPoints.Add(CBCGPPoint(rectArrow.right, rectArrow.top));
	}

	pGM->FillGeometry(CBCGPPolygonGeometry(arNextPoints), m_ColorTheme.m_brBorder);
}
//*******************************************************************************
BOOL CBCGPVisualScrollBar::OnDragThumb(const CBCGPPoint& pt)
{
	if (!m_bIsDraggingThumb || m_dblTotal <= 0.)
	{
		return FALSE;
	}

	double size = m_bIsHorizontal ? m_rectScroll.Width() : m_rectScroll.Height();
	double dblScrollRatio = size / m_dblTotal;
	double dblOffsetOld = m_dblOffset;
	double delta = m_bIsHorizontal ? pt.x - m_ptDragThumbLast.x : pt.y - m_ptDragThumbLast.y;

	m_dblOffset = min(max(0, m_dblOffset + delta / dblScrollRatio), m_dblTotal - size); 

	m_ptDragThumbLast = pt;

	ReposThumb();
	
	if (m_pOwner != NULL)
	{
		m_pOwner->OnScroll(this, m_dblOffset - dblOffsetOld);
	}
	else if (m_pOwnerContainer != NULL)
	{
		m_pOwnerContainer->OnScroll(this, m_dblOffset - dblOffsetOld);
	}
	return TRUE;
}
//*******************************************************************************
void CBCGPVisualScrollBar::OnScrollStep(BOOL bNext)
{
	if (m_dblTotal <= 0.)
	{
		return;
	}

	double size = m_bIsHorizontal ? m_rectScroll.Width() : m_rectScroll.Height();
	double dblScrollRatio = size / m_dblTotal;
	double dblOffsetOld = m_dblOffset;
	double dblOffset = m_dblOffset;
	double delta = m_dblStep == 0. ? (m_rectPrev.Width() * dblScrollRatio) : m_dblStep;

	if (bNext)
	{
		dblOffset += delta;
	}
	else
	{
		dblOffset -= delta;
	}

	m_dblOffset = min(max(0, dblOffset), m_dblTotal - size); 

	ReposThumb();

	if (m_pOwner != NULL)
	{
		m_pOwner->OnScroll(this, m_dblOffset - dblOffsetOld);
	}
	else if (m_pOwnerContainer != NULL)
	{
		m_pOwnerContainer->OnScroll(this, m_dblOffset - dblOffsetOld);
	}
}
