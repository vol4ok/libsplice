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
#include "misc.h"
#include "ldasm.h"

#define SPLICE32_SIZE			5
#define SPLICE64_SIZE			14
#define OLD_CODE_BUFFER_SIZE	64

#define set_jump32(_src,_dst) p8(_src)[0] = 0xE9; p32((_src)+1)[0] = (u32)(_dst) - (u32)(_src) - 5
#define set_jump64(_src,_dst) p64(_src)[0] = 0x25FF; p64((_src)+6)[0] = (u64)(_dst)

#ifdef _WIN64

int splice(void *proc, void *new_proc, void **old_proc)
{
	u8			*src, *old, *new;
	u32			all_len = 0;
	ldasm_data	ld;
	u64			cr0;
	KIRQL		irql;

	/* dummy check */
	if (!proc || !new_proc || !old_proc)
		return FALSE;

	/* alloc buffer for original code */
	*old_proc = mem_alloc(OLD_CODE_BUFFER_SIZE);

	if (!*old_proc) {
		dbg_msg("ERROR: mem_alloc failed!\n");
		return FALSE;
	}

	src = proc;
	old = *old_proc;
	new = new_proc;

	/* already hooked? */
	if (p16(src)[0] == 0x25FF && p32(src+2)[0] == 0) {

		/* set jump to previous hook */
		set_jump64(old, p64(src+6)[0]);

		/* replace jump address */
		irql = global_lock();
		cr0 = mem_open();

		p64(src+6)[0] = (u64)new;

		mem_close(cr0);
		global_unlock(irql);

		return TRUE;
	}

	/* move first bytes of proc to the buffer */
	do {

		/* disasm instruction */
		u32 len = ldasm(src, &ld, 1);

		dbg_msg("disasm len = %u opcd = %02x\n",len,src[ld.opcd_offset]);

		/* check instruction */
		if (ld.flags & F_INVALID
			|| (len == 1 && (src[ld.opcd_offset] == 0xCC || src[ld.opcd_offset] == 0xC3))
			|| (len == 3 && src[ld.opcd_offset] == 0xC2)
			|| len + all_len + SPLICE64_SIZE > OLD_CODE_BUFFER_SIZE) {

				dbg_msg("ERROR: can't move instruction!\n");
				break;
		}

		/* move instruction */
		memcpy(old, src, len);

		/* if instruction has relative offset, calculate new offset */
		if (ld.flags & F_RELATIVE) {
			if (ld.opcd_size == 2) {
				if ( _abs64((u64)(src + *((s32*)(old+1))) - (u64)old) > 2147483647 ){
					/* if jump greater then 2GB offset exit */
					dbg_msg("ERROR: offset more then 2Gb! (1)\n");
					break;
				}else
					p32(old+2)[0] += (u32)(src - old);
			} else {
				if ( _abs64((u64)(src + *((s32*)(old+1))) - (u64)old) > 2147483647 ) {
					/* if jump greater then 2GB offset exit */
					dbg_msg("ERROR: offset more then 2Gb! (2)\n");
					break;
				} else
					p32(old+1)[0] += (u32)(src - old);
			}
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

	dbg_msg("set jum from 0x%p to 0x%p\n",old,src);
	/* set jump form spliced bytes to original code */
	set_jump64(old, src);

	src = proc;

	dbg_msg("set jump from 0x%p to 0x%p\n",src,new);
	irql = global_lock();
	cr0 = mem_open();
	/* set jump form original code to new proc */
	set_jump64(src, new);
	mem_close(cr0);
	global_unlock(irql);

	return TRUE;
}

#else /* _WIN64 */

int splice(void *proc, void *new_proc, void **old_proc)
{
	u8			*src, *old, *new;
	u32			all_len = 0;
	ldasm_data	ld;
	u64			cr0;
	KIRQL		irql; 

	/* dummy check */
	if (!proc || !new_proc || !old_proc)
		return FALSE;

	/* alloc buffer for original code */
	*old_proc = mem_alloc(OLD_CODE_BUFFER_SIZE);
	if (!*old_proc) 
		return FALSE;

	src = proc;
	old = *old_proc;
	new = new_proc;

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

		/* move instruction */
		memcpy(old, src, len);

		/* if instruction has relative offset, calculate new offset */
		if (ld.flags & F_RELATIVE) {
			if (ld.opcd_size == 2) {
				p32(old+2)[0] += (u32)(src - old);
			} else {
				p32(old+1)[0] += (u32)(src - old);
			}
		}

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

	irql = global_lock();
	cr0 = mem_open();
	/* set jump form original code to new proc */
	set_jump32(src, new);
	mem_close(cr0);
	global_unlock(irql);

	return TRUE;
}

#endif /* _WIN64 */