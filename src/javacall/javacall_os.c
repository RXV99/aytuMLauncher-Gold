#include "vitajme.h"
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/sysmem.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Javacall OS/Threading HAL (mapping phoneME to Vita)
// These functions match the phoneME javacall_os / javacall_memory interfaces.
// -----------------------------------------------------------------------------

// We define dummy types here that would normally come from javacall_time.h etc.
typedef long long javacall_int64;
typedef unsigned int javacall_result;
#define JAVACALL_OK 0
#define JAVACALL_FAIL 1

// -----------------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------------

void* javacall_malloc(int size) {
    // Vita SDK's newlib malloc maps to sceKernelAllocMemBlock internally
    return malloc(size);
}

void javacall_free(void* ptr) {
    free(ptr);
}

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

javacall_int64 javacall_time_get_ms(void) {
    return (javacall_int64)(sceKernelGetProcessTimeWide() / 1000ULL);
}

// -----------------------------------------------------------------------------
// Threading
// -----------------------------------------------------------------------------

typedef SceUID javacall_handle;

// PhoneME expects pthreads-like behavior
javacall_result javacall_thread_create(javacall_handle *thread_handle, 
                                       void* (*entry_point)(void*), 
                                       void *param, 
                                       int priority, 
                                       int stack_size) {
    if (!thread_handle) return JAVACALL_FAIL;
    
    // clamp stack size
    if (stack_size < 0x10000) stack_size = 0x10000;
    
    // Priority: phoneME 1 (lowest) to 10 (highest)
    // Vita priority: 0x10 (highest) to 0xAF (lowest), 0x40 is default
    int vita_prio = 0x40; // simplified mapping for now
    
    *thread_handle = sceKernelCreateThread("jvm_thread", (SceKernelThreadEntry)entry_point, vita_prio, stack_size, 0, 0, NULL);
    if (*thread_handle < 0) return JAVACALL_FAIL;
    
    int res = sceKernelStartThread(*thread_handle, sizeof(void*), &param);
    return (res < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_mutex_create(javacall_handle *mutex_handle) {
    *mutex_handle = sceKernelCreateMutex("jvm_mutex", 0, 0, NULL);
    return (*mutex_handle < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_mutex_lock(javacall_handle mutex_handle) {
    return (sceKernelLockMutex(mutex_handle, 1, NULL) < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_mutex_unlock(javacall_handle mutex_handle) {
    return (sceKernelUnlockMutex(mutex_handle, 1) < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_mutex_destroy(javacall_handle mutex_handle) {
    sceKernelDeleteMutex(mutex_handle);
    return JAVACALL_OK;
}

// Condvars map nicely to Vita Semaphores with initial count 0
javacall_result javacall_condvar_create(javacall_handle *condvar_handle) {
    *condvar_handle = sceKernelCreateSema("jvm_cond", 0, 0, 1, NULL);
    return (*condvar_handle < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_condvar_wait(javacall_handle condvar_handle, javacall_handle mutex_handle, javacall_int64 timeout_ms) {
    sceKernelUnlockMutex(mutex_handle, 1);
    
    int res;
    if (timeout_ms <= 0) {
        res = sceKernelWaitSema(condvar_handle, 1, NULL);
    } else {
        unsigned int timeout_us = (unsigned int)(timeout_ms * 1000);
        res = sceKernelWaitSemaCB(condvar_handle, 1, &timeout_us);
    }
    
    sceKernelLockMutex(mutex_handle, 1, NULL);
    return (res < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_condvar_signal(javacall_handle condvar_handle) {
    sceKernelSignalSema(condvar_handle, 1);
    return JAVACALL_OK;
}

javacall_result javacall_condvar_destroy(javacall_handle condvar_handle) {
    sceKernelDeleteSema(condvar_handle);
    return JAVACALL_OK;
}
