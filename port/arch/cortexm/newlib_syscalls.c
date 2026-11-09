/*
 * newlib system calls for the Debug build (printf to the log): stdout and
 * stderr only, no files. _write() is in src/util/ambi_log.c (it feeds the
 * log). The heap for newlib-nano is the linker's ._user_heap_stack area
 * (from `end`, _Min_Heap_Size bytes).
 */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

int _close(int fd);
int _fstat(int fd, struct stat *st);
int _isatty(int fd);
int _lseek(int fd, int off, int whence);
int _read(int fd, char *buf, int len);
void *_sbrk(ptrdiff_t incr);

int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    (void)fd;
    return 1;
}

int _lseek(int fd, int off, int whence)
{
    (void)fd;
    (void)off;
    (void)whence;
    errno = ESPIPE;
    return -1;
}

int _read(int fd, char *buf, int len)
{
    (void)fd;
    (void)buf;
    (void)len;
    errno = EBADF;
    return -1;
}

extern uint8_t end[];            /* linker: start of ._user_heap_stack */
extern uint8_t _Min_Heap_Size[]; /* linker symbol: its address is the size */

void *_sbrk(ptrdiff_t incr)
{
    static uint8_t *brk = end;
    if (incr < 0 || (size_t)(brk - end) + (size_t)incr > (size_t)_Min_Heap_Size) {
        errno = ENOMEM;
        return (void *)-1;
    }
    uint8_t *prev = brk;
    brk += incr;
    return prev;
}
