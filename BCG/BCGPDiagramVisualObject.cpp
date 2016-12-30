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
// BCGPDiagramVisualObject.cpp: implementation of the CBCGPDiagramVisualObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bcgcbpro.h"
#include "BCGPDiagramVisualObject.h"
#include "BCGPDiagramVisualContainer.h"
#include "BCGPMath.h"

#include <float.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define BCGP_DIAGRAM_ID_INPLACE		1
#define BCGP_DIAGRAM_TEXT_PADDING	2

IMPLEMENT_DYNCREATE(CBCGPDiagramVisualObject, CBCGPBaseVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramAnchorPointObject, CBCGPDiagramVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramConnector, CBCGPDiagramVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramGroupObject, CBCGPDiagramVisualObject)
IMPLEMENT_DYNCREATE(CBCGPDiagramTextDataObject, CBCGPVisualDataObject)

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramAnchorPoint

CBCGPDiagramAnchorPoint::CBCGPDiagramAnchorPoint () : m_idObject (), m_nConnectionPort (0), m_ptNullAnchor ()
{
}

CBCGPDiagramAnchorPoint::CBCGPDiagramAnchorPoint (CBCGPDiagramItemID idObject, UINT nConnectionPort)
{
	m_idObject = idObject;
	m_nConnectionPort = nConnectionPort;

	m_ptNullAnchor.SetPoint (0, 0);
}

