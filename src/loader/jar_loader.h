#ifndef JAR_LOADER_H
#define JAR_LOADER_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Opens a JAR file for reading.
 * @param path Path to the .jar file.
 * @return true on success, false on error.
 */
bool jar_open(const char *path);

/**
 * Closes the currently open JAR file.
 */
void jar_close(void);

/**
 * Reads a file from the open JAR into a newly allocated memory buffer.
 * The caller is responsible for freeing the buffer using free().
 * 
 * @param filename Name of the file inside the JAR (e.g., "META-INF/MANIFEST.MF", "com/example/MyClass.class")
 * @param out_size Pointer to store the size of the extracted file.
 * @return Pointer to the extracted data, or NULL if not found or error.
 */
void* jar_extract_file(const char *filename, size_t *out_size);

#endif // JAR_LOADER_H
