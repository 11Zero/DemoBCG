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
// BCGPRegistry.cpp: implementation of the CBCGPRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BCGPRegistry.h"

#define READ_ONLY_KEYS	(KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY)

IMPLEMENT_DYNCREATE(CBCGPRegistry, CObject)

CBCGPRegistry::CBCGPRegistry()
{
}

CBCGPRegistry::CBCGPRegistry(BOOL bAdmin, BOOL bReadOnly) :
	m_bReadOnly (bReadOnly)
{
	m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	m_bAdmin = bAdmin;
	m_dwUserData = 0;
}

CBCGPRegistry::~CBCGPRegistry()
{
	Close();
}

BOOL CBCGPRegistry::VerifyKey (LPCTSTR pszPath)
{
	ASSERT (m_hKey);
	
	HKEY hKey; // New temporary hKey
	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}
	
	LONG ReturnValue = RegOpenKeyEx (m_hKey, strPath, 0L,
		m_bReadOnly ? READ_ONLY_KEYS : KEY_ALL_ACCESS, &hKey);
	
	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;
	
	if(ReturnValue == ERROR_SUCCESS)
	{
		//If the key exists, then close it.
		RegCloseKey (hKey);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPRegistry::VerifyValue (LPCTSTR pszValue)
{
	ASSERT(m_hKey);
	LONG lReturn = RegQueryValueEx(m_hKey, pszValue, NULL,
		NULL, NULL, NULL);

	m_Info.lMessage = lReturn;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(lReturn == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGPRegistry::CreateKey (LPCTSTR pszPath)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (pszPath != NULL);
	HKEY hKeySave = m_hKey;

	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}

	DWORD dw;

	LONG ReturnValue = RegCreateKeyEx (m_hKey, strPath, 0L, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
		&m_hKey, &dw);

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	TRACE(_T("Can't create registry key: %s\n"), strPath);
	m_hKey = hKeySave;
	return FALSE;
}

BOOL CBCGPRegistry::Open (LPCTSTR pszPath)
{
	ASSERT (pszPath != NULL);

	HKEY hKeySave = m_hKey;
	m_sPath = pszPath;

	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}

	LONG ReturnValue = RegOpenKeyEx (m_hKey, strPath, 0L,
		m_bReadOnly ? READ_ONLY_KEYS : KEY_ALL_ACCESS, &m_hKey);

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	m_hKey = hKeySave;
	return FALSE;
}

void CBCGPRegistry::Close()
{
	if (m_hKey)
	{
		RegCloseKey (m_hKey);
		m_hKey = NULL;
	}
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, int iVal)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	DWORD dwValue;

	ASSERT(m_hKey);
	
	dwValue = (DWORD)iVal;
	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_DWORD,
		(CONST BYTE*) &dwValue, sizeof(DWORD));

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = sizeof(DWORD);
	m_Info.dwType = REG_DWORD;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, DWORD dwVal)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_DWORD,
		(CONST BYTE*) &dwVal, sizeof(DWORD));

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, double dVal)
{
    if (m_bReadOnly)
    {
        ASSERT (FALSE);
        return FALSE;
    }

    ASSERT(m_hKey);
    LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_BINARY,
        (CONST BYTE*)&dVal, sizeof (double));

    if(ReturnValue == ERROR_SUCCESS)
        return TRUE;

    return FALSE;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, LPCTSTR pszData)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	ASSERT(pszData);
	ASSERT(AfxIsValidAddress(pszData, _tcslen(pszData), FALSE));

	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_SZ,
		(CONST BYTE*) pszData, (DWORD) (_tcslen(pszData) + 1) * (sizeof *pszData));

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = (DWORD) _tcslen(pszData) + 1;
	m_Info.dwType = REG_SZ;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CStringList& scStringList)
{
		return Write (pszKey, (CObject&) scStringList);
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CObList& list)
{
	return Write (pszKey, (CObject&) list);
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CObject& obj)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			obj.Serialize (ar);
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CObject* pObj)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			ar << pObj;
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CByteArray& bcArray)
{
	return Write (pszKey, (CObject&) bcArray);
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CDWordArray& dwcArray)
{
	return Write (pszKey, (CObject&) dwcArray);
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CWordArray& wcArray)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			ar << (int) wcArray.GetSize ();
			for (int i = 0; i < wcArray.GetSize (); i ++)
			{
				ar << wcArray [i];
			}

			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, CStringArray& scArray)
{
	return Write (pszKey, (CObject&) scArray);
}

