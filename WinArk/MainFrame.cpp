// MainDlg.cpp : implementation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrame.h"
#include "DriverHelper.h"
#include "SecurityHelper.h"
#include "ClipboardHelper.h"
#include "SecurityInformation.h"
#include "NewKeyDlg.h"
#include "RenameKeyCommand.h"


#define IDC_VIEW_PROCESS 0xDAD

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) {
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}


BOOL CMainFrame::OnIdle() {
	UpdateUI();
	UIUpdateChildWindows();
	return FALSE;
}

void CMainFrame::UpdateUI() {

	return;
}

void CMainFrame::InitProcessTable() {
	BarDesc bars[] = {
		{19,"������",0},
		{19,"����ID",0},
		{9,"�Ự",0},
		{20,"�û���",0},
		{12,"���ȼ�",0},
		{9,"�߳���",0},
		{9,"�����",0},
		{19,"����",0},
		{20,"����ʱ��",0},
		{30,"����",0},
		{20,"�ļ�����",0},
		{20,"�ļ��汾",0},
		{56,"ӳ��·��",0},
		{856,"�����в���",0}
	};
	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_ProcTable = new CProcessTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height * 2;
	rect.bottom -= height;
	m_ProcTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_ProcTable->ShowWindow(SW_SHOW);
	m_ProcTable->m_Images.Create(16, 16, ILC_COLOR32, 64, 32);
	m_ProcTable->Refresh();
	m_hwndArray[static_cast<int>(TabColumn::Process)] = m_ProcTable->m_hWnd;
}

void CMainFrame::InitNetworkTable() {
	BarDesc bars[] = {
		{20,"������",0},
		{20,"����ID",0},
		{10,"Э��",0},
		{15,"״̬",0},
		{20,"���ص�ַ",0},
		{10,"���ض˿�",0},
		{20,"Զ�̵�ַ",0},
		{10,"Զ�̶˿�",0},
		{20,"����ʱ��",0},
		{30,"ģ����",0},
		{256,"ģ��·��",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_NetTable = new CNetwrokTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_NetTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::Network)] = m_NetTable->m_hWnd;
}

void CMainFrame::InitKernelModuleTable() {
	BarDesc bars[] = {
		{20,"�����ļ���",0},
		{20,"ӳ���ַ",0},
		{10,"ӳ���С",0},
		{20,"����˳��",0},
		{260,"ȫ·��",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_KernelModuleTable = new CKernelModuleTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_KernelModuleTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::KernelModule)] = m_KernelModuleTable->m_hWnd;
}

void CMainFrame::InitDriverTable() {
	BarDesc bars[] = {
		{20,"����������",0},
		{20,"��ʾ����",0},
		{10,"״̬",0},
		{20,"��������",0},
		{20,"��������",0},
		{20,"����",0},
		{260,"������·��",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_DriverTable = new CDriverTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_DriverTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::Driver)] = m_DriverTable->m_hWnd;
}

void CMainFrame::InitServiceTable() {
	BarDesc bars[] = {
		{20,"������",0},
		{20,"��ʾ����",0},
		{10,"״̬",0},
		{20,"��������",0},
		{20,"����ID"},
		{30,"������"},
		{20,"��������",0},
		{260,"������·��",0},
		{30,"�˻���",0},
		{20,"�������",0},
		{50,"����",0},
		{50,"����Ȩ��",0},
		{20,"������",0},
		{20,"����",0},
		{50,"���ܵĿ���",0},
		{30,"��ȫ��ʶ��",0},
		{20,"Security ID����"}
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_ServiceTable = new CServiceTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_ServiceTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::Service)] = m_ServiceTable->m_hWnd;
}

