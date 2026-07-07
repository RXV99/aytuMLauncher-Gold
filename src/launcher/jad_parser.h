#ifndef JAD_PARSER_H
#define JAD_PARSER_H

#include <stdbool.h>

typedef struct {
    char name[128];
    char vendor[128];
    char version[32];
    char main_class[256];
    char jar_url[256];
} JadInfo;

/**
 * Parses a .jad file or a MANIFEST.MF file from a JAR to extract MIDlet properties.
 * 
 * @param path The path to the .jad file, or a memory buffer containing MANIFEST.MF
 * @param is_file True if path is a file path, false if it's a null-terminated string buffer
 * @param out_info Pointer to JadInfo struct to populate
 * @return true if successfully parsed, false otherwise
 */
bool jad_parse(const char *data, bool is_file, JadInfo *out_info);

#endif // JAD_PARSER_H
