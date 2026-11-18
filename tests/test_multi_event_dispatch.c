#include "frnl_event_dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(x) do { if (!(x)) { fprintf(stderr, "failure: %s at line %d\n", #x, __LINE__); return 1; } } while (0)

static int first_calls, second_calls, notices;
static int first(const void *p, void *ctx) { (void)p; (void)ctx; ++first_calls; return 0; }
static int second(const void *p, void *ctx) { (void)p; (void)ctx; ++second_calls; return 0; }
static void notify(const char *component, const char *event, const void *payload, void *ctx) {
  (void)payload; (void)ctx;
  if (strcmp(component, "panel") == 0 && strcmp(event, "submit") == 0) ++notices;
}
int main(void) {
  frnl_event_binding events[2] = {
    {"panel", "idle", "hover", "onHover"},
    {"panel", "idle", "submit", "onSubmit"}
  };
  frnl_handler_binding handlers[2] = {
    {"panel", "onHover", first, NULL},
    {"panel", "onSubmit", second, NULL}
  };
  frnl_event_dispatcher d = {events, 2, 2, handlers, 2, 2, notify, NULL};
  REQUIRE(frnl_event_invoke(&d, "panel", "idle", "submit", NULL) == FRNL_EVENT_OK);
  REQUIRE(first_calls == 0 && second_calls == 1 && notices == 1);
  REQUIRE(frnl_event_invoke(&d, "panel", "idle", "hover", NULL) == FRNL_EVENT_OK);
  REQUIRE(first_calls == 1 && second_calls == 1 && notices == 1);
  REQUIRE(frnl_event_invoke(&d, "panel", "idle", "missing", NULL) == FRNL_EVENT_UNKNOWN_EVENT);
  REQUIRE(frnl_event_invoke(&d, "panel", "active", "submit", NULL) == FRNL_EVENT_UNKNOWN_STATE);
  REQUIRE(frnl_event_register(&d, (frnl_event_binding){"panel","idle","submit","onSubmit"}) == FRNL_EVENT_DUPLICATE);
  REQUIRE(frnl_event_register(&d, (frnl_event_binding){"panel","idle","click","onClick"}) == FRNL_EVENT_CAPACITY);
  REQUIRE(frnl_handler_register(&d, (frnl_handler_binding){"panel","onSubmit",second,NULL}) == FRNL_EVENT_DUPLICATE);
  REQUIRE(frnl_handler_register(&d, (frnl_handler_binding){"panel","onClick",second,NULL}) == FRNL_EVENT_CAPACITY);
  return 0;
}
