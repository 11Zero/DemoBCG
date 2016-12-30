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
// BCGPRotationObject.h: interface for the CBCGPRotationObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPROTATIONOBJECT_H__090DE9EA_51CA_4887_AC31_6E94FEE97550__INCLUDED_)
#define AFX_BCGPROTATIONOBJECT_H__090DE9EA_51CA_4887_AC31_6E94FEE97550__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGCBPro.h"
#include "BCGPRadialMenu.h"

class CBCGPRotationCtrl;

class BCGCBPRODLLEXPORT CBCGPRotationObject : public CBCGPRadialMenuObject  
{
	friend class CBCGPRotationCtrl;

public:
	enum RotationElement
	{
		BCGP_ROTATION_FIRST                = -1,
		BCGP_ROTATION_NONE                 = BCGP_ROTATION_FIRST,
		BCGP_ROTATION_UP                   = 0,
		BCGP_ROTATION_CLOCKWISE            = 1,
		BCGP_ROTATION_LEFT                 = 2,
		BCGP_ROTATION_NARROW_FIELD_OF_VIEW = 3,
		BCGP_ROTATION_DOWN                 = 4,
		BCGP_ROTATION_WIDEN_FIELD_OF_VIEW  = 5,
		BCGP_ROTATION_RIGHT                = 6,
		BCGP_ROTATION_COUNTER_CLOCKWISE    = 7,
		BCGP_ROTATION_RESET                = 8,
		BCGP_ROTATION_LAST                 = BCGP_ROTATION_RESET
	};

	CBCGPRotationObject();
	virtual ~CBCGPRotationObject();

	RotationElement GetClicked() const
	{
		return (RotationElement)m_nLastClicked;
	}

	void EnablePart(RotationElement id, BOOL bEnable = TRUE);
	BOOL IsPartEnabled(RotationElement id) const;

	void SetNotifyCommandID(UINT nCmdID)
	{
		m_nNotifyCmdID = nCmdID;
	}

	UINT GetNotifyCommandID() const
	{
		return m_nNotifyCmdID;
	}

protected:
	virtual BOOL NotifyCommand();

	UINT m_nNotifyCmdID;
};

class BCGCBPRODLLEXPORT CBCGPRotationCtrl : public CBCGPRadialMenu
{
	DECLARE_DYNAMIC(CBCGPRotationCtrl)

public:
	CBCGPRotationCtrl()
	{
		m_pRotationObject = NULL;
		m_nDlgCode = DLGC_WANTARROWS | DLGC_WANTCHARS;
		EnableTooltip();
	}

	virtual ~CBCGPRotationCtrl()
	{
		if (m_pRotationObject != NULL)
		{
			delete m_pRotationObject;
		}
	}

	virtual CBCGPRotationObject* GetRotationObject()
	{
		if (m_pRotationObject == NULL)
		{
			m_pRotationObject = new CBCGPRotationObject();
			m_pRotationObject->m_pCtrl = this;
		}

		return m_pRotationObject;
	}

protected:
	virtual CBCGPBaseVisualObject* GetVisualObject()
	{
		return GetRotationObject();
	}

	virtual int GetIconSize()
	{
		return 16;
	}

protected:
	CBCGPRotationObject*	m_pRotationObject;
};

#endif // !defined(AFX_BCGPROTATIONOBJECT_H__090DE9EA_51CA_4887_AC31_6E94FEE97550__INCLUDED_)
