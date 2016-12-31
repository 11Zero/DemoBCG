// XTPFlowGraphUndoManager.cpp : implementation of the CXTPFlowGraphUndoManager class.
//
// This file is a part of the XTREME TOOLKIT PRO MFC class library.
// (c)1998-2011 Codejock Software, All Rights Reserved.
//
// THIS SOURCE FILE IS THE PROPERTY OF CODEJOCK SOFTWARE AND IS NOT TO BE
// RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE EXPRESSED WRITTEN
// CONSENT OF CODEJOCK SOFTWARE.
//
// THIS SOURCE CODE CAN ONLY BE USED UNDER THE TERMS AND CONDITIONS OUTLINED
// IN THE XTREME TOOLKIT PRO LICENSE AGREEMENT. CODEJOCK SOFTWARE GRANTS TO
// YOU (ONE SOFTWARE DEVELOPER) THE LIMITED RIGHT TO USE THIS SOFTWARE ON A
// SINGLE COMPUTER.
//
// CONTACT INFORMATION:
// support@codejock.com
// http://www.codejock.com
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Common/Resource.h"
#include "Common/XTPDrawHelpers.h"
#include "Common/XTPPropExchange.h"
#include "Common/XTPImageManager.h"

#include "XTPFlowGraphControl.h"
#include "XTPFlowGraphNode.h"
#include "XTPFlowGraphNodes.h"
#include "XTPFlowGraphPage.h"
#include "XTPFlowGraphUndoManager.h"
#include "XTPFlowGraphConnection.h"
#include "XTPFlowGraphConnectionPoint.h"
#include "XTPFlowGraphConnections.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int CONTEXT_NONE = 0;
const int CONTEXT_UNDO = 1;
const int CONTEXT_REDO = 2;

//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoCommand

CXTPFlowGraphUndoCommand::CXTPFlowGraphUndoCommand(int nID)
	: m_nID(nID)
{

}

CXTPFlowGraphUndoCommand::~CXTPFlowGraphUndoCommand()
{
}

void CXTPFlowGraphUndoCommand::Undo()
{
	ASSERT(FALSE);
}


/////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoManager
CXTPFlowGraphUndoManager::CXTPFlowGraphUndoManager()
{
	m_nUndoContext = CONTEXT_NONE;

	m_pUndoGroup = NULL;
	m_bEnabled = TRUE;

}

CXTPFlowGraphUndoManager::~CXTPFlowGraphUndoManager()
{
	Clear();

	SAFE_DELETE(m_pUndoGroup);
}

void CXTPFlowGraphUndoManager::Clear()
{
	int i;
	for (i = 0; i < m_arrUndoCommands.GetSize(); i++)
	{
		delete m_arrUndoCommands[i];
	}
	m_arrUndoCommands.RemoveAll();

	for (i = 0; i < m_arrRedoCommands.GetSize(); i++)
	{
		delete m_arrRedoCommands[i];
	}
	m_arrRedoCommands.RemoveAll();

	SAFE_DELETE(m_pUndoGroup);
}

void CXTPFlowGraphUndoManager::StartGroup()
{
	if (m_pUndoGroup != NULL)
	{
		m_arrGroupStack.Add(m_pUndoGroup);
	}

	m_pUndoGroup = new CXTPFlowGraphUndoGroupCommand(this);
}

void CXTPFlowGraphUndoManager::EndGroup()
{
	if (m_pUndoGroup->GetCount() == 0)
	{
		delete m_pUndoGroup;
	}
	else
	{
		if (m_arrGroupStack.GetSize() > 0)
			m_arrGroupStack[m_arrGroupStack.GetSize() - 1]->Add(m_pUndoGroup);
		else if (m_nUndoContext  != CONTEXT_UNDO)
			m_arrUndoCommands.Add(m_pUndoGroup);
		else
			m_arrRedoCommands.Add(m_pUndoGroup);
	}
	m_pUndoGroup = NULL;

	if (m_arrGroupStack.GetSize())
	{
		m_pUndoGroup = m_arrGroupStack[m_arrGroupStack.GetSize() - 1];
		m_arrGroupStack.RemoveAt(m_arrGroupStack.GetSize() - 1);
	}
}


void CXTPFlowGraphUndoManager::AddUndoCommand(CXTPFlowGraphUndoCommand* pCommand)
{
	if (!m_bEnabled)
	{
		delete pCommand;
		return;
	}

	if (m_pUndoGroup)
	{
		m_pUndoGroup->Add(pCommand);
	}
	else if (m_nUndoContext != CONTEXT_UNDO)
	{
		m_arrUndoCommands.Add(pCommand);
	}
	else
	{
		m_arrRedoCommands.Add(pCommand);
	}

	if (m_nUndoContext == CONTEXT_NONE)
	{
		for (int i = 0; i < m_arrRedoCommands.GetSize(); i++)
		{
			delete m_arrRedoCommands[i];
		}
		m_arrRedoCommands.RemoveAll();
	}
}


