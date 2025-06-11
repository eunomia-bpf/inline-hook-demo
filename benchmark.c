#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "hook.h"

#define ITERATIONS 100000000
#define WARMUP_ITERATIONS 10000

// Global counter to prevent compiler optimizations
volatile int global_counter = 0;

// Benchmark target function
void benchmark_target_function()
{
    global_counter++;
}

// Hook function for benchmarking
void benchmark_hook_function()
{
    global_counter += 2; // Different behavior to verify hook is working
}

// Get current time in nanoseconds
static uint64_t get_time_ns()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Benchmark direct function calls
static double benchmark_direct_calls()
{
    uint64_t start, end;
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        benchmark_target_function();
    }
    
    start = get_time_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        benchmark_target_function();
    }
    end = get_time_ns();
    
    return (double)(end - start) / ITERATIONS;
}

// Benchmark hooked function calls
static double benchmark_hooked_calls()
{
    uint64_t start, end;
    
    // Install hook
    inline_hook(benchmark_target_function, benchmark_hook_function);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        benchmark_target_function();
    }
    
    start = get_time_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        benchmark_target_function();
    }
    end = get_time_ns();
    
    // Remove hook
    remove_hook(benchmark_target_function);
    
    return (double)(end - start) / ITERATIONS;
}

// Benchmark hook installation time
static double benchmark_hook_installation()
{
    uint64_t start, end;
    const int hook_iterations = 1000;
    
    start = get_time_ns();
    for (int i = 0; i < hook_iterations; i++) {
        inline_hook(benchmark_target_function, benchmark_hook_function);
        remove_hook(benchmark_target_function);
    }
    end = get_time_ns();
    
    return (double)(end - start) / hook_iterations;
}

// Function with more complex operations for comparison
void complex_target_function()
{
    volatile int temp = 0;
    for (int i = 0; i < 10; i++) {
        temp += i * i;
    }
    global_counter += temp;
}

void complex_hook_function()
{
    volatile int temp = 0;
    for (int i = 0; i < 10; i++) {
        temp += i * i * 2;
    }
    global_counter += temp;
}

static double benchmark_complex_direct_calls()
{
    uint64_t start, end;
    const int iterations = ITERATIONS / 10; // Fewer iterations for complex function
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS / 10; i++) {
        complex_target_function();
    }
    
    start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        complex_target_function();
    }
    end = get_time_ns();
    
    return (double)(end - start) / iterations;
}

static double benchmark_complex_hooked_calls()
{
    uint64_t start, end;
    const int iterations = ITERATIONS / 10; // Fewer iterations for complex function
    
    // Install hook
    inline_hook(complex_target_function, complex_hook_function);
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS / 10; i++) {
        complex_target_function();
    }
    
    start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        complex_target_function();
    }
    end = get_time_ns();
    
    // Remove hook
    remove_hook(complex_target_function);
    
    return (double)(end - start) / iterations;
}

int main()
{
    printf("Inline Hook Performance Benchmark\n");
    printf("==================================\n");
    printf("Iterations: %d\n", ITERATIONS);
    printf("Warmup iterations: %d\n\n", WARMUP_ITERATIONS);
    
    // Reset counter
    global_counter = 0;
    
    // Benchmark 1: Simple function calls
    printf("1. Simple Function Call Benchmark:\n");
    double direct_time = benchmark_direct_calls();
    printf("   Direct calls:    %.2f ns/call\n", direct_time);
    
    int direct_counter = global_counter;
    global_counter = 0;
    
    double hooked_time = benchmark_hooked_calls();
    printf("   Hooked calls:    %.2f ns/call\n", hooked_time);
    printf("   Overhead:        %.2f ns/call (%.1fx slower)\n", 
           hooked_time - direct_time, hooked_time / direct_time);
    
    int hooked_counter = global_counter;
    printf("   Verification:    Direct=%d, Hooked=%d (should be 2x)\n\n", 
           direct_counter, hooked_counter);
    
    // Benchmark 2: Complex function calls
    printf("2. Complex Function Call Benchmark:\n");
    global_counter = 0;
    
    double complex_direct_time = benchmark_complex_direct_calls();
    printf("   Direct calls:    %.2f ns/call\n", complex_direct_time);
    
    int complex_direct_counter = global_counter;
    global_counter = 0;
    
    double complex_hooked_time = benchmark_complex_hooked_calls();
    printf("   Hooked calls:    %.2f ns/call\n", complex_hooked_time);
    printf("   Overhead:        %.2f ns/call (%.1fx slower)\n", 
           complex_hooked_time - complex_direct_time, 
           complex_hooked_time / complex_direct_time);
    
    int complex_hooked_counter = global_counter;
    printf("   Verification:    Direct=%d, Hooked=%d\n\n", 
           complex_direct_counter, complex_hooked_counter);
    
    // Benchmark 3: Hook installation/removal
    printf("3. Hook Installation/Removal Benchmark:\n");
    double hook_install_time = benchmark_hook_installation();
    printf("   Hook install+remove: %.2f ns/operation\n\n", hook_install_time);
    
    // Summary
    printf("Summary:\n");
    printf("--------\n");
    printf("Simple function hook overhead:  %.1f%% (%.2f ns)\n", 
           ((hooked_time - direct_time) / direct_time) * 100, 
           hooked_time - direct_time);
    printf("Complex function hook overhead: %.1f%% (%.2f ns)\n", 
           ((complex_hooked_time - complex_direct_time) / complex_direct_time) * 100, 
           complex_hooked_time - complex_direct_time);
    printf("Hook installation cost:         %.2f μs\n", hook_install_time / 1000.0);
    
    // Architecture info
    printf("\nArchitecture: ");
#if defined(__x86_64__) || defined(_M_X64)
    printf("x86_64\n");
#elif defined(__aarch64__) || defined(_M_ARM64)
    printf("ARM64\n");
#elif defined(__arm__) || defined(_M_ARM)
    printf("ARM32\n");
#else
    printf("Unknown\n");
#endif
    
    return 0;
} 