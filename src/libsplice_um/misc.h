#ifndef _MISC_
#define _MSIC_

int mem_exec(void* addr, u32 size);
int mem_open(void* addr, u32 size, u32* protect);
int mem_close(void* addr, u32 size, u32 protect);

#endif /* _MSIC_ */