BOOL CBCGPRegistry::Write(LPCTSTR pszKey, const CRect& rect)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			ar << rect;
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGPRegistry::Write(LPCTSTR pszKey, LPPOINT& lpPoint)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	BYTE* pData = NULL;
	DWORD dwLen = 0;

	{
		CMemFile file(32);
		CArchive ar(&file, CArchive::store);

		CDWordArray dwcArray;
		dwcArray.SetSize(2);
		dwcArray.SetAt(0, lpPoint->x);
		dwcArray.SetAt(1, lpPoint->y);

		ASSERT(dwcArray.IsSerializable());
		dwcArray.Serialize(ar);
		ar.Close();

		dwLen = (DWORD) file.GetLength();
		pData = file.Detach();
	}

	ASSERT(pData != NULL && dwLen > 0);

	LONG lReturn = RegSetValueEx(m_hKey, pszKey, 0, REG_BINARY,
		pData, dwLen);
	
	m_Info.lMessage = lReturn;
	m_Info.dwSize = dwLen;
	m_Info.dwType = REG_POINT;

	if (pData != NULL)
	{
		free(pData);
		pData = NULL;
	}

	if(lReturn == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGPRegistry::Write (LPCTSTR pszKey, LPBYTE pData, UINT nBytes)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (m_hKey != NULL);
	ASSERT (pszKey != NULL);
	ASSERT (pData != NULL);
	ASSERT (AfxIsValidAddress (pData, nBytes, FALSE));

	LONG lResult = ::RegSetValueEx (m_hKey, pszKey, NULL, REG_BINARY,
									pData, nBytes);

	m_Info.lMessage = lResult;
	m_Info.dwSize = nBytes;
	m_Info.dwType = REG_BINARY;

	return (lResult == ERROR_SUCCESS);
}

BOOL CBCGPRegistry::Read(LPCTSTR pszKey, int& iVal)
{
	ASSERT(m_hKey);

	DWORD dwType;
	DWORD dwSize = sizeof (DWORD);
	DWORD dwDest;

	LONG lReturn = RegQueryValueEx (m_hKey, (LPTSTR) pszKey, NULL,
		&dwType, (BYTE *) &dwDest, &dwSize);

	m_Info.lMessage = lReturn;
	m_Info.dwType = dwType;
	m_Info.dwSize = dwSize;

	if(lReturn == ERROR_SUCCESS)
	{
		iVal = (int)dwDest;
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, DWORD& dwVal)
{
	ASSERT(m_hKey);

	DWORD dwType;
	DWORD dwSize = sizeof (DWORD);
	DWORD dwDest;

	LONG lReturn = RegQueryValueEx (m_hKey, (LPTSTR) pszKey, NULL, 
		&dwType, (BYTE *) &dwDest, &dwSize);

	m_Info.lMessage = lReturn;
	m_Info.dwType = dwType;
	m_Info.dwSize = dwSize;

	if(lReturn == ERROR_SUCCESS)
	{
		dwVal = dwDest;
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, double& dVal)
{
    ASSERT(m_hKey);

    DWORD dwType;
    DWORD dwSize = sizeof (double);
    double dDest;

    LONG lReturn = RegQueryValueEx (m_hKey, (LPTSTR) pszKey, NULL, 
        &dwType, (BYTE *) &dDest, &dwSize);

    m_Info.lMessage = lReturn;
    m_Info.dwType = dwType;
    m_Info.dwSize = dwSize;

    if (lReturn == ERROR_SUCCESS)
    {
        dVal = dDest;
        return TRUE;
    }

    return FALSE;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CString& sVal)
{
    ASSERT (m_hKey != NULL); 
    ASSERT (pszKey != NULL); 
	
    UINT  nBytes = 0; 
    DWORD dwType = 0; 
    DWORD dwCount = 0;
	
    LONG lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType, 
		NULL, &dwCount); 
	
    if (lResult == ERROR_SUCCESS && dwCount > 0)
    { 
		nBytes = dwCount; 
		ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);

		
		BYTE* pData = new BYTE [nBytes + 1];
		
		lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType, 
			pData, &dwCount); 
		
		if (lResult == ERROR_SUCCESS &&  dwCount > 0) 
		{ 
			ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);
			sVal = (TCHAR*)pData; 
		} 
		else
		{
			sVal.Empty ();
		}
		
		delete [] pData;
		pData = NULL; 
    } 
	else
	{
		sVal.Empty ();
	}
	
    m_Info.lMessage = lResult; 
    m_Info.dwType = dwType; 
    m_Info.dwSize = nBytes; 
	
    return lResult == ERROR_SUCCESS;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CStringList& scStringList)
{
	scStringList.RemoveAll ();
	return Read (pszKey, (CObject&) scStringList);
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CByteArray& bcArray)
{
	bcArray.RemoveAll();
	return Read (pszKey, (CObject&) bcArray);
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CDWordArray& dwcArray)
{
	dwcArray.RemoveAll ();
	dwcArray.SetSize (0);
	return Read (pszKey, (CObject&) dwcArray);
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CWordArray& wcArray)
{
	wcArray.SetSize (0);

	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		int iSize;
		ar >> iSize;

		wcArray.SetSize (iSize);
		for (int i = 0; i < iSize; i ++)
		{
			ar >> wcArray [i];
		}

		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPRegistry::Read ()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CStringArray& scArray)
{
	scArray.RemoveAll ();
	return Read (pszKey, (CObject&) scArray);
}

