#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

void* __dso_handle = 0;

int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle){ (void)func; (void)arg; (void)dso_handle; return 0; }
int __aeabi_atexit(void* obj, void (*func)(void*), void* dso){ (void)obj; (void)func; (void)dso; return 0; }

void _exit(int status){ (void)status; for(;;){} }

static char heap[1024*1024];
static char* brk = heap;
void* _sbrk(ptrdiff_t incr){ char* prev = brk; brk += incr; return prev; }

int _write(int fd, const void* buf, unsigned int count){ (void)fd; (void)buf; (void)count; return -1; }
int _read(int fd, void* buf, unsigned int count){ (void)fd; (void)buf; (void)count; return -1; }
int _close(int fd){ (void)fd; return -1; }
int _lseek(int fd, int offset, int whence){ (void)fd; (void)offset; (void)whence; return -1; }
int _fstat(int fd, struct stat* st){ (void)fd; if(st){ st->st_mode = S_IFCHR; } return 0; }
int _isatty(int fd){ (void)fd; return 1; }
int _open(const char* path, int flags, int mode){ (void)path; (void)flags; (void)mode; return -1; }
int _getpid(void){ return 1; }
int _kill(int pid, int sig){ (void)pid; (void)sig; return -1; }

void _init(void){}
void _fini(void){}