BOOL CXTPFlowGraphUndoManager::CanUndo() const
{
	return m_arrUndoCommands.GetSize() > 0;
}

BOOL CXTPFlowGraphUndoManager::CanRedo() const
{
	return m_arrRedoCommands.GetSize() > 0;
}

void CXTPFlowGraphUndoManager::Undo()
{
	if (m_arrUndoCommands.GetSize() > 0)
	{
		m_nUndoContext = 1;

		CXTPFlowGraphUndoCommand* pUndoCommand = m_arrUndoCommands[m_arrUndoCommands.GetSize() - 1];

		pUndoCommand->Undo();
		delete pUndoCommand;

		m_arrUndoCommands.RemoveAt(m_arrUndoCommands.GetSize() - 1);

		m_nUndoContext = 0;
	}
}

void CXTPFlowGraphUndoManager::Redo()
{
	if (m_arrRedoCommands.GetSize() > 0)
	{
		m_nUndoContext = 2;

		CXTPFlowGraphUndoCommand* pUndoCommand = m_arrRedoCommands[m_arrRedoCommands.GetSize() - 1];

		pUndoCommand->Undo();
		delete pUndoCommand;

		m_arrRedoCommands.RemoveAt(m_arrRedoCommands.GetSize() - 1);

		m_nUndoContext = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoGroupCommand

CXTPFlowGraphUndoGroupCommand::CXTPFlowGraphUndoGroupCommand(CXTPFlowGraphUndoManager* pManager)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_GROUP)
{
	m_pManager = pManager;
}

CXTPFlowGraphUndoGroupCommand::~CXTPFlowGraphUndoGroupCommand()
{
	for (int i = (int)m_arrUndoCommands.GetSize() - 1; i >= 0; i--)
	{
		delete m_arrUndoCommands[i];
	}
	m_arrUndoCommands.RemoveAll();
}

void CXTPFlowGraphUndoGroupCommand::Add(CXTPFlowGraphUndoCommand* pCommand)
{
	m_arrUndoCommands.Add(pCommand);
}

void CXTPFlowGraphUndoGroupCommand::Undo()
{
	m_pManager->StartGroup();

	for (int i = (int)m_arrUndoCommands.GetSize() - 1; i >= 0; i--)
	{
		m_arrUndoCommands[i]->Undo();
		delete m_arrUndoCommands[i];
	}
	m_arrUndoCommands.RemoveAll();

	m_pManager->EndGroup();
}

int CXTPFlowGraphUndoGroupCommand::GetCount() const
{
	return (int)m_arrUndoCommands.GetSize();
}


//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoAddNodeCommand

CXTPFlowGraphUndoAddNodeCommand::CXTPFlowGraphUndoAddNodeCommand(CXTPFlowGraphNode* pNode)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_ADDNODE)
{
	m_pNode = pNode;
	m_pNode->InternalAddRef();
}

CXTPFlowGraphUndoAddNodeCommand::~CXTPFlowGraphUndoAddNodeCommand()
{
	m_pNode->InternalRelease();
}

void CXTPFlowGraphUndoAddNodeCommand::Undo()
{
	m_pNode->Remove();
}

//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoAddConnectionCommand

CXTPFlowGraphUndoAddConnectionCommand::CXTPFlowGraphUndoAddConnectionCommand(CXTPFlowGraphConnection* pConnection)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_ADDCONNECTION)
{
	m_pConnection = pConnection;
	m_pConnection->InternalAddRef();
}

CXTPFlowGraphUndoAddConnectionCommand::~CXTPFlowGraphUndoAddConnectionCommand()
{
	m_pConnection->InternalRelease();
}

void CXTPFlowGraphUndoAddConnectionCommand::Undo()
{
	m_pConnection->Remove();
}


//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoDeleteNodeCommand

CXTPFlowGraphUndoDeleteNodeCommand::CXTPFlowGraphUndoDeleteNodeCommand(CXTPFlowGraphNode* pNode)
	: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_DELETENODE)
{
	m_pNode = pNode;
	m_pNode->InternalAddRef();

	m_pPage = pNode->GetPage();
}

CXTPFlowGraphUndoDeleteNodeCommand::~CXTPFlowGraphUndoDeleteNodeCommand()
{
	m_pNode->InternalRelease();
}

