//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
//#include "3d.h"
//#include "tool.h"
//#include "utils.h"
//#include "pcwin.h"
#include "compiler.h"
#ifdef LEGACY_HANDLE_ALLOCATOR
#include "libs/threadlib.h"
#endif
#include "libs/memlib.h"
#include "libs/log.h"
#include "libs/misc.h"

#ifdef LEGACY_HANDLE_ALLOCATOR

#define GetToolFrame() 1

//#define MEM_DEBUG

#ifdef MEM_DEBUG
#include <crtdbg.h>
#endif

#define LEAK_DEBUG

#ifdef LEAK_DEBUG
BOOLEAN leak_debug;
int leak_idx = -1;
int leak_size = -1;
#endif
static Mutex _MemoryLock;

//#define MAX_EXTENTS 100000
#define MAX_EXTENTS (MEM_HANDLE) 32766
		// Changed to 32766 to get rid of warnings that an expression
		// 'h <= MAX_EXTENTS' is always true due to limited range of
		// MEM_HANDLE.

/* Keep track of memory allocations. */

typedef struct _szMemoryNode {
	void *memory;
	struct _szMemoryNode *next;
	MEM_HANDLE handle;
	MEM_SIZE size;
	int refcount;
} szMemoryNode;

static szMemoryNode extents[MAX_EXTENTS];
static szMemoryNode *freeListHead = NULL;

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** CheckMemory()
**
** DESCRIPTION
** Checks integrity of memory allocation: diagnostic only.
** Either aborts or spits messages to stdout if there's a
** problem.
**
** INPUT
**
** OUTPUT
**
** HISTORY
**      04-Sept-96:AKL Creation.
**
** ASSUMPTIONS
**
**END*/

#ifdef MEM_DEBUG

void CheckMemory(void)
{
	// Send all reports to STDOUT
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );

	_CrtCheckMemory();
	fflush(stdout);
}
#endif

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** mem = SafeMalloc (coreSize, diagnostic)
**
** DESCRIPTION
** Mallocs the required memory. If the malloc fails, the game
** is killed with a diagnostic string.
**
** INPUT
** int coreSize = How much memory to malloc (in bytes).
** char *diagnostic = String identifying the caller.
**
** OUTPUT
** void *mem = Ptr to memory.  Never returns NULL.
**
** HISTORY
**      14-Jan-08:Cleanups and conversion from MallocWithRetry.
**      06-Nov-96:AKL Creation.
**
** ASSUMPTIONS
**
**END*/

static void *
SafeMalloc (int bytes, char *diagStr)
{
	void *ptr;
	
	ptr = malloc (bytes);
	if (!ptr)
	{
		log_add (log_Fatal, "Malloc failed for %s. #Bytes %d.", diagStr, bytes);
		fflush (stderr);
		explode ();
	}
	return ptr;
}

