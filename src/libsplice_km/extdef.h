#ifndef _EXTDEF_
#define _EXTDEF_

typedef 
NTSTATUS (*PZWTERMINATEPROCESS)(
								IN HANDLE   ProcessHandle,
								IN NTSTATUS ExitStatus
								);

typedef (*PZWCREATETHREAD)(
						   OUT PHANDLE            ThreadHandle,
						   IN  ACCESS_MASK        DesiredAccess,
						   IN  POBJECT_ATTRIBUTES ObjectAttributes,
						   IN  HANDLE             ProcessHandle,
						   OUT PCLIENT_ID         ClientId,
						   IN  PCONTEXT           ThreadContext,
						   IN  PVOID              UserStack,
						   IN  BOOLEAN            CreateSuspended
						   );

typedef NTSTATUS (*PZWTERMINATETHREAD)(
									   IN HANDLE   ThreadHandle,
									   IN NTSTATUS ExitStatus
									   );

typedef NTSTATUS (*PZWRESUMETHREAD)(
									IN  HANDLE ThreadHandle,
									OUT PULONG PreviousSuspendCount
									);

NTSYSAPI
NTSTATUS
NTAPI 
ZwQuerySystemInformation(
						 IN ULONG SystemInformationClass,
						 IN PVOID SystemInformation,
						 IN ULONG SystemInformationLength,
						 OUT PULONG ReturnLength
						 );

#pragma pack (push, 1)

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	ULONG   Unknown;
	HANDLE	Section;
	PVOID	MappedBase;
	PVOID	ImageBase;
	ULONG	ImageSize;
	ULONG	Flags;
	USHORT	LoadOrderIndex;
	USHORT	InitOrderIndex;
	USHORT	LoadCount;
	USHORT	OffsetToFileName;
	UCHAR	FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

// Information Class 11
typedef struct _RTL_PROCESS_MODULES { 
	ULONG	ModulesCount;
	RTL_PROCESS_MODULE_INFORMATION Modules[0];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

#ifndef _AMD64_ 
//for _AMD64_ this types already defined in WDK
typedef struct _KTRAP_FRAME {
	ULONG   DbgEbp;
	ULONG   DbgEip;
	ULONG   DbgArgMark;
	ULONG   DbgArgPointer;
	ULONG   TempSegCs;
	ULONG   TempEsp;
	ULONG   Dr0;
	ULONG   Dr1;
	ULONG   Dr2;
	ULONG   Dr3;
	ULONG   Dr6;
	ULONG   Dr7;
	ULONG   SegGs;
	ULONG   SegEs;
	ULONG   SegDs;
	ULONG   Edx;
	ULONG   Ecx;
	ULONG   Eax;
	ULONG   PreviousPreviousMode;	
	struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
	ULONG   SegFs;	
	ULONG   Edi;
	ULONG   Esi;
	ULONG   Ebx;
	ULONG   Ebp;
	ULONG   ErrCode;
	ULONG   Eip;
	ULONG   SegCs;
	ULONG   EFlags;
	ULONG   HardwareEsp;
	ULONG   HardwareSegSs;
	ULONG   V86Es;
	ULONG   V86Ds;
	ULONG   V86Fs;
	ULONG   V86Gs;
} KTRAP_FRAME;

typedef KTRAP_FRAME *PKTRAP_FRAME;
typedef KTRAP_FRAME *PKEXCEPTION_FRAME;

#endif

#pragma pack (pop)

typedef VOID (*PKIDISPATCHEXCEPTION)(
									 IN PEXCEPTION_RECORD ExceptionRecord,
									 IN PKEXCEPTION_FRAME ExceptionFrame,
									 IN PKTRAP_FRAME      TrapFrame,
									 IN KPROCESSOR_MODE   PreviousMode,
									 IN BOOLEAN           FirstChance
									 );

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,	// 0 Y N
	SystemProcessorInformation, // 1 Y N
	SystemPerformanceInformation, // 2 Y N
	SystemTimeOfDayInformation, // 3 Y N
	SystemNotImplemented1, // 4 Y N
	SystemProcessesAndThreadsInformation, // 5 Y N
	SystemCallCounts, // 6 Y N
	SystemConfigurationInformation, // 7 Y N
	SystemProcessorTimes, // 8 Y N
	SystemGlobalFlag, // 9 Y Y
	SystemNotImplemented2, // 10 Y N
	SystemModuleInformation, // 11 Y N
	SystemLockInformation, // 12 Y N
	SystemNotImplemented3, // 13 Y N
	SystemNotImplemented4, // 14 Y N
	SystemNotImplemented5, // 15 Y N
	SystemHandleInformation, // 16 Y N
	SystemObjectInformation, // 17 Y N
	SystemPagefileInformation, // 18 Y N
	SystemInstructionEmulationCounts, // 19 Y N
	SystemInvalidInfoClass1, // 20
	SystemCacheInformation, // 21 Y Y
	SystemPoolTagInformation, // 22 Y N
	SystemProcessorStatistics, // 23 Y N
	SystemDpcInformation, // 24 Y Y
	SystemNotImplemented6, // 25 Y N
	SystemLoadImage, // 26 N Y
	SystemUnloadImage, // 27 N Y
	SystemTimeAdjustment, // 28 Y Y
	SystemNotImplemented7, // 29 Y N
	SystemNotImplemented8, // 30 Y N
	SystemNotImplemented9, // 31 Y N
	SystemCrashDumpInformation, // 32 Y N
	SystemExceptionInformation, // 33 Y N
	SystemCrashDumpStateInformation, // 34 Y Y/N
	SystemKernelDebuggerInformation, // 35 Y N
	SystemContextSwitchInformation, // 36 Y N
	SystemRegistryQuotaInformation, // 37 Y Y
	SystemLoadAndCallImage, // 38 N Y
	SystemPrioritySeparation, // 39 N Y
	SystemNotImplemented10, // 40 Y N
	SystemNotImplemented11, // 41 Y N
	SystemInvalidInfoClass2, // 42
	SystemInvalidInfoClass3, // 43
	SystemTimeZoneInformation, // 44 Y N
	SystemLookasideInformation, // 45 Y N
	SystemSetTimeSlipEvent, // 46 N Y
	SystemCreateSession, // 47 N Y
	SystemDeleteSession, // 48 N Y
	SystemInvalidInfoClass4, // 49
	SystemRangeStartInformation, // 50 Y N
	SystemVerifierInformation, // 51 Y Y
	SystemAddVerifier, // 52 N Y
	SystemSessionProcessesInformation // 53 Y N
} SYSTEM_INFORMATION_CLASS;

#endif /* _EXTDEF_ */