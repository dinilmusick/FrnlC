#include "frnl_event_dispatch.h"
#include <assert.h>

static int inner_calls, outer_calls, original_notices, replacement_notices, fail_inner = 1;
static frnl_event_dispatcher *runtime;
static void replacement_listener(const char *c,const char *e,const void *p,void *ctx) {
  (void)c;(void)e;(void)p;(void)ctx; ++replacement_notices;
}
static void original_listener(const char *c,const char *e,const void *p,void *ctx) {
  (void)c;(void)e;(void)p;(void)ctx; ++original_notices;
}
static int inner_handler(const void *payload,void *ctx) {
  (void)payload;(void)ctx; ++inner_calls;
  runtime->listener = replacement_listener;
  return fail_inner;
}
static int outer_handler(const void *payload,void *ctx) {
  (void)ctx; ++outer_calls;
  return frnl_event_invoke(runtime,"inner","ready","fire",payload) != FRNL_EVENT_OK;
}
int main(void) {
  int payload = 7;
  frnl_event_binding events[2] = {
    {"outer","ready","fire","run"}, {"inner","ready","fire","run"}
  };
  frnl_handler_binding handlers[2] = {
    {"outer","run",outer_handler,0}, {"inner","run",inner_handler,0}
  };
  frnl_event_dispatcher d = {events,2,2,handlers,2,2,original_listener,0};
  runtime = &d;
  assert(frnl_event_invoke(&d,"outer","ready","fire",&payload) == FRNL_EVENT_HANDLER_FAILED);
  assert(inner_calls == 1 && outer_calls == 1 && original_notices == 0 && replacement_notices == 0);
  fail_inner = 0;
  d.listener = original_listener;
  assert(frnl_event_invoke(&d,"outer","ready","fire",&payload) == FRNL_EVENT_OK);
  assert(inner_calls == 2 && outer_calls == 2);
  /* Nested dispatch retains the outer listener snapshot across mutation. */
  assert(original_notices == 2 && replacement_notices == 0);
  assert(frnl_event_invoke(&d,"inner","ready","fire",&payload) == FRNL_EVENT_OK);
  assert(original_notices == 2 && replacement_notices == 1);
  return 0;
}
