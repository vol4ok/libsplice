/*
 *
 * Copyright (c) 2009-2011
 * vol4ok <admin@vol4ok.net> PGP KEY ID: 26EC143CCDC61C9D
 *
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#include <wdm.h>
#include "defines.h"
#include "splice.h"

#define	DRIVER_NAME			L"example"
#define IOCTL_TEST			CTL_CODE( FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS )

typedef  NTSTATUS (*NT_CREATE_FILE)(
	__out     PHANDLE FileHandle,
	__in      ACCESS_MASK DesiredAccess,
	__in      POBJECT_ATTRIBUTES ObjectAttributes,
	__out     PIO_STATUS_BLOCK IoStatusBlock,
	__in_opt  PLARGE_INTEGER AllocationSize,
	__in      ULONG FileAttributes,
	__in      ULONG ShareAccess,
	__in      ULONG CreateDisposition,
	__in      ULONG CreateOptions,
	__in      PVOID EaBuffer,
	__in      ULONG EaLength
	);

static NT_CREATE_FILE oldNtCreateFile = NULL;

NTSTATUS newNtCreateFile(
	__out     PHANDLE FileHandle,
	__in      ACCESS_MASK DesiredAccess,
	__in      POBJECT_ATTRIBUTES ObjectAttributes,
	__out     PIO_STATUS_BLOCK IoStatusBlock,
	__in_opt  PLARGE_INTEGER AllocationSize,
	__in      ULONG FileAttributes,
	__in      ULONG ShareAccess,
	__in      ULONG CreateDisposition,
	__in      ULONG CreateOptions,
	__in      PVOID EaBuffer,
	__in      ULONG EaLength
	)
{
	DbgPrint("NtCreateFile\n");
	return oldNtCreateFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength
		);
}

NTSTATUS test()
{
	PVOID	ntCreateFile = NULL;
	UNICODE_STRING	ntCreateFileName;

	RtlInitUnicodeString(&ntCreateFileName, L"NtCreateFile");
	ntCreateFile = MmGetSystemRoutineAddress(&ntCreateFileName);

	if(!ntCreateFile) {
		dbg_msg("get NtCreateFile address failed!\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (splice(ntCreateFile, &newNtCreateFile, ppv(&oldNtCreateFile))) {
		dbg_msg("NtCreateFile successfully spliced!\n");
		return STATUS_SUCCESS;
	} else {
		dbg_msg("Failed to splice NtCreateFile!\n");
		return STATUS_UNSUCCESSFUL;
	}
}

NTSTATUS
DispatchDeviceControl(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    ULONG               inBufLength;
    ULONG               outBufLength;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
    outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    switch ( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
    case IOCTL_TEST:
		ntStatus = test();
		Irp->IoStatus.Information = 0;
		break;
    default:
    	Irp->IoStatus.Information = 0;
        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
	}
	
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return ntStatus;
}

VOID
DriverUnload(
    __in PDRIVER_OBJECT DriverObject
    )
{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING uniWin32NameString;

	dbg_msg("%ws driver unloaded\n",DRIVER_NAME);

    RtlInitUnicodeString( &uniWin32NameString, L"\\DosDevices\\"DRIVER_NAME );
    IoDeleteSymbolicLink( &uniWin32NameString );
    if (deviceObject != NULL)
        IoDeleteDevice(deviceObject);
}

NTSTATUS
DispatchClose(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
DispatchCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
	NTSTATUS        ntStatus = STATUS_SUCCESS;
	
    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return ntStatus;
}

NTSTATUS
DriverEntry(
    __in PDRIVER_OBJECT   DriverObject,
    __in PUNICODE_STRING  RegistryPath
    )
{
    NTSTATUS        	ntStatus;
    UNICODE_STRING  	ntUnicodeString;
    UNICODE_STRING  	ntWin32NameString;
    PDEVICE_OBJECT  	deviceObject = NULL;

	dbg_msg("%ws driver loaded\n",DRIVER_NAME);
    
    RtlInitUnicodeString( &ntUnicodeString, L"\\Device\\"DRIVER_NAME );
    ntStatus = IoCreateDevice(
        DriverObject,
        0,
        &ntUnicodeString,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &deviceObject 
		);
    
    if (!NT_SUCCESS(ntStatus))
        return ntStatus;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
    DriverObject->DriverUnload = DriverUnload;

    RtlInitUnicodeString( &ntWin32NameString, L"\\DosDevices\\"DRIVER_NAME );
    ntStatus = IoCreateSymbolicLink( &ntWin32NameString, &ntUnicodeString );

    if (!NT_SUCCESS(ntStatus))
        IoDeleteDevice(deviceObject);

    return ntStatus;
}