#pragma once

#include "arena.h"
#include "base.h"

typedef struct {
  void *internal_state;
} platform_state;

b8 platform_startup(
    mem_arena *arena,
    platform_state *plat_state,
    const char *app_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height
);

void platform_shutdown(platform_state *plat_state);

b8 platform_pump_messages(platform_state *plat_state);
