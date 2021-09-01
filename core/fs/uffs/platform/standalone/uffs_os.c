/*
  This file is part of UFFS, the Ultra-low-cost Flash File System.

  Copyright (C) 2005-2009 Ricky Zheng <ricky_gz_zheng@yahoo.co.nz>

  UFFS is free software; you can redistribute it and/or modify it under
  the GNU Library General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any
  later version.

  UFFS is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
  or GNU Library General Public License, as applicable, for more details.

  You should have received a copy of the GNU General Public License
  and GNU Library General Public License along with UFFS; if not, write
  to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA  02110-1301, USA.

  As a special exception, if other files instantiate templates or use
  macros or inline functions from this file, or you compile this file
  and link it with other works to produce a work based on this file,
  this file does not by itself cause the resulting work to be covered
  by the GNU General Public License. However the source code for this
  file must still be made available in accordance with section (3) of
  the GNU General Public License v2.

  This exception does not invalidate any other reasons why a work based
  on this file might be covered by the GNU General Public License.
*/

/**
 * \file uffs_os_posix.c
 * \brief Emulation on POSIX host. This is just a dumb implementation, does not really create semaphores.
 * \author Ricky Zheng
 */

#include "uffs_config.h"
#include "uffs/uffs_os.h"
#include "uffs/uffs_public.h"

#define PFX "os  : "


int uffs_SemCreate(OSSEM *sem)
{
    return 0;
}

int uffs_SemWait(OSSEM sem)
{
    return 0;
}

int uffs_SemSignal(OSSEM sem)
{
    return 0;
}

int uffs_SemDelete(OSSEM *sem)
{
    return 0;
}

int uffs_OSGetTaskId(void)
{
	return 0;
}

unsigned int uffs_GetCurDateTime(void)
{
    return 0;
}

#if CONFIG_USE_SYSTEM_MEMORY_ALLOCATOR > 0
static void * sys_malloc(struct uffs_DeviceSt *dev, unsigned int size)
{
	return NULL;
}

static URET sys_free(struct uffs_DeviceSt *dev, void *p)
{
	return U_SUCC;
}

void uffs_MemSetupSystemAllocator(uffs_MemAllocator *allocator)
{
	allocator->malloc = sys_malloc;
	allocator->free = sys_free;
}
#endif

void uffs_SetupDebugOutput(void)
{
}
