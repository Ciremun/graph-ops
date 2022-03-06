#pragma once

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <conio.h>
#include <windows.h>
typedef HANDLE handle_t;
#define IO_BAD_FILE_HANDLE INVALID_HANDLE_VALUE
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
typedef int handle_t;
#define IO_BAD_FILE_HANDLE -1
#endif // _WIN32

#define error(fmt, ...) fprintf(stderr, "error: " fmt "\n", __VA_ARGS__)

#define UNMAP_AND_CLOSE_FILE(file)       \
    do                                   \
    {                                    \
        if (!unmap_and_close_file(file)) \
            exit(1);                     \
    } while (0)

#define CLOSE_FILE(handle)       \
    do                           \
    {                            \
        if (!close_file(handle)) \
            exit(1);             \
    } while (0)

#define UNMAP_FILE(file)       \
    do                         \
    {                          \
        if (!unmap_file(file)) \
            exit(1);           \
    } while (0)

#define MAP_FILE_(file_ptr)      \
    do                           \
    {                            \
        if (!map_file(file_ptr)) \
            exit(1);             \
    } while (0)

#define TRUNCATE_FILE(file_ptr, new_size)       \
    do                                          \
    {                                           \
        if (!truncate_file(file_ptr, new_size)) \
            exit(1);                            \
    } while (0)

#define EXIT_IF_BAD_FILE_HANDLE(handle)   \
    do                                    \
    {                                     \
        if (handle == IO_BAD_FILE_HANDLE) \
            exit(1);                      \
    } while (0)

typedef enum
{
    IO_READ_WRITE = 0,
    IO_READ_ONLY,
} flag_t;

typedef struct
{
#ifdef _WIN32
    HANDLE hMap;
#endif // _WIN32
    handle_t handle;
    flag_t access;
    size_t size;
    uint8_t *start;
} File;

File open_or_create_file(const char *path, flag_t access, int create);
File open_file(const char *path, flag_t access);
File create_file(const char *path, flag_t access);
File open_and_map_file(const char *path, flag_t access);
File create_and_map_file(const char *path, flag_t access);
int close_file(File f);
int file_exists(const char *path);
int truncate_file(File *f, size_t new_size);
int get_file_size(File *f);
int map_file(File *f);
int unmap_file(File f);
int unmap_and_close_file(File f);
