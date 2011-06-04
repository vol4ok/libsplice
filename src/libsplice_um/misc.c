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
#include "misc.h"

int mem_exec(void* addr, u32 size)
{
	u32 temp;
	return VirtualProtect(addr,size,PAGE_EXECUTE_READWRITE,&temp);
}

int mem_open(void* addr, u32 size, u32* protect)
{
	return VirtualProtect(addr,size,PAGE_EXECUTE_READWRITE,protect);
}

int mem_close(void* addr, u32 size, u32 protect)
{
	u32 temp;
	return VirtualProtect(addr,size,protect,&temp);
}