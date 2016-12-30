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
// BCGPDiagramVisualContainer.cpp: implementation of the CBCGPDiagramVisualContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPMath.h"
#include "BCGPDiagramVisualContainer.h"
#include "BCGPDiagramVisualObject.h"
#include "BCGPDiagramShape.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualContainer notification messages:

UINT BCGM_DIAGRAM_ITEM_CHANGED = ::RegisterWindowMessage (_T("BCGM_DIAGRAM_ITEM_CHANGED"));
UINT BCGM_DIAGRAM_BEGININPLACEEDIT = ::RegisterWindowMessage (_T("BCGM_DIAGRAM_BEGININPLACEEDIT"));
UINT BCGM_DIAGRAM_ENDINPLACEEDIT = ::RegisterWindowMessage (_T("BCGM_DIAGRAM_ENDINPLACEEDIT"));
UINT BCGM_DIAGRAM_POS_SIZE_CHANGED = ::RegisterWindowMessage (_T("BCGM_DIAGRAM_POS_SIZE_CHANGED"));

IMPLEMENT_DYNCREATE(CBCGPDiagramVisualContainer, CBCGPVisualContainer)

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualContainer

CBCGPDiagramVisualContainer::CBCGPDiagramVisualContainer(CWnd* pWndOwner) 
	: CBCGPVisualContainer (pWndOwner)
{
	m_nLastID = 0;

	m_bIsEditAnchorMode = FALSE;

	m_bEnableInplaceEdit = FALSE;
	m_pInplaceEditItem = NULL;

	m_brAnchorFill = CBCGPBrush(CBCGPColor::DarkOrange, .5);
	m_brAnchorOutline = CBCGPBrush(CBCGPColor::Firebrick);

	m_bInsideUpdateItems = FALSE;
}

