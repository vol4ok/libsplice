#ifndef _MISC_
#define _MSIC_

#include <ntifs.h>
#include "defines.h"

static void global_lock_proc(PKDPC dpc, PKDPC *cpu_dpc, void *arg1, void *arg2);
KIRQL global_lock();
void global_unlock(KIRQL old_irql);
u64 mem_open();
void mem_close(u64 cr0);

#endif /* _MSIC_ */
