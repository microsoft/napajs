#pragma once

#include <napa-c.h>
#include <napa/memory/allocator.h>
#include <napa/memory/common.h>

#define NAPA_MALLOC(size) ::napa_malloc(size)
#define NAPA_FREE(pointer, sizeHint) ::napa_free(pointer, sizeHint)

#define NAPA_SET_DEFAULT_ALLOCATOR(malloc, free) ::napa_allocator_set(malloc, free)
#define NAPA_RESET_DEFAULT_ALLOCATOR() ::napa_allocator_set(napa_malloc, napa_free)
#define NAPA_ALLOCATE(size) ::napa_allocate(size)
#define NAPA_DEALLOCATE(pointer, sizeHint) ::napa_deallocate(pointer, sizeHint)

#define NAPA_DEFAULT_ALLOCATOR napa::memory::GetDefaultAllocator()
#define NAPA_CRT_ALLOCATOR napa::memory::GetCrtAllocator()

#define NAPA_MAKE_UNIQUE napa::memory::MakeUnique
#define NAPA_MAKE_SHARED napa::memory::MakeShared
#define NAPA_ALLOCATE_SHARED napa::memory::AllocateShared