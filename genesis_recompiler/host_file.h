/* Replace a completed temporary file without deleting the previous save first. */
#ifndef GENESIS_HOST_FILE_H
#define GENESIS_HOST_FILE_H
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
static int host_file_replace(const char *temporary,const char *path) {
#ifdef _WIN32
    /* Callers use generated ASCII filenames relative to the working directory. */
    return MoveFileExA(temporary,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
    return rename(temporary,path)==0;
#endif
}
#endif
