class Registry
{
private:
	HKEY hKey;
	HKEY hKeyAutorun;

	bool OpenRegKey();
	bool CloseRegKey();
	void RefreshDesktopIcons();
	void RedrawDesktop();
	bool OpenAutorunRegKey();
	bool CloseAutorunRegKey();
	LPWSTR GetExecutablePath();

public:
	Registry();
	bool IsHidden();
	bool ToggleHiddenIcon(HWND hWnd);
	bool IsAutorun();
	bool SetAutorun(HWND hWnd);
};
