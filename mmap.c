#include "mmap.h"
#include <errno.h>

int fd_mem;

void *map_physical_exact(uint64_t phys_addr, uint64_t mapto, size_t len)
{
        void *virt_addr;
	int err;

        virt_addr = mmap(mapto, len, PROT_WRITE | PROT_READ, MAP_SHARED|MAP_FIXED,
                    fd_mem, (off_t) phys_addr);

        if (virt_addr == MAP_FAILED) {
		err = errno;
                printf("Error mapping physical memory 0x%016" PRIx64 "[0x%zx] ERRNO=%d\n",
                        phys_addr, len, err);
                return NULL;
        }

        return virt_addr;
}

void *map_physical(uint64_t phys_addr, size_t len)
{
        void *virt_addr;
	int err;

        virt_addr = mmap(NULL, len, PROT_WRITE | PROT_READ, MAP_SHARED,
                    fd_mem, (off_t) phys_addr);

        if (virt_addr == MAP_FAILED) {
		err = errno;
                printf("Error mapping physical memory 0x%016" PRIx64 "[0x%zx] ERRNO=%d\n",
                        phys_addr, len, err);
                return NULL;
        }

        return virt_addr;
}

void unmap_physical(void *virt_addr, size_t len)
{
        munmap(virt_addr, len);
}
