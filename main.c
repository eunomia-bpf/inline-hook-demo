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
