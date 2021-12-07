#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <string>
#include "SysRecycleReg.h"

Registry::Registry() {}

bool Registry::OpenRegKey(HKEY* phKey)
{
	char KeyName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel";
	long lResult;

	lResult = ::RegCreateKeyEx(HKEY_CURRENT_USER, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, phKey, NULL);
	if (lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::CloseRegKey(HKEY hKey)
{
	long lResult;

	lResult = ::RegCloseKey(hKey);
	if (lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::OpenAutorunRegKey(HKEY* phKey)
{
	char KeyName[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	long lResult;

	lResult = ::RegCreateKeyEx(HKEY_CURRENT_USER, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, phKey, NULL);
	if (lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::CloseAutorunRegKey(HKEY hKey)
{
	long lResult;

	lResult = ::RegCloseKey(hKey);
	if (lResult == ERROR_SUCCESS) return true;
	return false;
}

bool Registry::IsHidden()
{
	char ValueName[] = "{645FF040-5081-101B-9F08-00AA002F954E}";
	DWORD value = 0;
	DWORD dwLen = sizeof(DWORD);
	long lResult = 0;
	HKEY hKey = NULL;

	if (OpenRegKey(&hKey))
	{
		lResult = ::RegQueryValueEx(hKey, ValueName, NULL, NULL, (LPBYTE)&value, &dwLen);
		CloseRegKey(hKey);
	}

	if (lResult == ERROR_SUCCESS)
	{
		if (value == 1) return true;
		else return false;
	}
	return false;
}

bool Registry::IsAutorun()
{
	char ValueName[] = "SysRecycle";
	BYTE byteArray[2048] = { 0 };
	DWORD dwLen = sizeof(byteArray);
	HKEY hKey = NULL;

	if (OpenAutorunRegKey(&hKey))
	{
		long lResult = ::RegQueryValueEx(hKey, ValueName, NULL, NULL, byteArray, &dwLen);
		if (lResult == ERROR_SUCCESS)
		{
			LPWSTR executablePath = GetExecutablePath();
			bool match = true;
			for (DWORD ct = 0; ct < dwLen; ct++, executablePath++)
			{
				if (executablePath[0] != byteArray[ct])
				{
					match = false;
					break;
				}
			}

			return match;
		}
		CloseAutorunRegKey(hKey);
	}

	return false;
}

LPWSTR Registry::GetExecutablePath()
{
	int pNumArgs = 0;
	LPWSTR commandLine = GetCommandLineW();
	LPWSTR* szArglist = CommandLineToArgvW(commandLine, &pNumArgs);

	if (pNumArgs > 0) return szArglist[0];
	return NULL;
}

bool Registry::ToggleHiddenIcon(HWND hWnd)
{
	char ValueName[] = "{645FF040-5081-101B-9F08-00AA002F954E}";
	DWORD value = 0;
	DWORD dwLen = sizeof(DWORD);
	long lResult = 0;
	HKEY hKey = NULL;

	if (IsHidden()) value = 0;
	else value = 1;

	if (OpenRegKey(&hKey))
	{
		lResult = ::RegSetValueEx(hKey, ValueName, 0, REG_DWORD, (LPBYTE)&value, dwLen);
		CloseRegKey(hKey);
	}

	if (lResult == ERROR_SUCCESS)
	{
		RedrawDesktop();
		return true;
	}
	else MessageBox(hWnd, "Could not access the Registry to remove Recycle Bin from desktop.", "Error", 0);
	return false;
}

bool Registry::SetAutorun(HWND hWnd)
{
	LPCWSTR keyName = L"SysRecycle";
	size_t dwLen = sizeof(DWORD);
	LSTATUS lResult = 0;
	HKEY hKey = NULL;

	if (OpenRegKey(&hKey))
	{
		bool isAutorun = IsAutorun();
		if (isAutorun)
		{
			lResult = ::RegDeleteKeyValueW(hKey, NULL, keyName);
		}
		else
		{
			const size_t count = MAX_PATH * 2;
			wchar_t szValue[count] = {};

			LPWSTR pathToExe = GetExecutablePath();

			wcscpy_s(szValue, count, L"\"");
			wcscat_s(szValue, count, pathToExe);
			wcscat_s(szValue, count, L"\" ");

			dwLen = (wcslen(szValue) + 1) * 2;

			lResult = ::RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, (KEY_WRITE | KEY_READ), NULL, &hKey, NULL);
			if (lResult == ERROR_SUCCESS)
			{
				lResult = ::RegSetValueExW(hKey, keyName, 0, REG_SZ, (BYTE*)szValue, (DWORD)dwLen);
			}
		}
		CloseRegKey(hKey);
		return lResult == ERROR_SUCCESS;
	}
	else
	{
		MessageBox(hWnd, "Could not access the Registry to change Autorun for SysRecycle.", "Error", 0);
		return false;
	}
}

void Registry::RedrawDesktop()
{
	::SHChangeNotify(SHCNE_ASSOCCHANGED, 0, 0, 0);
}