BOOL CBCGPRegistry::Read(LPCTSTR pszKey, CRect& rect)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		ar >> rect;
		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPRegistry::Read ()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CBCGPRegistry::Read(LPCTSTR pszKey, LPPOINT& lpPoint)
{
	ASSERT(m_hKey);

	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	Read (pszKey, &pData, &uDataSize);

	try
	{
		if (pData != NULL && m_Info.lMessage == ERROR_SUCCESS && m_Info.dwType == REG_BINARY)
		{
			CMemFile file (pData, uDataSize);
			CArchive ar (&file, CArchive::load);

			ar.m_bForceFlat = FALSE;
			ASSERT(ar.IsLoading());

			CDWordArray dwcArray;
			ASSERT(dwcArray.IsSerializable());
			dwcArray.Serialize(ar);
			ar.Close();

			if (dwcArray.GetSize() == 2)
			{
				lpPoint->x = dwcArray.GetAt(0);
				lpPoint->y = dwcArray.GetAt(1);

				bSucess = TRUE;
			}
		}
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPRegistry::Read ()!\n"));
	}

	m_Info.dwType = REG_POINT;
	m_Info.dwSize = sizeof(POINT);

	if (pData != NULL)
	{
		delete [] pData;
		pData = NULL;
	}

	return bSucess;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, BYTE** ppData, UINT* pBytes)
{
	ASSERT (m_hKey != NULL);
	ASSERT (pszKey != NULL);
	ASSERT(ppData != NULL);
	ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;

	DWORD dwType, dwCount;
	LONG lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType,
		NULL, &dwCount);

	if (lResult == ERROR_SUCCESS && dwCount > 0) 
	{
		*pBytes = dwCount;
		ASSERT (dwType == REG_BINARY);

		*ppData = new BYTE [*pBytes];

		lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType,
			*ppData, &dwCount);

		if (lResult == ERROR_SUCCESS)
		{
			ASSERT (dwType == REG_BINARY);
		}
		else
		{
			delete [] *ppData;
			*ppData = NULL;
		}
	}

	m_Info.lMessage = lResult;
	m_Info.dwType = REG_BINARY;
	m_Info.dwSize = *pBytes;

	return (lResult == ERROR_SUCCESS);
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CObList& list)
{
	while (!list.IsEmpty ())
	{
		delete list.RemoveHead ();
	}

	return Read (pszKey, (CObject&) list);
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CObject& obj)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		obj.Serialize (ar);
		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPRegistry::Read ()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CBCGPRegistry::Read (LPCTSTR pszKey, CObject*& pObj)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		ar >> pObj;

		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGPRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGPRegistry::Read ()!\n"));
	}

	delete [] pData;
	return bSucess;
}

