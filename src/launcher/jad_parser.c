#include "jad_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <psp2/io/fcntl.h>

// strdup is not available in VitaSDK newlib without _POSIX_C_SOURCE
static char* vjme_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

static void trim_whitespace(char *str) {
    if (!str) return;
    
    // Trim trailing
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
    
    // Trim leading
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

static void parse_line(const char *line, JadInfo *info) {
    char key[128] = {0};
    char val[256] = {0};
    
    const char *colon = strchr(line, ':');
    if (!colon) return;
    
    size_t key_len = colon - line;
    if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
    strncpy(key, line, key_len);
    
    strncpy(val, colon + 1, sizeof(val) - 1);
    
    trim_whitespace(key);
    trim_whitespace(val);
    
    if (strcmp(key, "MIDlet-Name") == 0) {
        strncpy(info->name, val, sizeof(info->name) - 1);
    } else if (strcmp(key, "MIDlet-Vendor") == 0) {
        strncpy(info->vendor, val, sizeof(info->vendor) - 1);
    } else if (strcmp(key, "MIDlet-Version") == 0) {
        strncpy(info->version, val, sizeof(info->version) - 1);
    } else if (strcmp(key, "MIDlet-1") == 0) {
        // Format: Name, Icon, ClassName
        // We want the ClassName (last part)
        char *last_comma = strrchr(val, ',');
        if (last_comma) {
            char *cls = last_comma + 1;
            trim_whitespace(cls);
            strncpy(info->main_class, cls, sizeof(info->main_class) - 1);
        }
    } else if (strcmp(key, "MIDlet-Jar-URL") == 0) {
        strncpy(info->jar_url, val, sizeof(info->jar_url) - 1);
    }
}

static bool parse_buffer(const char *buffer, JadInfo *info) {
    char *buf_copy = vjme_strdup(buffer);
    if (!buf_copy) return false;
    
    char *line = strtok(buf_copy, "\r\n");
    while (line) {
        parse_line(line, info);
        line = strtok(NULL, "\r\n");
    }
    
    free(buf_copy);
    return true;
}

bool jad_parse(const char *data, bool is_file, JadInfo *out_info) {
    if (!data || !out_info) return false;
    
    memset(out_info, 0, sizeof(JadInfo));
    
    if (!is_file) {
        return parse_buffer(data, out_info);
    }
    
    // Read from file
    SceUID fd = sceIoOpen(data, SCE_O_RDONLY, 0777);
    if (fd < 0) return false;
    
    SceOff size = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);
    
    if (size <= 0 || size > 1024 * 64) {
        sceIoClose(fd);
        return false; // Invalid or too large
    }
    
    char *buffer = malloc(size + 1);
    if (!buffer) {
        sceIoClose(fd);
        return false;
    }
    
    int read = sceIoRead(fd, buffer, size);
    sceIoClose(fd);
    
    if (read != size) {
        free(buffer);
        return false;
    }
    
    buffer[size] = '\0';
    bool res = parse_buffer(buffer, out_info);
    free(buffer);
    
    return res;
}
