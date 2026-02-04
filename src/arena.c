#include "arena.h"
#include "base.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

mem_arena *arena_create(u64 reserve_size, u64 commit_size) {
  u32 pagesize = get_pagesize();

  reserve_size = ALIGN_UP_POW2(reserve_size, pagesize);
  commit_size = ALIGN_UP_POW2(commit_size, pagesize);

  mem_arena *arena = mem_reserve(reserve_size);

  if (!mem_commit(arena, commit_size)) {
    return NULL;
  }

  arena->reserve_size = reserve_size;
  arena->commit_size = commit_size;
  arena->pos = ARENA_BASE_POS;
  arena->commit_pos = commit_size;

  return arena;
}

void arena_destroy(mem_arena *arena) {
  mem_release(arena, arena->reserve_size);
}

void *arena_push(mem_arena *arena, u64 size, b32 zero) {
  u64 pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
  u64 new_pos = pos_aligned + size;

  if (new_pos > arena->reserve_size) {
    perror("Failed to allocate memory");
    exit(1);
  }

  if (new_pos > arena->commit_pos) {
    u64 new_commit_pos = new_pos;
    new_commit_pos += arena->commit_size - 1;
    new_commit_pos -= new_commit_pos % arena->commit_size;
    new_commit_pos = MIN(new_commit_pos, arena->reserve_size);

    u8 *mem = (u8 *)arena + arena->commit_pos;
    u64 commit_size = new_commit_pos - arena->commit_pos;

    if (!mem_commit(mem, commit_size)) {
      return NULL;
    }

    arena->commit_pos = new_commit_pos;
  }

  arena->pos = new_pos;

  u8 *out = (u8 *)arena + pos_aligned;

  if (zero) {
    memset(out, 0, size);
  }

  return out;
}

void arena_pop(mem_arena *arena, u64 size) {
  size = MIN(size, arena->pos - ARENA_BASE_POS);
  arena->pos -= size;
}

void arena_pop_to(mem_arena *arena, u64 pos) {
  u64 size = pos < arena->pos ? arena->pos - pos : 0;
  arena_pop(arena, size);
}

void arena_clear(mem_arena *arena) { arena_pop_to(arena, ARENA_BASE_POS); }

mem_arena_temp arena_temp_begin(mem_arena *arena) {
  return (mem_arena_temp){
      .arena = arena,
      .start_pos = arena->pos,
  };
}

void arena_temp_end(mem_arena_temp temp) {
  arena_pop_to(temp.arena, temp.start_pos);
}

thread_local static mem_arena *_scratch_arena[] = {NULL, NULL};

mem_arena_temp arena_scratch_get(mem_arena **conflicts, u32 num_conflicts) {
  i32 scratch_index = -1;

  for (i32 i = 0; i < 2; i++) {
    b32 conflict_found = FALSE;

    for (u32 j = 0; j < num_conflicts; j++) {
      if (_scratch_arena[i] == conflicts[j]) {
        conflict_found = TRUE;
        break;
      }
    }

    if (!conflict_found) {
      scratch_index = i;
      break;
    }
  }

  if (scratch_index == -1) {
    return (mem_arena_temp){0};
  }

  mem_arena **selected = &_scratch_arena[scratch_index];

  if (*selected == NULL) {
    *selected = arena_create(MiB(64), MiB(1));
  }

  return arena_temp_begin(*selected);
}

void arena_scratch_release(mem_arena_temp scratch);

#if P_LINUX

#include <sys/mman.h>
#include <unistd.h>

u32 get_pagesize(void) { return (u32)sysconf(_SC_PAGESIZE); }

void *mem_reserve(u64 size) {
  void *out = mmap(NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (out == MAP_FAILED)
    return NULL;

  return out;
}

b32 mem_commit(void *ptr, u64 size) {
  i32 ret = mprotect(ptr, size, PROT_READ | PROT_WRITE);
  return ret == 0;
}

b32 mem_decommit(void *ptr, u64 size) {
  i32 ret = mprotect(ptr, size, PROT_NONE);
  return ret == 0;
}

b32 mem_release(void *ptr, u64 size) {
  i32 ret = munmap(ptr, size);
  return ret == 0;
}

#elif P_WINDOWS

#include <windows.h>

u32 get_pagesize(void) {
  SYSTEM_INFO sysinfo = {0};
  GetSystemInfo(&sysinfo);

  return sysinfo.dwPageSize;
}

void *mem_reserve(u64 size) {
  return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
}

b32 mem_commit(void *ptr, u64 size) {
  void *ret = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
  return ret != NULL;
}

b32 mem_decommit(void *ptr, u64 size) {
  return VirtualFree(ptr, size, MEM_DECOMMIT);
}

b32 mem_release(void *ptr, u64 size) {
  return VirtualFree(ptr, size, MEM_RELEASE);
}

#endif
