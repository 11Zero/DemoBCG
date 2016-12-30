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
// BCGPBaseInfo.cpp: implementation of the CBCGPRibbonInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPBaseInfo.h"
#include "BCGPTagManager.h"
#include <io.h>
#include <direct.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR CBCGPBaseInfo::s_szTag_Header       = _T("HEADER");
LPCTSTR CBCGPBaseInfo::s_szTag_Version      = _T("VERSION");
LPCTSTR CBCGPBaseInfo::s_szTag_VersionStamp = _T("VERSION_STAMP");
LPCTSTR CBCGPBaseInfo::s_szTag_Name         = _T("NAME");
LPCTSTR CBCGPBaseInfo::s_szTag_Value        = _T("VALUE");
LPCTSTR CBCGPBaseInfo::s_szTag_ID           = _T("ID");
LPCTSTR CBCGPBaseInfo::s_szTag_ElementName  = _T("ELEMENT_NAME");

CBCGPBaseInfo::CBCGPBaseInfo(DWORD dwVersion, DWORD dwFlags)
	: m_dwVersion     (dwVersion)
	, m_dwVersionStamp(0)
	, m_dwFlags       (dwFlags)
{
}

CBCGPBaseInfo::~CBCGPBaseInfo()
{
}

CBCGPBaseInfo::XBase* CBCGPBaseInfo::CreateBaseFromTag (const CString& tag, XCreateBaseFromNameProc lpCreateProc)
{
	if (lpCreateProc == NULL)
	{
		ASSERT(FALSE);
		return NULL;
	}

	CBCGPBaseInfo::XBase* base = NULL;

	CString strElementName;
	{
		CBCGPTagManager tm (tag);
		tm.ReadString (CBCGPBaseInfo::s_szTag_ElementName, strElementName);
	}

	if (!strElementName.IsEmpty ())
	{
		base = (lpCreateProc)(strElementName);
		if (base != NULL)
		{
			base->FromTag (tag);
		}
	}

	return base;
}


CBCGPBaseInfo::XID::XID ()
	: m_Value (0)
{
}

CBCGPBaseInfo::XID::~XID ()
{
}

BOOL CBCGPBaseInfo::XID::FromTag (const CString& strTag)
{
	CBCGPTagManager tm (strTag);

	BOOL bResult = tm.ReadString (s_szTag_Name, m_Name);

	if (tm.ReadUInt (s_szTag_Value, m_Value))
	{
		bResult = TRUE;
	}

	return bResult;
}

void CBCGPBaseInfo::XID::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_Name, m_Name));
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteUInt (s_szTag_Value, m_Value, 0));
}


CBCGPBaseInfo::XImage::XImage () : m_bDontScale(FALSE)
{
}
CBCGPBaseInfo::XImage::~XImage ()
{
}

BOOL CBCGPBaseInfo::XImage::FromTag (const CString& strTag)
{
	if (!m_ID.FromTag (strTag))
	{
		m_ID.m_Name = strTag;
	}

	return TRUE;
}

void CBCGPBaseInfo::XImage::ToTag (CString& strTag) const
{
	if (m_Image.IsValid ())
	{
		m_ID.ToTag (strTag);
	}
}


CBCGPBaseInfo::XBase::XBase(const CString& strElementName)
	: m_strElementName (strElementName)
{
}

CBCGPBaseInfo::XBase::~XBase()
{
}

void CBCGPBaseInfo::XBase::ToTag (CString& strTag) const
{
	CBCGPTagManager::WriteTag (strTag, CBCGPTagManager::WriteString (s_szTag_ElementName, m_strElementName));
}


CBCGPBaseInfoLoader::CBCGPBaseInfoLoader(CBCGPBaseInfo& info, LPCTSTR lpszResType, DWORD dwFlags)
	: m_Info       (info)
	, m_lpszResType(lpszResType)
	, m_hInstance  (NULL)
{
	m_Info.SetFlags (dwFlags);
}

CBCGPBaseInfoLoader::~CBCGPBaseInfoLoader()
{
}

BOOL CBCGPBaseInfoLoader::Load (UINT uiResID, LPCTSTR lpszResType, HINSTANCE hInstance/* = NULL*/)
{
	return Load (MAKEINTRESOURCE (uiResID), lpszResType, hInstance);
}

