// test_104_10.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinInet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

typedef DWORD (WINAPI* pgetadaptersinfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen );

BOOL IsPhysicalAddress(const char *pAdapterName)
{
	LPTSTR lpSubKey = L"System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\";
	wchar_t szAdapterKey[MAX_PATH + 1];
	wchar_t szDataBuf[MAX_PATH + 1];
	DWORD dwDataLen = MAX_PATH;
	DWORD dwType = REG_SZ;
	HKEY hNetKey = NULL;
	HKEY hLocalNet = NULL;

	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hNetKey)) {
		cout << "fail at regopenkeyex" << lpSubKey << endl;
		return FALSE;
	}

	wchar_t szAdapterName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, pAdapterName,  strlen(pAdapterName)+1, szAdapterName, MAX_PATH/sizeof(wchar_t));  
	swprintf_s(szAdapterKey, L"%s\\Connection\\", szAdapterName);
	if(ERROR_SUCCESS != RegOpenKeyEx(hNetKey ,szAdapterKey ,0 ,KEY_READ, &hLocalNet)) {
		cout << "fail at regopenkeyex" << szAdapterKey << endl;
		RegCloseKey(hNetKey);
		return FALSE;
	}
	if (ERROR_SUCCESS != RegQueryValueEx(hLocalNet, L"PnpInstanceID", 0, &dwType, (BYTE *)szDataBuf, &dwDataLen)) {
		cout << "fail at RegQueryValueEx" << endl;
		RegCloseKey(hLocalNet);
		RegCloseKey(hNetKey);
		return FALSE;
	}
	if (wcsncmp(szDataBuf, L"PCI", wcslen(L"PCI")) && wcsncmp(szDataBuf, L"USB", wcslen(L"USB"))) {
		cout << "fail at wcsncmp" << endl;
		RegCloseKey(hLocalNet);
		RegCloseKey(hNetKey);
		return FALSE;
	}

	RegCloseKey(hLocalNet);
	RegCloseKey(hNetKey);

	return TRUE;
}

size_t ADTCountMachineIdentifier(void)
{
	
	HINSTANCE hDll = NULL;
	hDll = LoadLibrary(L"iphlpapi.dll");
	size_t nCount = 0;
	if(hDll != NULL) {
		pgetadaptersinfo GetAdaptersInfo = NULL;
		GetAdaptersInfo = (pgetadaptersinfo)GetProcAddress(hDll,"GetAdaptersInfo");
		if(GetAdaptersInfo != NULL) {
			IP_ADAPTER_INFO AdapterInfo[16];
			DWORD bufLen = sizeof(AdapterInfo);
			if( GetAdaptersInfo(AdapterInfo,&bufLen) == ERROR_SUCCESS ) {
				PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
				while (NULL != pAdapterInfo) {
					cout << pAdapterInfo->AdapterName <<endl;
					if (IsPhysicalAddress(pAdapterInfo->AdapterName)) {
						++nCount;
					}
					pAdapterInfo = pAdapterInfo->Next;
				}
			}
		}
	}
	if(!nCount)
	{
		throw new CFileException(4);//operating system compatibility -- cannot read mac address from the machines
	}
	return nCount;
}

int _tmain(int argc, _TCHAR* argv[])
{
	size_t nCount = ADTCountMachineIdentifier();
	std::cout << nCount <<std::endl;
	cin >> nCount;
	return 0;
}