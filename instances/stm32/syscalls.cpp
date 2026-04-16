/***
 * CHATGPT GENERATED CODE !!!
 * This code provides implementations of system call stubs required for
 * newlib on an embedded system, specifically for an STM32H5xx microcontroller.
 */

// syscalls.cpp
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

extern "C" {

// -----------------------------------------------------------------------------
// Heap / malloc support (_sbrk_r)
// -----------------------------------------------------------------------------
extern char _end; // Provided by the linker script
static char* heap_end = nullptr;

void* _sbrk_r(struct _reent* r, ptrdiff_t incr) {
    (void)r;
    if (!heap_end) {
        heap_end = &_end;
    }

    char* prev_heap_end = heap_end;
    heap_end += incr;
    return reinterpret_cast<void*>(prev_heap_end);
}

// -----------------------------------------------------------------------------
// Minimal stub implementations for newlib syscalls
// -----------------------------------------------------------------------------
int _getpid_r(struct _reent* r) {
    (void)r;
    return 1; // Dummy PID
}

int _kill_r(struct _reent* r, int pid, int sig) {
    (void)r;
    (void)pid;
    (void)sig;
    return -1; // Unsupported
}

int _fstat_r(struct _reent* r, int fd, struct stat* st) {
    (void)r;
    (void)fd;
    if (st) {
        std::memset(st, 0, sizeof(*st));
        st->st_mode = S_IFCHR; // Character device
    }
    return 0;
}

int _isatty_r(struct _reent* r, int fd) {
    (void)r;
    (void)fd;
    return 1;
}

int _close_r(struct _reent* r, int fd) {
    (void)r;
    (void)fd;
    return 0;
}

int _close(int fd) {
    (void)fd;
    return 0;
}

_off_t _lseek_r(struct _reent* r, int fd, _off_t ptr, int dir) {
    (void)r;
    (void)fd;
    (void)ptr;
    (void)dir;
    return 0;
}

_ssize_t _read_r(struct _reent* r, int fd, void* ptr, size_t len) {
    (void)r;
    (void)fd;
    (void)ptr;
    (void)len;
    return 0;
}

_ssize_t _write_r(struct _reent* r, int fd, const void* ptr, size_t len) {
    (void)r;
    (void)fd;
    (void)ptr;
    return static_cast<_ssize_t>(len); // Pretend all bytes were written
}

// -----------------------------------------------------------------------------
// Exit / abort
// -----------------------------------------------------------------------------
void _exit(int status) {
    (void)status;
    while (1) {
        __asm__("wfi"); // Wait for interrupt
    }
}

void abort(void) {
    while (1) {
        __asm__("wfi"); // Wait forever
    }
}

} // extern "C"
