/*
 * Copyright (C) 2013 Damien Zammit <damien@zamaudio.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <inttypes.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>

extern int fd_mem;
void *map_physical(uint64_t phys_addr, size_t len);
void unmap_physical(void *virt_addr, size_t len);
