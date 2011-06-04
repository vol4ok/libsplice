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

#include "defines.h"
#include "splice.h"

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

void test()
{
	PVOID	createFile = NULL;

	createFile = GetProcAddress(LoadLibrary("kernel32.dll"),"CreateFileA");
	if (!createFile)
		return;

	if (splice(createFile, &newCreateFile, ppv(&oldCreateFile))) {
		dbg_msg("CreateFile successfully spliced!\n");
	} else {
		dbg_msg("Failed to splice CreateFile!\n");
	}

	CreateFileA(NULL,0,0,NULL,0,0,NULL);
	CreateFileA(NULL,0,0,NULL,0,0,NULL);
	CreateFileA(NULL,0,0,NULL,0,0,NULL);
}

int __cdecl main(int argc, char* argv[])
{
	test();
	getchar();
	return 0;
}
