#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <string>
#include "SysRecycleReg.h"

bool Registry::OpenRegKey()
{
	char KeyName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel";
	long lResult;

	lResult = ::RegCreateKeyEx(HKEY_CURRENT_USER, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hKey, NULL); 
	if(lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::CloseRegKey()
{
	long lResult;

	lResult = ::RegCloseKey(hKey);
	if(lResult == ERROR_SUCCESS) return true;
	return false;
}
	
bool Registry::OpenAutorunRegKey()
{
	char KeyName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	long lResult;

	lResult = ::RegCreateKeyEx(HKEY_CURRENT_USER, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hKeyAutorun, NULL); 
	if(lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::CloseAutorunRegKey()
{
	long lResult;

	lResult = ::RegCloseKey(hKeyAutorun);
	if(lResult == ERROR_SUCCESS) return true;
	return false;
}
Registry::Registry()
{
}

bool Registry::IsHidden()
{
    char ValueName[] = "{645FF040-5081-101B-9F08-00AA002F954E}";
	DWORD value;
	DWORD dwLen = sizeof(DWORD);
	long lResult;

	if(OpenRegKey())
	{
		lResult = ::RegQueryValueEx(hKey, ValueName, NULL, NULL, (LPBYTE)&value, &dwLen);
		CloseRegKey();
	}

	if(lResult == ERROR_SUCCESS)
	{
		if(value == 1) return true;
		else return false;
	}
	return false;
}

bool Registry::IsAutorun()
{
    char ValueName[] = "SysRecycle";
	DWORD value;
	DWORD dwLen = sizeof(DWORD);
	long lResult;

	if(OpenAutorunRegKey())
	{
		lResult = ::RegQueryValueEx(hKey, ValueName, NULL, NULL, (LPBYTE)&value, &dwLen);
		CloseAutorunRegKey();
	}

	if(lResult == ERROR_SUCCESS)
	{
		if(value == 1) return true;
		else return false;
	}
	return false;
}

bool Registry::ToggleHiddenIcon(HWND hWnd)
{
    char ValueName[] = "{645FF040-5081-101B-9F08-00AA002F954E}";
	DWORD value;
	DWORD dwLen = sizeof(DWORD);
	long lResult;

	if(IsHidden()) value = 0;
	else value = 1;

	if(OpenRegKey())
	{
		lResult = ::RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (LPBYTE)&value, dwLen);
		CloseRegKey();
	}

	if(lResult == ERROR_SUCCESS)
	{
		RedrawDesktop();
		// RefreshDesktopIcons();
		return true;
	}
	else MessageBox(hWnd, "Could not access the Registry to remove Recycle Bin from desktop.", "Error", 0);
	return false;
}

bool Registry::SetAutorun(HWND hWnd)
{
    char ValueName[] = "SysRecycle";
	DWORD value;
	DWORD dwLen = sizeof(DWORD);
	long lResult;

	if(IsAutorun()) value = 0;
	else value = 1;

	if(OpenRegKey())
	{
		lResult = ::RegSetValueEx(hKey, ValueName, 0, REG_SZ, (LPBYTE)&value, dwLen);
		CloseRegKey();
	}

	if(lResult == ERROR_SUCCESS)
	{
		RedrawDesktop();
		// RefreshDesktopIcons();
		return true;
	}
	else MessageBox(hWnd, "Could not access the Registry to change Autorun for SysRecycle.", "Error", 0);
	return false;
}

void Registry::RefreshDesktopIcons()
{
    char val[256];
    HKEY hKey;

    long lResult =::RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop\\WindowMetrics", 0, KEY_READ, &hKey);
    if(lResult != ERROR_SUCCESS) return;
	char buff[256] = {0};
    DWORD sz = sizeof(buff);
    DWORD typ = REG_SZ;
	::RegQueryValueEx(hKey,"Shell Icon Size",0,&typ,(LPBYTE)buff,&sz);
	::RegCloseKey(hKey);

    int i = atoi(buff);
    sprintf_s(val,"%d",i + 1);

    ::RegOpenKeyEx(HKEY_CURRENT_USER,"Control Panel\\Desktop\\WindowMetrics",0,KEY_WRITE,&hKey);
	::RegSetValueEx(hKey,"Shell Icon Size",0,REG_SZ,(LPBYTE)val,(DWORD)strlen(val));
	::RegCloseKey(hKey);

    ::SendMessage(HWND_BROADCAST,WM_SETTINGCHANGE,SPI_SETNONCLIENTMETRICS,NULL);

	i = atoi(buff);
	sprintf_s(val,"%d",i);
    RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop\\WindowMetrics", 0,KEY_WRITE, &hKey);
	::RegSetValueEx(hKey,"Shell Icon Size",0,REG_SZ,(LPBYTE)val, (DWORD)strlen(val));
	::RegCloseKey(hKey);

	::SendMessage(HWND_BROADCAST,WM_SETTINGCHANGE,SPI_SETNONCLIENTMETRICS,NULL);
}

void Registry::RedrawDesktop()
{
	::SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
}