BOOL CBCGPBaseInfoLoader::Load (LPCTSTR lpszResID, LPCTSTR lpszResType, HINSTANCE hInstance/* = NULL*/)
{
	if (lpszResID == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	if (lpszResType == NULL)
	{
		lpszResType = m_lpszResType;
	}

	if (lpszResType == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;

	if (hInstance == NULL)
	{
		hInstance = AfxFindResourceHandle(lpszResID, lpszResType);
	}

	if (hInstance == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_hInstance = hInstance;
	HRSRC hRsrc = ::FindResource(m_hInstance, lpszResID, lpszResType);	
	
	if (hRsrc == NULL)
	{
		return FALSE;
	}

	DWORD nLength = ::SizeofResource(m_hInstance, hRsrc);
	if (nLength == 0)
	{
		return FALSE;
	}

	HGLOBAL hGlobal = ::LoadResource(m_hInstance, hRsrc);
	if (hGlobal == NULL)
	{
		return FALSE;
	}

	LPTSTR lpszXML = NULL;
	CBCGPTagManager::UTF8ToString ((LPCSTR)::LockResource (hGlobal), lpszXML, nLength);

	::UnlockResource (hGlobal);
	::FreeResource (hGlobal);

	if (lpszXML != NULL)
	{
		bRes = LoadFromBuffer (lpszXML);
		delete [] lpszXML;
	}

	return bRes;
}

BOOL CBCGPBaseInfoLoader::LoadFromFile (LPCTSTR lpszFileName)
{
	if (lpszFileName == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CFileException e;
	CFile file;

	if (!file.Open (lpszFileName, CFile::modeRead | CFile::typeBinary, &e))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	int nLength = (int)file.GetLength ();

	LPBYTE lpszBuffer = new BYTE[nLength];
	file.Read (lpszBuffer, nLength);

	LPTSTR lpszXML = NULL;
	CBCGPTagManager::UTF8ToString ((LPCSTR)lpszBuffer, lpszXML, nLength);

	delete [] lpszBuffer;

	file.Close();

	BOOL bRes = FALSE;

	if (lpszXML != NULL)
	{
		bRes = LoadFromBuffer (lpszXML);
		delete [] lpszXML;
	}

	return bRes;
}

BOOL CBCGPBaseInfoLoader::LoadFromBuffer (LPCTSTR lpszBuffer)
{
	if (lpszBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return GetInfo ().FromTag (lpszBuffer);
}


BOOL CBCGPBaseInfoWriter::FileExists (const CString& fileName)
{
	return _taccess (fileName, 0) != -1;
}

BOOL CBCGPBaseInfoWriter::IsFileReadOnly (const CString& fileName)
{
	if (FileExists (fileName))
	{
		return (GetFileAttributes(fileName) & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY;
	}

	return FALSE;
}

CString CBCGPBaseInfoWriter::CorrectDirectoryName(const CString& dirName)
{
    CString dir(dirName);

    if (!dir.IsEmpty())
    {
        if (dir.GetAt(dir.GetLength() - 1) != TCHAR('\\'))
        {
            dir += TCHAR('\\');
        }
    }

    return dir;
}

BOOL CBCGPBaseInfoWriter::CreateDirectory (const CString& dirName)
{
	if (FileExists (dirName))
	{
		return TRUE;
	}

	if (dirName.IsEmpty())
	{
		return FALSE;
	}

	CStringArray sa;
	CString s(dirName);
	
	int pos = s.Find(TCHAR('\\'));

	while (pos != -1)
	{
		CString sp = s.Left(pos);
		s = s.Right(s.GetLength() - pos - 1);

		if (!sp.IsEmpty())
		{
			sa.Add(sp);
		}

		pos = s.Find(TCHAR('\\'));

		if (pos == -1 && !s.IsEmpty())
		{
			sa.Add(s);
		}
	}

	BOOL bRes = TRUE;

	CString strDir;
	for (int i = 0; i < sa.GetSize (); i++)
	{
		strDir += sa[i];
		if (!FileExists (strDir))
		{
			if (i == 0)
			{
				bRes = FALSE;
				break;
			}

			if (_tmkdir (strDir) == -1)
			{
				bRes = FALSE;
				break;
			}
		}
		else if ((GetFileAttributes (strDir) & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			bRes = FALSE;
			break;
		}

		strDir += TCHAR('\\');
	}

	return bRes;
}

CString CBCGPBaseInfoWriter::PrepareFileName (const CString& fileName)
{
	CString strFile (fileName);

	if (!strFile.IsEmpty())
	{
		strFile.TrimLeft (_T(" "));
		strFile.TrimRight (_T(" "));

		if (strFile[0] == TCHAR('\"'))
		{
			strFile.TrimLeft (_T("\""));
			strFile.TrimRight (_T("\""));
		}

		if (strFile.Find (TCHAR('/')) != -1)
		{
			strFile.Replace (TCHAR('/'), TCHAR('\\'));
		}

		if (strFile.Find (_T(".\\")) == 0)
		{
			strFile = strFile.Right (strFile.GetLength () - 2);
		}
	}
	
	return strFile;
}

CString CBCGPBaseInfoWriter::CorrectFileName(const CString& dirName, const CString& fileName)
{
	CString str;
	AfxFullPath (str.GetBuffer (_MAX_PATH), CBCGPBaseInfoWriter::CorrectDirectoryName (dirName) + fileName);
	str.ReleaseBuffer ();

	return str;
}

void CBCGPBaseInfoWriter::ParseFileName(const CString& pathName, CString& fileName, CString& extName)
{
	fileName.Empty ();
	extName.Empty ();

#if _MSC_VER < 1400
	_tsplitpath (pathName, NULL, NULL, fileName.GetBuffer (_MAX_FNAME), extName.GetBuffer (_MAX_EXT));
#else
	_tsplitpath_s (pathName, NULL, 0, NULL, 0, fileName.GetBuffer (_MAX_FNAME), _MAX_FNAME, extName.GetBuffer (_MAX_EXT), _MAX_EXT);
#endif

	fileName.ReleaseBuffer ();
	extName.ReleaseBuffer ();
}

void CBCGPBaseInfoWriter::ParseFilePath(const CString& pathName, CString& dirName, CString& fileName)
{
	dirName.Empty ();
	fileName.Empty ();

	TCHAR szDrive[_MAX_DRIVE];
	TCHAR szDir[_MAX_DIR];
	TCHAR szName[_MAX_FNAME];
	TCHAR szExt[_MAX_EXT];
	
#if _MSC_VER < 1400
	_tsplitpath (pathName, szDrive, szDir, szName, szExt);
	_tmakepath (dirName.GetBuffer (_MAX_PATH), szDrive, szDir, NULL, NULL);
#else
	_tsplitpath_s (pathName, szDrive, _MAX_DRIVE, szDir, _MAX_DIR, szName, _MAX_FNAME, szExt, _MAX_EXT);
	_tmakepath_s (dirName.GetBuffer (_MAX_PATH), _MAX_PATH, szDrive, szDir, NULL, NULL);
#endif

	dirName.ReleaseBuffer ();

#if _MSC_VER < 1400
	_tmakepath (fileName.GetBuffer (_MAX_FNAME), NULL, NULL, szName, szExt);
#else
	_tmakepath_s (fileName.GetBuffer (_MAX_PATH), _MAX_PATH, NULL, NULL, szName, szExt);
#endif

	fileName.ReleaseBuffer ();
}

class CFileWriter
{
public:
	CFileWriter()
	{
	}
	virtual ~CFileWriter()
	{
	}

	virtual BOOL Write (const CString& filename)
	{
		if (CBCGPBaseInfoWriter::IsFileReadOnly (filename))
		{
			//CFileParser::ReportSaveLoadException (filename, NULL, TRUE, IDP_ERROR_FILE_READONLY);
			return FALSE;
		}

		CFile* pFile = new CStdioFile;
		CFileException fe;

		if (!pFile->Open (filename, GetFlags (), &fe))
		{
			delete pFile;

			//CFileParser::ReportSaveLoadException (filename, &fe, TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
			return FALSE;
		}

		BOOL bResult = FALSE;

		TRY
		{
			CWaitCursor wait;

			bResult = Write (*pFile);
			
			pFile->Close();
			delete pFile;
		}
		CATCH_ALL(e)
		{
			pFile->Abort();
			delete pFile;
		}
		END_CATCH_ALL

		return bResult;
	}

protected:
	virtual DWORD GetFlags () const
	{
		return CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive;
	}
	virtual BOOL Write (CFile& /*file*/)
	{
		return FALSE;
	}
};

class CFileWriterText: public CFileWriter
{
public:
	CFileWriterText(const CStringArray& text)
		: CFileWriter ()
		, m_Text      (text)
	{

	}
	virtual ~CFileWriterText()
	{
	}

protected:
	virtual BOOL Write (CFile& file)
	{
		CStdioFile& f = static_cast<CStdioFile&>(file);

		int count = (int) m_Text.GetSize ();

		for (int i = 0; i < count; i++)
		{
			f.WriteString (m_Text[i]);
			if (i < (count - 1))
			{
				f.WriteString (CBCGPTagManager::s_szSLFCR);
			}
		}

		return TRUE;
	}

private:
	const CStringArray& m_Text;
};

class CFileWriterBinary: public CFileWriter
{
public:
	CFileWriterBinary (LPCVOID lpBuffer, DWORD dwCount)
		: CFileWriter ()
		, m_lpBuffer  (lpBuffer)
		, m_dwCount   (dwCount)
	{
	}
	virtual ~CFileWriterBinary ()
	{
	}

protected:
	virtual BOOL Write (CFile& file)
	{
		if (m_lpBuffer != NULL && m_dwCount > 0)
		{
			file.Write (m_lpBuffer, m_dwCount);
		}

		return TRUE;
	}

protected:
	virtual DWORD GetFlags () const
	{
		return CFileWriter::GetFlags () | CFile::typeBinary;
	}

private:
	LPCVOID	m_lpBuffer;
	DWORD	m_dwCount;
};


CBCGPBaseInfoWriter::CBCGPBaseInfoWriter(CBCGPBaseInfo& info)
	: m_Info(info)
{
}

CBCGPBaseInfoWriter::~CBCGPBaseInfoWriter()
{
}

BOOL CBCGPBaseInfoWriter::CheckFiles(const CStringArray& sa) const
{
	BOOL bRes = TRUE;

	for (int i = 0; i < sa.GetSize (); i++)
	{
		if (!CBCGPBaseInfoWriter::FileExists (sa[i]))
		{
			continue;
		}

		if (CBCGPBaseInfoWriter::IsFileReadOnly (sa[i]))
		{
			ErrorReportFileRO (sa[i]);

			bRes = FALSE;
			break;
		}
	}

	return bRes;
}

void CBCGPBaseInfoWriter::ErrorReportFolder (const CString& /*strName*/) const
{
}

void CBCGPBaseInfoWriter::ErrorReportFileRO (const CString& /*strName*/) const
{
}

BOOL CBCGPBaseInfoWriter::WriteText (const CString& strFileName, const CStringArray& text) const
{
	CFileWriterText writer (text);
	return writer.CFileWriter::Write (strFileName);
}

BOOL CBCGPBaseInfoWriter::WriteBinary (const CString& strFileName, LPCVOID lpBuffer, int count) const
{
	CFileWriterBinary writer ((LPCVOID)lpBuffer, count);
	return writer.CFileWriter::Write (strFileName);
}

BOOL CBCGPBaseInfoWriter::SaveInfo (const CString& strFileName)
{
	if (strFileName.IsEmpty ())
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CString strBuffer (CBCGPTagManager::s_szXML_UTF8_Prefix);
	GetInfo ().ToTag (strBuffer);

	BOOL bRes = FALSE;

	LPSTR lpBuffer = NULL;
	int count = CBCGPTagManager::StringToUTF8 ((LPCTSTR)strBuffer, lpBuffer);

	if (lpBuffer != NULL)
	{
		bRes = WriteBinary (strFileName, (LPCVOID)lpBuffer, count);

		delete [] lpBuffer;
	}

	return bRes;
}
