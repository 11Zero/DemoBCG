//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of BCGControlBar Library Professional Edition
// Copyright (C) 1998-2014 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************
//
// BCGBarContainer.h: interface for the CBCGPBarContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPBARCONTAINER_H__12D1AE1B_A125_4319_B94F_1A4F0C396BFD__INCLUDED_)
#define AFX_BCGPBARCONTAINER_H__12D1AE1B_A125_4319_B94F_1A4F0C396BFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"

class CBCGPBaseControlBar;
class CBCGPControlBar;
class CBCGPDockingControlBar;
class CBCGPSlider;


class BCGCBPRODLLEXPORT CBCGPBarContainer : public CObject  
{
	friend class CBCGPBarContainerManager;
	friend class CBCGPRecentContainerInfo;

	DECLARE_DYNAMIC (CBCGPBarContainer);
public:
	CBCGPBarContainer  (CBCGPBarContainerManager* pManager = NULL,
						CBCGPDockingControlBar* pLeftBar = NULL, 
						CBCGPDockingControlBar* pRightBar = NULL, 
					    CBCGPSlider* pSlider = NULL);
	virtual ~CBCGPBarContainer();

	CBCGPDockingControlBar* AddControlBar (CBCGPDockingControlBar* pBar);	
	BOOL AddSubContainer (CBCGPBarContainer* pContainer, BOOL bRightNodeNew);

	enum BC_FIND_CRITERIA
	{
		BC_FIND_BY_LEFT_BAR,
		BC_FIND_BY_RIGHT_BAR,
		BC_FIND_BY_SLIDER,
		BC_FIND_BY_CONTAINER
	};

	CBCGPBarContainer* FindSubContainer (const CObject* pObject, 
										 BC_FIND_CRITERIA findCriteria);

	BOOL IsContainerEmpty () const;
	BOOL IsLeftPartEmpty (BOOL bCheckVisibility = FALSE) const;
	BOOL IsRightPartEmpty (BOOL bCheckVisibility = FALSE) const;

	BOOL IsContainerVisible () const;

	int  GetNodeCount () const;
	int  GetTotalReferenceCount () const;

	const CBCGPDockingControlBar* GetLeftBar () const {return m_pBarLeftTop;}
	const CBCGPDockingControlBar* GetRightBar () const {return m_pBarRightBottom;}
	const CBCGPSlider* GetSlider () const {return m_pSlider;}
	const CBCGPBarContainer* GetLeftContainer () const {return m_pLeftContainer;}
	const CBCGPBarContainer* GetRightContainer () const {return m_pRightContainer;}

	DWORD GetRecentSliderStyle () const {return m_dwRecentSliderStyle;}
	CRect GetRecentSliderRect  () const {return m_rectRecentSlider;}

	CBCGPBarContainer* GetParentContainer () const {return m_pParentContainer;}

	void SetBar (CBCGPDockingControlBar* pBar, BOOL bLeft);
	void SetContainer (CBCGPBarContainer* pContainer, BOOL bLeft);
	void SetSlider (CBCGPSlider* pSlider) {m_pSlider = pSlider;}

	BOOL IsLeftContainer () const;
	BOOL IsLeftBar (CBCGPDockingControlBar* pBar) const;

	void SetParentContainer (CBCGPBarContainer* p) {m_pParentContainer = p;}

	BOOL SetUpByID (UINT nID, CBCGPDockingControlBar* pBar);
	CBCGPDockingControlBar* FindTabbedBar (UINT nID);

// Overrides
	virtual void GetWindowRect  (CRect& rect, BOOL bIgnoreVisibility = FALSE) const;
	virtual void GetMinSize	    (CSize& size) const;	
	virtual void GetMinSizeLeft (CSize& size) const;	
	virtual void GetMinSizeRight(CSize& size) const;	
	virtual int  GetResizeStep  () const;

