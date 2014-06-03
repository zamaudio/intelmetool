#include "mmap.h"

void *map_physical(uint64_t phys_addr, size_t len)
{
        void *virt_addr;

        virt_addr = mmap(0, len, PROT_WRITE | PROT_READ, MAP_SHARED,
                    fd_mem, (off_t) phys_addr);

        if (virt_addr == MAP_FAILED) {
                printf("Error mapping physical memory 0x%08" PRIx64 "[0x%zx]\n",
                        phys_addr, len);
                return NULL;
        }

        return virt_addr;
}

void unmap_physical(void *virt_addr, size_t len)
{
        munmap(virt_addr, len);
}
