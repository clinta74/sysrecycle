//---------------------------------------------------------------------------
#define WM_TRAYNOTIFY  (WM_USER + 1001)
#define WM_FILEDELETED (WM_USER + 1002)
//---------------------------------------------------------------------------
NOTIFYICONDATA IconData;
HMENU PopupMenu;
HMENU MainMenu;
MENUITEMINFO ExitMenuItem;
HICON EmptyIcon;
HICON FullIcon;
ULONG m_ulSHChangeNotifyRegister;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool OpenRecycleBin(HWND hWnd);
long NumFilesInBin(HWND hWnd);
void EmptyRecycleBin(HWND hWnd);