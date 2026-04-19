#include <windows.h>
#include <stdio.h>
#include <shlobj.h>

#include "SysRecycle.h"
#include "SysRecycleReg.h"

const int IDC_TRAY1      = 1000;

const int ID_POPUP_ABOUT = 2;
const int ID_POPUP_HIDERECYCLEBIN = 3;
const int ID_POPUP_EMPTYRECYCLEBIN = 4;
const int ID_POPUP_OPENRECYCLEBIN = 5;
const int ID_POPUP_RUNATSTARTUP = 40001;
const wchar_t *HINT_MESSAGE = L"SysRecycle";
Registry *Reg;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wcex{};

	if (lstrcmp(lpCmdLine, L"aero") == 0)
	{
		EmptyIcon = LoadIcon(hInstance, L"EMPTYICON");
		FullIcon = LoadIcon(hInstance, L"FULLICON");
	}
	else
	{
		EmptyIcon = LoadIcon(hInstance, L"EMPTYICONMETRO");
		FullIcon = LoadIcon(hInstance, L"FULLICONMETRO");
	}

	Reg = new Registry();

	if (!nShowCmd) exit(0);
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
	wcex.lpszClassName= L"MyWindowClass";
	wcex.hIconSm= EmptyIcon;

	/* Now we can go ahead and register our new window class */
	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindow(L"MyWindowClass", L"The title of the Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
	{
		MessageBox(NULL, L"Failed to create application window.", L"Error", 0);
		exit(1);
	}

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

	MainMenu = LoadMenu(hInstance, L"POPUPMENU");
	if (MainMenu == NULL)
	{
		MessageBox(hWnd, L"Menu not found.", L"Error", 0);
		exit(0);
	}

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
        m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWnd,             // Hwnd to receive notification
                SHCNRF_ShellLevel,                                            // Event types of interest (sources)
                SHCNE_RMDIR|SHCNE_DELETE,                                     // Events of interest - use SHCNE_ALLEVENTS for all events
                WM_FILEDELETED,                                               // Notification message to be sent upon the event
                1,                                                            // Number of entries in the pfsne array
                &shCNE);  
		// Array of SHChangeNotifyEntry structures that
		// contain the notifications. This array should                           
		// always be set to one when calling SHChnageNotifyRegister
        // or SHChangeNotifyDeregister will not work properly.

        if (m_ulSHChangeNotifyRegister == 0)
        {
            MessageBox(hWnd, L"Failed to register shell change notifications. File deletion events will not be detected.", L"Error", 0);
            exit(1);
        }
    }
    else
    {
        MessageBox(hWnd, L"Failed to get desktop folder location. Shell notifications cannot be registered.", L"Error", 0);
        exit(1);
    }

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
	wchar_t menuText[256];

	switch (message)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case ID_POPUP_OPENRECYCLEBIN:
			OpenRecycleBin(hWnd);
			break;
		case ID_POPUP_EMPTYRECYCLEBIN:
			EmptyRecycleBin(hWnd);
			if (NumFilesInBin(hWnd) == 0)
			{
				IconData.hIcon = EmptyIcon;
			}
			else
			{
				IconData.hIcon = FullIcon;
			}
			Shell_NotifyIcon(NIM_MODIFY, &IconData);
			break;
		case ID_POPUP_HIDERECYCLEBIN:
			Reg->ToggleHiddenIcon(hWnd);
			break;
		case ID_POPUP_ABOUT:
			MessageBox(hWnd,
				L"SysRecycle (Freeware)\n"
				L"for Windows 10 and 11\n"
				L"Version 6.1\n"
				L"by Clinton Andrews\n"
				L"clinton.andrews@att.net\n"
				L"Copyright (C) 2021 Clinton Andrews\n\n"
				L"THIS SOFTWARE IS PROVIDED 'AS IS'. THE DEVELOPER OF THIS SOFTWARE WILL NOT BE "
				L"HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR "
				L"CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOSS OF USE, DATA, OR "
				L"PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF "
				L"LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE "
				L"OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF "
				L"ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
				L"THIS SOFTWARE CAN NOT BE RESOLD WITH EXPRESS PREMISSION OF THE AUTHOR "
				L"CLINTON ANDREWS."
				,L"About", 0);
			break;
		case 1:
			DestroyWindow(hWnd);
			break;
		case ID_POPUP_RUNATSTARTUP:
			Reg->SetAutorun(hWnd);
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
			if(!GetMenuItemInfo(PopupMenu, ID_POPUP_HIDERECYCLEBIN, false, &menuItemInfo))
				DestroyWindow(hWnd);
			else
			{
				if(Reg->IsHidden())
					menuItemInfo.fState = MFS_CHECKED | MFS_ENABLED;
				else
					menuItemInfo.fState = MFS_ENABLED;
				SetMenuItemInfo(PopupMenu, ID_POPUP_HIDERECYCLEBIN, false, &menuItemInfo);
			}

			menuItemInfo.fMask = MIIM_STATE;
			if (!GetMenuItemInfo(PopupMenu, ID_POPUP_RUNATSTARTUP, false, &menuItemInfo))
				DestroyWindow(hWnd);
			else
			{
				if (Reg->IsAutorun())
					menuItemInfo.fState = MFS_CHECKED | MFS_ENABLED;
				else
					menuItemInfo.fState = MFS_ENABLED;
				SetMenuItemInfo(PopupMenu, ID_POPUP_RUNATSTARTUP, false, &menuItemInfo);
			}

			menuItemInfo.fMask = MIIM_TYPE;
			menuItemInfo.fType = MFT_STRING;
			if(!GetMenuItemInfo(PopupMenu, 6, false, &menuItemInfo))
				DestroyWindow(hWnd);
			else
			{
				menuItemInfo.fState = MFS_ENABLED;
				menuItemInfo.dwTypeData = menuText;
				swprintf_s(menuText, _countof(menuText), L"%lld files", NumFilesInBin(hWnd));
				menuItemInfo.cch = (UINT)wcslen(menuText);
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
		DestroyMenu(MainMenu);  // Also destroys PopupMenu (it is a submenu owned by MainMenu)
		delete Reg;
		Reg = nullptr;
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
	HINSTANCE result;

	wchar_t path[] = L"::{645FF040-5081-101B-9F08-00AA002F954E}";
	result = ShellExecute(hWnd, L"explore", path, NULL, NULL, SW_SHOWNORMAL);

	if((INT_PTR)result > 32) return true;
	wchar_t msg[100];
	swprintf_s(msg, _countof(msg), L"Error Code: %llu Could not open the Recycle Bin!", (ULONG_PTR)result);
	MessageBox(hWnd, msg, L"Error", 0);
	return false;
}

long long NumFilesInBin(HWND hWnd)
{
	SHQUERYRBINFO info = {0};
	info.cbSize = sizeof(info);
	HRESULT result = SHQueryRecycleBin(0, &info);
	if (result != S_OK)
	{
		MessageBox(hWnd, L"Could not get the number of files in Recycle Bin!", L"Error", 0);
		return 0;
	}
	return info.i64NumItems;
}

void EmptyRecycleBin(HWND hWnd)
{
	if (NumFilesInBin(hWnd) == 0) return;
	HRESULT result = SHEmptyRecycleBin(NULL, NULL, NULL);
	if (result != S_OK)
		MessageBox(hWnd, L"Could not empty the files in the Recycle Bin!", L"Error", 0);
}