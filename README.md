libsplice
=========
Library for kernel and user mode splicing for Windows (x86 and x64)

Description
--------
Disassembles and moves the first instructions of procedure and transfers them in a separate buffer. Replaces the first byte of the procedure on a long jump to the target procedure. 
32-bit splicing replaces at least the 5 bytes of proc, 64-bit replaces 14 bytes.

Syntax
------
`int splice(void *proc, void *new_proc, void **old_proc);`

Parameters
----------
- *proc* – address of procedure for splicing
- *new_proc* — address of new procedure
- *old_proc* — pointer to variable, to store address of old procedure
- If the function succeeds, the return value is a nonzero value.
If the function fails, the return value is zero.

Example
-------
	typedef HANDLE (WINAPI *CREATE_FILE) (
		LPCTSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile
		);

	static CREATE_FILE oldCreateFile = NULL;

	HANDLE WINAPI newCreateFile(
		LPCTSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD dwCreationDisposition,
		DWORD dwFlagsAndAttributes,
		HANDLE hTemplateFile
		)
	{
		printf("CreateFile\n");
		return oldCreateFile(
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile
			);
	}

	void example()
	{
		PVOID	createFile = NULL;

		createFile = GetProcAddress(LoadLibrary("kernel32.dll"),"CreateFileA");
		if (!createFile)
			return;

		splice(createFile, &newCreateFile, ppv(&oldCreateFile);
	}