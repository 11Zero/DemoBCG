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
// BCGBarContainerManager.h: interface for the CBCGPBarContainerManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPBARCONTAINERMANAGER_H__BBF050E7_D49D_4AD4_AE08_CD8E4AA69E52__INCLUDED_)
#define AFX_BCGPBARCONTAINERMANAGER_H__BBF050E7_D49D_4AD4_AE08_CD8E4AA69E52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPSlider.h"

class CBCGPControlBar;
class CBCGPBarContainer;

class BCGCBPRODLLEXPORT CBCGPBarContainerManager : public CObject  
{
	friend class CBCGPBarContainer;
	friend class CBCGPMultiMiniFrameWnd;
	friend class CBCGPDockingControlBar;
	friend class CBCGPRecentContainerInfo;

	DECLARE_DYNCREATE (CBCGPBarContainerManager)
public:
	CBCGPBarContainerManager();
	virtual ~CBCGPBarContainerManager();

	virtual BOOL Create (CWnd* pParentWnd, CBCGPSlider* pDefaultSlider, 
						 CRuntimeClass* pContainerRTC = NULL);
	void SetContainerRTC (CRuntimeClass* pContainerRTC)
	{
		m_pContainerRTC = pContainerRTC;
	}

	CRuntimeClass* GetContainerRTC () const {return m_pContainerRTC;}

	virtual BOOL AddContainerManager (CBCGPBarContainerManager& srcManager, BOOL bOuterEdge);
	virtual BOOL AddContainerManager (CBCGPDockingControlBar* pTargetControlBar, 
										DWORD dwAlignment, 
										CBCGPBarContainerManager& srcManager,
										BOOL bCopy);
	virtual BOOL AddContainerManagerToTabWnd (CBCGPDockingControlBar* pTargetControlBar, 
											  CBCGPBarContainerManager& srcManager);
	virtual void AddControlBar (CBCGPDockingControlBar* pControlBarToAdd);
	virtual CBCGPDockingControlBar* AddControlBarToRecentContainer (CBCGPDockingControlBar* pBarToAdd, 
												 CBCGPBarContainer* pRecentContainer);
	virtual BOOL InsertControlBar (CBCGPDockingControlBar* pControlBarToInsert,
								   CBCGPDockingControlBar* pTargetControlBar,
								   DWORD dwAlignment,
								   LPCRECT lpRect = NULL,
								   BCGP_DOCK_METHOD dockMethod = BCGP_DM_UNKNOWN);

	virtual int  OnSliderMove (CBCGPSlider* pSlider, UINT uFlags, int nOffset, HDWP& hdwp);
	virtual void GetWindowRect (CRect& rect) const;
	virtual void GetAvailableSpace (CRect& rect) const;
	virtual BOOL RemoveControlBarFromContainer (CBCGPDockingControlBar* pControlBar);
	virtual void ResizeBarContainers (UINT nSide, BOOL bExpand, int nOffset, HDWP& hdwp);
	virtual void ResizeBarContainers (CRect rect, HDWP& hdwp);
	virtual BOOL ReplaceControlBar (CBCGPDockingControlBar* pBarOld, CBCGPDockingControlBar* pBarNew);
	virtual void GetMinMaxOffset (CBCGPSlider* pSlider, int& nMinOffset, int& nMaxOffset, 
								  int& nStep);

	virtual BOOL OnShowControlBar (CBCGPDockingControlBar* pBar, BOOL bShow);
	virtual BOOL IsRootContainerVisible () const;

	virtual void GetMinSize (CSize& size);
	virtual int  GetVisibleBarCount () const;
	virtual void EnableGrippers (BOOL bEnable);
	virtual void HideAll ();

	virtual BOOL DoesContainFloatingBar ();
	virtual BOOL DoesAllowDynInsertBefore () const;

	virtual CBCGPDockingControlBar* ControlBarFromPoint (CPoint point, int nSensitivity, 
														 BOOL bExactBar, BOOL& bIsTabArea,
														 BOOL& bCaption);

	virtual BOOL CheckForMiniFrameAndCaption (CPoint point, CBCGPDockingControlBar** ppTargetControlBar);

	virtual CBCGPBaseControlBar* GetFirstBar () const;
	virtual CWnd* GetDockSite () {return m_pDockSite;}
	virtual CWnd* GetFirstVisibleBar () const;

	virtual BOOL CanBeAttached () const;

	CBCGPSlider* GetDefaultSlider () const {return m_pDefaultSlider;}

	void ReleaseEmptyContainers ();

	//virtual void Move (int nOffset);

	void	AddControlBarToList (CBCGPDockingControlBar* pControlBarToAdd); 
	BOOL	IsEmpty () const;
	int		GetTotalRefCount () const;
	int		GetBarCount () const { return (int) m_lstControlBars.GetCount (); }
	int		GetNodeCount () const;

	virtual void SetResizeMode (BOOL bResize);
	virtual void RemoveSlider (CBCGPSlider* pSlider);

	virtual void StoreRecentDockInfo (CBCGPDockingControlBar* pBar);

	BOOL IsAutoHideMode () const
	{
		return (m_pDefaultSlider != NULL && m_pDefaultSlider->IsAutoHideMode ());
	}

	void SetDefaultSliderForControlBars (CBCGPSlider* pSlider);
	void Serialize (CArchive& ar);
	CBCGPDockingControlBar* FindTabbedBar (UINT nID);
	virtual CBCGPBarContainer* FindContainer (CBCGPDockingControlBar* pBar, BOOL& bLeftBar);

	void AddControlBarsToList (CObList* plstControlBars, CObList* plstSliders);
	void RemoveAllControlBarsAndSliders ();

	void CalcRects (CRect& rectOriginal, CRect& rectInserted, CRect& rectSlider,
				DWORD& dwSliderStyle, DWORD dwAlignment, CSize sizeMinOriginal,
				CSize sizeMinInserted);
	void NotifySlider ();

	void RemoveNonValidBars ();
	BOOL CheckAndRemoveNonValidBar (CWnd* pWnd);

protected:
	CBCGPSlider* CreateSlider (CRect rectSlider, DWORD dwSliderStyle, int nSliderID = -1);
	UINT FindBar (CPoint pt, CBCGPControlBar** ppBar, POSITION& posRet);
	UINT FindBar (CRect rect, CBCGPControlBar** ppBar, POSITION& posRet);
	BOOL AddControlBarAndSlider (CBCGPDockingControlBar* pBarOriginal, CBCGPDockingControlBar* pBarToInsert, 
								 POSITION posNearestBar, DWORD dwAlignment);
	BOOL AddControlBarAndContainer (CBCGPDockingControlBar* pBarOriginal, 
									  CBCGPBarContainer* pContainerToInsert, 
									  DWORD dwAlignment);
protected:
	CObList				m_lstControlBars;
	CObList				m_lstSliders;
	CBCGPBarContainer*	m_pRootContainer;
	CRuntimeClass*		m_pContainerRTC;
	CBCGPSlider*		m_pDefaultSlider;

	BOOL				m_bDestroyRootContainer;
	
	CWnd*				m_pDockSite;
};

#endif // !defined(AFX_BCGPBARCONTAINERMANAGER_H__BBF050E7_D49D_4AD4_AE08_CD8E4AA69E52__INCLUDED_)
