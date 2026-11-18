#include "frnl_event_dispatch.h"
#include <stdio.h>

/* The selected handler is a value snapshot: registry mutation by a callback
   must not change which callback finishes the in-flight dispatch. */
static frnl_event_dispatcher *active;
static int first_calls, second_calls, original_notices, changed_notices;
static int replacement(const void *payload, void *ctx) {
  (void)payload; (void)ctx; ++second_calls; return 0;
}
static void original_listener(const char *c, const char *e, const void *p, void *ctx) {
  (void)c; (void)e; (void)p; (void)ctx; ++original_notices;
}
static void changed_listener(const char *c, const char *e, const void *p, void *ctx) {
  (void)c; (void)e; (void)p; (void)ctx; ++changed_notices;
}
static int first(const void *payload, void *ctx) {
  (void)payload; (void)ctx; ++first_calls;
  active->handlers[0].callback = replacement;
  active->listener = changed_listener;
  return 0;
}
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "failed line %d: %s\n", __LINE__, #expr); return 1; } } while (0)
int main(void) {
  frnl_event_binding events[1] = {{"panel","ready","click","run"}};
  frnl_handler_binding handlers[1] = {{"panel","run",first,0}};
  frnl_event_dispatcher dispatcher = {events,1,1,handlers,1,1,original_listener,0};
  active = &dispatcher;
  CHECK(frnl_event_invoke(&dispatcher,"panel","ready","click",0) == FRNL_EVENT_OK);
  CHECK(first_calls == 1 && second_calls == 0);
  CHECK(original_notices == 1 && changed_notices == 0);
  CHECK(frnl_event_invoke(&dispatcher,"panel","ready","click",0) == FRNL_EVENT_OK);
  CHECK(first_calls == 1 && second_calls == 1);
  CHECK(original_notices == 1 && changed_notices == 1);
  return 0;
}
