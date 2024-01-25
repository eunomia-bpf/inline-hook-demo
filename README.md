# Inline Hook Demonstration

This project demonstrates how to implement an Inline Hook, a technique used to intercept function calls by modifying a target function in memory to redirect control flow.

Inline hooks are commonly used in various applications such as software debugging, malware analysis, and reverse engineering.

## Introduction

Inline Hooking involves inserting a jump instruction at the beginning of a target function, redirecting execution to a user-defined hook function. This can be challenging in modern operating systems due to memory protection mechanisms like DEP (Data Execution Prevention) and ASLR (Address Space Layout Randomization), as well as compiler optimizations that may alter function structures.

The similar approach is used in our project [bpftime](https://github.com/eunomia-bpf/bpftime)

> bpftime is an userspace eBPF runtime that allows existing eBPF applications to operate in unprivileged userspace using the same libraries and toolchains. It offers Uprobe and Syscall tracepoints for eBPF, with significant performance improvements over kernel uprobe and without requiring manual code instrumentation or process restarts. The runtime facilitates interprocess eBPF maps in userspace shared memory, and is also compatible with kernel eBPF maps, allowing for seamless operation with the kernel's eBPF infrastructure. It includes a high-performance LLVM JIT for various architectures, alongside a lightweight JIT for x86 and an interpreter.

## Steps to Implement Inline Hook

1. **Find the Address of the Target Function**: Determine the memory address of the function you wish to hook.
2. **Backup Original Instructions**: Backup the original instructions of the target function to restore later.
3. **Write the Jump Instruction**: Insert a jump instruction at the start of the target function to redirect execution to the hook function.
4. **Create Your Hook Function**: Implement the hook function that will execute in place of the original function's beginning.
5. **Modify Memory Permissions**: Use `mprotect` to change the memory page permissions to writable.
6. **Restore Memory Permissions**: Revert the memory page permissions to their original state after modification.

## Implementation Examples

This example shows how to perform an inline hook for `my_function`. It's a basic implementation suitable for educational purposes.

```c
void inline_hook(void *orig_func, void *hook_func) {
    // Store the original bytes of the function.
    unsigned char orig_bytes[5];
    memcpy(orig_bytes, orig_func, 5);

    // Make the memory page writable.
    mprotect(get_page_addr(orig_func), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);

    // Write a JMP instruction at the start of the original function.
    *((unsigned char *)orig_func + 0) = 0xE9;
    *((void **)((unsigned char *)orig_func + 1)) = (unsigned char *)hook_func - (unsigned char *)orig_func - 5;

    // Restore the memory page permissions.
    mprotect(get_page_addr(orig_func), getpagesize(), PROT_READ | PROT_EXEC);
}
```

### For x86

The main.c file contains an example of hooking function:

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "hook.h"

// This is the hook function.
void my_hook_function()
{
    printf("Hello from hook!\n");
}

// This is the original function to hook.
void my_function()
{
    printf("Hello, world!\n");
}

int main()
{

    my_function();

    inline_hook(my_function, my_hook_function);

    // Now calling the function will actually call the hook function.
    my_function();

    remove_hook(my_function);

    // Now calling the function will call the original function.
    my_function();

    return 0;
}
```

Build and run the example for x86.

```console
$ make
$ ./maps
Hello, world!
Hello from hook!
Hello, world!
```

### For ARM32

In ARM32, account for the Program Counter (PC) usually being 2 instructions ahead.

```console
$ make arm
$ ./maps-arm32
Hello, world!
Hello from hook!
Hello, world!
```

### For ARM64

ARM64 requires additional considerations, such as creating a trampoline for larger offsets.

```console
$ make arm64
$ ./maps-arm64
Hello, world!
Hello from hook!
Hello, world!
```

## Disclaimer

This code is for educational purposes and might not work in all systems or configurations. It might violate some OS or hardware protection mechanisms. Always ensure you understand the potential consequences of your modifications.

## License

This project is open-sourced under the MIT License. See the LICENSE file for more details.

## Contributions

Contributions are welcome! Please submit a pull request or open an issue to propose changes or improvements.
