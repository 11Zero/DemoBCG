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
// BCGPDiagramVisualContainer.h: interface for the CBCGPDiagramVisualContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDIAGRAMVISUALCONTAINER_H__D2AF8324_E7CC_4EF5_97D6_41CC2B92F0D0__INCLUDED_)
#define AFX_BCGPDIAGRAMVISUALCONTAINER_H__D2AF8324_E7CC_4EF5_97D6_41CC2B92F0D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualContainer.h"
#include "BCGPDiagramVisualObject.h"

/////////////////////////////////////////////////////////////////////////////
// BCGP_DIAGRAM_ITEM_INFO struct, 
// used for BCGM_DIAGRAM_ITEM_CHANGED, BCGM_DIAGRAM_BEGININPLACEEDIT, BCGM_DIAGRAM_ENDINPLACEEDIT messages

struct BCGCBPRODLLEXPORT BCGP_DIAGRAM_ITEM_INFO
{
	CBCGPDiagramVisualObject* pItem;	// An item
	CBCGPDiagramItemID	idItem;			// ID of an item
    int					nDataIndex;		// Index of the data object
	DWORD				dwResultCode;	// Message specific
};

BCGCBPRODLLEXPORT extern UINT BCGM_DIAGRAM_ITEM_CHANGED;
BCGCBPRODLLEXPORT extern UINT BCGM_DIAGRAM_BEGININPLACEEDIT;
BCGCBPRODLLEXPORT extern UINT BCGM_DIAGRAM_ENDINPLACEEDIT;

BCGCBPRODLLEXPORT extern UINT BCGM_DIAGRAM_POS_SIZE_CHANGED;

//////////////////////////////////////////////////////////////////////
// CBCGPDiagramVisualContainer

class CBCGPDiagramItemID;
class CBCGPDiagramAnchorPoint;

class BCGCBPRODLLEXPORT CBCGPDiagramVisualContainer : public CBCGPVisualContainer  
{
	DECLARE_DYNCREATE(CBCGPDiagramVisualContainer)

public:
// Construction
	CBCGPDiagramVisualContainer(CWnd* pWndOwner = NULL);
	virtual ~CBCGPDiagramVisualContainer();

// Operations:
	// Items and connectors:
	CBCGPDiagramItemID AddItem (CBCGPBaseVisualObject* pObject, BOOL bAutoDestroy = TRUE);
	CBCGPBaseVisualObject* GetItem (CBCGPDiagramItemID id) const;
	CBCGPDiagramItemID AddConnector (CBCGPDiagramConnector* pObject, BOOL bAutoDestroy = TRUE);
	CBCGPDiagramConnector* GetConnector (CBCGPDiagramItemID id) const;

	BOOL Remove (CBCGPDiagramItemID id, BOOL bRebuildContainer = FALSE);
	void RemoveAllConnectors (BOOL bRebuildContainer = FALSE);
	virtual void RemoveAll();

	void UpdateItems (CItemIDList& lstItems, BOOL bRedraw);

	virtual CBCGPDiagramVisualObject* CreateCustomObject(const CString& /*strName*/) const {	return NULL;	}

	struct HitTestAnchorOptions
	{
		HitTestAnchorOptions () { bIgnoreSelection = FALSE; }

		BOOL bIgnoreSelection;
	};

	// Anchors:
	CBCGPPoint CalculatePoint (const CBCGPDiagramAnchorPoint& anchor) const;
	CBCGPDiagramAnchorPoint NewAnchorFromPoint (CBCGPPoint pt, HitTestAnchorOptions* pOptions = NULL) const;
	UINT HitTestAnchorPoint (CBCGPPoint pt, CBCGPDiagramAnchorPoint& anchor, CBCGPDiagramVisualObject*& pOwnerObj) const;

	virtual void NotifyConnectedObject (const CBCGPDiagramAnchorPoint& anchorObject, const CBCGPDiagramItemID& idConnector, BOOL bAdd);
	virtual void NotifyConnectorOnDisconnect (const CBCGPDiagramItemID& idObjectToDisconnect, const CBCGPDiagramItemID& idConnector);

	const CBCGPBrush& GetAnchorFillBrush () const { return m_brAnchorFill; }
	const CBCGPBrush& GetAnchorOutlineBrush () const { return m_brAnchorOutline; }

//	// New objects:
// 	virtual CBCGPDiagramConnector* OnStartAddNewConnector(const CBCGPPoint& /*pt*/) { return NULL; }
// 	virtual BOOL OnFinishAddNewConnector(CBCGPDiagramConnector* /*pObj*/, const CBCGPRect& /*rect*/) { return TRUE; }

	// In-place edit:
	BOOL EnableInplaceEdit (BOOL bEnable = TRUE);
	BOOL IsInplaceEditEnabled () const
	{
		return m_bEnableInplaceEdit;
	}

	virtual BOOL EditItem (CBCGPDiagramVisualObject* pItem, const CBCGPPoint* lptClick = NULL);
	virtual BOOL EndEditItem (BOOL bUpdateData = TRUE);
	BOOL IsInplaceEdit () const;
	CWnd* GetInplaceEditWnd ();

	virtual BOOL CanEndInplaceEditOnChar (UINT nChar, UINT nRepCnt, UINT nFlags) const;
	virtual BOOL OnInplaceEditKeyDown (MSG* pMsg);
	virtual void OnItemChanged (CBCGPDiagramVisualObject* pItem, int nDataObject);

	// Layout:
	virtual void FirePosSizeChangedEvent(CBCGPBaseVisualObject* pObject, CRect rectOld);

// Implementation:
protected:
	virtual void MoveTrackingPoints(CBCGPPoint pt);
	virtual void OnRemove (CBCGPBaseVisualObject* pObject);
	virtual void OnAdd (CBCGPBaseVisualObject* pObject);

	CBCGPDiagramItemID NewId (BOOL bConnector);
	void RebuildContainer ();

// Overrides:
public:
	virtual BOOL OnMouseDown(int nButton, const CBCGPPoint& pt);
	virtual void OnMouseUp(int nButton, const CBCGPPoint& pt);
	virtual BOOL OnMouseDblClick(int /*nButton*/, const CBCGPPoint& /*pt*/);
	virtual void OnMouseMove(const CBCGPPoint& pt);
	virtual void OnMouseLeave();
	virtual void OnCancelMode();
	virtual BOOL OnKeyboardDown(UINT nChar, UINT nRepCnt, UINT nFlags);

// Attributes:
protected:
	CMap<int, int, CBCGPBaseVisualObject*, CBCGPBaseVisualObject*> m_mapItems;		// nodes
	CMap<int, int, CBCGPDiagramConnector*, CBCGPDiagramConnector*> m_mapConnectors;	// edges
	int	m_nLastID;

	BOOL m_bIsEditAnchorMode;

	BOOL m_bEnableInplaceEdit;
	CBCGPDiagramVisualObject*	m_pInplaceEditItem;

	CBCGPBrush					m_brAnchorFill;
	CBCGPBrush					m_brAnchorOutline;

private:
	BOOL	m_bInsideUpdateItems;
};

#endif // !defined(AFX_BCGPDIAGRAMVISUALCONTAINER_H__D2AF8324_E7CC_4EF5_97D6_41CC2B92F0D0__INCLUDED_)