void CXTPFlowGraphUndoDeleteNodeCommand::Undo()
{
	m_pNode->InternalAddRef();

	m_pPage->GetNodes()->AddNode(m_pNode);
}

//////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoDeleteConnectionCommand

CXTPFlowGraphUndoDeleteConnectionCommand::CXTPFlowGraphUndoDeleteConnectionCommand(CXTPFlowGraphConnection* pConnection)
	: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_DELETECONNECTION)
{
	m_pConnection = pConnection;
	m_pConnection->InternalAddRef();

	m_pPage = pConnection->GetPage();
}

CXTPFlowGraphUndoDeleteConnectionCommand::~CXTPFlowGraphUndoDeleteConnectionCommand()
{
	m_pConnection->InternalRelease();
}

void CXTPFlowGraphUndoDeleteConnectionCommand::Undo()
{
	if (!(m_pConnection->GetInputPoint() && m_pConnection->GetInputPoint()->GetNode()
		 && m_pConnection->GetInputPoint()->GetNode()->GetPage() == m_pPage))
		 return;

	if (!(m_pConnection->GetOutputPoint() && m_pConnection->GetOutputPoint()->GetNode()
		&& m_pConnection->GetOutputPoint()->GetNode()->GetPage() == m_pPage))
		return;

	m_pConnection->InternalAddRef();

	m_pPage->GetConnections()->AddConnection(m_pConnection);
}

///////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoSetNodePositionCommand

CXTPFlowGraphUndoSetNodePositionCommand::CXTPFlowGraphUndoSetNodePositionCommand(CXTPFlowGraphNode* pNode, CPoint ptOldPosition)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_SETNODEPOSITION)
{
	m_ptOldPosition = ptOldPosition;

	m_pNode = pNode;
	m_pNode->InternalAddRef();
}

CXTPFlowGraphUndoSetNodePositionCommand::~CXTPFlowGraphUndoSetNodePositionCommand()
{
	m_pNode->InternalRelease();
}

void CXTPFlowGraphUndoSetNodePositionCommand::Undo()
{
	m_pNode->SetLocation(m_ptOldPosition);
}

///////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoSetNodeSizeCommand

CXTPFlowGraphUndoSetNodeSizeCommand::CXTPFlowGraphUndoSetNodeSizeCommand(CXTPFlowGraphNode* pNode, CSize szOldSize)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_SETNODESIZE)
{
	m_szOldSize = szOldSize;

	m_pNode = pNode;
	m_pNode->InternalAddRef();
}

CXTPFlowGraphUndoSetNodeSizeCommand::~CXTPFlowGraphUndoSetNodeSizeCommand()
{
	m_pNode->InternalRelease();
}

void CXTPFlowGraphUndoSetNodeSizeCommand::Undo()
{
	m_pNode->SetSize(m_szOldSize);
}

///////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoSetNodePositionCommand

CXTPFlowGraphUndoSetNodeCaptionCommand::CXTPFlowGraphUndoSetNodeCaptionCommand(CXTPFlowGraphElement* pNode, CString strOldCaption)
	: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_SETNODECAPTION)
{
	m_strOldCaption = strOldCaption;

	m_pNode = pNode;
	m_pNode->InternalAddRef();
}

CXTPFlowGraphUndoSetNodeCaptionCommand::~CXTPFlowGraphUndoSetNodeCaptionCommand()
{
	m_pNode->InternalRelease();
}

void CXTPFlowGraphUndoSetNodeCaptionCommand::Undo()
{
	m_pNode->SetCaption(m_strOldCaption);
}

///////////////////////////////////////////////////////////////////////////
// CXTPFlowGraphUndoSetConnectionPositionCommand

CXTPFlowGraphUndoSetConnectionPositionCommand::CXTPFlowGraphUndoSetConnectionPositionCommand(CXTPFlowGraphConnection* pConnection, CPoint ptOldPosition)
: CXTPFlowGraphUndoCommand(XTP_ID_FLOWGRAPHUNDO_SETCONNECTIONPOSITION)
{
	m_ptOldPosition = ptOldPosition;

	m_pConnection = pConnection;
	m_pConnection->InternalAddRef();
}

CXTPFlowGraphUndoSetConnectionPositionCommand::~CXTPFlowGraphUndoSetConnectionPositionCommand()
{
	m_pConnection->InternalRelease();
}

void CXTPFlowGraphUndoSetConnectionPositionCommand::Undo()
{
	m_pConnection->SetControlPoint(m_ptOldPosition);
}