MEM_HANDLE
mem_allocate (MEM_SIZE coreSize, MEM_FLAGS flags)
{
	szMemoryNode *node;

	LockMutex (_MemoryLock);

#ifdef MEM_DEBUG
	CheckMemory();
#endif

	if ((node = freeListHead) == NULL)
		log_add (log_Error, "mem_allocate: out of extents.");
	else if ((node->memory = MallocWithRetry (coreSize, "mem_allocate:")) == 0
			&& coreSize)
		log_add (log_Error, "mem_allocate: couldn't allocate %ld bytes.",
				coreSize);
	else
	{
		freeListHead = node->next;

		node->size = coreSize;
		node->handle = node - extents + 1;
		node->refcount = 0;
		if (flags & MEM_ZEROINIT)
				memset (node->memory, 0, node->size);
#ifdef LEAK_DEBUG
		if (leak_debug)
		{
			log_add (log_Debug, "alloc %d: %p, %lu", (int) node->handle,
					(void *) node->memory, node->size);
			// Preferred form:
			//log_add (log_Debug, "alloc %d: %#8" PRIxPTR ", %lu",
			//		(int) node->handle,	(intptr_t) node->memory, node->size);
		}
		if (node->handle == leak_idx && node->size == leak_size)
			node = node;
		if (node->size == leak_size)
			node = node;
#endif /* LEAK_DEBUG */

		UnlockMutex (_MemoryLock);
		return (node->handle);
	}

	UnlockMutex (_MemoryLock);
	return (0);
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** size = mem_get_size(h)
**
** DESCRIPTION
**      Returns the number of bytes in the given memory allocation.
**
** INPUT
**      int h = Handle to memory allocation.
**
** OUTPUT
**      int size = Number of bytes in allocation or zero on error.
**
** HISTORY
**      09-Jul-96:AKL Creation. Original version in LIBS\MEMORY\DATAINFO.C.
**
** ASSUMPTIONS
**
**END*/

MEM_SIZE
mem_get_size (MEM_HANDLE h)
{
	LockMutex (_MemoryLock);
	if (h > 0 && h <= MAX_EXTENTS && extents[h - 1].handle == h)
	{
		UnlockMutex (_MemoryLock);
		return ((int)extents[h - 1].size);
	}
	UnlockMutex (_MemoryLock);

	return (0);
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** done = mem_init(coreSize, pma, diskName)
**
** DESCRIPTION
** Initialize data that tracks allocation extents.
**
** INPUT
** int coreSize = IGNORED!
** int * pma = IGNORED!
** PSTR diskName = IGNORED!
**
** OUTPUT
**      int done = TRUE.
**
** HISTORY
**      09-Jul-96:AKL Creation. Original version in LIBS\MEMORY\INIT.C.
**
** ASSUMPTIONS
**
**END*/

BOOLEAN
mem_init (void)
{
	int i;

	_MemoryLock = CreateMutex ("memory lock", SYNC_CLASS_RESOURCE);
	LockMutex (_MemoryLock);

	freeListHead = &extents[0];
	for (i=0; i<MAX_EXTENTS; i++)
	{
		extents[i].memory = NULL;
		extents[i].handle = -1;
		extents[i].next = &extents[i+1];
		extents[i].size = 0;
	}
	extents[MAX_EXTENTS-1].next = NULL;

	UnlockMutex (_MemoryLock);

	return TRUE;
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** mem_uninit()
**
** DESCRIPTION
** Free allocated memory.
**
** INPUT
**
** OUTPUT
**
** HISTORY
**      14-Oct-96:AKL Creation.
**
** ASSUMPTIONS
**
**END*/

BOOLEAN
mem_uninit(void)
{
	int i;

	LockMutex (_MemoryLock);

	for (i=0; i<MAX_EXTENTS; i++)
	{
		if (extents[i].handle != -1)
		{
			log_add (log_Debug, "LEAK: unreleased extent %d: %p, %lu",
					extents[i].handle, (void *) extents[i].memory,
					extents[i].size);
			// Preferred form:
			//log_add (log_Debug, "LEAK: unreleased extent %d: %#8" PRIxPTR
			//		", %lu", extents[i].handle,
			//		(intptr_t) extents[i].memory, extents[i].size);
			fflush (stderr);
			extents[i].handle = -1;
			if (extents[i].memory)
			{
				free (extents[i].memory);
				extents[i].memory = 0;
			}
		}
		extents[i].size = 0;
	}
	freeListHead = 0;

	UnlockMutex (_MemoryLock);

	return (TRUE);
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** done = mem_release(h)
**
** DESCRIPTION
**      Frees the given memory.
**
** INPUT
** int h = Handle to memory to be released.
**
** OUTPUT
**      int done = TRUE.
**
** HISTORY
**      09-Jul-96:AKL Creation. Original version in LIBS\MEMORY\ALLOC.C.
**
** ASSUMPTIONS
**
**END*/

BOOLEAN
mem_release(MEM_HANDLE h)
{
	if (h == 0)
		return (TRUE);

	LockMutex (_MemoryLock);

	--h;
	if (h < 0 || h >= MAX_EXTENTS)
		log_add (log_Debug, "LEAK: attempt to release invalid extent %d", h);
	else if (extents[h].handle == -1)
		log_add (log_Debug, "LEAK: attempt to release unallocated extent %d",h);
	else if (extents[h].refcount == 0)
	{
#ifdef LEAK_DEBUG
		if (leak_debug)
		{
			log_add (log_Debug, "free %d: %p",
					extents[h].handle, (void *) extents[h].memory);
			// Preferred form:
			//log_add (log_Debug, "free %d: %#8" PRIxPTR,
			//		extents[h].handle, (intptr_t) extents[h].memory);
		}
#endif
		if (extents[h].memory)
		{
			free (extents[h].memory);
			extents[h].memory = 0;
		}
		extents[h].handle = -1;
		extents[h].next = freeListHead;
		freeListHead = &extents[h];
		UnlockMutex (_MemoryLock);
		return TRUE;
	}

	UnlockMutex (_MemoryLock);
	return FALSE;
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** memp = mem_lock (h)
**
** DESCRIPTION
**      Converts a MEM_HANDLE into its equivalent pointer.
**
** INPUT
** int h = Handle to memory to be accessed.
**
** OUTPUT
**      void * memp = Ptr to memory.
**
** HISTORY
**      09-Jul-96:AKL Creation. Original version in LIBS\MEMORY\SIMPLE.C.
**
** ASSUMPTIONS
**
**END*/

void *
mem_lock (MEM_HANDLE h)
{
	LockMutex (_MemoryLock);

	if (h > 0 && h <= MAX_EXTENTS && extents[h - 1].handle == h)
	{
		++extents[h - 1].refcount;
		UnlockMutex (_MemoryLock);
		return (extents[h - 1].memory);
	}

	UnlockMutex (_MemoryLock);
	return (0);
}

/*****************************************************************************/
/*FUNCTION
**
** SYNOPSIS
** done = mem_simple_unaccess(h)
**
** DESCRIPTION
**      Drops the refcount on a piece of memory.
**
** INPUT
** int h = Handle to memory to be unaccessed.
**
** OUTPUT
**      int done = TRUE.
**
** HISTORY
**      09-Jul-96:AKL Creation. Original version in LIBS\MEMORY\SIMPLE.C.
**
** ASSUMPTIONS
**
**END*/

BOOLEAN
mem_unlock (MEM_HANDLE h)
{
	LockMutex (_MemoryLock);
	if (h > 0 && h <= MAX_EXTENTS && extents[h - 1].handle == h)
	{
		if (extents[h - 1].refcount)
			--extents[h - 1].refcount;
		UnlockMutex (_MemoryLock);
		return (1);
	}

	UnlockMutex (_MemoryLock);
	return (0);
}

static void *
_alloc_mem (int size)
{
	void *p;
	int h;

	h = mem_allocate (sizeof (MEM_HEADER) + size, DEFAULT_MEM_FLAGS);
	p = (void *)mem_lock (h);
	if (p)
	{
		((MEM_HEADER *) p)->handle = h;
		p = (char *)p + sizeof (MEM_HEADER);
	}

	return (p);
}

void *
HMalloc (int size)
{
    void *p;

	if (size == 0) return (0);

    if ((p = _alloc_mem(size)) == NULL) {
        log_add (log_Fatal, "Fatal Error: HMalloc(): out of memory.");
		fflush (stderr);
        explode ();
    }
    return (p);
}

void
HFree (void *p)
{
	if (p)
	{
		MEM_HEADER *hdr;
		MEM_HANDLE h;

		hdr = GET_MEM_HEADER(p);
		h = hdr->handle;
		mem_unlock (h);
		mem_release (h);
	}
}

void *
HCalloc (int size)
{
	void *p;

	p = HMalloc (size);
	if (p)
		memset (p, 0, size);

	return (p);
}

void *
HRealloc (void *p, int size)
{
	void *np;
	int osize;

	if ((np = _alloc_mem (size)) && p)
	{
		MEM_HEADER *hdr;
		MEM_HANDLE h;

		hdr = GET_MEM_HEADER (p);
		h = hdr->handle;
		osize = mem_get_size (h) - sizeof (MEM_HEADER);
		if (size > osize)
			size = osize;
		memcpy (np, p, size);
		HFree (p);
	}

	return (np);
}

#else /* !LEGACY_HANDLE_ALLOCATOR */

BOOLEAN
mem_init (void)
{
	return TRUE;
}

BOOLEAN
mem_uninit(void)
{
	return (TRUE);
}

void *
HMalloc (int size)
{
	void *p;

	if (size == 0) return NULL;

	if (size < 0) 
	{
		log_add (log_Fatal, "Fatal Error: Request for negative amount of memory %d!", size);
		fflush (stderr);
		explode ();
	}

	if ((p = malloc (size)) == NULL) 
	{
		log_add (log_Fatal, "Fatal Error: HMalloc(): out of memory.");
		fflush (stderr);
		explode ();
	}
	return (p);
}

void
HFree (void *p)
{
	if (p)
	{
		free (p);
	}
}

void *
HCalloc (int size)
{
	void *p;

	p = HMalloc (size);
	memset (p, 0, size);

	return (p);
}

void *
HRealloc (void *p, int size)
{
	return realloc (p, size);
}

#endif /* LEGACY_HANDLE_ALLOCATOR */
