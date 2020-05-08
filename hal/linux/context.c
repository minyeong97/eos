#include <core/eos.h>
#include <core/eos_internal.h>
#include "emulator_asm.h"

typedef struct _os_context {
	/* low address */
	/* high address */	
} _os_context_t;

void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
	//PRINT("reg1  =0x%x\n", ctx->reg1);
	//PRINT("reg2  =0x%x\n", ctx->reg2);
	//PRINT("reg3  =0x%x\n", ctx->reg3);
	//...
}

addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {
	int32u_t *ptr = stack_base;
	*ptr = NULL;
	*(ptr + 1) = NULL;
	*(ptr + 2) = entry;
	*(ptr + 3) = 1;
	*(ptr + 4) = NULL;
	*(ptr + 5) = NULL;
	*(ptr + 6) = NULL;
	*(ptr + 7) = NULL;
	*(ptr + 8) = NULL;
	*(ptr + 9) = NULL;

	return (addr_t)(ptr + 9);
}

void _os_restore_context(addr_t sp) {
	PRINT("%p\n", sp);
	extern _eflags;
	__asm__ __volatile__("\
		movl %1, %%esp;\
		popl %%edi;\
		popl %%esi;\
		popl %%ebx;\
		popl %%edx;\
		popl %%ecx;\
		popl %%eax;\
		popl %0;\
		ret;"
		:"=r"(_eflags):"r"(sp));
			
}

addr_t _os_save_context() {
	extern _eflags;
	__asm__ __volatile__("\
		pushl 4(%%ebp);\
		pushl $0;\
		pushl %%eax;\
		pushl %%ecx;\
		pushl %%edx;\
		pushl %%ebx;\
		pushl %%esi;\
		pushl %%edi;\
		pushl 4(%%ebp);\
		pushl (%%ebp);\
		movl %%esp, %%eax;\
		addl $8, %%eax;"	
		:: "r"(_eflags));
}
