
#include "platform.h"
#include <xcb/xproto.h>

#if P_LINUX

#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <sys/time.h>
#include <xcb/xcb.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  Display *display;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_win;
} internal_state;

b8 platform_startup(
    mem_arena *arena,
    platform_state *plat_state,
    const char *app_name,
    i32 x,
    i32 y,
    i32 width,
    i32 height
) {
  // Create the internal state
  plat_state->internal_state = PUSH_STRUCT(arena, internal_state);
  internal_state *state = (internal_state *)plat_state->internal_state;

  state->display = XOpenDisplay(NULL);

  XAutoRepeatOff(state->display);

  state->connection = XGetXCBConnection(state->display);

  if (xcb_connection_has_error(state->connection)) {
    return FALSE;
  }

  const struct xcb_setup_t *setup = xcb_get_setup(state->connection);

  xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
  i32 screen_p = 0;
  for (i32 s = screen_p; s > 0; s--) {
    xcb_screen_next(&it);
  }

  state->screen = it.data;

  state->window = xcb_generate_id(state->connection);

  u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

  u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS |
                     XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                     XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                     XCB_EVENT_MASK_POINTER_MOTION |
                     XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  u32 value_list[] = {state->screen->black_pixel, event_values};

  xcb_void_cookie_t cookie = xcb_create_window(
      state->connection,
      XCB_COPY_FROM_PARENT,
      state->window,
      state->screen->root,
      x,
      y,
      width,
      height,
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      state->screen->root_visual,
      event_mask,
      value_list
  );

  xcb_change_property(
      state->connection,
      XCB_PROP_MODE_REPLACE,
      state->window,
      XCB_ATOM_WM_NAME,
      XCB_ATOM_STRING,
      8,
      strlen(app_name),
      app_name
  );

  return TRUE;
}

void platform_shutdown(platform_state *plat_state) {}

b8 platform_pump_messages(platform_state *plat_state) {}

#endif