	virtual void OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow);

	virtual void RemoveControlBar (CBCGPDockingControlBar* pBar);
	virtual void DeleteControlBar (CBCGPDockingControlBar* pBar, 
								   BC_FIND_CRITERIA barType);
	virtual void StoreRecentDockInfo (CBCGPDockingControlBar* pBar);

	void OnDeleteHideControlBar (CBCGPDockingControlBar* pBar, BOOL bHide);


	virtual void Move (CPoint ptNewLeftTop);

	virtual int StretchContainer (int nOffset, BOOL bStretchHorz, BOOL bLeftBar, 
						  BOOL bMoveSlider, HDWP& hdwp);
	virtual int OnMoveInternalSlider (int nOffset, HDWP& hdwp);
	virtual void ResizeBar (int nOffset, CBCGPControlBar* pBar, CBCGPBarContainer* pContainer, 
								  BOOL bHorz, BOOL bLeftBar, HDWP& hdwp);
	virtual int CalcAvailableBarSpace (int nRequiredOffset, CBCGPControlBar* pBar,
								CBCGPBarContainer* pContainer, BOOL bLeftBar);
	virtual CSize CalcAvailableSpace (CSize sizeStretch, BOOL bLeftBar);

	virtual void ResizeContainer (CRect rect, HDWP& hdwp, BOOL bRedraw = FALSE);
	virtual void ResizePartOfContainer (int nOffset, BOOL bLeftPart, HDWP& hdwp);

	void SetRecentPercent (int nRecentPercent) {m_nRecentPercent = nRecentPercent;};
	int GetRecentPercent () {return m_nRecentPercent;}
	void CalculateRecentSize ();


	void CheckSliderVisibility ();
	void RemoveNonValidBars ();

	BOOL IsDisposed () const {return m_bDisposed;}

	void AddRef ();
	DWORD Release ();
	LONG GetRefCount () {return m_dwRefCount;}

	void Serialize (CArchive& ar);
	void ReleaseEmptyContainer ();

	virtual CBCGPBarContainer* Copy (CBCGPBarContainer* pParentContainer);

	CList<UINT, UINT>* GetAssociatedSiblingBarIDs (CBCGPDockingControlBar* pBar);

	static BOOL m_bMaintainPercentage;
	static BOOL m_bRetainInternalSliderPosition;

protected:
	CBCGPDockingControlBar*		m_pBarLeftTop;
	CBCGPDockingControlBar*		m_pBarRightBottom;
	CBCGPSlider*				m_pSlider;

	CBCGPBarContainer*			m_pLeftContainer;
	CBCGPBarContainer*			m_pRightContainer;
	CBCGPBarContainer*			m_pParentContainer;

	CBCGPBarContainerManager*	m_pContainerManager;

	int							m_nRecentPercent;
	BOOL						m_bIsRecentSliderHorz;

	LONG						m_dwRefCount;

	DWORD						m_dwRecentSliderStyle;
	CRect						m_rectRecentSlider;

	UINT	m_nSavedLeftBarID;
	UINT	m_nSavedRightBarID;
	UINT	m_nSavedSliderID;

	CList<UINT, UINT> m_lstSavedSiblingBarIDsLeft;
	CList<UINT, UINT> m_lstSavedSiblingBarIDsRight;

	CRect	m_rectSavedSliderRect;
	BOOL	m_bSavedSliderVisibility;

	BOOL	m_bDisposed;

protected:
	void AddNode (CBCGPBarContainer* pContainer);
	void CleanUp ();
	
	void SetContainerManager (CBCGPBarContainerManager* p, BOOL bDeep = FALSE);

	void MoveWnd (CWnd* pWnd, int nOffset, BOOL bHorz);

	void SaveTabbedBar (CArchive& ar, CBCGPDockingControlBar* pBar);
	CBCGPDockingControlBar* LoadTabbedBar (CArchive& ar, CList<UINT, UINT>& lstBarIDs);
	void FreeReleasedContainer ();

	BOOL IsSliderHorz () const;
};

class CBCGPBarContainerGC
{
	CObList m_lstDisposedContainers;
public:
	void AddContainer (CBCGPBarContainer* p) 
	{
		m_lstDisposedContainers.AddTail (p);
	}
	void CleanUp ()
	{
		while (!CBCGPBarContainerGC::m_lstDisposedContainers.IsEmpty ())
		{
			delete m_lstDisposedContainers.RemoveHead ();
		}
	}
	~CBCGPBarContainerGC ()
	{
		CBCGPBarContainerGC::CleanUp ();
	}
};

#endif // !defined(AFX_BCGPBARCONTAINER_H__12D1AE1B_A125_4319_B94F_1A4F0C396BFD__INCLUDED_)