CBCGPDiagramAnchorPoint::CBCGPDiagramAnchorPoint (const CBCGPDiagramAnchorPoint& src)
{
	*this = src;
}
//*******************************************************************************
const CBCGPDiagramAnchorPoint& CBCGPDiagramAnchorPoint::operator=(const CBCGPDiagramAnchorPoint& src)
{
	m_idObject = src.m_idObject;
	m_nConnectionPort = src.m_nConnectionPort;

	m_ptNullAnchor = src.m_ptNullAnchor;
	
	return *this;
}
//*******************************************************************************
BOOL CBCGPDiagramAnchorPoint::IsNull () const
{
	return m_idObject.IsNull ();
}
//*******************************************************************************
BOOL CBCGPDiagramAnchorPoint::IsEqual (const CBCGPDiagramAnchorPoint& src) const
{
	return	(m_idObject == src.m_idObject) && 
			(m_nConnectionPort == src.m_nConnectionPort) &&
			(!m_idObject.IsNull () || m_ptNullAnchor == src.m_ptNullAnchor);
}
//*******************************************************************************
CBCGPDiagramAnchorPoint CBCGPDiagramAnchorPoint::NullAnchor (CBCGPPoint pt)
{
	CBCGPDiagramAnchorPoint dummyAnchor;
	dummyAnchor.m_ptNullAnchor = pt;

	return dummyAnchor;
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualObject

CBCGPDiagramVisualObject::CBCGPDiagramVisualObject(CBCGPVisualContainer* pContainer)
{
	m_nInPlaceEditIndex = -1;
	m_pWndInPlace = NULL;

	m_brFill = CBCGPBrush(CBCGPColor(CBCGPColor::DimGray), CBCGPBrush::BCGP_NO_GRADIENT, 0.2);
	m_brOutline = CBCGPBrush(CBCGPColor(CBCGPColor::DimGray), CBCGPBrush::BCGP_NO_GRADIENT, 0.8);

	m_Thickness = 1.0;

	if (pContainer != NULL)
	{
		ASSERT_VALID(pContainer);
		pContainer->Add(this);
	}
	else
	{
		m_pWndOwner = NULL;
	}
}

CBCGPDiagramVisualObject::CBCGPDiagramVisualObject(const CBCGPDiagramVisualObject& src)
{
	m_nInPlaceEditIndex = -1;
	m_pWndInPlace = NULL;

	CopyFrom(src);
}

CBCGPDiagramVisualObject::~CBCGPDiagramVisualObject()
{
	OnDestroyWindow ();
}
//******************************************************************************************
void CBCGPDiagramVisualObject::OnDestroyWindow ()
{
	if (m_pWndInPlace != NULL)
	{
		m_pWndInPlace->DestroyWindow ();
		delete m_pWndInPlace;
		m_pWndInPlace = NULL;
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetItemID (const CBCGPDiagramItemID& id)
{
	m_ItemId = id;
}
//*******************************************************************************
const CBCGPDiagramVisualContainer* CBCGPDiagramVisualObject::GetParentDiagram () const
{
	return DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, m_pParentContainer);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetAutoDestroy (BOOL bAutoDestroy)
{
	m_bIsAutoDestroy = bAutoDestroy;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetOutlineColor(const CBCGPColor& color)
{
	CBCGPBrush brush;
	if (color != CBCGPColor())
	{
		brush = CBCGPBrush(color);
	}

	SetOutlineBrush(brush);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetFillColor(const CBCGPColor& color)
{
	CBCGPBrush brush;
	if (color != CBCGPColor())
	{
		brush = CBCGPBrush(color);
	}

	SetFillBrush(brush);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetShadowColor(const CBCGPColor& color)
{
	CBCGPBrush brush;
	if (color != CBCGPColor())
	{
		brush = CBCGPBrush(color);
	}

	SetShadowBrush(brush);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetOutlineBrush(const CBCGPBrush& brush)
{
	m_brOutline = brush;

	SetDirty();
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetFillBrush(const CBCGPBrush& brush)
{
	m_brFill = brush;

	SetDirty();
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetShadowBrush(const CBCGPBrush& brush)
{
	m_brShadow = brush;

	SetDirty();
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetThickness (double dThickness)
{
	m_Thickness = dThickness;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetStrokeStyle (const CBCGPStrokeStyle& strokeStyle)
{
	m_StrokeStyle.CopyFrom (strokeStyle);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetConnectionPorts()
{
	CBCGPRect rect = GetRect();
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
		{
			return;
		}
	}

	rect.OffsetRect (-m_ptDrawOffset);

	m_mapConnectionPorts[CP_Center] = rect.CenterPoint();

	m_mapConnectionPorts[CP_TopLeft] = rect.TopLeft ();
	m_mapConnectionPorts[CP_BottomRight] = rect.BottomRight ();
	m_mapConnectionPorts[CP_TopRight] = CBCGPPoint (rect.right, rect.top);
	m_mapConnectionPorts[CP_BottomLeft] = CBCGPPoint (rect.left, rect.bottom);
	
	m_mapConnectionPorts[CP_Left] = CBCGPPoint (rect.left, rect.CenterPoint().y);
	m_mapConnectionPorts[CP_Right] = CBCGPPoint (rect.right, rect.CenterPoint().y);
	m_mapConnectionPorts[CP_Top] = CBCGPPoint (rect.CenterPoint().x, rect.top);
	m_mapConnectionPorts[CP_Bottom] = CBCGPPoint (rect.CenterPoint().x, rect.bottom);
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::GetConnectionPort (UINT nConnectionPortID, CBCGPPoint& pt) const
{
	CBCGPPoint ptConPoint;
	if (m_mapConnectionPorts.Lookup (nConnectionPortID, ptConPoint))
	{
		pt = ptConPoint;
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
CBCGPDiagramAnchorPoint CBCGPDiagramVisualObject::UseConnectionPort (UINT nConnectionPortID, CBCGPPoint* pDefaultPoint) const
{
	CBCGPPoint pt;
	if (!GetConnectionPort (nConnectionPortID, pt))
	{
		pt = GetRect ().CenterPoint ();

		if (pDefaultPoint != NULL)
		{
			ASSERT(AfxIsValidAddress (pDefaultPoint, sizeof(CBCGPPoint)));
			
			pt = *pDefaultPoint;
		}

		return CBCGPDiagramAnchorPoint::NullAnchor (pt);
	}

	if (!GetItemID ().IsNull ())
	{
		CBCGPDiagramAnchorPoint anchor (GetItemID (), nConnectionPortID);
		anchor.m_ptNullAnchor = pt;
		
		return anchor;
	}
	
	return CBCGPDiagramAnchorPoint::NullAnchor (pt);
}
//*******************************************************************************
UINT CBCGPDiagramVisualObject::HitTestConnectionPort(const CBCGPPoint& pt) const
{
	for (POSITION pos = m_mapConnectionPorts.GetStartPosition (); pos != NULL;)
	{
		UINT uiHitTest = 0;
		CBCGPPoint ptConPoint;
		
		m_mapConnectionPorts.GetNextAssoc (pos, uiHitTest, ptConPoint);
		
		if (MakeTrackMarker (ptConPoint).PtInRect(pt))
		{
			return uiHitTest;
		}
	}

	CBCGPRect rectObject = GetRect();
	rectObject.Normalize();
	
	if (!rectObject.PtInRect(pt))
	{
		return HTNOWHERE;
	}
	
	return  HTCLIENT;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::DrawConnectionPorts(CBCGPGraphicsManager* pGM,
								  const CBCGPBrush& brOutline, const CBCGPBrush& brFill)
{
	for (POSITION pos = m_mapConnectionPorts.GetStartPosition (); pos != NULL;)
	{
		UINT uiConnectionPortID = 0;
		CBCGPPoint pt;
		
		m_mapConnectionPorts.GetNextAssoc (pos, uiConnectionPortID, pt);

		if (HasConnectors (uiConnectionPortID))
		{
			CBCGPRect rect = MakeTrackMarker (pt);
			
			pGM->FillEllipse(rect, brFill);
			pGM->DrawEllipse(rect, brOutline);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnDraw(CBCGPGraphicsManager* pGM, 
									  const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}
	
	CBCGPRoundedRect rr(m_rect, 5, 5);
	
	pGM->FillRoundedRectangle(rr, m_brFill);
	pGM->DrawRoundedRectangle(rr, m_brOutline, m_Thickness * GetScaleRatioMid (), &m_StrokeStyle);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	CBCGPRect rectOld = GetRect ();

	CBCGPBaseVisualObject::SetRect (rect, FALSE);

	SetConnectionPorts ();
	UpdateConnections (FALSE);

	CBCGPDiagramVisualContainer* pContainer = (CBCGPDiagramVisualContainer*)GetParentDiagram ();
	if (pContainer != NULL)
	{
		pContainer->FirePosSizeChangedEvent (this, rectOld);
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnScaleRatioChanged(const CBCGPSize& sizeScaleRatioOld)
{
	CBCGPBaseVisualObject::OnScaleRatioChanged (sizeScaleRatioOld);

	for (int i = 0; i < GetDataCount (); i++)
	{
		CBCGPDiagramTextDataObject* pTextData = GetTextData (i);
		if (pTextData != NULL)
		{
			ASSERT_VALID (pTextData);

			pTextData->SetFontScale (m_sizeScaleRatio.cy);
		}
	}
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::HasConnectors (UINT nConnectionPortID) const
{
	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		if (nConnectionPortID == m_lstIncidentConnectors.GetNext (pos).m_nConnectionPortID)
		{
			return TRUE;
		}	
	}
	
	return FALSE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::GetConnectors (UINT nConnectionPortID, CItemIDList& lstConnectors) const
{
 	lstConnectors.RemoveAll ();
 	
	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		IncidentEdge edge = m_lstIncidentConnectors.GetNext (pos);

		if (edge.m_nConnectionPortID == nConnectionPortID)
		{
			lstConnectors.AddTail (edge.m_idIncidentConnector);
		}
	}

	return !lstConnectors.IsEmpty ();
}
//*******************************************************************************
void CBCGPDiagramVisualObject::GetAllConnectors (CItemIDList& lstConnectors) const
{
	lstConnectors.RemoveAll ();

	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		IncidentEdge edge = m_lstIncidentConnectors.GetNext (pos);
		lstConnectors.AddTail (edge.m_idIncidentConnector);
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnConnectionAdded (UINT nConnectionPortID, CBCGPDiagramItemID idIncidentConnector, CBCGPDiagramItemID /*idAdjacentObject*/)
{
	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		const IncidentEdge& edge = m_lstIncidentConnectors.GetNext (pos);
		
		if (edge.m_nConnectionPortID == nConnectionPortID &&
			edge.m_idIncidentConnector == idIncidentConnector)
		{
			return; // already added
		}
	}

	IncidentEdge edgeNew;
	edgeNew.m_nConnectionPortID = nConnectionPortID;
	edgeNew.m_idIncidentConnector = idIncidentConnector;

	m_lstIncidentConnectors.AddTail (edgeNew);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnConnectionRemoved (UINT nConnectionPortID, CBCGPDiagramItemID idIncidentConnector, CBCGPDiagramItemID /*idAdjacentObject*/)
{
	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		POSITION posSave = pos;
		const IncidentEdge& edge = m_lstIncidentConnectors.GetNext (pos);

		if (edge.m_nConnectionPortID == nConnectionPortID &&
			edge.m_idIncidentConnector == idIncidentConnector)
		{
			m_lstIncidentConnectors.RemoveAt (posSave);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::UpdateConnections (BOOL bRedraw)
{
	CItemIDList lstConnectors;
	GetAllConnectors (lstConnectors);
	
	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
	if (pContainer != NULL)
	{
		pContainer->UpdateItems (lstConnectors, bRedraw);
	}
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnAdd (CBCGPDiagramVisualContainer* /*pContainer*/)
{
}
//*******************************************************************************
void CBCGPDiagramVisualObject::OnRemove (CBCGPDiagramVisualContainer* pContainer)
{
	ASSERT_VALID (pContainer);

	//DisconnectAll ();
	for (POSITION pos = m_lstIncidentConnectors.GetHeadPosition (); pos != NULL; )
	{
		IncidentEdge& edge = m_lstIncidentConnectors.GetNext (pos);
		pContainer->NotifyConnectorOnDisconnect(GetItemID (), edge.m_idIncidentConnector);
	}

 	m_lstIncidentConnectors.RemoveAll ();
}
//*******************************************************************************
int CBCGPDiagramVisualObject::AddTextData (const CString& str, const CBCGPColor& clrText)
{
	CBCGPVisualDataObject* pObject = new CBCGPDiagramTextDataObject (str, clrText);

	int nIndex = AddData(pObject);

	if (nIndex == -1)
	{
		delete pObject;
	}

	return nIndex;
}
//*******************************************************************************
CBCGPDiagramTextDataObject* CBCGPDiagramVisualObject::GetTextData (int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		return NULL;
	}
	
	return DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[nIndex]);
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::SetTextData (const CString& str, int nIndex, BOOL bRedraw)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		ASSERT(FALSE);
		return FALSE;
	}
	
	CBCGPDiagramTextDataObject* pTextData = DYNAMIC_DOWNCAST(CBCGPDiagramTextDataObject, m_arData[nIndex]);
	if (pTextData == NULL)
	{
		return FALSE;
	}

	SetDirty();
	pTextData->SetText (str, CBCGPColor(), FALSE);
	
	if (bRedraw)
	{
		Redraw();
	}
	
	return TRUE;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::RemoveTextData (int nIndex, BOOL bRedraw)
{
	if (nIndex < 0 || nIndex >= m_arData.GetSize())
	{
		ASSERT(FALSE);
		return;
	}

	delete m_arData[nIndex];
	m_arData.RemoveAt (nIndex);
	
	SetDirty();
	
	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::OnEdit (const CBCGPPoint* /*ppt*/)
{
	ASSERT_VALID (this);

	int nDataIndex = 0;
	CBCGPDiagramTextDataObject* pTextData = GetTextData (0);
	if (pTextData == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID (pTextData);

	CBCGPRect rectEdit = GetRect ();
	rectEdit.OffsetRect (-m_ptScrollOffset);
	AdjustInPlaceEditRect (rectEdit);

	BOOL bDefaultFormat = FALSE;
	m_pWndInPlace = CreateInPlaceEdit (rectEdit, pTextData, bDefaultFormat);

	if (m_pWndInPlace != NULL)
	{
		if (bDefaultFormat)
		{
			m_pWndInPlace->SetWindowText (pTextData->ToString ());
		}

		SetInPlaceEditFont ();
		m_pWndInPlace->SetFocus ();

		m_nInPlaceEditIndex = nDataIndex;
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::OnEndEdit ()
{
	m_nInPlaceEditIndex = -1;
	OnDestroyWindow ();
	return TRUE;
}
//*******************************************************************************
CWnd* CBCGPDiagramVisualObject::CreateInPlaceEdit (CRect rectEdit, CBCGPDiagramTextDataObject* pTextData, BOOL& bDefaultFormat)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pTextData);

	CEdit* pWndEdit = new CEdit;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | ES_AUTOVSCROLL;
	
	if (pTextData->GetTextFormat ().IsWordWrap ())
	{
		dwStyle |= ES_MULTILINE;
	}
	else
	{
		dwStyle |= ES_AUTOHSCROLL; // no wrapping, use scrolling instead
	}
	
	if (IsReadOnly ())
	{
		dwStyle |= ES_READONLY;
	}
	
	switch (pTextData->GetTextFormat ().GetTextAlignment ())
	{
	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_TRAILING:
		dwStyle |= ES_RIGHT;
		break;
		
	case CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER:
		dwStyle |= ES_CENTER;
		break;
	}

	pWndEdit->Create (dwStyle, rectEdit, GetOwner (), BCGP_DIAGRAM_ID_INPLACE);
	
	if (IsReadOnly ())
	{
		pWndEdit->SetReadOnly (TRUE);
	}
	
	bDefaultFormat = TRUE;
	return pWndEdit;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::AdjustInPlaceEditRect (CBCGPRect& rectEdit)
{
	rectEdit.DeflateRect (BCGP_DIAGRAM_TEXT_PADDING, BCGP_DIAGRAM_TEXT_PADDING);
}
//*******************************************************************************
void CBCGPDiagramVisualObject::SetInPlaceEditFont ()
{
	if (m_pWndInPlace->GetSafeHwnd () == NULL)
	{
		return;
	}

	ASSERT_VALID (m_pWndInPlace);
	
	if (GetOwner ()->GetSafeHwnd () != NULL)
	{
		m_pWndInPlace->SetFont (GetOwner ()->GetFont ());
	}
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::ValidateInPlaceEdit ()
{
	ASSERT_VALID (this);
	ASSERT (IsInPlaceEdit ());

	CBCGPDiagramTextDataObject* pTextData = GetTextData (m_nInPlaceEditIndex);
	if (pTextData == NULL || m_pWndInPlace == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ASSERT_VALID (pTextData);
	ASSERT_VALID (m_pWndInPlace);
	ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));

	CString str;
	m_pWndInPlace->GetWindowText (str);

	return pTextData->ValidateString (str);
}
//*******************************************************************************
BOOL CBCGPDiagramVisualObject::UpdateData ()
{
	if (IsInPlaceEdit ())
	{
		CBCGPDiagramTextDataObject* pTextData = GetTextData (m_nInPlaceEditIndex);
		if (pTextData == NULL || m_pWndInPlace == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}
		
		ASSERT_VALID (pTextData);
		ASSERT_VALID (m_pWndInPlace);
		ASSERT (::IsWindow (m_pWndInPlace->GetSafeHwnd ()));
		
		CString strEdit;
		m_pWndInPlace->GetWindowText (strEdit);

		BOOL bIsChanged = pTextData->ToString () != strEdit;
		BOOL bRes = TRUE;

		if (bIsChanged)
		{
			bRes = pTextData->ParseString (strEdit);
		}

		if (bRes && bIsChanged)
		{
			CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
			if (pContainer != NULL)
			{
				pContainer->OnItemChanged (this, m_nInPlaceEditIndex);
			}
		}

		return bRes;
	}

	return FALSE;
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPDiagramVisualObject::CreateCopy () const
{
	ASSERT_VALID(this);

	CRuntimeClass* pRTC = GetRuntimeClass();
	if (pRTC == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CObject* pNewObj = pRTC->CreateObject();
	if (pNewObj == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CBCGPDiagramVisualObject* pNewDiagram = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pNewObj);
	if (pNewDiagram  == NULL)
	{
		ASSERT(FALSE);
		delete pNewObj;
		return NULL;
	}

	pNewDiagram->CopyFrom(*this);
	return pNewDiagram;
}
//*******************************************************************************
void CBCGPDiagramVisualObject::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPBaseVisualObject::CopyFrom(srcObj);

	const CBCGPDiagramVisualObject& src = (const CBCGPDiagramVisualObject&)srcObj;

	m_brFill = src.m_brFill;
	m_brOutline = src.m_brOutline;
	m_brShadow = src.m_brShadow;

	m_Thickness = src.m_Thickness;
	m_StrokeStyle.CopyFrom (src.m_StrokeStyle);
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramAnchorPointObject

CBCGPDiagramAnchorPointObject::CBCGPDiagramAnchorPointObject()
{
}

CBCGPDiagramAnchorPointObject::CBCGPDiagramAnchorPointObject(
	CBCGPPoint /*pt*/, CBCGPVisualContainer* pContainer)
	: CBCGPDiagramVisualObject (pContainer)
{
}

CBCGPDiagramAnchorPointObject::CBCGPDiagramAnchorPointObject(const CBCGPDiagramAnchorPointObject& src)
{
	CopyFrom(src);
}
//*******************************************************************************
void CBCGPDiagramAnchorPointObject::SetConnectionPorts ()
{
	const CBCGPRect& rect = GetRect();
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
		{
			return;
		}
	}
	
	m_mapConnectionPorts[CP_Center] = rect.CenterPoint();
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramConnector

CBCGPDiagramConnector::CBCGPDiagramConnector(CBCGPVisualContainer* pContainer)
{
	Init ();

	if (pContainer != NULL)
	{
		ASSERT_VALID(pContainer);
		pContainer->Add(this);
	}
	else
	{
		m_pWndOwner = NULL;
	}
}
//*******************************************************************************
CBCGPDiagramConnector::CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2)
{
	Init ();

	InsertPoint (0, pt1);
	InsertPoint (1, pt2);

	SetRect (CBCGPRect (pt1, pt2));
}
//*******************************************************************************
CBCGPDiagramConnector::CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, BCGP_CURVE_TYPE curveType)
{
	Init ();

	m_curveType = curveType;

	InsertPoint (0, pt1);
	InsertPoint (1, pt2);
	InsertPoint (2, pt3);
}
//*******************************************************************************
CBCGPDiagramConnector::CBCGPDiagramConnector(const CBCGPPoint& pt1, const CBCGPPoint& pt2, const CBCGPPoint& pt3, const CBCGPPoint& pt4, BCGP_CURVE_TYPE curveType)
{
	Init ();

	m_curveType = curveType;

	InsertPoint (0, pt1);
	InsertPoint (1, pt2);
	InsertPoint (2, pt3);
	InsertPoint (3, pt4);
}
//*******************************************************************************
void CBCGPDiagramConnector::Init ()
{
	m_nTrackPointIndex = -1;
	m_curveType = BCGP_CURVE_TYPE_SPLINE;

	m_brOutline = CBCGPBrush (CBCGPColor::Gray);
}
//*******************************************************************************
CBCGPDiagramConnector::CBCGPDiagramConnector(const CBCGPDiagramConnector& src)
{
	m_nTrackPointIndex = -1;

	CopyFrom(src);
}
//*******************************************************************************
CBCGPDiagramConnector::~CBCGPDiagramConnector()
{
	CBCGPDiagramVisualContainer* pParentContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, m_pParentContainer);
	if (pParentContainer != NULL)
	{
		OnRemove (pParentContainer);
	}
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::Connect (
			  CBCGPDiagramVisualObject* pObject1, CBCGPDiagramVisualObject* pObject2,
			  UINT nConnectionPortId1, UINT nConnectionPortId2)
{
	if (pObject1 == NULL || pObject2 == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pObject1);
	ASSERT_VALID(pObject2);

	if (pObject1->GetParentContainer () == NULL || 
		pObject1->GetParentContainer () != pObject2->GetParentContainer ())
	{
		return FALSE;
	}

	pObject1->SetConnectionPorts ();
	pObject2->SetConnectionPorts ();

	return Connect (
		pObject1->UseConnectionPort (nConnectionPortId1),
		pObject2->UseConnectionPort (nConnectionPortId2));
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::Connect (const CBCGPPoint& start, const CBCGPPoint& end)
{
	ASSERT_VALID (this);

	return Connect (CBCGPDiagramAnchorPoint::NullAnchor (start), CBCGPDiagramAnchorPoint::NullAnchor (end));
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::Connect (CBCGPDiagramAnchorPoint start, CBCGPDiagramAnchorPoint end)
{
	ASSERT_VALID(this);

	m_arPoints.Add (start);
	m_arPoints.Add (end);

	if (!IsConnected ())
	{
		SetRect (GetBoundsRect ());
	}
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::IsConnected () const
{
	const int size = (int)m_arPoints.GetSize ();
	if (size == 0)
	{
		return FALSE;
	}

	for (int i = 0; i < size; i++)
	{
		if (!m_arPoints[i].IsNull ())
		{
			return TRUE;
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::InsertPoint (int nIndex, const CBCGPPoint& pt)
{
	ASSERT_VALID (this);

	if (nIndex >= 0 && nIndex <= m_arPoints.GetSize ())
	{
		CBCGPDiagramAnchorPoint anchor = CBCGPDiagramAnchorPoint::NullAnchor (pt);
		m_arPoints.InsertAt (nIndex, anchor);

		if (!IsConnected ())
		{
			SetRect (GetBoundsRect ());
		}
		return TRUE;
	}
	
	return FALSE;
}
//*******************************************************************************
int CBCGPDiagramConnector::GetPoints(CBCGPPointsArray& arPoints) const
{
	arPoints.RemoveAll ();

	CBCGPDiagramVisualContainer* pDiagramContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, m_pParentContainer);

	const int size = (int)m_arPoints.GetSize ();
	if (size == 0)
	{
		return 0;
	}

	arPoints.SetSize (size);
	CBCGPPoint* pPoints = arPoints.GetData ();
	for (int i = 0; i < size; i++)
	{
		*pPoints++ = ((pDiagramContainer != NULL) ? 
			pDiagramContainer->CalculatePoint (m_arPoints[i]) :
			m_arPoints[i].m_ptNullAnchor);
	}

	return size;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetCurveType (BCGP_CURVE_TYPE type)
{
	m_curveType = type;

	Redraw ();
}
//*******************************************************************************
void CBCGPDiagramConnector::SetBeginArrow (CBCGPDiagramConnector::BCGP_ARROW_SHAPE shape)
{
	m_arrowBegin.m_nShape = shape;
	m_arrowBegin.m_ptOffset = CBCGPPoint (0.0, 0.0);

	Redraw ();
}
//*******************************************************************************
void CBCGPDiagramConnector::SetBeginArrow (CBCGPDiagramConnector::BCGP_ARROW_SHAPE shape, double dLength, double dWidth)
{
	m_arrowBegin.m_nShape = shape;
	m_arrowBegin.m_dLength = dLength;
	m_arrowBegin.m_dWidth = dWidth;
	m_arrowBegin.m_ptOffset = CBCGPPoint (0.0, 0.0);

	Redraw ();
}
//*******************************************************************************
CBCGPDiagramConnector::BCGP_ARROW_SHAPE CBCGPDiagramConnector::GetBeginArrow (double* pLength, double* pWidth) const
{
    if (pLength != NULL)
    {
        *pLength = m_arrowBegin.m_dLength;
    }
	
    if (pWidth != NULL)
    {
        *pWidth = m_arrowBegin.m_dWidth;
    }
	
	return m_arrowBegin.m_nShape;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetBeginArrowOutlineBrush (const CBCGPBrush& brush)
{
	m_arrowBegin.m_brOutline = brush;

	SetDirty();
}
//*******************************************************************************
const CBCGPBrush& CBCGPDiagramConnector::GetBeginArrowOutlineBrush () const
{
	return m_arrowBegin.m_brOutline;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetBeginArrowFillBrush (const CBCGPBrush& brush)
{
	m_arrowBegin.m_brFill = brush;

	SetDirty();
}
//*******************************************************************************
const CBCGPBrush& CBCGPDiagramConnector::GetBeginArrowFillBrush () const
{
	return m_arrowBegin.m_brFill;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetEndArrow (CBCGPDiagramConnector::BCGP_ARROW_SHAPE shape)
{
	m_arrowEnd.m_nShape = shape;
	m_arrowEnd.m_ptOffset = CBCGPPoint (0.0, 0.0);

	Redraw ();
}
//*******************************************************************************
void CBCGPDiagramConnector::SetEndArrow (CBCGPDiagramConnector::BCGP_ARROW_SHAPE shape, double dLength, double dWidth)
{
	m_arrowEnd.m_nShape = shape;
	m_arrowEnd.m_dLength = dLength;
	m_arrowEnd.m_dWidth = dWidth;
	m_arrowEnd.m_ptOffset = CBCGPPoint (0.0, 0.0);

	Redraw ();
}
//*******************************************************************************
CBCGPDiagramConnector::BCGP_ARROW_SHAPE CBCGPDiagramConnector::GetEndArrow (double* pLength, double* pWidth) const
{
    if (pLength != NULL)
    {
        *pLength = m_arrowEnd.m_dLength;
    }
	
    if (pWidth != NULL)
    {
        *pWidth = m_arrowEnd.m_dWidth;
    }
	
	return m_arrowEnd.m_nShape;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetEndArrowOutlineBrush (const CBCGPBrush& brush)
{
	m_arrowEnd.m_brOutline = brush;

	SetDirty();
}
//*******************************************************************************
const CBCGPBrush& CBCGPDiagramConnector::GetEndArrowOutlineBrush () const
{
	return m_arrowEnd.m_brOutline;
}
//*******************************************************************************
void CBCGPDiagramConnector::SetEndArrowFillBrush (const CBCGPBrush& brush)
{
	m_arrowEnd.m_brFill = brush;

	SetDirty();
}
//*******************************************************************************
const CBCGPBrush& CBCGPDiagramConnector::GetEndArrowFillBrush () const
{
	return m_arrowEnd.m_brFill;
}
//*******************************************************************************
CBCGPDiagramAnchorPoint& CBCGPDiagramConnector::AnchorPoint (int nIndex)
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < m_arPoints.GetSize ());
	return m_arPoints[nIndex];
}
//*******************************************************************************
int CBCGPDiagramConnector::FindAnchorPointIndex (CBCGPDiagramAnchorPoint& anchor) const
{
	for (int i = 0; i < m_arPoints.GetSize (); i++)
	{
		if (m_arPoints[i].IsEqual (anchor))
		{
			return i;
		}
	}
	
	return -1;
}
//*******************************************************************************
UINT CBCGPDiagramConnector::HitTestAnchorPoint (const CBCGPPoint& pt, int& nIndex) const
{
	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer ,GetParentContainer ());
	
	if (pContainer == NULL)
	{
		nIndex = -1;
		return HTNOWHERE;
	}

	for (int i = 0; i < m_arPoints.GetSize (); i++)
	{
		CBCGPPoint ptAnchor = pContainer->CalculatePoint (m_arPoints[i]);
		ptAnchor.Offset (-m_ptScrollOffset);
		if (MakeTrackMarker (ptAnchor).PtInRect (pt))
		{
			int uiHitTest = CP_None;
			if (i == 0)
			{
				uiHitTest = CBCGPDiagramVisualObject::CP_Begin;
			}
			else if (i == m_arPoints.GetSize () - 1)
			{
				uiHitTest = CBCGPDiagramVisualObject::CP_End;
			}
			else if (i >= 1)
			{
				uiHitTest = CP_CustomFirst + i;
			}

			nIndex = i;
			return uiHitTest;
		}
	}

	nIndex = -1;
	return HTNOWHERE;
}
//*******************************************************************************
int CBCGPDiagramConnector::HitTest(const CBCGPPoint& pt) const
{
	if (CBCGPDiagramVisualObject::HitTest (pt) == HTNOWHERE)
	{
		return HTNOWHERE;
	}

	int i = 0;

	UINT uiHitTest = HitTestAnchorPoint (pt, i);
	if (uiHitTest != HTNOWHERE)
	{
		if (IsEditMode() && (m_uiEditFlags & BCGP_EDIT_NOMOVE) == 0)
		{
			return uiHitTest;
		}
		else
		{
			return HTCLIENT;
		}
	}

	CBCGPPointsArray arPoints;
	
	if (m_bIsSelected)
	{
		GetTrackedPoints (arPoints);
	}
	else
	{
		GetPoints (arPoints);
	}

	arPoints.Offset (-m_ptScrollOffset);

	for (i = 1; i < arPoints.GetSize (); i++)
	{
		const double TOLERANCE = 8;
		if (bcg_pointInLine (arPoints[i - 1], arPoints[i], pt, TOLERANCE) > 0)
		{
			return (IsEditMode() && (m_uiEditFlags & BCGP_EDIT_NOMOVE) == 0) ? HTCAPTION : HTCLIENT;
		}
	}

	return HTNOWHERE;
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::OnSetMouseCursor(const CBCGPPoint& pt)
{
	int nHitTest = HitTest(pt);

	switch (nHitTest)
	{
	case HTTOPLEFT:		// CP_Begin
	case HTBOTTOMRIGHT:	// CP_End
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
		return TRUE;

	case HTCAPTION:
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_SIZEALL));
		return TRUE;
	}

	if (nHitTest > CP_CustomFirst)
	{
		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
		return TRUE;
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPDiagramConnector::BeginTrackAnchorPoint (CBCGPDiagramAnchorPoint anchor)
{
	m_nTrackPointIndex = FindAnchorPointIndex (anchor);

	m_arTrackPoints.RemoveAll ();
	m_arTrackPoints.Append (m_arPoints);
}
//*******************************************************************************
void CBCGPDiagramConnector::EndTrackAnchorPoint (BOOL bSaveChanges)
{
	if (m_nTrackPointIndex != -1)
	{
		if (bSaveChanges)
		{
			CBCGPDiagramAnchorPoint& anchor = AnchorPoint (m_nTrackPointIndex);

			if (!GetTrackedAnchorPoint ().IsEqual (anchor))
			{
				CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
				if (pContainer != NULL)
				{
					pContainer->NotifyConnectedObject (anchor, GetItemID (), FALSE);
				}

				anchor = GetTrackedAnchorPoint ();

				if (pContainer != NULL)
				{
					pContainer->NotifyConnectedObject (anchor, GetItemID (), TRUE);
				}
			}
		}

		if (bSaveChanges)
		{
			if (!m_rectTrack.IsRectEmpty())
			{
				SetRect (m_rectTrack);
			}
		}

		m_nTrackPointIndex = -1;
	}
}
//*******************************************************************************
const CBCGPDiagramAnchorPoint CBCGPDiagramConnector::GetTrackedAnchorPoint () const
{
	ASSERT(m_nTrackPointIndex >= 0);
	ASSERT(m_nTrackPointIndex < m_arTrackPoints.GetSize ());
	return m_arTrackPoints[m_nTrackPointIndex];
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::SetTrackedAnchorPoint (const CBCGPDiagramAnchorPoint& anchor)
{
	ASSERT(m_nTrackPointIndex >= 0);
	ASSERT(m_nTrackPointIndex < m_arTrackPoints.GetSize ());

	if (anchor.IsEqual (m_arTrackPoints[m_nTrackPointIndex]))
	{
		return FALSE;
	}

	m_arTrackPoints[m_nTrackPointIndex] = anchor;

	SetTrackingRect (GetBoundsRect ());
	return TRUE;
}
//*******************************************************************************
void CBCGPDiagramConnector::OnEndTrackingPoints (BOOL bSaveChanges)
{
	if (!bSaveChanges)
	{
		return;
	}

	const int size = (int) m_arPoints.GetSize ();

	if (m_arTrackPoints.GetSize () != size)
	{
		ASSERT(FALSE);
		return;
	}

	for (int i = 0; i < size; i++)
	{
		if (!m_arPoints[i].IsEqual (m_arTrackPoints[i]))
		{
			m_arPoints[i] = m_arTrackPoints[i];
		}
	}
}
//*******************************************************************************
int CBCGPDiagramConnector::GetTrackedPoints(CBCGPPointsArray& arPoints) const
{
	arPoints.RemoveAll ();

	CBCGPDiagramVisualContainer* pDiagramContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, m_pParentContainer);
	if (pDiagramContainer == NULL)
	{
		return 0;
	}

	const int size = (int)m_arTrackPoints.GetSize ();
	if (size == 0)
	{
		return 0;
	}

	arPoints.SetSize (size);
	CBCGPPoint* pPoints = arPoints.GetData ();
	for (int i = 0; i < size; i++)
	{
		*pPoints++ = pDiagramContainer->CalculatePoint (m_arTrackPoints[i]);
	}

	return size;
}
//*******************************************************************************
CBCGPRect CBCGPDiagramConnector::GetBoundsRect () const
{
	CBCGPRect rect;
	const AnchorPointsArray& arPoints = m_bIsSelected ? m_arTrackPoints : m_arPoints;

	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
	
	for (int i = 0; i < arPoints.GetSize (); i++)
	{
		CBCGPPoint pt = arPoints[i].m_ptNullAnchor;

		if (pContainer != NULL)
		{
			pt = pContainer->CalculatePoint (arPoints[i]);
		}

		if (i == 0)
		{
			rect.SetRect (pt, pt);
		}
		else if (!rect.PtInRect (pt))
		{
			rect.left   = min(pt.x, rect.left  );
			rect.right  = max(pt.x, rect.right );
			rect.top    = min(pt.y, rect.top   );
			rect.bottom = max(pt.y, rect.bottom);
		}
	}

	if (arPoints.GetSize () > 0)
	{
		if (rect.Height () == 0)
		{
			rect.InflateRect (0, 1);
		}

		if (rect.Width () == 0)
		{
			rect.InflateRect (1, 0);
		}
	}

	return rect;
}
//*******************************************************************************
BOOL CBCGPDiagramConnector::GetArrowPoints (CBCGPPoint ptStart, CBCGPPoint ptEnd, CBCGPDiagramConnector::Arrow arrow,
											CBCGPPointsArray& pts)
{
	pts.RemoveAll ();

	if (arrow.IsNull ())
	{
		return FALSE;
	}

	pts.SetSize (10);

    double dLineLength = bcg_distance (ptStart, ptEnd);

    double cosT = (dLineLength > DBL_EPSILON) ? bcg_clamp ((ptEnd.x - ptStart.x) / dLineLength, -1.0, 1.0) : 1.0;
    double sinT = (dLineLength > DBL_EPSILON) ? bcg_clamp ((ptEnd.y - ptStart.y) / dLineLength, -1.0, 1.0) : 0.0;

	cosT *= m_sizeScaleRatio.cx;
	sinT *= m_sizeScaleRatio.cy;

	pts[0] = ptStart;
// 	if (dOffset >= 0)
// 	{
// 		pts[0].x += dOffset * cosT;
// 		pts[0].y += dOffset * sinT;
// 	}

    pts[1].x = 0;
    pts[1].y = - arrow.m_dWidth;
	
    pts[2].x = 0;
    pts[2].y = + arrow.m_dWidth;
	
    pts[3].x = arrow.m_dLength / 2;
    pts[3].y = 0;
	
    pts[4].x = arrow.m_dLength / 2;
    pts[4].y = - arrow.m_dWidth;
	
    pts[5].x = arrow.m_dLength / 2;
    pts[5].y = + arrow.m_dWidth;
	
    pts[6].x = arrow.m_dLength;
    pts[6].y = 0;
	
    pts[7].x = arrow.m_dLength;
    pts[7].y = - arrow.m_dWidth;
	
    pts[8].x = arrow.m_dLength;
    pts[8].y = + arrow.m_dWidth;
	
    pts[9].x = arrow.m_dWidth * 2 / 3;
    pts[9].y = 0;
	
    for (int i = 1; i < 10; ++i)
    {
        double x = pts[i].x * cosT - pts[i].y * sinT;
        double y = pts[i].x * sinT + pts[i].y * cosT;
        pts[i].x = pts[0].x + x;
        pts[i].y = pts[0].y + y;
    }
	
    return TRUE;
}
//*******************************************************************************
CBCGPGeometry* CBCGPDiagramConnector::CreateArrowGeometry (const CBCGPDiagramConnector::Arrow& arrow, const CBCGPPointsArray& arPoints, CBCGPPoint& ptLineOffset)
{
	if (arPoints.GetSize () < 10)
	{
		return NULL;
	}

	CBCGPGeometry* pGeometry = NULL;
	ptLineOffset = arPoints[6] - arPoints[0];

	switch (arrow.m_nShape)
	{
    case BCGP_ARROW_TRIANGLE:
    case BCGP_ARROW_FILLEDTRIANGLE:
		{
			CBCGPComplexGeometry* pShape = new CBCGPComplexGeometry;
			pShape->SetStart (arPoints[0]);
			pShape->AddLine (arPoints[7]);
			pShape->AddLine (arPoints[8]);
			pShape->SetClosed ();
			pGeometry = pShape;
		}
		break;

    case BCGP_ARROW_STEALTH:
		{
			CBCGPComplexGeometry* pShape = new CBCGPComplexGeometry;
			pShape->SetStart (arPoints[0]);
			pShape->AddLine (arPoints[7]);
			pShape->AddLine (arPoints[9]);
			pShape->AddLine (arPoints[8]);
			pShape->SetClosed ();
			pGeometry = pShape;
			
			ptLineOffset = arPoints[9] - arPoints[0];
		}
        break;

    case BCGP_ARROW_OPEN:
		{
			CBCGPComplexGeometry* pShape = new CBCGPComplexGeometry;
			pShape->SetStart (arPoints[7]);
			pShape->AddLine (arPoints[0]);
			pShape->AddLine (arPoints[8]);
			pShape->SetClosed (FALSE);
			pGeometry = pShape;

			ptLineOffset = CBCGPPoint (0.0, 0.0);
		}
        break;

    case BCGP_ARROW_CIRCLE:
    case BCGP_ARROW_FILLEDCIRCLE:
        {
            CBCGPRect rect (arPoints[3], arPoints[3]);
            rect.InflateRect (arrow.m_dLength / 2, arrow.m_dLength / 2);
            pGeometry = new CBCGPEllipseGeometry (rect);
        }
        break;

    case BCGP_ARROW_DIAMOND:
    case BCGP_ARROW_FILLEDDIAMOND:
		{
			CBCGPComplexGeometry* pShape = new CBCGPComplexGeometry;
			pShape->SetStart (arPoints[0]);
			pShape->AddLine (arPoints[4]);
			pShape->AddLine (arPoints[6]);
			pShape->AddLine (arPoints[5]);
			pShape->SetClosed ();
			pGeometry = pShape;
		}
        break;

	default:
		ptLineOffset = CBCGPPoint (0.0, 0.0);
		break;
	}

	return pGeometry;
}
//*******************************************************************************
void CBCGPDiagramConnector::OnAdd (CBCGPDiagramVisualContainer* pContainer)
{
	ASSERT_VALID (pContainer);

	CBCGPDiagramVisualObject::OnAdd (pContainer);
	
	for (int i = 0; i < m_arPoints.GetSize (); i++)
	{
		pContainer->NotifyConnectedObject (m_arPoints[i], GetItemID (), TRUE);
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::OnRemove (CBCGPDiagramVisualContainer* pContainer)
{
	ASSERT_VALID (pContainer);

	CBCGPDiagramVisualObject::OnRemove (pContainer);

	for (int i = 0; i < m_arPoints.GetSize (); i++)
	{
		pContainer->NotifyConnectedObject (m_arPoints[i], GetItemID (), FALSE);
	}

	m_arPoints.RemoveAll ();
}
//*******************************************************************************
void CBCGPDiagramConnector::DrawConnection(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pGM);

	CBCGPPointsArray arPoints;
	
	if (m_bIsSelected)
	{
		GetTrackedPoints (arPoints);
	}
	else
	{
		GetPoints (arPoints);
	}

	arPoints.Offset (-m_ptScrollOffset);

	if (arPoints.GetSize () > 1)
	{
		arPoints [0] += m_arrowBegin.m_ptOffset;
		arPoints [arPoints.GetSize () - 1] += m_arrowEnd.m_ptOffset;
	}

	const double dLineWidth = (m_bIsSelected ? (m_Thickness + 2.0) : m_Thickness) * GetScaleRatioMid();

	if (arPoints.GetSize () <= 2)
	{
		pGM->DrawLines (arPoints, brOutline, dLineWidth, &m_StrokeStyle);
	}
	else if (m_curveType == BCGP_CURVE_TYPE_LINE)
	{
		CBCGPComplexGeometry geometry;
		geometry.SetClosed (FALSE);
		geometry.AddPoints (arPoints, CBCGPPolygonGeometry::BCGP_CURVE_TYPE_LINE);
		pGM->DrawGeometry (geometry, brOutline, dLineWidth, &m_StrokeStyle);
	}
	else
	{
		CBCGPSplineGeometry geometry(arPoints, CBCGPSplineGeometry::BCGP_SPLINE_TYPE_HERMITE, FALSE);
		pGM->DrawGeometry (geometry, brOutline, dLineWidth, &m_StrokeStyle);
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::DrawBeginArrow(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline, const CBCGPBrush& brFill)
{
	if (m_arrowBegin.IsNull ())
	{
		return;
	}

	CBCGPPointsArray arPoints;
	if ((m_bIsSelected ? GetTrackedPoints (arPoints) : GetPoints (arPoints)) < 2)
	{
		return;
	}

	arPoints.Offset (-m_ptScrollOffset);

	CBCGPPoint ptLineOffset = CBCGPPoint (0.0, 0.0);

	CBCGPPointsArray arArrowPoints;
	if (GetArrowPoints (arPoints[0], arPoints[1], m_arrowBegin, arArrowPoints))
	{
		CBCGPGeometry* pGeometry  = CreateArrowGeometry (m_arrowBegin, arArrowPoints, ptLineOffset);
		if (pGeometry != NULL)
		{
			ASSERT_VALID(pGeometry);
			
			switch (m_arrowBegin.m_nShape)
			{
			case BCGP_ARROW_STEALTH:
			case BCGP_ARROW_FILLEDTRIANGLE:
			case BCGP_ARROW_FILLEDCIRCLE:
			case BCGP_ARROW_FILLEDDIAMOND:
				pGM->FillGeometry (*pGeometry, brFill);
				break;
			}
			
			CBCGPStrokeStyle stroke (
				CBCGPStrokeStyle::BCGP_CAP_STYLE_SQUARE, CBCGPStrokeStyle::BCGP_CAP_STYLE_SQUARE,
				CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT, CBCGPStrokeStyle::BCGP_LINE_JOIN_MITER);
			pGM->DrawGeometry (*pGeometry, brOutline, m_Thickness * GetScaleRatioMid(), &stroke);
			delete pGeometry;
		}
	}

	m_arrowBegin.m_ptOffset = ptLineOffset;
}
//*******************************************************************************
void CBCGPDiagramConnector::DrawEndArrow(CBCGPGraphicsManager* pGM, const CBCGPBrush& brOutline, const CBCGPBrush& brFill)
{
	if (m_arrowEnd.IsNull ())
	{
		return;
	}

	CBCGPPointsArray arPoints;
	if ((m_bIsSelected ? GetTrackedPoints (arPoints) : GetPoints (arPoints)) < 2)
	{
		return;
	}

	arPoints.Offset (-m_ptScrollOffset);

	CBCGPPoint ptLineOffset = CBCGPPoint (0.0, 0.0);
	
	CBCGPPointsArray arArrowPoints;
	if (GetArrowPoints (arPoints[arPoints.GetSize () - 1], arPoints[arPoints.GetSize () - 2], m_arrowEnd, arArrowPoints))
	{
		CBCGPGeometry* pGeometry  = CreateArrowGeometry (m_arrowEnd, arArrowPoints, ptLineOffset);
		if (pGeometry != NULL)
		{
			ASSERT_VALID(pGeometry);

			switch (m_arrowEnd.m_nShape)
			{
			case BCGP_ARROW_STEALTH:
			case BCGP_ARROW_FILLEDTRIANGLE:
			case BCGP_ARROW_FILLEDCIRCLE:
			case BCGP_ARROW_FILLEDDIAMOND:
				pGM->FillGeometry (*pGeometry, brFill);
				break;
			}
			
			CBCGPStrokeStyle stroke (
				CBCGPStrokeStyle::BCGP_CAP_STYLE_SQUARE, CBCGPStrokeStyle::BCGP_CAP_STYLE_SQUARE,
				CBCGPStrokeStyle::BCGP_CAP_STYLE_FLAT, CBCGPStrokeStyle::BCGP_LINE_JOIN_MITER);
			pGM->DrawGeometry (*pGeometry, brOutline, m_Thickness * GetScaleRatioMid(), &stroke);
			delete pGeometry;
		}
	}

	m_arrowEnd.m_ptOffset = ptLineOffset;
}
//*******************************************************************************
void CBCGPDiagramConnector::OnDraw(CBCGPGraphicsManager* pGM, 
								   const CBCGPRect& /*rectClip*/, DWORD dwFlags)
{
	if ((dwFlags & BCGP_DRAW_STATIC) != BCGP_DRAW_STATIC)
	{
		return;
	}
	
	ASSERT_VALID (this);
	ASSERT_VALID (pGM);
	DrawBeginArrow (pGM, 
		(m_arrowBegin.m_brOutline.IsEmpty () ? m_brOutline : m_arrowBegin.m_brOutline), 
		(m_arrowBegin.m_brFill.IsEmpty () ? m_brOutline : m_arrowBegin.m_brFill));
	DrawEndArrow (pGM, 
		(m_arrowEnd.m_brOutline.IsEmpty () ? m_brOutline : m_arrowEnd.m_brOutline), 
		(m_arrowEnd.m_brFill.IsEmpty () ? m_brOutline : m_arrowEnd.m_brFill));
	DrawConnection (pGM, m_brOutline);
}
//*******************************************************************************
void CBCGPDiagramConnector::DrawTrackingRect(CBCGPGraphicsManager* pGM,
							  const CBCGPBrush& brOutline, const CBCGPBrush& brFill)
{
	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
	if (pContainer == NULL)
	{
		return;
	}

	const double scaleRatio = GetScaleRatioMid();

	for (POSITION pos = m_mapTrackRects.GetStartPosition (); pos != NULL;)
	{
		UINT uiHitTest = 0;
		CBCGPRect rect;
		
		m_mapTrackRects.GetNextAssoc (pos, uiHitTest, rect);
		
		rect.OffsetRect(-m_ptScrollOffset);
		
		int nAnchorIndex = -1;
		switch (uiHitTest)
		{
		case CBCGPDiagramVisualObject::CP_Begin:
			nAnchorIndex = 0;
			break;

		case CBCGPDiagramVisualObject::CP_End:
			nAnchorIndex = (int)m_arTrackPoints.GetSize () - 1;
			break;

		default:
			nAnchorIndex = uiHitTest - CBCGPDiagramVisualObject::CP_CustomFirst;
			break;
		}

		BOOL bLockedAnchor = FALSE;
		if (nAnchorIndex >= 0 && nAnchorIndex < m_arTrackPoints.GetSize ())
		{
			bLockedAnchor = !m_arTrackPoints[nAnchorIndex].IsNull ();
		}

		switch (uiHitTest)
		{
		case HTTOPLEFT:
		case HTBOTTOMRIGHT:
		case HTTOPRIGHT:
		case HTBOTTOMLEFT:
			pGM->FillEllipse(rect, bLockedAnchor ? pContainer->GetAnchorFillBrush () :		brFill);
			pGM->DrawEllipse(rect, bLockedAnchor ? pContainer->GetAnchorOutlineBrush () :	brOutline, scaleRatio);
			break;
			
		default:
			pGM->FillRectangle(rect, bLockedAnchor ? pContainer->GetAnchorFillBrush () :	brFill);
			pGM->DrawRectangle(rect, bLockedAnchor ? pContainer->GetAnchorOutlineBrush () :	brOutline, scaleRatio);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::SetRect(const CBCGPRect& rect, BOOL bRedraw)
{
	CBCGPRect rectOld = GetRect ();

	if (m_rect != rect)
	{
		m_rect = rect;

		if (m_sizeScaleRatio == CBCGPSize(1., 1.))
		{
			m_rectOriginal = m_rect;
		}

		if (m_bIsSelected)
		{
			m_rectTrack = rect;
		}

		SetDirty();
	}

	RecalcPoints ();

	if (m_bIsSelected)
	{
		OnEndTrackingPoints (TRUE);
	}

	SetConnectionPorts ();
	UpdateConnections (FALSE);

	CBCGPDiagramVisualContainer* pContainer = (CBCGPDiagramVisualContainer*)GetParentDiagram ();
	if (pContainer != NULL)
	{
		pContainer->FirePosSizeChangedEvent (this, rectOld);
	}

	if (m_bIsSelected)
	{
		SetSelected();
	}

	if (bRedraw)
	{
		Redraw();
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::RecalcPoints()
{
	const CBCGPRect& rect = m_bIsSelected ? m_rectTrack : m_rect;
	AnchorPointsArray& arPoints = m_bIsSelected ? m_arTrackPoints : m_arPoints;

	const int size = (int)arPoints.GetSize ();
	if (size == 0)
	{
		return;
	}

	const CBCGPRect rectOld = GetBoundsRect ();

	if (rectOld == rect)
	{
		return;
	}

	const CBCGPPoint ptOffset = rect.TopLeft () - rectOld.TopLeft ();
	const double dblAspectRatioX = (rectOld.Width () == 0.0) ? 1.0 : rect.Width () / rectOld.Width ();
	const double dblAspectRatioY = (rectOld.Height () == 0.0) ? 1.0 : rect.Height () / rectOld.Height ();

	for (int i = 0; i < size; i++)
	{
		if (arPoints[i].IsNull ())
		{
			arPoints[i].m_ptNullAnchor.Scale(CBCGPPoint (dblAspectRatioX, dblAspectRatioY), rectOld.TopLeft ());
			arPoints[i].m_ptNullAnchor.Offset (ptOffset);
		}
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::SetConnectionPorts()
{
	const CBCGPRect& rect = GetRect();
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rect.Width() < sizeMin.cx || rect.Height() < sizeMin.cy)
		{
			return;
		}
	}

	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
	if (pContainer == NULL)
	{
		return;
	}

	if (m_arPoints.GetSize () >= 2)
	{
		m_mapConnectionPorts[CP_Begin] = pContainer->CalculatePoint (m_arPoints[0]);
		m_mapConnectionPorts[CP_End] = pContainer->CalculatePoint (m_arPoints[m_arPoints.GetSize () - 1]);
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::SetSelected(BOOL bSet)
{
	if (bSet)
	{
		m_rect = GetBoundsRect ();
	}

	CBCGPBaseVisualObject::SetSelected(bSet);

	if (!bSet)
	{
		m_nTrackPointIndex = -1;
		m_arTrackPoints.RemoveAll ();
	}
}
//*******************************************************************************
void CBCGPDiagramConnector::SetTrackingRect(const CBCGPRect& rectTrack)
{
	const CBCGPSize sizeMin = GetMinSize();
	
	if (!sizeMin.IsEmpty())
	{
		if (rectTrack.Width() < sizeMin.cx || rectTrack.Height() < sizeMin.cy)
		{
			return;
		}
	}
	
	m_rectTrack = rectTrack;
	
	if ((m_uiEditFlags & BCGP_EDIT_NOSIZE) == BCGP_EDIT_NOSIZE)
	{
		return;
	}

	CBCGPDiagramVisualContainer* pContainer = DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainer, GetParentContainer ());
	if (pContainer == NULL)
	{
		return;
	}

	if (m_arTrackPoints.GetSize () == 0)
	{
		m_arTrackPoints.Append (m_arPoints);
	}

	RecalcPoints ();

	if (m_arTrackPoints.GetSize () >= 2)
	{
		m_mapTrackRects[CP_Begin] = MakeTrackMarker (pContainer->CalculatePoint (m_arTrackPoints[0]));
		m_mapTrackRects[CP_End] = MakeTrackMarker (pContainer->CalculatePoint (m_arTrackPoints[m_arTrackPoints.GetSize () - 1]));
	}

	for (int i = 1; i < m_arTrackPoints.GetSize () - 1; i++)
	{
		m_mapTrackRects[CP_CustomFirst + i] = MakeTrackMarker (pContainer->CalculatePoint (m_arTrackPoints[i]));
	}

}
//*******************************************************************************
void CBCGPDiagramConnector::CopyFrom(const CBCGPBaseVisualObject& srcObj)
{
	CBCGPDiagramVisualObject::CopyFrom(srcObj);

	const CBCGPDiagramConnector& src = (const CBCGPDiagramConnector&)srcObj;

	m_curveType = src.m_curveType;
	m_arrowBegin.CopyFrom (src.m_arrowBegin);
	m_arrowEnd.CopyFrom (src.m_arrowEnd);

	CBCGPPointsArray arPoints;
	src.GetPoints (arPoints);

	for (int i = 0; i < arPoints.GetSize (); i++)
	{
		InsertPoint (i, arPoints[i]);
	}
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramGroupObject

CBCGPDiagramGroupObject::CBCGPDiagramGroupObject(CBCGPVisualContainer* pContainer)
	: CBCGPDiagramVisualObject (pContainer)
{
}

CBCGPDiagramGroupObject::CBCGPDiagramGroupObject(const CBCGPDiagramConnector& src)
	: CBCGPDiagramVisualObject (src)
{
}

CBCGPDiagramGroupObject::~CBCGPDiagramGroupObject()
{
}

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramTextDataObject

CBCGPDiagramTextDataObject::CBCGPDiagramTextDataObject(LPCTSTR lpszText, const CBCGPColor& color)
{
	SetText (CString(lpszText), color);
}
//*******************************************************************************
CBCGPDiagramTextDataObject::CBCGPDiagramTextDataObject(LPCTSTR lpszText, const CBCGPBrush& brush)
{
	SetText (lpszText, brush);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetText(LPCTSTR lpszText, BOOL bRedraw)
{
	m_strText = lpszText;

	if (m_pParentVisual != NULL)
	{
		m_pParentVisual->SetDirty();

		if (bRedraw)
		{
			m_pParentVisual->Redraw();
		}
	}
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetText(const CString& str, const CBCGPColor& clrText, BOOL bRedraw)
{
	CBCGPBrush brText(clrText);
	SetText(str, brText, bRedraw);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetText(const CString& str, const CBCGPBrush& brText, BOOL bRedraw)
{
	m_strText = str;
	m_brText = brText;	

	CreateResources ();

	if (m_pParentVisual != NULL)
	{
		m_pParentVisual->SetDirty();

		if (bRedraw)
		{
			m_pParentVisual->Redraw();
		}
	}
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetTextColor(const CBCGPColor& color)
{
	CBCGPBrush brush;
	if (color != CBCGPColor())
	{
		brush = CBCGPBrush(color);
	}

	SetTextBrush(brush);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetTextBrush(const CBCGPBrush& brush)
{
	if (!brush.IsEmpty())
	{
		m_brText = brush;	
	}

	CreateResources();
	
	if (m_pParentVisual != NULL)
	{
		m_pParentVisual->SetDirty();
	}
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetTextFormat(const CBCGPTextFormat& textFormat)
{
	m_textFormat = textFormat;

	if (m_pParentVisual != NULL)
	{
		m_pParentVisual->SetDirty();
	}
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetSize (const CBCGPSize& size)
{
	m_size = size;
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetFontScale (double dScaleRatio)
{
	m_textFormat.Scale (dScaleRatio);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::SetDefaultTextFormat(CBCGPTextFormat& tf)
{
	LOGFONT lf;
	globalData.fontBold.GetLogFont(&lf);
	
	tf.CreateFromLogFont(lf);
	
	tf.SetTextAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
	tf.SetTextVerticalAlignment(CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_CENTER);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::CreateResources()
{
	if (m_textFormat.GetFontFamily().IsEmpty())
	{
		SetDefaultTextFormat(m_textFormat);
	}
}
//*******************************************************************************
CBCGPSize CBCGPDiagramTextDataObject::GetDefaultSize(CBCGPGraphicsManager* pGM, const CBCGPDiagramVisualObject* pParentObject)
{
	ASSERT_VALID(pGM);
	
	CreateResources();

	double dblWidth = 0.;
	if (m_textFormat.IsWordWrap())
	{
		if (pParentObject != NULL)
		{
			ASSERT_VALID(pParentObject);
			dblWidth = pParentObject->GetRect ().Width () - 2 * BCGP_DIAGRAM_TEXT_PADDING * pParentObject->GetScaleRatio().cx;
		}

		if (dblWidth <= 0.)
		{
			CBCGPTextFormat textFormat;
			textFormat.CopyFrom (m_textFormat);
			textFormat.SetWordWrap (FALSE);
			return pGM->GetTextSize(m_strText, textFormat);
		}
	}

	return pGM->GetTextSize(m_strText, m_textFormat, dblWidth);
}
//*******************************************************************************
void CBCGPDiagramTextDataObject::Draw(CBCGPGraphicsManager* pGM, const CBCGPRect& rect, const CBCGPRect& /*rectClip*/)
{
	ASSERT_VALID(pGM);

	if (rect.IsRectEmpty() || m_strText.IsEmpty())
	{
		return;
	}

	CreateResources();

	CBCGPTextFormat tf = m_textFormat;
	CBCGPSize size = pGM->GetTextSize (m_strText, tf, rect.Width ());

	if (size.cx > rect.Width ())
	{
		tf.SetTextAlignment (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	}
	else if (size.cy > rect.Height ())
	{
		tf.SetTextVerticalAlignment (CBCGPTextFormat::BCGP_TEXT_ALIGNMENT_LEADING);
	}

	tf.SetClipText (TRUE);
	pGM->DrawText(m_strText, rect, tf, m_brText);
}
//*******************************************************************************
CString CBCGPDiagramTextDataObject::ToString () const
{
	return m_strText;
}
//*******************************************************************************
BOOL CBCGPDiagramTextDataObject::ParseString (const CString& str)
{
	m_strText = str;
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPDiagramTextDataObject::ValidateString (const CString&)
{
	return TRUE;
}
