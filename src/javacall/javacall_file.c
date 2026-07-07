#include "vitajme.h"
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <string.h>
#include <stdio.h>

// -----------------------------------------------------------------------------
// Javacall File HAL (mapping phoneME to Vita SceIo)
// -----------------------------------------------------------------------------

typedef unsigned int javacall_result;
typedef SceUID javacall_handle;
#define JAVACALL_OK 0
#define JAVACALL_FAIL 1

javacall_result javacall_file_open(const char *filename, int flags, javacall_handle *handle) {
    if (!filename || !handle) return JAVACALL_FAIL;
    
    // Convert generic flags to SCE_O_* flags
    // (Actual conversion depends on exact phoneME flag values)
    int sce_flags = SCE_O_RDWR; // Simplification
    
    char path[512];
    snprintf(path, sizeof(path), "%s%s", JAVA_DATA_ROOT, filename);
    
    *handle = sceIoOpen(path, sce_flags, 0777);
    return (*handle < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_file_close(javacall_handle handle) {
    return (sceIoClose(handle) < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_file_read(javacall_handle handle, unsigned char *buffer, int length, int *bytes_read) {
    int res = sceIoRead(handle, buffer, length);
    if (res < 0) return JAVACALL_FAIL;
    if (bytes_read) *bytes_read = res;
    return JAVACALL_OK;
}

javacall_result javacall_file_write(javacall_handle handle, const unsigned char *buffer, int length, int *bytes_written) {
    int res = sceIoWrite(handle, buffer, length);
    if (res < 0) return JAVACALL_FAIL;
    if (bytes_written) *bytes_written = res;
    return JAVACALL_OK;
}

javacall_result javacall_file_seek(javacall_handle handle, long long offset, int whence) {
    // whence mapping: 0=SET, 1=CUR, 2=END
    int sce_whence = SCE_SEEK_SET;
    if (whence == 1) sce_whence = SCE_SEEK_CUR;
    else if (whence == 2) sce_whence = SCE_SEEK_END;
    
    return (sceIoLseek(handle, offset, sce_whence) < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}

javacall_result javacall_file_delete(const char *filename) {
    char path[512];
    snprintf(path, sizeof(path), "%s%s", JAVA_DATA_ROOT, filename);
    return (sceIoRemove(path) < 0) ? JAVACALL_FAIL : JAVACALL_OK;
}
