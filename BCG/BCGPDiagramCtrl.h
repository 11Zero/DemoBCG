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
// BCGPDiagramCtrl.h: interface for the CBCGPDiagramVisualContainerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPDIAGRAMCTRL_H__182C4628_79EF_4D94_B038_BC1320766A9E__INCLUDED_)
#define AFX_BCGPDIAGRAMCTRL_H__182C4628_79EF_4D94_B038_BC1320766A9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPVisualCtrl.h"
#include "BCGPDiagramVisualContainer.h"

class BCGCBPRODLLEXPORT CBCGPDiagramVisualContainerCtrl : public CBCGPVisualContainerCtrl  
{
	DECLARE_DYNAMIC(CBCGPDiagramVisualContainerCtrl)
		
	// Construction
public:
	CBCGPDiagramVisualContainerCtrl();
	virtual ~CBCGPDiagramVisualContainerCtrl();

	// Attributes:
	CBCGPDiagramVisualContainerCtrl* GetDiagramContainer()
	{
		return DYNAMIC_DOWNCAST(CBCGPDiagramVisualContainerCtrl, GetVisualContainer());
	}

	// Operations
	virtual CBCGPVisualContainer* GetVisualContainer()
	{
		if (m_pContainer == NULL)
		{
			m_pContainer = new CBCGPDiagramVisualContainer;
		}
		
		return m_pContainer;
	}

	virtual BOOL CopyToClipboard()
	{
		return CBCGPVisualContainerCtrl::CopyToClipboard ();
	}

	virtual BOOL ExportToFile(const CString& strFilePath)
	{
		return CBCGPVisualContainerCtrl::ExportToFile (strFilePath);
	}

	// Overrides
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#endif // !defined(AFX_BCGPDIAGRAMCTRL_H__182C4628_79EF_4D94_B038_BC1320766A9E__INCLUDED_)
