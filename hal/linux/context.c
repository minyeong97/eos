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
	int32u_t *ptr = stack_base + stack_size / 4 - 1;
	*ptr = arg;
	*(ptr - 0) = NULL;
	*(ptr - 1) = entry;
	*(ptr - 2) = 0;
	*(ptr - 3) = NULL;
	*(ptr - 4) = NULL;
	*(ptr - 5) = NULL;
	*(ptr - 6) = NULL;
	*(ptr - 7) = NULL;
	*(ptr - 8) = NULL;
	PRINT("%p entry:%p\n", ptr-9, entry);

	return (addr_t)(ptr - 8);
}

void _os_restore_context(addr_t sp) {
	PRINT("%p\n", sp);
	extern _eflags;
	_eflags = 32;
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
		:"=m"(_eflags):"m"(sp));
		
	PRINT("%d \n", _eflags);
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
