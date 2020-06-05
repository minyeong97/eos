#include <core/eos.h>
#include <core/eos_internal.h>
#include "emulator_asm.h"

typedef struct _os_context {
	/* low address */
	int32u_t edi;
	int32u_t esi;
	int32u_t ebx;
	int32u_t edx;
	int32u_t ecx;
	int32u_t eax;
	int32u_t eflags;
	int32u_t entry;
	int32u_t return_addr;
	int32u_t arg;
	/* high address */	
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	//_os_context_t *ctx = (_os_context_t *)context;
	//PRINT("reg1  =0x%x\n", ctx->reg1);
	//PRINT("reg2  =0x%x\n", ctx->reg2);
	//PRINT("reg3  =0x%x\n", ctx->reg3);
	//...
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
	_os_context_t *context = (_os_context_t*)(stack_base + stack_size/4 + 8);
	context->arg = (int32u_t)arg;
	context->return_addr = (int32u_t)NULL;
	context->entry = (int32u_t)entry;
	context->eflags = (int32u_t)1;
	context->eax = (int32u_t)NULL;
	context->ecx = (int32u_t)NULL;
	context->ebx = (int32u_t)NULL;
	context->edx = (int32u_t)NULL;
	context->esi = (int32u_t)NULL;
	context->edi = (int32u_t)NULL;

	return (addr_t)(context);
}

void _os_restore_context(addr_t sp) {
	extern int32u_t _eflags;
	__asm__ __volatile__("\
		movl %1, %%esp;\
		popl %%edi;\
		popl %%esi;\
		popl %%ebx;\
		popl %%edx;\
		popl %%ecx;\
		popl %%eax;\
		popl %0;\
		movl $0, %%eax;\
		ret;"
		:"=m"(_eflags):"m"(sp));
}

addr_t _os_save_context() {
	extern int32u_t _eflags;
	__asm__ __volatile__("\
		pushl $resume_eip;\
		pushl %0;\
		pushl %%eax;\
		pushl %%ecx;\
		pushl %%edx;\
		pushl %%ebx;\
		pushl %%esi;\
		pushl %%edi;\
		pushl 4(%%ebp);\
		pushl (%%ebp);\
		movl %%esp, %%eax;\
		addl $8, %%eax;\
	resume_eip:"
		:: "m"(_eflags));
}
