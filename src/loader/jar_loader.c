#include "jar_loader.h"
#include "vitajme.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include miniz for ZIP parsing
#include "miniz.h"

static mz_zip_archive g_zip;
static bool g_is_open = false;

bool jar_open(const char *path) {
    if (g_is_open) {
        jar_close();
    }
    
    memset(&g_zip, 0, sizeof(g_zip));
    
    if (!mz_zip_reader_init_file(&g_zip, path, 0)) {
        VJME_ERR("Failed to open JAR as ZIP: %s", path);
        return false;
    }
    
    g_is_open = true;
    VJME_LOG("Opened JAR: %s", path);
    return true;
}

void jar_close(void) {
    if (g_is_open) {
        mz_zip_reader_end(&g_zip);
        g_is_open = false;
    }
}

void* jar_extract_file(const char *filename, size_t *out_size) {
    if (!g_is_open || !filename || !out_size) return NULL;
    
    // Attempt to locate file in archive
    int file_index = mz_zip_reader_locate_file(&g_zip, filename, NULL, 0);
    if (file_index < 0) {
        // Not found
        return NULL;
    }
    
    // Get file info to determine uncompressed size
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&g_zip, file_index, &file_stat)) {
        return NULL;
    }
    
    size_t uncomp_size = (size_t)file_stat.m_uncomp_size;
    
    // Allocate memory for the file content
    void *buffer = malloc(uncomp_size + 1); // +1 for safety null terminator
    if (!buffer) {
        VJME_ERR("Failed to allocate %zu bytes for JAR file extraction", uncomp_size);
        return NULL;
    }
    
    // Extract file into memory
    if (!mz_zip_reader_extract_to_mem(&g_zip, file_index, buffer, uncomp_size, 0)) {
        VJME_ERR("Failed to extract file %s from JAR", filename);
        free(buffer);
        return NULL;
    }
    
    // Null-terminate just in case it's a text file
    ((char*)buffer)[uncomp_size] = '\0';
    
    *out_size = uncomp_size;
    return buffer;
}