void CMainFrame::InitDriverInterface() {
	// ��λ�����������ļ�����ȡ��ϵͳĿ¼��Ȼ��װ
#ifdef _WIN64
	auto hRes = ::FindResource(nullptr, MAKEINTRESOURCE(IDR_X64_DRIVER), L"BIN");
#else
	auto hRes = ::FindResource(nullptr, MAKEINTRESOURCE(IDR_X86_DRIVER), L"BIN");
#endif // __WIN64
	if (!hRes)
		return;

	auto hGlobal = ::LoadResource(nullptr, hRes);
	if (!hGlobal)
		return;

	auto size = ::SizeofResource(nullptr, hRes);
	void* pBuffer = ::LockResource(hGlobal);

	if (!DriverHelper::IsDriverLoaded()) {
		if (!SecurityHelper::IsRunningElevated()) {
			if (AtlMessageBox(nullptr, L"Kernel Driver not loaded. Some functionality will not be available. Install?", IDS_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES) {
				if (!SecurityHelper::RunElevated(L"install", false)) {
					AtlMessageBox(*this, L"Error running driver installer", IDS_TITLE, MB_ICONERROR);
				}
			}
		}
		else {
			if (!DriverHelper::InstallDriver(false,pBuffer,size) || !DriverHelper::LoadDriver()) {
				MessageBox(L"Failed to install driver. Some functionality will not be available.", L"WinArk", MB_ICONERROR);
			}
		}
	}
	if (DriverHelper::IsDriverLoaded()) {
		if (DriverHelper::GetVersion() < DriverHelper::GetCurrentVersion()) {
			auto response = AtlMessageBox(nullptr, L"�������£�A newer driver is available with new functionality. Update?��",
				IDS_TITLE, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
			if (response == IDYES) {
				if (SecurityHelper::IsRunningElevated()) {
					if (!DriverHelper::UpdateDriver(pBuffer,size))
						AtlMessageBox(nullptr, L"Failed to update driver", IDS_TITLE, MB_ICONERROR);
				}
				else {
					DriverHelper::CloseDevice();
					SecurityHelper::RunElevated(L"update", false);
				}
			}
		}
	}

	::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void CMainFrame::InitRegistryView() {
	// ע���
	RECT clientRect, rect;
	GetClientRect(&clientRect);
	m_TabCtrl.GetWindowRect(&rect);
	clientRect.top += rect.bottom - rect.top;
	m_StatusBar.GetWindowRect(&rect);
	clientRect.bottom -= rect.bottom - rect.top;

	m_RegView.Create(m_hWnd, clientRect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	m_RegView.m_MainSplitter.ShowWindow(SW_SHOW);
	m_hwndArray[static_cast<int>(TabColumn::Registry)] = m_RegView.m_MainSplitter.m_hWnd;
}

// �豸�б�
void CMainFrame::InitDeviceView() {
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	auto hWnd = m_DevView.Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_hwndArray[static_cast<int>(TabColumn::Device)] = m_DevView.m_hWnd;
}

// ������ͼ
void CMainFrame::InitWindowsView() {
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	// WS_CLIPCHILDREN �Ӵ������򸸴��ڲ��������,�Ӵ������л���
	// �����õĻ��������ڻ��ƻ��ڵ�ס�Ӵ���
	HWND hWnd = m_WinView.Create(m_hWnd, rect, nullptr, WS_CHILD|WS_VISIBLE | WS_CLIPSIBLINGS| WS_CLIPCHILDREN);
	m_hwndArray[static_cast<int>(TabColumn::Windows)] = m_WinView.m_hWnd;
}

void CMainFrame::InitKernelHookView() {
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height + 5;
	rect.bottom -= height;

	HWND hWnd = m_KernelHookView.Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_hwndArray[static_cast<int>(TabColumn::KernelHook)] = m_KernelHookView.m_hWnd;
}

void CMainFrame::InitKernelView() {
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height + 5;
	rect.bottom -= height;

	HWND hWnd = m_KernelView.Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_hwndArray[static_cast<int>(TabColumn::Kernel)] = m_KernelView.m_hWnd;
}

void CMainFrame::InitConfigView() {
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height + 5;
	rect.bottom -= height;

	HWND hWnd = m_SysConfigView.Create(m_hWnd, rect);
	m_hwndArray[static_cast<int>(TabColumn::Config)] = m_SysConfigView.m_hWnd;
}


LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, nullptr, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	/*m_CmdBar.AttachMenu(GetMenu());
	SetMenu(nullptr);*/

	UIAddMenu(IDR_MAINFRAME);
	UIAddMenu(IDR_CONTEXT);
	//InitCommandBar();

	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
	SetIcon(hIconSmall, FALSE);

	m_StatusBar.Create(m_hWnd);
	int parts[] = { 100,200,300,430,560,700,830,960,1100 };
	m_StatusBar.SetParts(_countof(parts), parts);
	m_StatusBar.ShowWindow(SW_SHOW);

	CTabCtrl tabCtrl;
	CRect r;
	GetClientRect(&r);
	r.bottom = 25;
	auto hTabCtrl = tabCtrl.Create(m_hWnd, &r, nullptr, WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS
		| TCS_HOTTRACK | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TABS,
		WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY, TabId);
	m_TabCtrl.SubclassWindow(hTabCtrl);
	// ��ʼ��ѡ���
	struct {
		PCWSTR Name;
	}columns[] = {
		L"����",
		L"�ں�ģ��",
		L"�ں�",
		L"�ں˹���",
		//L"Ӧ�ò㹳��",
		L"����",
		L"����",
		L"ע���",
		L"�豸",
		L"����",
		L"����",
		L"����"
	};

	int i = 0;
	for (auto& col : columns) {
		m_TabCtrl.InsertItem(i++, col.Name);
	}
	HFONT hFont = (HFONT)::GetStockObject(SYSTEM_FIXED_FONT);
	m_TabCtrl.SetFont(hFont, true);
	::DeleteObject(hFont);

	SetWindowLong(GWL_EXSTYLE, ::GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(m_hWnd, 0xffffff, 220, LWA_ALPHA);

	InitDriverInterface();
	InitProcessTable();
	InitNetworkTable();
	InitKernelModuleTable();
	InitDriverTable();
	InitServiceTable();

	InitRegistryView();
	InitDeviceView();
	InitWindowsView();
	InitKernelHookView();
	InitKernelView();
	InitConfigView();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	// UIAddChildWindowContainer(m_hWnd);

	return TRUE;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);

	bHandled = false;
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

void CMainFrame::CloseDialog(int nVal) {
	DestroyWindow();
	::PostQuitMessage(nVal);
}

LRESULT CMainFrame::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	RECT rect, statusRect, tabRect;
	GetClientRect(&rect);
	int iHorizontalUnit = LOWORD(GetDialogBaseUnits());
	int iVerticalUnit = HIWORD(GetDialogBaseUnits());
	m_TabCtrl.GetClientRect(&tabRect);
	m_StatusBar.GetClientRect(&statusRect);
	int statusHeight = statusRect.bottom - statusRect.top;

	int width = rect.right - rect.left;
	int tabHeight = tabRect.bottom - tabRect.top;
	::MoveWindow(m_TabCtrl.m_hWnd, rect.left, rect.top, width, tabHeight, true);
	int iX = rect.left;
	int iY = rect.top + tabHeight;
	int height = rect.bottom - tabHeight - statusHeight - rect.top;

	// �ж��Ƿ񴰿ڴ���
	bool existence = ::IsWindow(m_hwndArray[_index]);
	if (!existence) {
		return TRUE;
	}
	if (_index == static_cast<int>(TabColumn::Registry)) {
		::MoveWindow(m_RegView.m_hWnd, iX, iY, width, height, true);
		CRect rc;
		m_RegView.m_AddressBar.GetClientRect(&rc);
		int editHeight = rc.Height();
		::MoveWindow(m_hwndArray[_index], iX, iY, width, height - editHeight, true);
	}
	else
		::MoveWindow(m_hwndArray[_index], iX, iY, width, height, true);

	iY = rect.bottom - statusHeight;
	::MoveWindow(m_StatusBar.m_hWnd, iX, iY, width, statusHeight, true);
	if (_index == static_cast<int>(TabColumn::Device)) {
		::GetClientRect(m_hwndArray[_index], &rect);
		iX = rect.left;
		iY = rect.top;
		height = rect.bottom - rect.top;
		width = rect.right - rect.left;
		::MoveWindow(m_DevView.m_Splitter.m_hWnd, iX, iY, width, height, true);
	}
	if (_index == static_cast<int>(TabColumn::Windows)) {
		::GetClientRect(m_hwndArray[_index], &rect);
		iX = rect.left;
		iY = rect.top;
		height = rect.bottom - rect.top;
		width = rect.right - rect.left;
		::MoveWindow(m_WinView.m_Splitter.m_hWnd, iX, iY, width, height, true);
	}

	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);

	return TRUE;
}

LRESULT CMainFrame::OnTcnSelChange(int, LPNMHDR hdr, BOOL&) {
	int index = 0;

	index = m_TabCtrl.GetCurSel();
	m_ProcTable->ShowWindow(SW_HIDE);
	m_NetTable->ShowWindow(SW_HIDE);
	m_KernelModuleTable->ShowWindow(SW_HIDE);
	m_DriverTable->ShowWindow(SW_HIDE);
	m_ServiceTable->ShowWindow(SW_HIDE);
	m_RegView.ShowWindow(SW_HIDE);
	m_DevView.ShowWindow(SW_HIDE);
	m_WinView.ShowWindow(SW_HIDE);
	m_KernelHookView.ShowWindow(SW_HIDE);
	m_KernelView.ShowWindow(SW_HIDE);
	m_SysConfigView.ShowWindow(SW_HIDE);
	
	switch (static_cast<TabColumn>(index)) {
		case TabColumn::Process:
			m_ProcTable->ShowWindow(SW_SHOW);
			m_ProcTable->SetFocus();
			break;
		case TabColumn::Network:
			m_NetTable->ShowWindow(SW_SHOW);
			m_NetTable->SetFocus();
			break;
		case TabColumn::KernelModule:
			m_KernelModuleTable->ShowWindow(SW_SHOW);
			m_KernelModuleTable->SetFocus();
			break;
		case TabColumn::Driver:
			m_DriverTable->ShowWindow(SW_SHOW);
			m_DriverTable->SetFocus();
			break;
		case TabColumn::Registry:
			m_RegView.ShowWindow(SW_SHOW);
			break;
		case TabColumn::Device:
			m_DevView.ShowWindow(SW_SHOW);
			break;
		case TabColumn::Windows:
			m_WinView.ShowWindow(SW_SHOW);
			break;
		case TabColumn::KernelHook:
			m_KernelHookView.ShowWindow(SW_SHOW);
			break;
		case TabColumn::Service:
			m_ServiceTable->ShowWindow(SW_SHOW);
			m_ServiceTable->SetFocus();
			break;
		case TabColumn::Kernel:
			m_KernelView.ShowWindow(SW_SHOW);
			break;
		case TabColumn::Config:
			m_SysConfigView.ShowWindow(SW_SHOW);
		default:
			break;
	}
	::PostMessage(m_hWnd, WM_SIZE, 0, 0);
	_index = index;
	return 0;
}

void CMainFrame::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) {
	lpMMI->ptMinTrackSize.x = 550;
	lpMMI->ptMinTrackSize.y = 450;
}

bool CMainFrame::CanPaste() const {
	return false;
}

UINT CMainFrame::TrackPopupMenu(CMenuHandle menu, int x, int y) {
	return (UINT)m_CmdBar.TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y);
}

void CMainFrame::SetStartKey(const CString& key) {
	// m_RegView.SetStartKey(key);
}

void CMainFrame::SetStatusText(PCWSTR text) {
	m_StatusBar.SetText(1, m_StatusText = text, SBT_NOBORDERS);
}