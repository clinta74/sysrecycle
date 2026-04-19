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
	LPCWSTR ValueName = L"SysRecycle";
	wchar_t storedValue[MAX_PATH * 2] = { 0 };
	DWORD dwLen = sizeof(storedValue);
	HKEY hKey = NULL;
	bool match = false;

	if (OpenAutorunRegKey(&hKey))
	{
		long lResult = ::RegQueryValueExW(hKey, ValueName, NULL, NULL, (LPBYTE)storedValue, &dwLen);
		if (lResult == ERROR_SUCCESS)
		{
			LPWSTR executablePath = GetExecutablePath();
			if (executablePath != NULL)
			{
				// Build the same quoted path that SetAutorun writes
				wchar_t expected[MAX_PATH * 2] = {};
				wcscpy_s(expected, _countof(expected), L"\"");
				wcscat_s(expected, _countof(expected), executablePath);
				wcscat_s(expected, _countof(expected), L"\" ");
				match = (wcscmp(storedValue, expected) == 0);
				LocalFree(executablePath);
			}
		}
		CloseAutorunRegKey(hKey);
	}

	return match;
}

// Returns a LocalAlloc'd wide string with the full path to this executable.
// Caller must free the returned pointer with LocalFree().
LPWSTR Registry::GetExecutablePath()
{
	const DWORD size = MAX_PATH * 2;
	LPWSTR path = (LPWSTR)LocalAlloc(LPTR, size * sizeof(wchar_t));
	if (path == NULL) return NULL;
	if (GetModuleFileNameW(NULL, path, size) == 0)
	{
		LocalFree(path);
		return NULL;
	}
	return path;
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
			if (pathToExe == NULL)
			{
				CloseRegKey(hKey);
				return false;
			}

			wcscpy_s(szValue, count, L"\"");
			wcscat_s(szValue, count, pathToExe);
			wcscat_s(szValue, count, L"\" ");
			LocalFree(pathToExe);

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
