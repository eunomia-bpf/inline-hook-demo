#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64)
#define SIZE_ORIG_BYTES 16
static void inline_hook_replace_inst(void *orig_func, void *hook_func) {
    	// Write a jump instruction at the start of the original function.
	*((unsigned char *)orig_func + 0) = 0xE9; // JMP instruction
	*((void **)((unsigned char *)orig_func + 1)) =
		(unsigned char *)hook_func - (unsigned char *)orig_func - 5;
}
#elif defined(__aarch64__) || defined(_M_ARM64)
#define SIZE_ORIG_BYTES 32
static void inline_hook_replace_inst(void *orig_func, void *hook_func) {
    int offset = ((intptr_t)hook_func - (intptr_t)orig_func) / 4;
    if (offset < -0x2000000 || offset > 0x1ffffff) {
        printf("Offset %d out of range!\n", offset);
        exit(1);
    }
    uint32_t branch_instruction = 0x14000000 | (offset & 0x03ffffff);
    *((uint32_t*)orig_func) = branch_instruction;
}
#elif defined(__arm__) || defined(_M_ARM)
#define SIZE_ORIG_BYTES 20
static void inline_hook_replace_inst(void *orig_func, void *hook_func) {
	// Construct a branch instruction to the hook function.
    // The instruction for a branch in ARM is 0xEA000000 | ((<offset> / 4) & 0x00FFFFFF)
    // The offset needs to be divided by 4 because the PC advances by 4 bytes each step in ARM
    int offset = ((intptr_t)hook_func - (intptr_t)orig_func - 8) / 4;
    int branch_instruction = 0xEA000000 | (offset & 0x00FFFFFF);

    // Write the branch instruction to the start of the original function.
    *(int *)orig_func = branch_instruction;
}
#else
#error "Unsupported architecture"
#endif

void *get_page_addr(void *addr)
{
	return (void *)((uintptr_t)addr & ~(getpagesize() - 1));
}

unsigned char orig_bytes[SIZE_ORIG_BYTES];

void inline_hook(void *orig_func, void *hook_func)
{
	// Store the original bytes of the function.
	memcpy(orig_bytes, orig_func, SIZE_ORIG_BYTES);

	// Make the memory page writable.
	mprotect(get_page_addr(orig_func), getpagesize(),
		 PROT_READ | PROT_WRITE | PROT_EXEC);

    inline_hook_replace_inst(orig_func, hook_func);

	// Make the memory page executable only.
	mprotect(get_page_addr(orig_func), getpagesize(),
		 PROT_READ | PROT_EXEC);
}

void remove_hook(void *orig_func)
{
	// Make the memory page writable.
	mprotect(get_page_addr(orig_func), getpagesize(),
		 PROT_READ | PROT_WRITE | PROT_EXEC);

	// Restore the original bytes of the function.
	memcpy(orig_func, orig_bytes, SIZE_ORIG_BYTES);

	// Make the memory page executable only.
	mprotect(get_page_addr(orig_func), getpagesize(),
		 PROT_READ | PROT_EXEC);
}
