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
	//std::string getexepath();

public:
	Registry();
	bool IsHidden();
	bool ToggleHiddenIcon(HWND hWnd);
	bool IsAutorun();
	bool SetAutorun(HWND hWnd);
};
