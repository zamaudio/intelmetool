#include <inttypes.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

extern int fd_mem;
void *map_physical(uint64_t phys_addr, size_t len);
void unmap_physical(void *virt_addr, size_t len);