CBCGPDiagramVisualContainer::~CBCGPDiagramVisualContainer()
{
	EndEditItem (FALSE);

	RemoveAllConnectors ();
}
//*******************************************************************************
CBCGPDiagramItemID CBCGPDiagramVisualContainer::NewId (BOOL bConnector)
{
	int nId = ++m_nLastID;
	CBCGPBaseVisualObject* pObject;
	CBCGPDiagramConnector* pConnector;

	while (bConnector ? m_mapConnectors.Lookup (nId, pConnector) : m_mapItems.Lookup (nId, pObject))
	{
		nId = ++m_nLastID;
	}

	CBCGPDiagramItemID id;
	id.m_nId = nId;
	id.m_bConnector = bConnector;

	return id;
}
//*******************************************************************************
CBCGPDiagramItemID CBCGPDiagramVisualContainer::AddItem (CBCGPBaseVisualObject* pObject, BOOL bAutoDestroy)
{
	ASSERT_VALID (this);

	if (!CBCGPVisualContainer::Add (pObject, bAutoDestroy))
	{
		return CBCGPDiagramItemID ();
	}
	
	CBCGPDiagramVisualObject* pDiagramItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pObject);
	if (pDiagramItem != NULL)
	{
		return pDiagramItem->GetItemID ();
	}

	return CBCGPDiagramItemID ();
}
//*******************************************************************************
CBCGPDiagramItemID CBCGPDiagramVisualContainer::AddConnector (CBCGPDiagramConnector* pObject, BOOL bAutoDestroy)
{
	ASSERT_VALID (this);
	
	if (!CBCGPVisualContainer::Add (pObject, bAutoDestroy))
	{
		return CBCGPDiagramItemID ();
	}

	return pObject->GetItemID ();
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnAdd (CBCGPBaseVisualObject* pObject)
{
	CBCGPVisualContainer::OnAdd (pObject);

	CBCGPDiagramConnector* pConnector = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, pObject);
	if (pConnector != NULL)
	{
		CBCGPDiagramItemID id = NewId (TRUE);
		pConnector->SetItemID (id);
		m_mapConnectors.SetAt (id.m_nId, pConnector);

		pConnector->OnAdd (this);

		pConnector->SetRect (pConnector->GetBoundsRect ());
		return;
	}

	CBCGPDiagramVisualObject* pItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pObject);
	if (pItem != NULL)
	{
 		CBCGPDiagramItemID id = pItem->GetItemID ();

		if (id.IsNull ())
		{
			id = NewId (FALSE);
		}

		pItem->SetItemID (id);
		m_mapItems.SetAt (id.m_nId, pItem);

		pItem->OnAdd (this);
		return;
	}
}
//*******************************************************************************
CBCGPBaseVisualObject* CBCGPDiagramVisualContainer::GetItem (CBCGPDiagramItemID id) const
{
	if (id.m_bConnector)
	{
		return GetConnector (id);
	}
	else
	{
		CBCGPBaseVisualObject* pObject = NULL;
		if (m_mapItems.Lookup (id.m_nId, pObject))
		{
			ASSERT_VALID(pObject);
			return pObject;
		}
	}

	return NULL;
}
//*******************************************************************************
CBCGPDiagramConnector* CBCGPDiagramVisualContainer::GetConnector (CBCGPDiagramItemID id) const
{
	if (id.m_bConnector)
	{
		CBCGPDiagramConnector* pConnector = NULL;
		if (m_mapConnectors.Lookup (id.m_nId, pConnector))
		{
			ASSERT_VALID(pConnector);
			return pConnector;
		}
	}

	return NULL;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::Remove (CBCGPDiagramItemID id, BOOL bRebuildContainer)
{
	CBCGPBaseVisualObject* pObject = GetItem (id);
	if (pObject == NULL)
	{
		return FALSE;
	}

	ASSERT_VALID(pObject);

	int nIndex = FindIndex (pObject);
	if (nIndex >= 0 && nIndex < (int) m_arObjects.GetSize ())
	{
		m_arObjects[nIndex] = NULL;
	}

	OnRemove (pObject);

	if (pObject->IsAutoDestroy ())
	{
		delete pObject;
	}

	if (bRebuildContainer)
	{
		RebuildContainer ();
	}

	AdjustLayout();
	return TRUE;
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::RemoveAllConnectors (BOOL bRebuildContainer)
{
	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_arObjects[i]);
		if (pObject == NULL)
		{
			continue;
		}
		
		OnRemove (pObject);

		if (pObject->IsAutoDestroy ())
		{
			delete pObject;
		}
		else
		{
			pObject->SetParentContainer (NULL);
		}

		m_arObjects[i] = NULL;
	}

	m_mapConnectors.RemoveAll ();

	if (bRebuildContainer)
	{
		RebuildContainer ();
	}

	AdjustLayout();
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnRemove (CBCGPBaseVisualObject* pObject)
{
	ASSERT_VALID(pObject);

	CBCGPDiagramVisualObject* pDiagramItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pObject);
	if (pDiagramItem != NULL)
	{
		const CBCGPDiagramItemID& id = pDiagramItem->GetItemID ();

		pDiagramItem->OnRemove (this);

		if (id.m_bConnector)
		{
			m_mapConnectors.RemoveKey (id.m_nId);
		}
		else
		{
			m_mapItems.RemoveKey (id.m_nId);
		}
	}

	CBCGPVisualContainer::OnRemove (pObject);
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::RemoveAll()
{
	EndEditItem (FALSE);

	RemoveAllConnectors ();
	m_mapItems.RemoveAll ();
	m_mapConnectors.RemoveAll ();
	m_nLastID = 0;

	CBCGPVisualContainer::RemoveAll ();
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::UpdateItems (CItemIDList& lst, BOOL bRedraw)
{
	if (m_bInsideUpdateItems)
	{
// 		for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
// 		{
// 			CBCGPBaseVisualObject* pObject = GetItem (lst.GetNext (pos));
// 			if (pObject != NULL)
// 			{
// 				ASSERT_VALID(pObject);
// 				pObject->SetDirty (TRUE, bRedraw);
// 			}
// 		}
		return;
	}

	m_bInsideUpdateItems = TRUE;

	for (POSITION pos = lst.GetHeadPosition (); pos != NULL; )
	{
		CBCGPBaseVisualObject* pObject = GetItem (lst.GetNext (pos));
		CBCGPDiagramConnector* pConnector = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, pObject);

		if (pConnector != NULL)
		{
			pConnector->SetRect (pConnector->GetBoundsRect (), bRedraw);
		}
		else if (pObject != NULL)
		{
			ASSERT_VALID(pObject);
			pObject->SetDirty (TRUE, bRedraw);
		}
	}

	m_bInsideUpdateItems = FALSE;
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::RebuildContainer ()
{
	ASSERT_VALID (this);

	//--------------
	// Create a copy
	//--------------
	CArray<CBCGPBaseVisualObject*, CBCGPBaseVisualObject*>	arNewObjects;

	int i;
	for (i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPBaseVisualObject* pObject = m_arObjects[i];
		if (pObject != NULL)
		{
			arNewObjects.Add (pObject);
		}
	}
	
	//--------
	// Replace
	//--------
	m_arObjects.RemoveAll ();
	m_arObjects.Append (arNewObjects);
}
//*******************************************************************************
CBCGPPoint CBCGPDiagramVisualContainer::CalculatePoint (const CBCGPDiagramAnchorPoint& anchor) const
{
	if (anchor.IsNull ())
	{
		return anchor.m_ptNullAnchor;
	}

	CBCGPBaseVisualObject* pObject = GetItem (anchor.m_idObject);

	CBCGPDiagramVisualObject* pDiagramItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pObject);
	if (pDiagramItem != NULL)
	{
		CBCGPPoint pt;
		if (pDiagramItem->GetConnectionPort (anchor.m_nConnectionPort, pt))
		{
			pt.Offset (-pDiagramItem->GetDrawOffset ());
			return pt;
		}
	}

	CBCGPPoint pt;
	if (pObject != NULL)
	{
		ASSERT_VALID (pObject);
		pt = pObject->GetRect ().CenterPoint ();
		pt.Offset (-pObject->GetDrawOffset ());
	}

	return pt;
}
//*******************************************************************************
CBCGPDiagramAnchorPoint CBCGPDiagramVisualContainer::NewAnchorFromPoint (CBCGPPoint pt, HitTestAnchorOptions* pOptions) const
{
	BOOL bIgnoreSelection = FALSE;

	if (pOptions != NULL)
	{
		bIgnoreSelection = pOptions->bIgnoreSelection;
	}

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPDiagramVisualObject* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, m_arObjects[i]);
		if (pObject != NULL)
		{
			if (bIgnoreSelection && m_lstSel.Find (pObject) != NULL)
			{
				continue;
			}

			UINT uiConPoint = pObject->HitTestConnectionPort (pt);
			
			if (uiConPoint != CBCGPDiagramVisualObject::CP_None)
			{
				return pObject->UseConnectionPort (uiConPoint, &pt);
			}
		}
	}
	
	return CBCGPDiagramAnchorPoint::NullAnchor (pt);
}
//*******************************************************************************
UINT CBCGPDiagramVisualContainer::HitTestAnchorPoint (CBCGPPoint pt, CBCGPDiagramAnchorPoint& anchor, CBCGPDiagramVisualObject*& pOwnerObj) const
{
	ASSERT_VALID (this);

	for (int i = 0; i < m_arObjects.GetSize(); i++)
	{
		CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_arObjects[i]);
		if (pObject != NULL)
		{
			int nPointIndex = -1;
			UINT uiHitTest = pObject->HitTestAnchorPoint (pt, nPointIndex);
			
			if (nPointIndex >= 0 && nPointIndex < pObject->GetPointCount ())
			{
				anchor = pObject->AnchorPoint (nPointIndex);
				pOwnerObj = pObject;
				return uiHitTest;
			}
		}
	}

	anchor = CBCGPDiagramAnchorPoint::NullAnchor (pt);
	pOwnerObj = NULL;
	return HTNOWHERE;
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::NotifyConnectedObject (const CBCGPDiagramAnchorPoint& anchorObject, const CBCGPDiagramItemID& idConnector, BOOL bAdd)
{
	if (anchorObject.IsNull ())
	{
		return;
	}
	
	CBCGPDiagramVisualObject* pDiagramItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, GetItem (anchorObject.m_idObject));
	if (pDiagramItem != NULL)
	{
		if (bAdd)
		{
			pDiagramItem->OnConnectionAdded (
				anchorObject.m_nConnectionPort, idConnector, CBCGPDiagramItemID ());
		}
		else
		{
			pDiagramItem->OnConnectionRemoved (
				anchorObject.m_nConnectionPort, idConnector, CBCGPDiagramItemID ());
		}
	}
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::NotifyConnectorOnDisconnect (const CBCGPDiagramItemID& idObjectToDisconnect, const CBCGPDiagramItemID& idConnector)
{
	CBCGPDiagramConnector* pConnector = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, GetConnector (idConnector));
	if (pConnector != NULL)
	{
		//pConnector->DisconnectFrom (idObjectToDisconnect);
		for (int i = 0; i < pConnector->GetPointCount (); i++)
		{
			CBCGPDiagramAnchorPoint& ptAnchor = pConnector->AnchorPoint (i);

			if (idObjectToDisconnect == ptAnchor.m_idObject)
			{
				ptAnchor = CBCGPDiagramAnchorPoint::NullAnchor (CalculatePoint (ptAnchor));
			}
		}
	}
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::OnMouseDown(int nButton, const CBCGPPoint& pt)
{
	EndEditItem(TRUE);

	if (m_bIsEditMode)
	{
		//----------------------------
		// HitTest existing connectors
		//----------------------------
		if (nButton == 0 && !m_bAddNewObjectMode)
		{
			CBCGPDiagramAnchorPoint anchor;
			CBCGPDiagramVisualObject* pOwnerObject = NULL;
			UINT uiHitTest = HitTestAnchorPoint (pt, anchor, pOwnerObject);

			CBCGPDiagramConnector* pConnector = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, pOwnerObject);
			if (pConnector != NULL)
			{
				Select (pConnector);
				pConnector->BeginTrackAnchorPoint (anchor);
				pConnector->Redraw ();
				
				CBCGPBaseVisualObject* pObject = GetItem (anchor.m_idObject);
				if (pObject != NULL)
				{
					pObject->Redraw();
				}

				m_bIsEditAnchorMode = TRUE;	
				m_nDragMode = uiHitTest;		
				
				m_ptDragStart = m_ptDragFinish = pt;
				
				Redraw();		
				return TRUE;
			}
		}

		//------------------
		// Add new connector
		//------------------
		else if (nButton == 0 && m_bAddNewObjectMode)
		{
// 			CBCGPDiagramAnchorPoint anchor = NewAnchorFromPoint (pt);
// 
// 			m_pNewObject = OnStartAddNewConnector (pt);
// 
// 			if (m_pNewObject != NULL)
// 			{
// 				m_pNewObject->SetRect(CBCGPRect(pt, CBCGPSize(1., 1.)));
// 			}
// 
// 			Redraw();
// 			
// 			return TRUE;
		}

	}

	return CBCGPVisualContainer::OnMouseDown (nButton, pt);
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnMouseUp(int nButton, const CBCGPPoint& pt)
{
	if (m_bIsEditAnchorMode && m_nDragMode != HTNOWHERE)
	{
		m_bIsEditAnchorMode = FALSE;

		for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
		{
			CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_lstSel.GetNext(pos));
			if (pObject == NULL)
			{
				continue;
			}

			if (pObject->IsTrackingAnchorPoints ())
			{
				pObject->EndTrackAnchorPoint ();
				pObject->Redraw ();
			}
		}

		m_ptDragStart = m_ptDragFinish = CBCGPPoint(-1, -1);
		Redraw();
		return;
	}

	CBCGPVisualContainer::OnMouseUp (nButton, pt);
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::OnMouseDblClick(int nButton, const CBCGPPoint& pt)
{
	if (!IsInplaceEditEnabled ())
	{
		return FALSE;
	}

	CBCGPBaseVisualObject* pObject = GetFromPoint(pt);
	if (pObject != NULL)
	{
		ASSERT_VALID(pObject);
		if (pObject->OnMouseDblClick (nButton, pt))
		{
			return TRUE;
		}

		CBCGPDiagramVisualObject* pDiagramItem = DYNAMIC_DOWNCAST(CBCGPDiagramVisualObject, pObject);
		if (pDiagramItem != NULL)
		{
			return EditItem (pDiagramItem, &pt);
		}
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnMouseMove(const CBCGPPoint& pt)
{
	if (m_bIsEditAnchorMode && m_nDragMode != HTNOWHERE)
	{
		MoveTrackingPoints (pt);

		m_ptDragStart = pt;

		m_ptDragFinish = pt;

		SetCursor (AfxGetApp ()->LoadStandardCursor (IDC_CROSS));
		Redraw ();
		return;
	}

	CBCGPVisualContainer::OnMouseMove (pt);
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::MoveTrackingPoints(CBCGPPoint pt)
{
//	CBCGPPoint ptOffset = pt - m_ptDragStart;

	for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
	{
		CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_lstSel.GetNext(pos));
		if (pObject == NULL)
		{
			continue;
		}
		
		if (pObject->IsTrackingAnchorPoints ())
		{
			CBCGPDiagramAnchorPoint anchorOld = pObject->GetTrackedAnchorPoint ();

			HitTestAnchorOptions options;
			options.bIgnoreSelection = TRUE;

			if (pObject->SetTrackedAnchorPoint (NewAnchorFromPoint (pt + m_ptScrollOffset, &options)))
			{
				CBCGPBaseVisualObject* pOldConnectedObject = GetItem (anchorOld.m_idObject);
				if (pOldConnectedObject != NULL)
				{
					pOldConnectedObject->Redraw ();
				}
				
				CBCGPBaseVisualObject* pNewConnectedObject = GetItem (pObject->GetTrackedAnchorPoint ().m_idObject);
				if (pNewConnectedObject != NULL)
				{
					pNewConnectedObject->Redraw();
				}
			}

			pObject->Redraw ();
		}
	}
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnMouseLeave()
{
	for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
	{
		CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_lstSel.GetNext(pos));
		if (pObject == NULL)
		{
			continue;
		}

		if (pObject->IsTrackingAnchorPoints ())
		{
			pObject->EndTrackAnchorPoint (FALSE);
			pObject->Redraw ();
		}
	}

	m_bIsEditAnchorMode = FALSE;

	CBCGPVisualContainer::OnMouseLeave ();
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnCancelMode()
{
	for (POSITION pos = m_lstSel.GetHeadPosition(); pos != NULL;)
	{
		CBCGPDiagramConnector* pObject = DYNAMIC_DOWNCAST(CBCGPDiagramConnector, m_lstSel.GetNext(pos));
		if (pObject == NULL)
		{
			continue;
		}
		
		if (pObject->IsTrackingAnchorPoints ())
		{
			pObject->EndTrackAnchorPoint (FALSE);
			pObject->Redraw ();
		}
	}

	m_bIsEditAnchorMode = FALSE;

	EndEditItem (FALSE);
	
	CBCGPVisualContainer::OnCancelMode ();
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE)
	{
		CBCGPVisualContainer::OnKeyboardDown(nChar, nRepCnt, nFlags);

		EndEditItem (FALSE);
		return TRUE;
	}

	return CBCGPVisualContainer::OnKeyboardDown(nChar, nRepCnt, nFlags);
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::EnableInplaceEdit (BOOL bEnable)
{
	m_bEnableInplaceEdit = bEnable;
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::EditItem (CBCGPDiagramVisualObject* pItem, const CBCGPPoint* pptClick)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (!EndEditItem ())
	{
		return FALSE;
	}

	if (!pItem->OnEdit (pptClick))
	{
		return FALSE;
	}

	m_pInplaceEditItem = pItem;

	pItem->Redraw ();

	if (GetOwner ()->GetSafeHwnd () != NULL)
	{
		GetOwner ()->SetCapture ();
	}

	return TRUE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::EndEditItem (BOOL bUpdateData)
{
	ASSERT_VALID (this);

	if (m_pInplaceEditItem == NULL)
	{
		return TRUE;
	}

	ASSERT_VALID (m_pInplaceEditItem);

	if (bUpdateData)
	{
		if (!m_pInplaceEditItem->ValidateInPlaceEdit () || !m_pInplaceEditItem->UpdateData ())
		{
			return FALSE;
		}
	}

	CBCGPDiagramVisualObject* pRedrawItem = m_pInplaceEditItem;

	// Close in-place edit
	if (!m_pInplaceEditItem->OnEndEdit ())
	{
		return FALSE;
	}

	m_pInplaceEditItem = NULL;

	if (::GetCapture () == GetOwner ()->GetSafeHwnd ())
	{
		ReleaseCapture ();
	}
	
	pRedrawItem->Redraw ();
	
	return TRUE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::IsInplaceEdit () const
{
	return (m_pInplaceEditItem != NULL);
}
//*******************************************************************************
CWnd* CBCGPDiagramVisualContainer::GetInplaceEditWnd ()
{
	ASSERT_VALID (this);
	
	if (m_pInplaceEditItem == NULL)
	{
		return NULL;
	}

	ASSERT_VALID(m_pInplaceEditItem);

	return m_pInplaceEditItem->GetInPlaceEditWnd ();
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::CanEndInplaceEditOnChar (UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) const
{
	switch (nChar)
	{
	case VK_RETURN:
		{
			const BOOL bShift = (::GetAsyncKeyState (VK_SHIFT) & 0x8000) != 0;
			const BOOL bCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000) != 0;
			
			return (!bShift && !bCtrl); // If SHIFT or CONTROL - continue edit
		}
	}

	return FALSE;
}
//*******************************************************************************
BOOL CBCGPDiagramVisualContainer::OnInplaceEditKeyDown (MSG* pMsg)
{
	ASSERT_VALID (this);
	ASSERT (pMsg != NULL);

	const BOOL bCanEndEdit = CanEndInplaceEditOnChar ((UINT)pMsg->wParam, LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));

	switch (pMsg->wParam)
	{
	case VK_RETURN:
		if (bCanEndEdit)
		{
			if (!EndEditItem ())
			{
				MessageBeep ((UINT)-1);
			}
			else
			{
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::OnItemChanged (CBCGPDiagramVisualObject* pItem, int nDataObject)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pItem);

	if (GetOwner ()->GetSafeHwnd () == NULL)
	{
		return;
	}

	BCGP_DIAGRAM_ITEM_INFO ii;
	memset (&ii, 0, sizeof (BCGP_DIAGRAM_ITEM_INFO));
	ii.pItem = pItem;
	ii.idItem = pItem->GetItemID ();
	ii.nDataIndex = nDataObject;
	ii.dwResultCode = 0;

	GetOwner ()->PostMessage (BCGM_DIAGRAM_ITEM_CHANGED, GetOwner ()->GetDlgCtrlID (), LPARAM (&ii));
}
//*******************************************************************************
void CBCGPDiagramVisualContainer::FirePosSizeChangedEvent(CBCGPBaseVisualObject* /*pObject*/, CRect /*rectOld*/)
{
	if (m_pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	CWnd* pWndOwner = m_pWndOwner->GetOwner();
	if (pWndOwner->GetSafeHwnd() == NULL)
	{
		return;
	}

	pWndOwner->SendMessage(BCGM_DIAGRAM_POS_SIZE_CHANGED, (WPARAM)this, (LPARAM)0);
}
//*******************************************************************************
