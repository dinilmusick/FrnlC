#include "frnl_event_dispatch.h"
#include <assert.h>
static int calls, notifications, fail;
static int handler(const void *p,void *ctx) { (void)ctx; calls++; return fail || !p; }
static void listener(const char *c,const char *e,const void *p,void *ctx) { (void)c;(void)e;(void)p;(void)ctx;notifications++; }
int main(void) {
  frnl_event_binding events[2];
  frnl_handler_binding handlers[2];
  frnl_event_dispatcher d={events,0,2,handlers,0,2,listener,0};
  int payload=1;
  frnl_event_binding b={"button","idle","click","onClick"};
  frnl_handler_binding h={"button","onClick",handler,0};
  assert(frnl_event_register(&d,b)==FRNL_EVENT_OK);
  assert(frnl_event_register(&d,b)==FRNL_EVENT_DUPLICATE);
  assert(frnl_event_invoke(&d,"missing","idle","click",&payload)==FRNL_EVENT_UNKNOWN_COMPONENT);
  assert(frnl_event_invoke(&d,"button","active","click",&payload)==FRNL_EVENT_UNKNOWN_STATE);
  assert(frnl_event_invoke(&d,"button","idle","hover",&payload)==FRNL_EVENT_UNKNOWN_EVENT);
  assert(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_MISSING_HANDLER);
  assert(frnl_handler_register(&d,h)==FRNL_EVENT_OK);
  assert(frnl_handler_register(&d,h)==FRNL_EVENT_DUPLICATE);
  assert(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_OK);
  assert(calls==1 && notifications==1);
  fail=1;
  assert(frnl_event_invoke(&d,"button","idle","click",&payload)==FRNL_EVENT_HANDLER_FAILED);
  assert(calls==2 && notifications==1);
  assert(frnl_event_invoke(&d,0,"idle","click",&payload)==FRNL_EVENT_INVALID_ARGUMENT);
  return 0;
}
