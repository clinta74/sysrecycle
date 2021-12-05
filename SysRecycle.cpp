#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <assert.h>

#include "SysRecycle.h"
#include "SysRecycleReg.h"

const int IDC_TRAY1      = 1000;
const char *HINT_MESSAGE = "SysRecycle";
Registry *Reg;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wcex;
	EmptyIcon = LoadIcon(hInstance, "EMPTYICON");
	FullIcon = LoadIcon(hInstance, "FULLICON");

	Reg = new Registry();

	if (!nCmdShow) exit(0);
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc= (WNDPROC)WndProc;
	wcex.cbClsExtra= 0;
	wcex.cbWndExtra= 0;
	wcex.hInstance= hInstance;
	wcex.hIcon= EmptyIcon;
	wcex.hCursor= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName= 0;
	wcex.lpszClassName= "MyWindowClass";
	wcex.hIconSm= EmptyIcon;

	/* Now we can go ahead and register our new window class */
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindow("MyWindowClass", "The title of the Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	IconData.cbSize = sizeof(NOTIFYICONDATA);
	IconData.uID    = IDC_TRAY1;
	IconData.hWnd   = hWnd;
	IconData.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	IconData.uCallbackMessage = WM_TRAYNOTIFY;
	lstrcpy(IconData.szTip, HINT_MESSAGE);
	if (NumFilesInBin(hWnd) == 0)
		IconData.hIcon  = EmptyIcon;
	else
		IconData.hIcon  = FullIcon;

	MainMenu = LoadMenu(hInstance, "POPUPMENU");
	if (MainMenu == NULL)
		MessageBox(hWnd, "Menu not found.", "Error", 0);

	PopupMenu = GetSubMenu(MainMenu, 0);

	Shell_NotifyIcon(NIM_ADD,&IconData);

	LPITEMIDLIST ppidl;
    if(SHGetSpecialFolderLocation(hWnd, CSIDL_DESKTOP, &ppidl) == NOERROR)
    {
        SHChangeNotifyEntry shCNE;
        shCNE.pidl = ppidl;
        shCNE.fRecursive = TRUE;

        // Returns a positive integer registration identifier (ID).
        // Returns zero if out of memory or in response to invalid parameters.
        m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWnd,                                            // Hwnd to receive notification
                SHCNRF_ShellLevel,                                                                    // Event types of interest (sources)
                SHCNE_RMDIR|SHCNE_DELETE,                                              // Events of interest - use                                           // SHCNE_ALLEVENTS for all events
                WM_FILEDELETED,                                               // Notification message to be sent                                           // upon the event
                1,                                                                   // Number of entries in the pfsne array
                &shCNE);  // Array of SHChangeNotifyEntry structures that                           // contain the notifications. This array should                           // always be set to one when calling SHChnageNotifyRegister
                          // or SHChangeNotifyDeregister will not work properly.
    
        assert(m_ulSHChangeNotifyRegister != 0);    // Shell notification failed
    }
    else
        assert(FALSE);    // Failed to get desktop location

	UpdateWindow(hWnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO menuItemInfo = {0};
	char menuText[256];

	Registry *Reg = new Registry;

	switch (message)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case 5:
			OpenRecycleBin(hWnd);
			break;
		case 4:
			EmptyRecycleBin(hWnd);
			if (NumFilesInBin(hWnd) == 0)
			{
				IconData.hIcon = EmptyIcon;
				Shell_NotifyIcon(NIM_MODIFY, &IconData);
			}
			break;
		case 3:
			Reg->ToggleHiddenIcon(hWnd);
			break;
		case 2:
			MessageBox(hWnd,
				"SysRecycle (Freeware)\n"
				"for Windows 98, Me, 2000, XP, & Vista\n"
				"Version 3.0\n"
				"by Clinton Andrews\n"
				"intelliclint@chartermi.net\n"
				"Copyright (C) 2009 Clinton Andrews\n\n"
				"THIS SOFTWARE IS PROVIDED 'AS IS'. THE DEVELOPER OF THIS SOFTWARE WILL NOT BE "
				"HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR "
				"CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOSS OF USE, DATA, OR "
				"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF "
				"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE "
				"OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF "
				"ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
				"THIS SOFTWARE CAN NOT BE RESOLD WITH EXPRESS PREMISSION OF THE AUTHOR "
				"CLINTON ANDREWS."
				,"About", 0);
			break;
		case 1:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_TRAYNOTIFY:
		switch(LOWORD(lParam))
		{
		case WM_RBUTTONUP:
			POINT WinPoint;           // find the mouse cursor
			GetCursorPos(&WinPoint);  // using api function, store
			SetForegroundWindow(hWnd);
			menuItemInfo.cbSize = sizeof(MENUITEMINFO);
			menuItemInfo.fMask = MIIM_STATE;
			if(!GetMenuItemInfo(PopupMenu, 3, false, &menuItemInfo))
				exit(GetLastError());
			else
			{
				if(Reg->IsHidden())
					menuItemInfo.fState = MFS_CHECKED | MFS_ENABLED;
				else
					menuItemInfo.fState = MFS_ENABLED;
				SetMenuItemInfo(PopupMenu, 3, false, &menuItemInfo);
			}

			menuItemInfo.cbSize = sizeof(MENUITEMINFO);
			menuItemInfo.fMask = MIIM_TYPE;
			menuItemInfo.fType = MFT_STRING;
			if(!GetMenuItemInfo(PopupMenu, 6, false, &menuItemInfo))
				exit(GetLastError());
			else
			{
				menuItemInfo.fState = MFS_ENABLED;
				menuItemInfo.dwTypeData = menuText;
				sprintf_s(menuText, "%i files", NumFilesInBin(hWnd));
				menuItemInfo.cch = (UINT)strlen(menuText);
				SetMenuItemInfo(PopupMenu, 6, false, &menuItemInfo);
			}
			TrackPopupMenuEx(PopupMenu, TPM_VERTICAL, WinPoint.x, WinPoint.y, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0,0);
			break;
		case WM_LBUTTONDBLCLK:
			OpenRecycleBin(hWnd);
			break;
		}
		break;
	case WM_FILEDELETED:
		if (NumFilesInBin(hWnd) == 0)
		{
			IconData.hIcon = EmptyIcon;
			Shell_NotifyIcon(NIM_MODIFY, &IconData);
		}
		else
		{
			IconData.hIcon = FullIcon;
			Shell_NotifyIcon(NIM_MODIFY, &IconData);
		}

		break;
	case WM_DESTROY:
		if(m_ulSHChangeNotifyRegister)
			SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister);
		Shell_NotifyIcon(NIM_DELETE,&IconData);
		DestroyMenu(MainMenu);
		DestroyMenu(PopupMenu);
		PostQuitMessage(0);
		break;
	default:
		// We do not want to handle this message so pass back to Windows
		// to handle it in a default way
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool OpenRecycleBin(HWND hWnd)
{
	HINSTANCE Result;

	char path[] = "::{645FF040-5081-101B-9F08-00AA002F954E}";
	Result = ShellExecute(hWnd, "explore", path, NULL, NULL, SW_SHOWNORMAL);

	if((int)Result > 32) return true;
	char msg[50];
	sprintf_s(msg, "Could not open the Recycle Bin! %l", Result);
	MessageBox(hWnd, msg, "Error", 0);
	return false;
}

long NumFilesInBin(HWND hWnd)
{
	SHQUERYRBINFO info = {0};
	info.cbSize = sizeof(info);
	HRESULT result = SHQueryRecycleBin(0, &info);
	if (result)
	{
		MessageBox(hWnd, "Could get the number of files in Recycle Bin!", "Error", 0);
		return 0;
	}
	return info.i64NumItems;
}

void EmptyRecycleBin(HWND hWnd)
{
	if (NumFilesInBin(hWnd) == 0) return;
	HRESULT result = SHEmptyRecycleBin(NULL, NULL, NULL);
	if (result != S_OK)
		MessageBox(hWnd, "Could not empty the files in the Recycle Bin!", "Error", 0);
}