BOOL CBCGPRegistry::DeleteValue (LPCTSTR pszValue)
{
	ASSERT(m_hKey);
	LONG lReturn = RegDeleteValue(m_hKey, pszValue);


	m_Info.lMessage = lReturn;
	m_Info.dwType = 0L;
	m_Info.dwSize = 0L;

	if(lReturn == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGPRegistry::DeleteKey (LPCTSTR pszPath, BOOL bAdmin)
{
	if (m_bReadOnly)
	{
		return FALSE;
	}
	
	ASSERT (pszPath != NULL);
	
	CString strPath = pszPath;

	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}
	
	// open the key
	HKEY hSubKey;
	LONG lReturn = ::RegOpenKeyEx (bAdmin ? HKEY_LOCAL_MACHINE :
	HKEY_CURRENT_USER,
		strPath, 0L, KEY_ALL_ACCESS, &hSubKey);
	
	if(lReturn != ERROR_SUCCESS)
		return FALSE;
	
	// first, delete all subkeys (else it won't work on NT!)
	for( DWORD dwSubKeys = 1; dwSubKeys > 0; )
	{
		dwSubKeys = 0;
		
		// first get an info about this subkey ...
		DWORD dwSubKeyLen;
		if( ::RegQueryInfoKey( hSubKey, 0,0,0, &dwSubKeys, &dwSubKeyLen,
			0,0,0,0,0,0) != ERROR_SUCCESS)
		{
			::RegCloseKey(hSubKey);
			return FALSE;
		}
		
		if( dwSubKeys > 0 )
		{
			// call DeleteKey() recursivly
			LPTSTR szSubKeyName = new TCHAR[dwSubKeyLen + 1];
			
			if( ::RegEnumKey( hSubKey, 0, szSubKeyName, dwSubKeyLen+1) !=
				ERROR_SUCCESS
				|| ! DeleteKey( strPath + _T("\\") + szSubKeyName, bAdmin ) )
			{
				delete [] szSubKeyName;
				::RegCloseKey(hSubKey);
				return FALSE;
			}
			
			delete [] szSubKeyName;
		}
	}
	::RegCloseKey(hSubKey);
	
	// finally delete the whole key
	lReturn = ::RegDeleteKey (bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
		strPath);
	m_Info.lMessage = lReturn;
	m_Info.dwType = 0L;
	m_Info.dwSize = 0L;
	
	if(lReturn == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGPRegistry::ReadSubKeys(CStringArray& SubKeys)
{
	BOOL result = TRUE;
	DWORD rc = ERROR_SUCCESS;

	const DWORD iMaxChars = 1024;
	TCHAR szSubKey[iMaxChars] = {0};
	DWORD length = iMaxChars;

	ASSERT(m_hKey);

	int index = 0;
	rc = RegEnumKeyEx(m_hKey, index, szSubKey, &length, NULL, NULL, NULL, NULL);

	if( rc == ERROR_NO_MORE_ITEMS)
	{
		result = false;
	}
	else while(rc == ERROR_SUCCESS)
	{
		SubKeys.Add(szSubKey);
		length = iMaxChars;
		index++;
		rc = RegEnumKeyEx(m_hKey, index, szSubKey, &length, NULL, NULL, NULL, NULL);
		
	} // while

	return result;
}


BOOL CBCGPRegistry::ReadKeyValues(CStringArray &Values)
{
	DWORD rc = ERROR_SUCCESS;
	BOOL result = FALSE;

	const DWORD iMaxChars = 1024;
	TCHAR szValue[iMaxChars] = {0};
	DWORD length = iMaxChars;
	int index = 0;

	ASSERT(m_hKey);

	rc = RegEnumValue(m_hKey, index, szValue, &length, NULL, NULL, NULL, NULL);
	if( rc == ERROR_NO_MORE_ITEMS)
	{
	  result = FALSE;
	}
	else while( rc == ERROR_SUCCESS )
	{
		result = TRUE;
		Values.Add( szValue );
		length = iMaxChars;
		index++;
		rc = RegEnumValue(m_hKey, index, szValue, &length, NULL, NULL, NULL, NULL);
	}

	return result;

}

//////////////////////////////////////////////////////////////////////////////
// CBCGRegistrySP - Helper class that manages "safe" CBCGRegistry pointer

CRuntimeClass* CBCGPRegistrySP::m_pRTIDefault = NULL;

BOOL CBCGPRegistrySP::SetRuntimeClass (CRuntimeClass* pRTI)
{
	if (pRTI != NULL &&
		!pRTI->IsDerivedFrom (RUNTIME_CLASS (CBCGPRegistry)))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_pRTIDefault = pRTI;
	return TRUE;
}

CBCGPRegistry& CBCGPRegistrySP::Create (BOOL bAdmin, BOOL bReadOnly)
{
	if (m_pRegistry != NULL)
	{
		ASSERT (FALSE);
		ASSERT_VALID (m_pRegistry);

		return *m_pRegistry;
	}

	if (m_pRTIDefault == NULL)
	{
		m_pRegistry = new CBCGPRegistry;
	}
	else
	{
		ASSERT (m_pRTIDefault->IsDerivedFrom (RUNTIME_CLASS (CBCGPRegistry)));
		m_pRegistry = DYNAMIC_DOWNCAST (CBCGPRegistry, 
										m_pRTIDefault->CreateObject ());
	}

	ASSERT_VALID (m_pRegistry);

	m_pRegistry->m_bReadOnly = bReadOnly;
	m_pRegistry->m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	m_pRegistry->m_bAdmin = bAdmin;
	m_pRegistry->m_dwUserData = m_dwUserData;

	return *m_pRegistry;
}

