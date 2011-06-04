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

#include "misc.h"

static u32 locked;
static u32 inp_dpcs;
static u32 out_dpcs;
static u32 dpc_refs;

static void global_lock_proc(PKDPC dpc, PKDPC *cpu_dpc, void *arg1, void *arg2)
{
	_disable();
	lock_dec(&inp_dpcs);
	
	while (locked != 0) 
		KeStallExecutionProcessor(1);	
	
	_enable();
	
	if (lock_dec(&dpc_refs) == 0)
		ExFreePool(cpu_dpc);
	
	lock_dec(&out_dpcs);
}


KIRQL global_lock()
{
	KIRQL old_irql;
	PKDPC cpu_dpc;
	CCHAR n_cpu, i;
	
	n_cpu    = KeNumberProcessors;
	cpu_dpc  = ExAllocatePool(NonPagedPoolMustSucceed, sizeof(KDPC) * n_cpu);
	old_irql = KeRaiseIrqlToDpcLevel();
	locked   = 1;
	inp_dpcs = n_cpu - 1;
	out_dpcs = inp_dpcs;
	dpc_refs = inp_dpcs;
	
	for (i = 0; i < KeNumberProcessors; i++) 
		if (i != KeGetCurrentProcessorNumber()) {
			KeInitializeDpc(&cpu_dpc[i], global_lock_proc, cpu_dpc);
			KeSetTargetProcessorDpc(&cpu_dpc[i], i);
			KeSetImportanceDpc(&cpu_dpc[i], HighImportance);
			KeInsertQueueDpc(&cpu_dpc[i], NULL, NULL);
		}
	
	while (inp_dpcs != 0) 
		KeStallExecutionProcessor(1); 

	_disable();
	
	return old_irql;
}


void global_unlock(KIRQL old_irql)
{
	locked = 0;
	
	while (out_dpcs != 0) 
		KeStallExecutionProcessor(1); 

	_enable();
	
	KeLowerIrql(old_irql);
}

u64 mem_open()
{
	u64 cr0;
	
	_disable();
	cr0 = __readcr0();
	
	__writecr0(cr0 & 0xFFFEFFFF);
	
	return cr0;
}

void mem_close(u64 cr0)
{
	__writecr0(cr0);
	_enable();
}
