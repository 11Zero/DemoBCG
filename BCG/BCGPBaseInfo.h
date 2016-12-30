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
// BCGPBaseInfo.h: interface for the CBCGPRibbonInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BCGPBASEINFO_H__00B51C81_7927_41F2_A891_2F19CA309D97__INCLUDED_)
#define AFX_BCGPBASEINFO_H__00B51C81_7927_41F2_A891_2F19CA309D97__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _BCGSUITE_
#include "BCGPToolBarImages.h"
#endif

class CBCGPBaseInfo
{
public:
	static LPCTSTR s_szTag_Header;
	static LPCTSTR s_szTag_Version;
	static LPCTSTR s_szTag_VersionStamp;
	static LPCTSTR s_szTag_Name;
	static LPCTSTR s_szTag_Value;
	static LPCTSTR s_szTag_ID;
	static LPCTSTR s_szTag_ElementName;

public:
	class XID
	{
	public:
		XID ();
		~XID ();
		
        BOOL IsEmpty() const { return m_Name.IsEmpty() && m_Value == 0; }

		BOOL FromTag (const CString& strTag);
		void ToTag (CString& strTag) const;

		BOOL operator == (const XID& id)
		{
			return m_Name == id.m_Name && m_Value == id.m_Value;
		}

	public:
		CString	m_Name;
		UINT	m_Value;
	};
	
	class XImage
	{
	public:
		XImage ();
		~XImage ();

        BOOL IsEmpty() const { return m_ID.IsEmpty() || !m_Image.IsValid(); }

		BOOL FromTag (const CString& strTag);
		void ToTag (CString& strTag) const;

	public:
		XID                 m_ID;
		BOOL				m_bDontScale;
#ifndef _BCGSUITE_
		CBCGPToolBarImages	m_Image;
#else
		CMFCToolBarImages	m_Image;
#endif
	};
	typedef CArray<XImage*, XImage*> XArrayImages;

	class XBase
	{
	protected:
		XBase(const CString& strElementName);

	public:
		virtual ~XBase();

		inline const CString& GetElementName () const
		{
			return m_strElementName;
		}

		virtual BOOL FromTag (const CString& strTag) = 0;
		virtual void ToTag (CString& strTag) const;

	private:
		CString			m_strElementName;
	};

	virtual BOOL FromTag (const CString& strTag) = 0;
	virtual void ToTag (CString& strTag) const = 0;

	inline DWORD GetVersion () const
	{
		return m_dwVersion;
	}
	void SetVersion (DWORD dwVersion)
	{
		m_dwVersion = dwVersion;
	}
	
	inline DWORD GetVersionStamp () const
	{
		return m_dwVersionStamp;
	}
	void SetVersionStamp (DWORD dwVersionStamp)
	{
		m_dwVersionStamp = dwVersionStamp;
	}

	inline DWORD GetFlags () const
	{
		return m_dwFlags;
	}
	void SetFlags(DWORD dwFlags)
	{
		m_dwFlags = dwFlags;
	}

	typedef XBase* (*XCreateBaseFromNameProc)(const CString& name);
	static XBase* CreateBaseFromTag (const CString& tag, XCreateBaseFromNameProc lpCreateProc);

protected:
	CBCGPBaseInfo(DWORD dwVersion, DWORD dwFlags);
	virtual ~CBCGPBaseInfo();

private:
	DWORD m_dwVersion;
	DWORD m_dwVersionStamp;
	DWORD m_dwFlags;
};

class CBCGPBaseInfoLoader
{
public:
	virtual BOOL Load (UINT uiResID, LPCTSTR lpszResType = NULL, HINSTANCE hInstance = NULL);
	virtual BOOL Load (LPCTSTR lpszResID, LPCTSTR lpszResType = NULL, HINSTANCE hInstance = NULL);
	virtual BOOL LoadFromFile (LPCTSTR lpszFileName);	
	virtual BOOL LoadFromBuffer (LPCTSTR lpszBuffer);

protected:
	CBCGPBaseInfo& GetInfo ()
	{
		return m_Info;
	}
	const CBCGPBaseInfo& GetInfo () const
	{
		return m_Info;
	}

	HINSTANCE GetInstance () const
	{
		return m_hInstance;
	}

protected:
	CBCGPBaseInfoLoader(CBCGPBaseInfo& info, LPCTSTR lpszResType, DWORD dwFlags);
	virtual ~CBCGPBaseInfoLoader();

private:
	CBCGPBaseInfo&	m_Info;
	LPCTSTR			m_lpszResType;
	HINSTANCE		m_hInstance;
};

class CBCGPBaseInfoWriter  
{
public:
	static BOOL FileExists (const CString& fileName);
	static BOOL IsFileReadOnly (const CString& fileName);
	static CString CorrectDirectoryName(const CString& dirName);
	static BOOL CreateDirectory (const CString& dirName);
	static CString PrepareFileName (const CString& fileName);
	static CString CorrectFileName(const CString& dirName, const CString& fileName);
	static void ParseFileName (const CString& pathName, CString& fileName, CString& extName);
	static void ParseFilePath (const CString& pathName, CString& dirName, CString& fileName);

protected:
	CBCGPBaseInfo& GetInfo ()
	{
		return m_Info;
	}
	const CBCGPBaseInfo& GetInfo () const
	{
		return m_Info;
	}

	virtual BOOL CheckFiles(const CStringArray& sa) const;
	virtual void ErrorReportFolder (const CString& strName) const;
	virtual void ErrorReportFileRO (const CString& strName) const;

	virtual BOOL WriteText (const CString& strFileName, const CStringArray& text) const;
	virtual BOOL WriteBinary (const CString& strFileName, LPCVOID lpBuffer, int count) const;

	virtual BOOL SaveInfo (const CString& strFileName);

protected:
	CBCGPBaseInfoWriter(CBCGPBaseInfo& info);
	virtual ~CBCGPBaseInfoWriter();

private:
	CBCGPBaseInfo&	m_Info;
};

#endif // !defined(AFX_BCGPBASEINFO_H__00B51C81_7927_41F2_A891_2F19CA309D97__INCLUDED_)
