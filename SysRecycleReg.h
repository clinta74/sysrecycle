class Registry
{
private:
	bool OpenRegKey(HKEY* hKey);
	bool CloseRegKey(HKEY hKey);
	void RedrawDesktop();
	bool OpenAutorunRegKey(HKEY* hKey);
	bool CloseAutorunRegKey(HKEY hKey);
	LPWSTR GetExecutablePath();

public:
	Registry();
	bool IsHidden();
	bool ToggleHiddenIcon(HWND hWnd);
	bool IsAutorun();
	bool SetAutorun(HWND hWnd);
};
