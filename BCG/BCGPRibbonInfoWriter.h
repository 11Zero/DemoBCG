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
// BCGPRibbonInfoWriter.h: interface for the CBCGPRibbonInfoWriter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPRIBBONINFOWRITER_H__0A734EC3_E83C_4994_913B_EA675BDB0A9A__INCLUDED_)
#define AFX_BCGPRIBBONINFOWRITER_H__0A734EC3_E83C_4994_913B_EA675BDB0A9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BCGPRibbonInfo.h"

#ifndef BCGP_EXCLUDE_RIBBON

class CBCGPRibbonInfoWriter: public CBCGPBaseInfoWriter
{
public:
	CBCGPRibbonInfoWriter(CBCGPRibbonInfo& info);
	virtual ~CBCGPRibbonInfoWriter();

	virtual BOOL Save (const CString& strFolder, 
		const CString& strResourceName,
		const CString& strResourceFolder = _T("res\\BCGSoft_ribbon"), 
		const CString& strResourcePrefix = _T("IDR_BCGP"));

protected:
	virtual void CorrectImage (CBCGPRibbonInfo::XImage& image, const CString& strID, const CString& strFile);
	virtual void CorrectImages ();
	virtual void CorrectImages (CBCGPRibbonInfo::XElement& info, const CString& strFmt, int& group, int& palette);
	virtual void CorrectNames ();

	virtual void CorrectInfo ();

	virtual BOOL Prepare ();

	virtual BOOL SaveRC ();
	virtual BOOL SaveXML ();
	virtual BOOL SaveImages ();

	virtual CString GetFileName (const CString& strFileName) const;

	CBCGPRibbonInfo& GetRibbonInfo ()
	{
		return (CBCGPRibbonInfo&)GetInfo ();
	}

protected:
	class XResourceFile
	{
	public:	
		XResourceFile ()
		{
		}
		XResourceFile (const CString& strID, const CString& strType, const CString& strPath)
			: m_ID   (strID)
			, m_Type (strType)
			, m_Path (strPath)
		{
		}

		const CString& GetID () const
		{
			return m_ID;
		}
		const CString& GetType () const
		{
			return m_Type;
		}
		const CString& GetPath () const
		{
			return m_Path;
		}

		BOOL IsValid () const
		{
			return !m_ID.IsEmpty () && !m_Type.IsEmpty () && !m_Path.IsEmpty ();
		}

		CString Format () const;

	private:
		CString m_ID;
		CString m_Type;
		CString m_Path;
	};
	typedef CArray<XResourceFile*, XResourceFile*> XResourceFileArray;
	typedef CMapStringToPtr XResourceFileMap;

	int GetResourceIndex (const XResourceFileArray& array, const CString& strID);
	XResourceFileArray* GetResourcesArray (const CString& strType) const;
	BOOL AddResource (const XResourceFile& resource);
	void EmptyResources ();

protected:
	CString				m_ResourcePrefix;
	CString				m_ResourceFolder;
	CString				m_ResourceName;

	CString				m_Folder;

	XResourceFileMap	m_Files;
	
	BOOL				m_bConvertImage32;
};

class CBCGPRibbonCustomizationInfoWriter: public CBCGPBaseInfoWriter
{
public:
	CBCGPRibbonCustomizationInfoWriter(CBCGPRibbonCustomizationInfo& info);
	virtual ~CBCGPRibbonCustomizationInfoWriter();

	virtual BOOL Save (const CString& strFileName);

protected:
	CBCGPRibbonCustomizationInfo& GetRibbonCustomizationInfo ()
	{
		return (CBCGPRibbonCustomizationInfo&)GetInfo ();
	}
};

#endif // BCGP_EXCLUDE_RIBBON

#endif // !defined(AFX_BCGPRIBBONINFOWRITER_H__0A734EC3_E83C_4994_913B_EA675BDB0A9A__INCLUDED_)
