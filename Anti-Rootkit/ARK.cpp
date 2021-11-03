#include<ntifs.h>
#include<ntddk.h>
#include<intrin.h>
#include"util.h"
#include"AntiRootkit.h"
#include"kstring.h"
#include"FileManager.h"
#include"RegManager.h"
#include"ProcManager.h"
#include"khook.h"
#include"RecoverHook.h"



// define a tag (because of little endianess, viewed in PoolMon as 'arkv'
#define DRIVER_TAG 'vkra'

// 条件编译
#ifdef _WIN64
	// 64位环境
#else
	// 32位环境
#endif

UNICODE_STRING g_RegisterPath;
PDEVICE_OBJECT g_DeviceObject;

DRIVER_UNLOAD AntiRootkitUnload;
DRIVER_DISPATCH AntiRootkitDeviceControl, AntiRootkitCreateClose;
DRIVER_DISPATCH AntiRootkitRead, AntiRootkitWrite, AntiRootkitShutdown;


extern "C" NTSTATUS NTAPI ZwQueryInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength
);

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadVirtualMemory(
	_In_ HANDLE ProcessHandle,
	_In_opt_ PVOID BaseAddress,
	_Out_writes_bytes_(BufferSize) PVOID Buffer,
	_In_ SIZE_T BufferSize,
	_Out_opt_ PSIZE_T NumberOfBytesRead
);

NTKERNELAPI
NTSTATUS
NTAPI
ObReferenceObjectByName(
	_In_ PUNICODE_STRING ObjectName,
	_In_ ULONG Attributes,
	_In_opt_ PACCESS_STATE AccessState,
	_In_ POBJECT_TYPE ObjectType,
	_In_ KPROCESSOR_MODE AccessMode,
	_Inout_opt_ PVOID ParseContext,
	_Out_ PVOID *Object
);

