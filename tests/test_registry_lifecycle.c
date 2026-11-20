#include "frnl_event_dispatch.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"failure at %d: %s\n",__LINE__,#x); return 1; } } while (0)
static int calls;
static int handler(const void *payload, void *context) {
  (void)payload; (void)context; ++calls; return 0;
}
int main(void) {
  frnl_event_binding events[3] = {
    {"panel","ready","click","run"},
    {"panel","ready","hover","run"},
    {"panel","ready","focus","run"}
  };
  frnl_handler_binding handlers[2] = {
    {"panel","run",handler,0}, {"other","run",handler,0}
  };
  frnl_event_dispatcher d = {events,3,3,handlers,2,2,0,0};
  CHECK(frnl_event_unregister(&d,"panel","ready","hover") == FRNL_EVENT_OK);
  CHECK(d.event_count == 2 && events[2].component == 0);
  CHECK(frnl_event_invoke(&d,"panel","ready","focus",0) == FRNL_EVENT_OK);
  CHECK(calls == 1);
  CHECK(frnl_event_unregister(&d,"panel","ready","hover") == FRNL_EVENT_UNKNOWN_EVENT);
  CHECK(frnl_handler_unregister(&d,"panel","run") == FRNL_EVENT_OK);
  CHECK(d.handler_count == 1 && handlers[1].component == 0);
  CHECK(frnl_event_invoke(&d,"panel","ready","focus",0) == FRNL_EVENT_MISSING_HANDLER);
  CHECK(frnl_handler_register(&d,(frnl_handler_binding){"panel","run",handler,0}) == FRNL_EVENT_OK);
  CHECK(frnl_event_invoke(&d,"panel","ready","click",0) == FRNL_EVENT_OK);
  CHECK(calls == 2);
  CHECK(frnl_event_unregister(&d,"panel","ready","click") == FRNL_EVENT_OK);
  CHECK(frnl_event_unregister(&d,"panel","ready","focus") == FRNL_EVENT_OK);
  CHECK(d.event_count == 0);
  CHECK(frnl_handler_unregister(&d,"other","run") == FRNL_EVENT_OK);
  CHECK(frnl_handler_unregister(&d,"panel","run") == FRNL_EVENT_OK);
  CHECK(d.handler_count == 0);
  /* Invalid trailing entries must not cause partial mutation. */
  {
    frnl_event_binding bad_events[2] = {
      {"panel","ready","click","run"}, {"panel","ready","focus",0}
    };
    frnl_handler_binding bad_handlers[2] = {
      {"panel","run",handler,0}, {"panel","bad",0,0}
    };
    frnl_event_dispatcher bad = {bad_events,2,2,bad_handlers,2,2,0,0};
    CHECK(frnl_event_unregister(&bad,"panel","ready","click") == FRNL_EVENT_INVALID_ARGUMENT);
    CHECK(bad.event_count == 2);
    CHECK(frnl_handler_unregister(&bad,"panel","run") == FRNL_EVENT_INVALID_ARGUMENT);
    CHECK(bad.handler_count == 2);
  }
  return 0;
}
