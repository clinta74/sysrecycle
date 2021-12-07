//---------------------------------------------------------------------------
#define WM_TRAYNOTIFY  (WM_USER + 1001)
#define WM_FILEDELETED (WM_USER + 1002)
//---------------------------------------------------------------------------
NOTIFYICONDATA IconData{};
HMENU PopupMenu = NULL;
HMENU MainMenu = NULL;
MENUITEMINFO ExitMenuItem{};
HICON EmptyIcon = NULL;
HICON FullIcon= NULL;
ULONG m_ulSHChangeNotifyRegister = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool OpenRecycleBin(HWND hWnd);
long long NumFilesInBin(HWND hWnd);
void EmptyRecycleBin(HWND hWnd);