// 自身逻辑“最小化”
// 标识哪些逻辑属于关键逻辑，非关键逻辑
// 自身模块加载时，会遇到一些操作失败的情况，如创建线程失败，写入文件失败等。如果遇到一个小错误，
// 就认为全部执行失败，那么是不合理的。
// 系统处于低资源，中毒状态下，应保证驱动能在恶劣环境下，最大程度地完成逻辑处理
extern "C" NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	// 驱动被自加载了，会出现没有实体的驱动对象对应
	if (DriverObject == nullptr) {
		return STATUS_UNSUCCESSFUL;
	}
	// Set an Unload routine
	DriverObject->DriverUnload = AntiRootkitUnload;
	// Set dispatch routine the driver supports
	// 试图访问时
	DriverObject->MajorFunction[IRP_MJ_CREATE] = AntiRootkitCreateClose;
	// 结束访问时
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = AntiRootkitCreateClose;
	// 设备控制请求
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AntiRootkitDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_READ] = AntiRootkitRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = AntiRootkitWrite;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = AntiRootkitShutdown;

	// Create a device object
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\AntiRootkit");
	// 定义变量时初始化这些变量
	PDEVICE_OBJECT DeviceObject = nullptr;

	NTSTATUS status = IoCreateDevice(
		DriverObject,		// our driver object,
		0,					// no need for extra bytes
		&devName,			// the device name
		FILE_DEVICE_UNKNOWN,// device type
		0,					// characteristics flags,
		FALSE,				// not exclusive
		&DeviceObject		// the resulting pointer
	);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	// get I/O Manager's help
	// Large buffers may be expensive to copy
	//DeviceObject->Flags |= DO_BUFFERED_IO;

	// Large buffers
	//DeviceObject->Flags |= DO_DIRECT_IO;
	
	status = IoRegisterShutdownNotification(DeviceObject);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DeviceObject);
		return status;
	}
	
	// Create a symbolic link to the device object
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\AntiRootkit");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}

	g_DeviceObject = DeviceObject;

	// 所有的资源申请，请考虑失败的情况下，会引发什么问题
	g_RegisterPath.Buffer = (WCHAR*)ExAllocatePoolWithTag(PagedPool,
		RegistryPath->Length, DRIVER_TAG);
	if (g_RegisterPath.Buffer == nullptr) {
		KdPrint(("Failed to allocate memory\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	g_RegisterPath.MaximumLength = RegistryPath->Length;
	RtlCopyUnicodeString(&g_RegisterPath, (PUNICODE_STRING)RegistryPath);

	//KdPrint(("Copied registry path: %wZ\n", &g_RegisterPath));

	//test();
	// More generally, if DriverEntry returns any failure status,the Unload routine is not called.
	return STATUS_SUCCESS;
}

void AntiRootkitUnload(_In_ PDRIVER_OBJECT DriverObject) {
	//UnHookSSDT();
	ExFreePool(g_RegisterPath.Buffer);
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\AntiRootkit");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);
	IoUnregisterShutdownNotification(g_DeviceObject);
	// delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
	KdPrint(("Driver Unload called\n"));
}

NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status = STATUS_SUCCESS, ULONG_PTR info = 0) {
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, 0);
	return status;
}
/*
	In more complex cases,these could be separate functions,
	where in the Create case the driver can (for instance) check to see who the caller is
	and only let approved callers succeed with opening a device.
*/
_Use_decl_annotations_
NTSTATUS AntiRootkitCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);

	auto status = STATUS_SUCCESS;
	// 获取请求的当前栈空间
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	// 判断请求发给谁，是否是之前生成的控制设备
	if (DeviceObject != g_DeviceObject) {
		CompleteIrp(Irp);
		return status;
	}

	if (stack->MajorFunction == IRP_MJ_CREATE) {
		// verify it's WinArk client (very simple at the moment)
		HANDLE hProcess;
		status = ObOpenObjectByPointer(PsGetCurrentProcess(), OBJ_KERNEL_HANDLE, nullptr, 0, *PsProcessType, KernelMode, &hProcess);
		if (NT_SUCCESS(status)) {
			UCHAR buffer[280] = { 0 };
			status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, buffer, sizeof(buffer) - sizeof(WCHAR), nullptr);
			if (NT_SUCCESS(status)) {
				auto path = (UNICODE_STRING*)buffer;
				auto bs = wcsrchr(path->Buffer, L'\\');
				NT_ASSERT(bs);
				if (bs == nullptr || 0 != _wcsicmp(bs, L"\\WinArk.exe"))
					status = STATUS_ACCESS_DENIED;
			}
			ZwClose(hProcess);
		}
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;	// a polymorphic member, meaning different things in different request.
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// 在处理缓冲区的时候，内核中的内存是有限的，要注意防止缓冲区溢出
//		1.限制缓冲区的长度
_Use_decl_annotations_
NTSTATUS AntiRootkitDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	// get our IO_STACK_LOCATION
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	const auto& dic = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl;
	ULONG len = 0;

	//UNHOOK_SSDT64 unhookssdt;

	switch (dic.IoControlCode) {
		case IOCTL_CLEAR_SSDT_HOOK:
		{	// do the work
			/*memcpy(&unhookssdt, data, sizeof(UNHOOK_SSDT64));
			RecoverSSDT(unhookssdt.index, unhookssdt.Address, unhookssdt.ParamCount);*/
			break;
		}

		case IOCTL_GET_SERVICE_TABLE:
		{
			/*auto KiServiceTable = GetKiServiceTable();
			memcpy(data, &KiServiceTable, 8);*/
			break;
		}

		case IOCTL_GET_FUNCTION_ADDR:
		{
			/*ULONG index;
			memcpy(&index, data, 4);
			auto CurAddr = GetSSDTFuncCurAddr(index);
			memcpy(data, &CurAddr, 8);*/
			break;
		}

		case IOCTL_HOOK_SHADOW_SSDT:
		{
			//HookShadowSSDT();
			break;
		}

		case IOCTL_UNHOOK_SHADOW_SSDT:
		{
			//UnhookShadowSSDT();
			break;
		}
		
		case IOCTL_ARK_OPEN_PROCESS:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			// 获取输入和输出缓冲区的长度
			if (dic.InputBufferLength < sizeof(OpenProcessThreadData) || dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			// 获得缓冲区
			auto data = (OpenProcessThreadData*)Irp->AssociatedIrp.SystemBuffer;
			OBJECT_ATTRIBUTES attr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, 0);
			CLIENT_ID id{};
			id.UniqueProcess = UlongToHandle(data->Id);
			// 嵌套陷阱，ZwOpenProcess内部可能会执行第三方的回调函数，
			// 回调函数调用时，函数处于同一个线程，且共用一个线程栈，这里可能存在“栈溢出”的可能
			// 1.对于调用存在回调函数的api，尽可能少使用栈空间，大量内存，考虑申请内存
			// 2.对于自身执行在过滤驱动或回调函数中的代码，尽可能少使用栈空间
			// 3.自身执行在过滤驱动或回调函数中的代码，如需要调用嵌套的api,考虑另起线程，
			// 通过线程间通信把系统api调用的结果返回给最初线程。
			// 4.避免在代码中使用递归，如果非要使用，注意递归深度

			status = ZwOpenProcess((HANDLE*)data, data->AccessMask, &attr, &id);
			len = NT_SUCCESS(status) ? sizeof(HANDLE) : 0;
			break;
		}

		case IOCTL_ARK_GET_VERSION:
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			// 获得输出缓冲区的长度
			if (dic.OutputBufferLength < sizeof(USHORT)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			*(USHORT*)Irp->AssociatedIrp.SystemBuffer = DRIVER_CURRENT_VERSION;
			len = sizeof(USHORT);
			status = STATUS_SUCCESS;
			break;

		case IOCTL_ARK_SET_PRIORITY:
		{
			len = dic.InputBufferLength;
			if (len < sizeof(ThreadData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			// 获得缓冲区
			auto data = (ThreadData*)dic.Type3InputBuffer;
			if (data == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			__try {
				if (data->Priority < 1 || data->Priority>31) {
					status = STATUS_INVALID_PARAMETER;
					break;
				}

				PETHREAD Thread;
				status = PsLookupThreadByThreadId(UlongToHandle(data->ThreadId),&Thread);
				if (!NT_SUCCESS(status))
					break;

				KeSetPriorityThread(Thread, data->Priority);
				ObDereferenceObject(Thread);

				KdPrint(("Thread Priority change for %d to %d succeeded!\n",
					data->ThreadId, data->Priority));
			} __except (GetExceptionCode() == STATUS_ACCESS_VIOLATION
				? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
				status = STATUS_ACCESS_VIOLATION;
			}
			break;
		}

		case IOCTL_ARK_DUP_HANDLE:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(DupHandleData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if (dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			const auto data = static_cast<DupHandleData*>(Irp->AssociatedIrp.SystemBuffer);
			HANDLE hProcess;
			OBJECT_ATTRIBUTES procAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, OBJ_KERNEL_HANDLE);
			CLIENT_ID pid{};
			pid.UniqueProcess = UlongToHandle(data->SourcePid);
			status = ZwOpenProcess(&hProcess, PROCESS_DUP_HANDLE, &procAttributes, &pid);
			if (!NT_SUCCESS(status)) {
				KdPrint(("Failed to open process %d (0x%08X)\n", data->SourcePid, status));
				break;
			}

			HANDLE hTarget;
			status = ZwDuplicateObject(hProcess, ULongToHandle(data->Handle), NtCurrentProcess(),
				&hTarget, data->AccessMask, 0, data->Flags);
			ZwClose(hProcess);
			if (!NT_SUCCESS(status)) {
				KdPrint(("Failed to duplicate handle (0x%8X)\n", status));
				break;
			}

			*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hTarget;
			len = sizeof(HANDLE);
			break;
		}

		case IOCTL_ARK_OPEN_THREAD:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(OpenProcessThreadData) || dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			auto data = (OpenProcessThreadData*)Irp->AssociatedIrp.SystemBuffer;
			OBJECT_ATTRIBUTES attr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, 0);
			CLIENT_ID id{};
			id.UniqueThread = UlongToHandle(data->Id);
			status = ZwOpenThread((HANDLE*)data, data->AccessMask, &attr, &id);
			len = NT_SUCCESS(status) ? sizeof(HANDLE) : 0;
			break;
		}

		case IOCTL_ARK_OPEN_KEY:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			auto data = static_cast<KeyData*>(Irp->AssociatedIrp.SystemBuffer);
			if (dic.InputBufferLength < sizeof(KeyData) + ULONG((data->Length - 1) * 2)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if (dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if (data->Length > 2048) {
				status = STATUS_BUFFER_OVERFLOW;
				break;
			}

			UNICODE_STRING keyName;
			keyName.Buffer = data->Name;
			keyName.Length = keyName.MaximumLength = (USHORT)data->Length * sizeof(WCHAR);
			OBJECT_ATTRIBUTES keyAttr;
			InitializeObjectAttributes(&keyAttr, &keyName, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
			HANDLE hKey{ nullptr };
			status = ZwOpenKey(&hKey, data->Access, &keyAttr);
			if (NT_SUCCESS(status)) {
				*(HANDLE*)data = hKey;
				len = sizeof(HANDLE);
			}
			break;
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS AntiRootkitShutdown(PDEVICE_OBJECT, PIRP Irp) {
	return CompleteIrp(Irp, STATUS_SUCCESS);
}

NTSTATUS AntiRootkitRead(PDEVICE_OBJECT, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Read.Length;
	if (len == 0)
		return CompleteIrp(Irp, STATUS_INVALID_PARAMETER);

	// DO_DIRECT_IO 访问例子
	//auto buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	//if (!buffer)
	//	return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES);

	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}


NTSTATUS AntiRootkitWrite(PDEVICE_OBJECT, PIRP Irp) {
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Write.Length;

	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}

KEVENT kEvent;

KSTART_ROUTINE MyThreadFunc;
void MyThreadFunc(IN PVOID context) {
	PUNICODE_STRING str = (PUNICODE_STRING)context;
	KdPrint(("Kernel thread running: %wZ\n", str));
	MyGetCurrentTime();
	KdPrint(("Wait 3s!\n"));
	MySleep(3000);
	MyGetCurrentTime();
	KdPrint(("Kernel thread exit!\n"));
	KeSetEvent(&kEvent, 0, true);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

void CreateThreadTest() {
	HANDLE hThread;
	UNICODE_STRING ustrTest = RTL_CONSTANT_STRING(L"This is a string for test!");
	NTSTATUS status;

	KeInitializeEvent(&kEvent, 
		SynchronizationEvent,	// when this event is set, 
		// it releases at most one thread (auto reset)
		FALSE);

	status = PsCreateSystemThread(&hThread, 0, NULL, NULL, NULL, MyThreadFunc, (PVOID)&ustrTest);
	if (!NT_SUCCESS(status)) {
		KdPrint(("PsCreateSystemThread failed!"));
		return;
	}
	ZwClose(hThread);
	KeWaitForSingleObject(&kEvent, Executive, KernelMode, FALSE, NULL);
	KdPrint(("CreateThreadTest over!\n"));
}
void test() {
	//ULONG ul = 1234, ul0 = 0;
	//PKEY_VALUE_PARTIAL_INFORMATION pkvi;
	//RegCreateKey(L"\\Registry\\Machine\\Software\\AppDataLow\\Tencent\\{61B942F7-A946-4585-B624-B2C0228FFE8C}");
	//RegSetValueKey(L"\\Registry\\Machine\\Software\\AppDataLow\\Tencent\\{61B942F7-A946-4585-B624-B2C0228FFE8C}", L"key", REG_DWORD, &ul, sizeof(ul));
	//RegQueryValueKey(L"\\Registry\\Machine\\Software\\AppDataLow\\Tencent\\{61B942F7-A946-4585-B624-B2C0228FFE8C}", L"key", &pkvi);
	EnumSubKeyTest();
	/*memcpy(&ul0, pkvi->Data, pkvi->DataLength);
	KdPrint(("key: %d\n", ul0));*/
	/*ExFreePool(pkvi);*/
}



