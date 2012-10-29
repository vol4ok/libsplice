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

#include "splice.h"
#include "ldasm.h"
#include "misc.h"

#define SPLICE32_SIZE			5
#define SPLICE64_SIZE			14
#define OLD_CODE_BUFFER_SIZE	48

#define set_jump32(_src,_dst) p8(_src)[0] = 0xE9; p32((_src)+1)[0] = (u32)(_dst) - (u32)(_src) - 5
#define set_jump64(_src,_dst) p64(_src)[0] = 0x25FF; p64((_src)+6)[0] = (u64)(_dst)

#ifdef _WIN64

int splice(void *proc, void *new_proc, void **old_proc)
{
	u8			*src, *old, *new;
	u32			all_len = 0;
	ldasm_data	ld;
	u32			protect;

	/* dummy check */
	if (!proc || !new_proc || !old_proc)
		return FALSE;

	/* alloc buffer for original code */
	*old_proc = mem_alloc(OLD_CODE_BUFFER_SIZE);
	if (!*old_proc)
		return FALSE;

	/* set execute flag for page */
	mem_exec(*old_proc,OLD_CODE_BUFFER_SIZE);

	src = proc;
	old = *old_proc;
	new = new_proc;

	/* already hooked? */
	if (p16(src)[0] == 0x25FF && p32(src+2)[0] == 0) {

		/* set jump to previous hook */
		set_jump64(old, p64(src+6)[0]);
		
		/* replace jump address */
		mem_open(src,SPLICE64_SIZE,&protect);
		p64(src+6)[0] = (u64)new;
		mem_close(src,SPLICE64_SIZE,protect);

		return TRUE;
	}

	/* move first bytes of proc to the buffer */
	do {

		/* disasm instruction */
		u32 len = ldasm(src, &ld, 1);

		/* check instruction */
		if (ld.flags & F_INVALID
			|| (len == 1 && (src[ld.opcd_offset] == 0xCC || src[ld.opcd_offset] == 0xC3))
			|| (len == 3 && src[ld.opcd_offset] == 0xC2)
			|| len + all_len + SPLICE64_SIZE > OLD_CODE_BUFFER_SIZE) {

			break;
		}

		/* mov instruction */
		memcpy(old, src, len);

		/* if instruction has relative offset, calculate new offset */
		if (ld.flags & F_RELATIVE) {
			if ( _abs64((u64)(src + *((s32*)(old+ld.disp_offset))) - (u64)old) > INT_MAX )
			/* if jump greater then 2GB offset exit */
				break;
			else
				p32(old+ld.disp_offset)[0] += (u32)(src - old);
		}

		src += len;
		old += len;
		all_len += len;
	} while (all_len < SPLICE64_SIZE);

	/* is bytes successfully moved? */
	if (all_len < SPLICE64_SIZE){
		mem_free(*old_proc);
		*old_proc = NULL;
		return FALSE;
	}

	/* set jump form spliced bytes to original code */
	set_jump64(old, src);

	src = proc;

	mem_open(src,SPLICE64_SIZE,&protect);

	/* set jump form original code to new proc */
	set_jump64(src, new);
	
	mem_close(src,SPLICE64_SIZE,protect);

	return TRUE;
}

#else /* _WIN64 */

int splice(void *proc, void *new_proc, void **old_proc)
{
	u8			*src, *old, *new;
	u32			all_len = 0;
	ldasm_data	ld;
	u32			protect;

	/* dummy check */
	if (!proc || !new_proc || !old_proc)
		return FALSE;

	/* alloc buffer for original code */
	*old_proc = mem_alloc(OLD_CODE_BUFFER_SIZE);
	if (!*old_proc) 
		return FALSE;

	/* set execute flag for page */
	mem_exec(*old_proc,OLD_CODE_BUFFER_SIZE);

	src = proc;
	old = *old_proc;

	/* move first bytes of proc to the buffer */
	do {

		u32 len = ldasm(src, &ld, 0);

		/* check instruction */
		if (ld.flags & F_INVALID
			|| (len == 1 && (src[ld.opcd_offset] == 0xCC || src[ld.opcd_offset] == 0xC3))
			|| (len == 3 && src[ld.opcd_offset] == 0xC2)
			|| len + all_len + SPLICE32_SIZE > OLD_CODE_BUFFER_SIZE) {
				break;
		}

		/* mov instruction */
		memcpy(old, src, len);

		/* if instruction has relative offset, calculate new offset */
		if (ld.flags & F_RELATIVE)
			p32(old+ld.disp_offset)[0] += (u32)(src - old);

		src += len;
		old += len;
		all_len += len;
	} while (all_len < SPLICE32_SIZE);

	/* is bytes successfully moved? */
	if (all_len < SPLICE32_SIZE){
		mem_free(*old_proc);
		*old_proc = NULL;
		return FALSE;
	}

	/* set jump form spliced bytes to original code */
	set_jump32(old, src);

	src = proc;
	new = new_proc;

	mem_open(src,SPLICE32_SIZE,&protect);

	/* set jump form to new proc */
	set_jump32(src, new);

	mem_close(src,SPLICE32_SIZE,protect);

	return TRUE;
}

#endif /* _WIN64 */